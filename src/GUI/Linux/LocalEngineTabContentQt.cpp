/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "GUI/LocalEngineTabContentQt.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/ReportRepository.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/LocalEngineFactionUtils.hpp"
#include "Function/LocalEngineRunUtils.hpp"
#include "Function/LocalEngineSettingsUtils.hpp"

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

#include <filesystem>

namespace
{
std::filesystem::path filePath(const QString& path)
{
    return std::filesystem::path(QFile::encodeName(path).constData());
}

QString qtPath(const std::filesystem::path& path)
{
    return QFile::decodeName(path.string().c_str());
}
}

LocalEngineTabContentQt::LocalEngineTabContentQt(AppData& appData, AppConfig& appConfig, QWidget* parent)
    : QWidget(parent), appData_(appData), appConfig_(appConfig),
      localCheck_(new QCheckBox("Local", this)), controls_(new QWidget(this)),
      enginePath_(new QLineEdit(controls_)), factionName_(new QLineEdit(controls_)),
      playerNumber_(new QLineEdit(controls_)), ordersPath_(new QLineEdit(controls_)),
      mainFaction_(new QLineEdit(controls_)), turnLabel_(new QLabel(controls_)),
      terminal_(new QPlainTextEdit(controls_)), terminalInput_(new QLineEdit(controls_)),
      process_(new QProcess(this))
{
    defaultFolder_ = QDir(QString::fromStdWString(AppConfig::getExecutableDirectory())).filePath("LocalEngine");
    enginePath_->setText(QString::fromStdWString(appConfig_.getLocalEnginePath()));
    playerNumber_->setValidator(new QIntValidator(1, 999, playerNumber_));
    mainFaction_->setValidator(new QIntValidator(1, 999, mainFaction_));
    playerNumber_->setMaximumWidth(70);
    mainFaction_->setMaximumWidth(70);
    terminal_->setReadOnly(true);
    terminalInput_->setPlaceholderText("Input for running engine");
    terminalInput_->setEnabled(false);

    auto* browseEngineButton = new QPushButton("Browse", controls_);
    auto* newButton = new QPushButton("New", controls_);
    auto* addButton = new QPushButton("Add Faction", controls_);
    auto* browseOrdersButton = new QPushButton("Browse", controls_);
    auto* runButton = new QPushButton("Run", controls_);

    auto* engineRow = new QHBoxLayout;
    engineRow->addWidget(new QLabel("Engine", controls_));
    engineRow->addWidget(enginePath_, 1);
    engineRow->addWidget(browseEngineButton);
    auto* factionRow = new QHBoxLayout;
    factionRow->addWidget(new QLabel("Faction Name:", controls_));
    factionRow->addWidget(factionName_);
    factionRow->addWidget(addButton);
    auto* ordersRow = new QHBoxLayout;
    ordersRow->addWidget(new QLabel("Player Number:", controls_));
    ordersRow->addWidget(playerNumber_);
    ordersRow->addWidget(new QLabel("Orders", controls_));
    ordersRow->addWidget(ordersPath_, 1);
    ordersRow->addWidget(browseOrdersButton);
    auto* runRow = new QHBoxLayout;
    runRow->addWidget(new QLabel("Main Faction", controls_));
    runRow->addWidget(mainFaction_);
    runRow->addWidget(runButton);
    runRow->addWidget(turnLabel_);
    runRow->addStretch();
    auto* left = new QVBoxLayout;
    left->addLayout(engineRow);
    left->addWidget(newButton, 0, Qt::AlignLeft);
    left->addLayout(factionRow);
    left->addLayout(ordersRow);
    left->addLayout(runRow);
    left->addStretch();
    auto* right = new QVBoxLayout;
    right->addWidget(terminal_);
    right->addWidget(terminalInput_);
    auto* content = new QHBoxLayout(controls_);
    content->setContentsMargins(0, 0, 0, 0);
    content->addLayout(left, 1);
    content->addLayout(right, 1);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(localCheck_);
    layout->addWidget(controls_, 1);
    controls_->setEnabled(false);
    updateTurn(0);

    connect(localCheck_, &QCheckBox::toggled, this, &LocalEngineTabContentQt::enableLocal);
    connect(browseEngineButton, &QPushButton::clicked, this, &LocalEngineTabContentQt::browseEngine);
    connect(browseOrdersButton, &QPushButton::clicked, this, &LocalEngineTabContentQt::browseOrders);
    connect(newButton, &QPushButton::clicked, this, &LocalEngineTabContentQt::newGame);
    connect(addButton, &QPushButton::clicked, this, &LocalEngineTabContentQt::addFaction);
    connect(runButton, &QPushButton::clicked, this, &LocalEngineTabContentQt::runGame);
    for (QLineEdit* edit : { factionName_, playerNumber_, ordersPath_, mainFaction_ })
        connect(edit, &QLineEdit::editingFinished, this, &LocalEngineTabContentQt::saveSettings);
    connect(enginePath_, &QLineEdit::editingFinished, this, [this]() {
        appConfig_.setLocalEnginePath(enginePath_->text().toStdWString());
        if (!appConfig_.save())
            QMessageBox::warning(this, "Local Engine", "Unable to save the engine path.");
        loadSettings();
    });
    process_->setProcessChannelMode(QProcess::MergedChannels);
    connect(process_, &QProcess::readyReadStandardOutput, this, [this]() {
        terminal_->moveCursor(QTextCursor::End);
        terminal_->insertPlainText(QString::fromLocal8Bit(process_->readAllStandardOutput()));
        terminal_->moveCursor(QTextCursor::End);
    });
    connect(process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
        {
            runPending_ = false;
            setProcessRunning(false);
            QMessageBox::critical(this, "Local Engine", "Unable to start engine: " + process_->errorString());
        }
    });
    connect(process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
        setProcessRunning(false);
        terminal_->appendPlainText("\n[process exited]");
        const bool finalize = runPending_;
        runPending_ = false;
        if (status != QProcess::NormalExit || exitCode != 0)
            QMessageBox::critical(this, "Local Engine", QString("Engine failed (exit code %1).").arg(exitCode));
        else if (finalize)
            finishRun();
    });
    connect(terminalInput_, &QLineEdit::returnPressed, this, [this]() {
        if (process_->state() == QProcess::Running)
        {
            const QString input = terminalInput_->text();
            if (process_->write((input + '\n').toLocal8Bit()) < 0)
                QMessageBox::critical(this, "Local Engine", "Unable to send input to the engine.");
            else
            {
                terminal_->appendPlainText(input);
                terminalInput_->clear();
            }
        }
    });
}

LocalEngineTabContentQt::~LocalEngineTabContentQt()
{
    if (process_->state() != QProcess::NotRunning)
    {
        process_->disconnect(this);
        process_->kill();
        process_->waitForFinished();
    }
}

bool LocalEngineTabContentQt::isLocalEnabled() const { return localCheck_->isChecked(); }

void LocalEngineTabContentQt::enableLocal(bool enabled)
{
    if (enabled)
    {
        if (QMessageBox::question(this, "Local Engine", "Prepare local?",
                                  QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
        {
            localCheck_->setChecked(false);
            return;
        }
        if (QFileInfo(defaultFolder_).isSymLink() || !QDir().mkpath(defaultFolder_))
        {
            QMessageBox::critical(this, "Local Engine", "Unable to create the local engine folder:\n" + defaultFolder_);
            localCheck_->setChecked(false);
            return;
        }
        loadSettings();
    }
    controls_->setEnabled(enabled);
}

void LocalEngineTabContentQt::browseEngine()
{
    const QString selected = QFileDialog::getOpenFileName(this, "Select Engine",
        enginePath_->text().isEmpty() ? defaultFolder_ : enginePath_->text(), "All Files (*)");
    if (!selected.isEmpty())
    {
        enginePath_->setText(selected);
        appConfig_.setLocalEnginePath(selected.toStdWString());
        if (!appConfig_.save())
            QMessageBox::warning(this, "Local Engine", "Unable to save the engine path.");
        loadSettings();
    }
}

void LocalEngineTabContentQt::browseOrders()
{
    const QString selected = QFileDialog::getOpenFileName(this, "Select Orders",
        ordersPath_->text().isEmpty() ? defaultFolder_ : ordersPath_->text(),
        "Order Files (*.ord);;All Files (*)");
    if (selected.isEmpty()) return;
    if (!playerNumber_->hasAcceptableInput())
    {
        QMessageBox::warning(this, "Local Engine", "Enter a player number before selecting orders.");
        return;
    }
    QString destination;
    if (copyOrders(selected, playerNumber_->text().toInt(), destination))
        setOrdersPath(destination);
}

void LocalEngineTabContentQt::newGame()
{
    if (process_->state() != QProcess::NotRunning)
    {
        QMessageBox::warning(this, "Local Engine", "Wait for the engine to finish first.");
        return;
    }
    if (enginePath_->text().isEmpty() || !QFileInfo(enginePath_->text()).isExecutable())
    {
        QMessageBox::warning(this, "Local Engine", "Select an executable engine before starting a new game.");
        return;
    }
    const QString selectedEngine = QFileInfo(enginePath_->text()).canonicalFilePath();
    const QString localFolder = QFileInfo(defaultFolder_).canonicalFilePath();
    if (QFileInfo(defaultFolder_).isSymLink() || selectedEngine.isEmpty() || localFolder.isEmpty()
        || QFileInfo(selectedEngine).absolutePath() != localFolder)
    {
        QMessageBox::warning(this, "Local Engine",
            "To start a new game, select an engine directly inside:\n" + defaultFolder_);
        return;
    }
    if (QMessageBox::question(this, "Local Engine",
        "Clear the local engine folder and start a new game?\n\n" + defaultFolder_,
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) return;

    QDir directory(defaultFolder_);
    const QFileInfoList entries = directory.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    for (const QFileInfo& entry : entries)
    {
        const QString canonical = entry.canonicalFilePath();
        if (canonical == selectedEngine || selectedEngine.startsWith(canonical + QDir::separator()))
            continue;
        const bool removed = entry.isDir() && !entry.isSymLink()
            ? QDir(entry.absoluteFilePath()).removeRecursively()
            : QFile::remove(entry.absoluteFilePath());
        if (!removed)
        {
            QMessageBox::critical(this, "Local Engine", "Unable to remove:\n" + entry.absoluteFilePath());
            return;
        }
    }
    loadingSettings_ = true;
    factionName_->clear();
    playerNumber_->clear();
    ordersPath_->clear();
    mainFaction_->clear();
    updateTurn(0);
    loadingSettings_ = false;
    saveSettings();
    startProcess("new");
}

void LocalEngineTabContentQt::addFaction()
{
    if (enginePath_->text().isEmpty())
    {
        QMessageBox::warning(this, "Local Engine", "Select an engine before adding a faction.");
        return;
    }
    using Result = LocalEngineFactionUtils::AddFactionResult;
    const auto result = LocalEngineFactionUtils::addFaction(
        filePath(QFileInfo(enginePath_->text()).absoluteFilePath()), factionName_->text().toStdWString());
    switch (result)
    {
    case Result::Added: factionName_->clear(); saveSettings(); return;
    case Result::EmptyFactionName: QMessageBox::warning(this, "Local Engine", "Enter a faction name."); return;
    case Result::DuplicateName: QMessageBox::warning(this, "Local Engine", "A faction with this name already exists."); return;
    case Result::PlayersFileMissing: QMessageBox::warning(this, "Local Engine", "players.out was not found beside the engine."); return;
    case Result::ReadError: QMessageBox::critical(this, "Local Engine", "Unable to read players.out."); return;
    case Result::WriteError: QMessageBox::critical(this, "Local Engine", "Unable to update players.out."); return;
    }
}

void LocalEngineTabContentQt::runGame()
{
    if (process_->state() != QProcess::NotRunning)
    {
        QMessageBox::warning(this, "Local Engine", "Wait for the engine to finish first.");
        return;
    }
    if (enginePath_->text().isEmpty() || !QFileInfo(enginePath_->text()).isExecutable())
    {
        QMessageBox::warning(this, "Local Engine", "Select an executable engine before running it.");
        return;
    }
    if (LocalEngineRunUtils::prepareRun(filePath(QFileInfo(enginePath_->text()).absoluteFilePath()))
        != LocalEngineRunUtils::RunStatus::Success)
    {
        QMessageBox::critical(this, "Local Engine", "Unable to prepare the local engine files.");
        return;
    }
    runPending_ = true;
    startProcess("run");
}

void LocalEngineTabContentQt::startProcess(const QString& action)
{
    const QFileInfo engine(enginePath_->text());
    terminal_->clear();
    setProcessRunning(true);
    process_->setWorkingDirectory(engine.absolutePath());
    process_->start(engine.absoluteFilePath(), { action });
}

void LocalEngineTabContentQt::setProcessRunning(bool running)
{
    localCheck_->setEnabled(!running);
    for (QLineEdit* edit : { enginePath_, factionName_, playerNumber_, ordersPath_, mainFaction_ })
        edit->setEnabled(!running);
    for (QPushButton* button : controls_->findChildren<QPushButton*>())
        button->setEnabled(!running);
    terminalInput_->setEnabled(running);
}

void LocalEngineTabContentQt::finishRun()
{
    const auto result = LocalEngineRunUtils::finalizeRun(
        filePath(QFileInfo(enginePath_->text()).absoluteFilePath()), mainFaction_->text().toStdWString(),
        filePath(QString::fromStdWString(appConfig_.getReportImportFolder())),
        filePath(QDir(QString::fromStdWString(AppConfig::getExecutableDirectory())).filePath("Reports")));
    if (result.status != LocalEngineRunUtils::RunStatus::Success)
    {
        QMessageBox::critical(this, "Local Engine", "Unable to create the local engine report.");
        return;
    }
    if (mainFaction_->text().isEmpty()) mainFaction_->setText(QString::number(result.mainFaction));
    updateTurn(result.currentTurn);
    saveSettings();
    const QString reportPath = qtPath(result.reportPath);
    appConfig_.setReportImportFolder(QFileInfo(reportPath).absolutePath().toStdWString());
    if (!appConfig_.save())
        QMessageBox::warning(this, "Local Engine", "Unable to save the report folder.");
    emit beforeReportImport();
    if (!AppDataUtils::importReportFromFile(appData_, &appConfig_, reportPath.toStdWString(), true, true, false))
    {
        QMessageBox::critical(this, "Local Engine", "Failed to load report:\n\n" +
                              QString::fromStdWString(appData_.reportRepository().getLastError()));
        return;
    }
    emit reportImported();
}

void LocalEngineTabContentQt::loadSettings()
{
    if (enginePath_->text().isEmpty()) return;
    const auto path = filePath(QFileInfo(enginePath_->text()).absoluteFilePath());
    LocalEngineSettingsUtils::Settings settings;
    const bool exists = QFileInfo::exists(qtPath(LocalEngineSettingsUtils::settingsPath(path)));
    if (exists && !LocalEngineSettingsUtils::load(path, settings))
    {
        QMessageBox::warning(this, "Local Engine", "Unable to load local engine settings.");
        return;
    }
    loadingSettings_ = true;
    factionName_->setText(QString::fromStdWString(settings.factionName));
    mainFaction_->setText(QString::fromStdWString(settings.mainFaction));
    playerNumber_->setText(QString::fromStdWString(settings.playerNumber));
    ordersPath_->setText(QString::fromStdWString(settings.ordersPath));
    updateTurn(settings.currentTurn);
    loadingSettings_ = false;
}

void LocalEngineTabContentQt::saveSettings()
{
    if (loadingSettings_ || !isLocalEnabled() || enginePath_->text().isEmpty()) return;
    LocalEngineSettingsUtils::Settings settings;
    settings.factionName = factionName_->text().toStdWString();
    settings.mainFaction = mainFaction_->text().toStdWString();
    settings.playerNumber = playerNumber_->text().toStdWString();
    settings.ordersPath = ordersPath_->text().toStdWString();
    settings.currentTurn = currentTurn_;
    if (!LocalEngineSettingsUtils::save(filePath(QFileInfo(enginePath_->text()).absoluteFilePath()), settings))
        QMessageBox::warning(this, "Local Engine", "Unable to save local engine settings.");
}

void LocalEngineTabContentQt::updateTurn(int turn)
{
    currentTurn_ = turn;
    turnLabel_->setText(QString("Current turn: %1").arg(turn));
}

void LocalEngineTabContentQt::setMainFactionNumber(int factionNumber)
{
    if (process_->state() != QProcess::NotRunning) return;
    mainFaction_->setText(QString::number(factionNumber));
    saveSettings();
}

void LocalEngineTabContentQt::setOrdersPath(const QString& path)
{
    ordersPath_->setText(path);
    saveSettings();
}

bool LocalEngineTabContentQt::copyOrders(const QString& sourcePath, int factionNumber, QString& copiedPath)
{
    if (process_->state() != QProcess::NotRunning)
    {
        QMessageBox::warning(this, "Local Engine", "Wait for the engine to finish before copying orders.");
        return false;
    }
    if (enginePath_->text().isEmpty())
    {
        QMessageBox::critical(this, "Local Engine", "Select a local engine before exporting orders.");
        return false;
    }
    const QString destination = QFileInfo(enginePath_->text()).dir().filePath("orders." + QString::number(factionNumber));
    std::error_code error;
    if (QFileInfo(sourcePath).absoluteFilePath() != QFileInfo(destination).absoluteFilePath())
        std::filesystem::copy_file(filePath(sourcePath), filePath(destination),
                                   std::filesystem::copy_options::overwrite_existing, error);
    if (error)
    {
        QMessageBox::critical(this, "Local Engine", "Failed to copy orders: " + QString::fromStdString(error.message()));
        return false;
    }
    copiedPath = destination;
    return true;
}
