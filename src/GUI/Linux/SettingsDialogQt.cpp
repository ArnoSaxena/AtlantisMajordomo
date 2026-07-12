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
 * File: SettingsDialogQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/SettingsDialogQt.hpp"
#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

SettingsDialogQt::SettingsDialogQt(AppData&   appData,
                                   AppConfig& appConfig,
                                   QWidget*   parent)
    : QDialog(parent)
    , appData_  (&appData)
    , appConfig_(&appConfig)
{
    setWindowTitle("Settings");
    setMinimumSize(700, 520);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // -----------------------------------------------------------------------
    // Form section — single-value settings
    // -----------------------------------------------------------------------
    auto* formLayout = new QFormLayout();
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setSpacing(8);

    // Ship ID threshold
    shipThresholdEdit_ = new QLineEdit(this);
    shipThresholdEdit_->setFixedWidth(100);
    formLayout->addRow("Ship ID threshold:", shipThresholdEdit_);

    // Data file path
    dataFilePathEdit_ = new QLineEdit(this);
    {
        auto* browseDataFileButton = new QPushButton("Browse...", this);
        browseDataFileButton->setFixedWidth(90);
        connect(browseDataFileButton, &QPushButton::clicked,
                this, &SettingsDialogQt::onBrowseDataFile);

        auto* dataFileRow = new QHBoxLayout();
        dataFileRow->addWidget(dataFilePathEdit_);
        dataFileRow->addWidget(browseDataFileButton);
        formLayout->addRow("Data file path:", dataFileRow);
    }

    // Report import folder
    reportFolderPathEdit_ = new QLineEdit(this);
    {
        auto* browseReportFolderButton = new QPushButton("Browse...", this);
        browseReportFolderButton->setFixedWidth(90);
        connect(browseReportFolderButton, &QPushButton::clicked,
                this, &SettingsDialogQt::onBrowseReportFolder);

        auto* reportFolderRow = new QHBoxLayout();
        reportFolderRow->addWidget(reportFolderPathEdit_);
        reportFolderRow->addWidget(browseReportFolderButton);
        formLayout->addRow("Report folder:", reportFolderRow);
    }

    // UI size mode — controls overall UI scaling: fonts, buttons, spacing, dialogs
    // Options: Auto, Compact, Standard, Large
    uiSizeModeCombo_ = new QComboBox(this);
    uiSizeModeCombo_->addItem("Auto");
    uiSizeModeCombo_->addItem("Compact");
    uiSizeModeCombo_->addItem("Standard");
    uiSizeModeCombo_->addItem("Large");
    formLayout->addRow("UI size mode:", uiSizeModeCombo_);

    // Map hex size — controls hexagon tile scaling independently from overall UI size
    // Allows combinations like "Compact UI with Large hex tiles"
    // Options: Small (0.95x), Medium (1.0x), Large (1.4x)
    mapHexSizeModeCombo_ = new QComboBox(this);
    mapHexSizeModeCombo_->addItem("Small");
    mapHexSizeModeCombo_->addItem("Medium");
    mapHexSizeModeCombo_->addItem("Large");
    formLayout->addRow("Map hex size:", mapHexSizeModeCombo_);

    // Checkboxes — placed as a horizontal group in a single form row
    onlyLeaderCanTeachCheck_ = new QCheckBox("Only leader can teach", this);
    leaderMagesCheck_         = new QCheckBox("Only leader Mages",     this);
    {
        auto* checkboxRow = new QHBoxLayout();
        checkboxRow->addWidget(onlyLeaderCanTeachCheck_);
        checkboxRow->addSpacing(20);
        checkboxRow->addWidget(leaderMagesCheck_);
        checkboxRow->addStretch();
        formLayout->addRow("", checkboxRow);
    }

    mainLayout->addLayout(formLayout);

    // -----------------------------------------------------------------------
    // List sections — full-month orders and magic triggers side by side
    // -----------------------------------------------------------------------
    auto* listsRow = new QHBoxLayout();
    listsRow->setSpacing(12);

    // Helper lambda to build a list-section group box
    auto makeListGroup = [this](const QString& title,
                                const QString& description,
                                QListWidget*&  listOut,
                                QLineEdit*&    inputOut,
                                auto addSlot,
                                auto removeSlot) -> QGroupBox*
    {
        auto* groupBox = new QGroupBox(title, this);
        auto* groupLayout = new QVBoxLayout(groupBox);
        groupLayout->setSpacing(6);

        auto* descLabel = new QLabel(description, this);
        descLabel->setWordWrap(true);
        groupLayout->addWidget(descLabel);

        listOut = new QListWidget(this);
        listOut->setMinimumHeight(160);
        groupLayout->addWidget(listOut);

        inputOut = new QLineEdit(this);
        inputOut->setPlaceholderText("Enter keyword…");
        groupLayout->addWidget(inputOut);

        auto* buttonRow = new QHBoxLayout();
        auto* addButton    = new QPushButton("Add",    this);
        auto* removeButton = new QPushButton("Remove", this);
        buttonRow->addWidget(addButton);
        buttonRow->addWidget(removeButton);
        buttonRow->addStretch();
        groupLayout->addLayout(buttonRow);

        connect(addButton,    &QPushButton::clicked, this, addSlot);
        connect(removeButton, &QPushButton::clicked, this, removeSlot);

        return groupBox;
    };

    auto* fullMonthGroup = makeListGroup(
        "Full-Month Order Keywords",
        "Keywords that identify full-month orders (e.g. MOVE, WORK).",
        fullMonthOrdersList_,
        fullMonthOrdersInput_,
        &SettingsDialogQt::onAddFullMonthOrder,
        &SettingsDialogQt::onRemoveFullMonthOrder);

    auto* magicTriggersGroup = makeListGroup(
        "Magic Skill Trigger Phrases",
        "Partial skill-name phrases that flag a unit as a mage.",
        magicTriggersList_,
        magicTriggersInput_,
        &SettingsDialogQt::onAddMagicTrigger,
        &SettingsDialogQt::onRemoveMagicTrigger);

    listsRow->addWidget(fullMonthGroup);
    listsRow->addWidget(magicTriggersGroup);
    mainLayout->addLayout(listsRow);

    // -----------------------------------------------------------------------
    // Button box — Apply / OK / Cancel
    // -----------------------------------------------------------------------
    auto* buttonBox = new QDialogButtonBox(this);
    auto* applyButton  = buttonBox->addButton(QDialogButtonBox::Apply);
    auto* okButton     = buttonBox->addButton(QDialogButtonBox::Ok);
    auto* cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(applyButton,  &QPushButton::clicked, this, &SettingsDialogQt::onApply);
    connect(okButton,     &QPushButton::clicked, this, &SettingsDialogQt::onAccepted);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);

    // -----------------------------------------------------------------------
    // Populate widgets from current AppData / AppConfig values
    // -----------------------------------------------------------------------
    shipThresholdEdit_->setText(
        QString::number(appData_->getShipStructureIdThreshold()));

    dataFilePathEdit_->setText(
        QString::fromStdWString(appConfig_->getDataFilePath()));

    reportFolderPathEdit_->setText(
        QString::fromStdWString(appConfig_->getReportImportFolder()));

    if (uiSizeModeCombo_)
    {
        const QString configuredMode = QString::fromStdWString(appConfig_->getUiSizeMode()).trimmed();
        const int modeIndex = uiSizeModeCombo_->findText(configuredMode, Qt::MatchFixedString | Qt::MatchCaseSensitive);
        uiSizeModeCombo_->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
    }

    if (mapHexSizeModeCombo_)
    {
        const QString configuredMode = QString::fromStdWString(appConfig_->getMapHexSizeMode()).trimmed();
        const int modeIndex = mapHexSizeModeCombo_->findText(configuredMode, Qt::MatchFixedString | Qt::MatchCaseSensitive);
        mapHexSizeModeCombo_->setCurrentIndex(modeIndex >= 0 ? modeIndex : 1); // default: Medium
    }

    onlyLeaderCanTeachCheck_->setChecked(appData_->getOnlyLeaderCanTeach());
    leaderMagesCheck_->setChecked(appData_->getLeaderMages());

    loadListFromCsv(fullMonthOrdersList_, Commands::getFullMonthOrderKeywordsCsv());
    loadListFromCsv(magicTriggersList_,   appData_->getMagicSkillTriggersCsv());
}

// ---------------------------------------------------------------------------
// Private — helpers
// ---------------------------------------------------------------------------

void SettingsDialogQt::loadListFromCsv(QListWidget* list, const std::wstring& csv)
{
    list->clear();
    if (csv.empty())
        return;

    // Split on comma; trim each token
    std::wstring remaining = csv;
    while (!remaining.empty())
    {
        const auto pos  = remaining.find(L',');
        std::wstring token = (pos == std::wstring::npos)
                             ? remaining
                             : remaining.substr(0, pos);

        // Trim leading/trailing whitespace
        const auto first = token.find_first_not_of(L" \t");
        const auto last  = token.find_last_not_of(L" \t");
        if (first != std::wstring::npos)
            token = token.substr(first, last - first + 1);

        if (!token.empty())
            list->addItem(QString::fromStdWString(token));

        if (pos == std::wstring::npos)
            break;
        remaining = remaining.substr(pos + 1);
    }
}

std::wstring SettingsDialogQt::buildCsvFromList(const QListWidget* list) const
{
    std::wstring csv;
    const int count = list->count();
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
            csv += L", ";
        csv += list->item(i)->text().toStdWString();
    }
    return csv;
}

bool SettingsDialogQt::addItemToList(QListWidget* list,
                                     QLineEdit*   input,
                                     const QString& listName)
{
    const QString text = input->text().trimmed();
    if (text.isEmpty())
        return false;

    // Duplicate check (case-insensitive)
    const int count = list->count();
    for (int i = 0; i < count; ++i)
    {
        if (QString::compare(list->item(i)->text(), text, Qt::CaseInsensitive) == 0)
        {
            QMessageBox::warning(this, "Duplicate Entry",
                QString("\"%1\" is already in %2.").arg(text, listName));
            return false;
        }
    }

    list->addItem(text);
    input->clear();
    return true;
}

bool SettingsDialogQt::applySettings()
{
    // Validate ship threshold
    bool ok = false;
    const int threshold = shipThresholdEdit_->text().trimmed().toInt(&ok);
    if (!ok)
    {
        QMessageBox::warning(this, "Invalid Value",
            "Ship ID threshold must be an integer.");
        shipThresholdEdit_->setFocus();
        return false;
    }

    // Apply to AppData
    appData_->setShipStructureIdThreshold(threshold);
    appData_->setMagicSkillTriggersCsv(buildCsvFromList(magicTriggersList_));
    appData_->setOnlyLeaderCanTeach(onlyLeaderCanTeachCheck_->isChecked());
    appData_->setLeaderMages(leaderMagesCheck_->isChecked());
    Commands::setFullMonthOrderKeywordsCsv(buildCsvFromList(fullMonthOrdersList_));

    // Apply to AppConfig + save
    appConfig_->setOnlyLeaderCanTeach(onlyLeaderCanTeachCheck_->isChecked());
    appConfig_->setLeaderMages(leaderMagesCheck_->isChecked());
    appConfig_->setFullMonthOrdersCsv(buildCsvFromList(fullMonthOrdersList_));
    appConfig_->setMagicSkillTriggersCsv(buildCsvFromList(magicTriggersList_));
    appConfig_->setDataFilePath(dataFilePathEdit_->text().toStdWString());
    appConfig_->setReportImportFolder(reportFolderPathEdit_->text().toStdWString());
    if (uiSizeModeCombo_)
    {
        appConfig_->setUiSizeMode(uiSizeModeCombo_->currentText().toStdWString());
    }
    if (mapHexSizeModeCombo_)
    {
        appConfig_->setMapHexSizeMode(mapHexSizeModeCombo_->currentText().toStdWString());
    }
    appConfig_->save();

    return true;
}

// ---------------------------------------------------------------------------
// Private slots — Apply / OK / Cancel
// ---------------------------------------------------------------------------

void SettingsDialogQt::onApply()
{
    applySettings();
}

void SettingsDialogQt::onAccepted()
{
    if (applySettings())
        accept();
}

// ---------------------------------------------------------------------------
// Private slots — Browse buttons
// ---------------------------------------------------------------------------

void SettingsDialogQt::onBrowseDataFile()
{
    const QString current = dataFilePathEdit_->text();
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Select Data File",
        current,
        "Data files (*.dat *.txt);;All files (*)");

    if (!path.isEmpty())
        dataFilePathEdit_->setText(path);
}

void SettingsDialogQt::onBrowseReportFolder()
{
    const QString current = reportFolderPathEdit_->text();
    const QString path = QFileDialog::getExistingDirectory(
        this,
        "Select Report Import Folder",
        current,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty())
        reportFolderPathEdit_->setText(path);
}

// ---------------------------------------------------------------------------
// Private slots — Full-month order keywords
// ---------------------------------------------------------------------------

void SettingsDialogQt::onAddFullMonthOrder()
{
    addItemToList(fullMonthOrdersList_, fullMonthOrdersInput_,
                  "Full-Month Order Keywords");
}

void SettingsDialogQt::onRemoveFullMonthOrder()
{
    const auto selected = fullMonthOrdersList_->selectedItems();
    for (QListWidgetItem* item : selected)
        delete item;
}

// ---------------------------------------------------------------------------
// Private slots — Magic skill triggers
// ---------------------------------------------------------------------------

void SettingsDialogQt::onAddMagicTrigger()
{
    addItemToList(magicTriggersList_, magicTriggersInput_,
                  "Magic Skill Trigger Phrases");
}

void SettingsDialogQt::onRemoveMagicTrigger()
{
    const auto selected = magicTriggersList_->selectedItems();
    for (QListWidgetItem* item : selected)
        delete item;
}
