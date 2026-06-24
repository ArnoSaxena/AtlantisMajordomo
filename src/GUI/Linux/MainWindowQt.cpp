/* 
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * File: MainWindowQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/MainWindowQt.hpp"
#include "GUI/ReportsTabContentQt.hpp"
#include "GUI/EventsTabContentQt.hpp"
#include "GUI/BattlesTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/DataSerializer.hpp"
#include "Data/Faction.hpp"
#include "Data/Unit.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <array>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Local helpers  (mirrors the anonymous-namespace helpers in MainWindowWin.cpp)
// ---------------------------------------------------------------------------
namespace
{

struct MainFactionExportContext
{
    int factionNumber { 0 };
    std::wstring password;
    int nextMonth { 0 };
    int nextYear  { 0 };
};

bool isLaterPeriod(int year, int month, int refYear, int refMonth)
{
    return (year > refYear) || (year == refYear && month > refMonth);
}

bool tryBuildMainFactionExportContext(const AppData& appData,
                                      MainFactionExportContext& ctx,
                                      std::wstring& errorMessage)
{
    const auto& factionRepo = appData.factionRepository();
    const Faction* mainFaction = nullptr;
    int mainFactionCount = 0;

    for (std::size_t i = 0; i < factionRepo.size(); ++i)
    {
        const Faction& faction = factionRepo.at(i);
        if (faction.isMainFaction())
        {
            ++mainFactionCount;
            mainFaction = &faction;
        }
    }

    if (mainFactionCount == 0 || mainFaction == nullptr)
    {
        errorMessage = L"Cannot export orders: no main faction is selected.";
        return false;
    }

    if (mainFactionCount > 1)
    {
        errorMessage = L"Cannot export orders: more than one main faction is selected.";
        return false;
    }

    int latestYear = 0, latestMonth = 0;
    bool hasPeriod = false;

    for (std::size_t i = 0; i < factionRepo.size(); ++i)
    {
        const Faction& faction = factionRepo.at(i);
        const int month = faction.getMonth();
        const int year  = faction.getYear();
        if (month < 1 || month > 12) continue;
        if (!hasPeriod || isLaterPeriod(year, month, latestYear, latestMonth))
        {
            latestYear  = year;
            latestMonth = month;
            hasPeriod   = true;
        }
    }

    if (!hasPeriod)
    {
        errorMessage = L"Cannot export orders: no valid month/year was found in loaded faction data.";
        return false;
    }

    ++latestMonth;
    if (latestMonth > 12) { latestMonth = 1; ++latestYear; }

    ctx.factionNumber = mainFaction->getFactionNumber();
    ctx.password      = mainFaction->getPassword();
    ctx.nextMonth     = latestMonth;
    ctx.nextYear      = latestYear;
    errorMessage.clear();
    return true;
}

std::wstring formatTwoDigits(int value)
{
    return (value < 10 ? L"0" : L"") + std::to_wstring(value);
}

std::wstring buildSuggestedOrdersFilename(const MainFactionExportContext& ctx)
{
    return L"orders_"
        + std::to_wstring(ctx.factionNumber)
        + L"_"
        + std::to_wstring(ctx.nextYear)
        + L"_"
        + formatTwoDigits(ctx.nextMonth)
        + L".ord";
}

std::wstring buildOrdersExportContent(const AppData& appData,
                                       int mainFactionNumber,
                                       const std::wstring& password)
{
    std::vector<const Unit*> units;
    const auto& unitRepo = appData.unitRepository();
    for (std::size_t i = 0; i < unitRepo.size(); ++i)
    {
        const Unit& unit = unitRepo.at(i);
        if (unit.getFactionNumber() == mainFactionNumber)
            units.push_back(&unit);
    }

    std::sort(units.begin(), units.end(),
              [](const Unit* a, const Unit* b) {
                  return a->getUnitNumber() < b->getUnitNumber();
              });

    std::wostringstream out;
    out << L"#atlantis " << mainFactionNumber << L" \"" << password << L"\"\n";
    for (const Unit* unit : units)
    {
        out << L"\nunit " << unit->getUnitNumber() << L"\n";
        for (const std::wstring& line : unit->getOrders())
            out << line << L"\n";
    }
    out << L"\n#end\n";
    return out.str();
}

bool saveTextFile(const std::wstring& path, const std::wstring& content)
{
    std::wofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

// Returns the parent directory of a path as a QString, or empty if unavailable.
QString dirFromPath(const std::wstring& path)
{
    if (path.empty()) return {};
    std::filesystem::path p(path);
    if (p.has_parent_path())
        return QString::fromStdWString(p.parent_path().wstring());
    return {};
}

} // namespace

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

MainWindow::MainWindow(AppData& appData, QWidget* parent)
    : QMainWindow(parent)
    , appData_(appData)
{
    appConfig_.load();
    applyConfigToAppData();

    setWindowTitle(kAboutAppName);
    resize(appConfig_.getMainWindowWidth(), appConfig_.getMainWindowHeight());

    setupMenus();
    setupTabs();

    // Defer heavy initialisation so the window is visible before any I/O.
    QTimer::singleShot(0, this, &MainWindow::deferredInit);
}

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------
// Config / AppData wiring
// ---------------------------------------------------------------------------

void MainWindow::applyConfigToAppData()
{
    appData_.setFlyingShipsCsv(appConfig_.getFlyingShipsCsv());
    appData_.setMagicSkillTriggersCsv(appConfig_.getMagicSkillTriggersCsv());
    appData_.setOnlyLeaderCanTeach(appConfig_.getOnlyLeaderCanTeach());
    appData_.setLeaderMages(appConfig_.getLeaderMages());
    Commands::setFullMonthOrderKeywordsCsv(appConfig_.getFullMonthOrdersCsv());
}

// ---------------------------------------------------------------------------
// Menu setup
// ---------------------------------------------------------------------------

void MainWindow::setupMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New",              this, &MainWindow::onFileNew);
    fileMenu->addAction("&Open...",          this, &MainWindow::onFileOpen);
    fileMenu->addAction("&Save",             this, &MainWindow::onFileSave);
    fileMenu->addSeparator();
    fileMenu->addAction("&Load Report",      this, &MainWindow::onFileLoadReport);
    fileMenu->addAction("&Import Data...",   this, &MainWindow::onFileImportData);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xport Orders...", this, &MainWindow::onFileExportOrders);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit",             this, &QWidget::close);

    QMenu* settingsMenu = menuBar()->addMenu("&Settings");
    settingsMenu->addAction("&Options",      this, &MainWindow::onSettingsOptions);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&Description",      this, &MainWindow::onHelpDescription);
    helpMenu->addAction("&About",            this, &MainWindow::onHelpAbout);
}

// ---------------------------------------------------------------------------
// Tab setup
// ---------------------------------------------------------------------------

void MainWindow::setupTabs()
{
    tabWidget_ = new QTabWidget(this);
    setCentralWidget(tabWidget_);

    auto addTab = [this](const QString& label) -> QWidget* {
        QWidget* w = new QWidget(tabWidget_);
        tabWidget_->addTab(w, label);
        return w;
    };

    reportsTab_  = addTab("Loaded Reports");
    mapTab_      = addTab("Map");
    eventsTab_   = addTab("Events");
    itemsTab_    = addTab("Items");
    skillsTab_   = addTab("Skills");
    factionsTab_ = addTab("Factions");
    battlesTab_  = addTab("Battles");
}

// ---------------------------------------------------------------------------
// Deferred initialisation  (equivalent to WM_APP_INIT + WM_APP_AUTOLOAD)
// ---------------------------------------------------------------------------

void MainWindow::deferredInit()
{
    // Reports tab
    reportsTabContent_ = new ReportsTabContentQt(appData_, appConfig_, reportsTab_);
    QVBoxLayout* reportsLayout = new QVBoxLayout(reportsTab_);
    reportsLayout->setContentsMargins(0, 0, 0, 0);
    reportsLayout->addWidget(reportsTabContent_);

    // Events tab
    eventsTabContent_ = new EventsTabContentQt(appData_, eventsTab_);
    QVBoxLayout* eventsLayout = new QVBoxLayout(eventsTab_);
    eventsLayout->setContentsMargins(0, 0, 0, 0);
    eventsLayout->addWidget(eventsTabContent_);

    // Battles tab
    battlesTabContent_ = new BattlesTabContentQt(appData_, battlesTab_);
    QVBoxLayout* battlesLayout = new QVBoxLayout(battlesTab_);
    battlesLayout->setContentsMargins(0, 0, 0, 0);
    battlesLayout->addWidget(battlesTabContent_);

    // TODO: Create remaining Qt tab content widgets here as they are implemented.

    autoLoad();
}

void MainWindow::autoLoad()
{
    // Auto-load the configured data file (equivalent to WM_APP_AUTOLOAD, data file part)
    const std::wstring dataFilePath = appConfig_.getDataFilePath();
    if (!dataFilePath.empty())
    {
        std::error_code ec;
        const std::filesystem::path dataPath(dataFilePath);
        if (std::filesystem::exists(dataPath, ec) && std::filesystem::is_regular_file(dataPath, ec))
        {
            auto& reportRepo = appData_.reportRepository();
            if (!reportRepo.addFromFile(dataFilePath,
                                        appData_.factionRepository(),
                                        appData_.regionRepository(),
                                        appData_.unitRepository(),
                                        appData_.battleRepository(),
                                        appData_.eventRepository(),
                                        appData_.itemRepository(),
                                        appData_.skillRepository(),
                                        appData_.structureRepository(),
                                        appData_.structInfoRepository(),
                                        appData_.orderRepository(),
                                        appData_.getShipStructureIdThreshold(),
                                        appData_.getFlyingShipTypeTokens(),
                                        appData_.getMagicSkillTriggerPhrases(),
                                        false))
            {
                QMessageBox::warning(this,
                    "Startup Load Error",
                    QString("Failed to auto-load the configured data file:\n\n%1")
                        .arg(QString::fromStdWString(reportRepo.getLastError())));
            }
        }
    }

    // Auto-load from the configured report import folder
    const std::wstring reportFolder = appConfig_.getReportImportFolder();
    if (!reportFolder.empty())
    {
        std::error_code ec;
        const std::filesystem::path folderPath(reportFolder);
        if (std::filesystem::exists(folderPath, ec) && std::filesystem::is_directory(folderPath, ec))
        {
            auto& reportRepo = appData_.reportRepository();
            std::vector<std::wstring> failedReports;

            const std::array<std::wstring, 4> allowedExtensions = { L".rep", L".txt", L".html", L".htm" };
            for (const auto& entry : std::filesystem::directory_iterator(folderPath, ec))
            {
                if (!entry.is_regular_file()) continue;

                std::wstring ext = entry.path().extension().wstring();
                std::transform(ext.begin(), ext.end(), ext.begin(), std::towlower);
                if (std::find(allowedExtensions.begin(), allowedExtensions.end(), ext)
                        == allowedExtensions.end())
                    continue;

                if (!reportRepo.addFromFile(entry.path().wstring(),
                                            appData_.factionRepository(),
                                            appData_.regionRepository(),
                                            appData_.unitRepository(),
                                            appData_.battleRepository(),
                                            appData_.eventRepository(),
                                            appData_.itemRepository(),
                                            appData_.skillRepository(),
                                            appData_.structureRepository(),
                                            appData_.structInfoRepository(),
                                            appData_.orderRepository(),
                                            appData_.getShipStructureIdThreshold(),
                                            appData_.getFlyingShipTypeTokens(),
                                            appData_.getMagicSkillTriggerPhrases(),
                                            true))
                {
                    failedReports.push_back(
                        entry.path().filename().wstring() + L": " + reportRepo.getLastError());
                }
            }

            if (!failedReports.empty())
            {
                std::wstring msg = L"Some reports failed to auto-load from the configured report folder:\n\n";
                for (std::size_t i = 0; i < failedReports.size() && i < 10; ++i)
                    msg += failedReports[i] + L"\n";
                if (failedReports.size() > 10)
                    msg += L"...and more\n";
                QMessageBox::warning(this, "Startup Load Error", QString::fromStdWString(msg));
            }
        }
    }

    refreshAllTabs();
}

void MainWindow::refreshAllTabs()
{
    if (reportsTabContent_)
        reportsTabContent_->refresh();

    if (eventsTabContent_)
        eventsTabContent_->refresh();

    if (battlesTabContent_)
        battlesTabContent_->refresh();

    // TODO: Call refresh() on remaining Qt tab content widgets as they are implemented.
}

// ---------------------------------------------------------------------------
// Qt event overrides
// ---------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* event)
{
    appConfig_.setMainWindowWidth(width());
    appConfig_.setMainWindowHeight(height());
    appConfig_.save();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    appConfig_.setMainWindowWidth(width());
    appConfig_.setMainWindowHeight(height());
    appConfig_.save();
}

// ---------------------------------------------------------------------------
// Menu slots — File
// ---------------------------------------------------------------------------

void MainWindow::onFileNew()
{
    appData_.clear();
    refreshAllTabs();
    QMessageBox::information(this, "New", "New session started.");
}

void MainWindow::onFileOpen()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Dataset",
        QString::fromStdWString(appConfig_.getSaveFilePath()),
        "Dataset Files (*.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    if (!DataSerializer::loadFromFile(appData_, path))
    {
        QMessageBox::critical(this, "Open Failed",
            QString("Failed to open dataset:\n\n%1")
                .arg(QString::fromStdWString(DataSerializer::getLastError())));
        return;
    }

    appConfig_.setSaveFilePath(path);
    appConfig_.save();
    refreshAllTabs();
    QMessageBox::information(this, "Open", "Dataset loaded successfully.");
}

void MainWindow::onFileSave()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Dataset",
        QString::fromStdWString(appConfig_.getSaveFilePath()),
        "Dataset Files (*.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    if (!DataSerializer::saveToFile(appData_, path))
    {
        QMessageBox::critical(this, "Save Failed",
            QString("Failed to save dataset:\n\n%1")
                .arg(QString::fromStdWString(DataSerializer::getLastError())));
        return;
    }

    appConfig_.setSaveFilePath(path);
    appConfig_.save();
    QMessageBox::information(this, "Save", "Dataset saved successfully.");
}

void MainWindow::onFileLoadReport()
{
    const QString initialDir = QString::fromStdWString(appConfig_.getReportImportFolder());
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Load Report",
        initialDir,
        "Report Files (*.rep *.txt *.html *.htm);;All Files (*)");

    if (files.isEmpty()) return;

    auto& reportRepo = appData_.reportRepository();
    std::vector<std::wstring> failedReports;
    bool savedFolder = false;

    for (const QString& file : files)
    {
        const std::wstring path = file.toStdWString();
        if (!reportRepo.addFromFile(path,
                                    appData_.factionRepository(),
                                    appData_.regionRepository(),
                                    appData_.unitRepository(),
                                    appData_.battleRepository(),
                                    appData_.eventRepository(),
                                    appData_.itemRepository(),
                                    appData_.skillRepository(),
                                    appData_.structureRepository(),
                                    appData_.structInfoRepository(),
                                    appData_.orderRepository(),
                                    appData_.getShipStructureIdThreshold(),
                                    appData_.getFlyingShipTypeTokens(),
                                    appData_.getMagicSkillTriggerPhrases(),
                                    true))
        {
            failedReports.push_back(
                std::filesystem::path(path).filename().wstring() + L": " + reportRepo.getLastError());
        }
        else if (!savedFolder)
        {
            appConfig_.setReportImportFolder(
                std::filesystem::path(path).parent_path().wstring());
            appConfig_.save();
            savedFolder = true;
        }
    }

    refreshAllTabs();

    if (!failedReports.empty())
    {
        std::wstring msg = L"Some reports failed to load:\n\n";
        for (const auto& err : failedReports)
            msg += err + L"\n";
        QMessageBox::warning(this, "Load Report", QString::fromStdWString(msg));
    }
}

void MainWindow::onFileImportData()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import Data",
        QString::fromStdWString(appConfig_.getDataFilePath()),
        "Data Files (*.rep *.txt *.html *.htm *.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    auto& reportRepo = appData_.reportRepository();
    if (!reportRepo.addFromFile(path,
                                appData_.factionRepository(),
                                appData_.regionRepository(),
                                appData_.unitRepository(),
                                appData_.battleRepository(),
                                appData_.eventRepository(),
                                appData_.itemRepository(),
                                appData_.skillRepository(),
                                appData_.structureRepository(),
                                appData_.structInfoRepository(),
                                appData_.orderRepository(),
                                appData_.getShipStructureIdThreshold(),
                                appData_.getFlyingShipTypeTokens(),
                                appData_.getMagicSkillTriggerPhrases(),
                                false))
    {
        QMessageBox::critical(this, "Import Data Failed",
            QString("Failed to import data:\n\n%1")
                .arg(QString::fromStdWString(reportRepo.getLastError())));
        return;
    }

    appConfig_.setDataFilePath(path);
    appConfig_.save();
    refreshAllTabs();
}

void MainWindow::onFileExportOrders()
{
    MainFactionExportContext ctx;
    std::wstring errorMessage;
    if (!tryBuildMainFactionExportContext(appData_, ctx, errorMessage))
    {
        QMessageBox::warning(this, "Export Orders", QString::fromStdWString(errorMessage));
        return;
    }

    // Determine initial save path: prefer configured export folder, fall back to save file dir
    QString initialDir = QString::fromStdWString(appConfig_.getExportOrdersFolder());
    if (initialDir.isEmpty())
        initialDir = dirFromPath(appConfig_.getSaveFilePath());

    const QString suggestedName = QString::fromStdWString(buildSuggestedOrdersFilename(ctx));
    const QString initialPath   = initialDir.isEmpty()
        ? suggestedName
        : initialDir + QLatin1Char('/') + suggestedName;

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Orders",
        initialPath,
        "Order Files (*.ord);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring content = buildOrdersExportContent(appData_, ctx.factionNumber, ctx.password);
    if (!saveTextFile(filePath.toStdWString(), content))
    {
        QMessageBox::critical(this, "Export Orders",
            QString("Failed to write orders file:\n\n%1").arg(filePath));
        return;
    }

    appConfig_.setExportOrdersFolder(
        std::filesystem::path(filePath.toStdWString()).parent_path().wstring());
    appConfig_.save();

    QMessageBox::information(this, "Export Orders",
        QString("Orders exported successfully to:\n\n%1").arg(filePath));
}

// ---------------------------------------------------------------------------
// Menu slots — Settings
// ---------------------------------------------------------------------------

void MainWindow::onSettingsOptions()
{
    // TODO: Implement a Qt SettingsDialog equivalent.
    QMessageBox::information(this, "Settings",
        "Settings dialog is not yet implemented for the Qt build.");
}

// ---------------------------------------------------------------------------
// Menu slots — Help
// ---------------------------------------------------------------------------

void MainWindow::onHelpDescription()
{
    const QString text =
        "Map View Legend:\n"
        "- Settlement marker in the center of the region:\n"
        "  - City: big black dot\n"
        "  - Town: black ring with small black dot in center\n"
        "  - Village: black ring\n"
        "- Hex fill color: the region terrain type\n"
        "- Grey line from hex centre to border: road connection in the respective direction\n"
        "- Gray dot at top left: indicates a structure in the region\n"
        "- Orange dot at top left: one of the present structures is a caravanserai\n"
        "- Black outline on the top left dot: one of the present structures is a shaft\n"
        "- Light blue dot at bottom left: one or more ships are present in the region\n"
        "- Black outline on the ship dot: one or more of the present ships are airborne\n"
        "- Red x at the bottom: a battle has taken place in the region in the last turn\n"
        "\n"
        "Unit List Legend:\n"
        "- Green background of unit number and name: the unit is on guard\n"
        "- Green structure name: the unit is owner of the structure\n";

    QMessageBox box(this);
    box.setWindowTitle("Description");
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void MainWindow::onHelpAbout()
{
    const QString text =
        QString("<b>%1</b><br><br>%2<br><br>Version %3")
            .arg(kAboutAppName)
            .arg(kAboutDescription)
            .arg(kAboutVersion);

    QMessageBox::about(this,
        QString("About %1").arg(kAboutAppName),
        text);
}
