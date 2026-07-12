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
 * File: SettingsDialogQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QDialog>

class AppConfig;
class AppData;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;

/**
 * @brief Modal settings dialog for the Qt / Linux build.
 *
 * Mirrors SettingsDialog for the Win32 build. Presents a QFormLayout for
 * single-value settings (ship-ID threshold, data-file path, report-folder
 * path, leader/mage flags) and two side-by-side QGroupBox panels with
 * editable QListWidget sets for full-month order keywords and magic-skill
 * trigger phrases. A QDialogButtonBox provides Apply (save without closing),
 * OK (save and close), and Cancel (close without saving).
 *
 * On Apply or OK the dialog writes all values directly into @p appData and
 * @p appConfig and calls AppConfig::save(). The caller should invoke
 * refreshAllTabs() after a successful exec() == QDialog::Accepted.
 */
class SettingsDialogQt : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialogQt(AppData& appData, AppConfig& appConfig,
                               QWidget* parent = nullptr);
    ~SettingsDialogQt() override = default;

    SettingsDialogQt(const SettingsDialogQt&) = delete;
    SettingsDialogQt& operator=(const SettingsDialogQt&) = delete;
    SettingsDialogQt(SettingsDialogQt&&) = delete;
    SettingsDialogQt& operator=(SettingsDialogQt&&) = delete;

private slots:
    void onApply();
    void onAccepted();
    void onBrowseDataFile();
    void onBrowseReportFolder();
    void onAddFullMonthOrder();
    void onRemoveFullMonthOrder();
    void onAddMagicTrigger();
    void onRemoveMagicTrigger();

private:
    /** Validates inputs, writes to AppData/AppConfig, saves config.
     *  Returns false and shows a warning when validation fails. */
    bool applySettings();

    void    loadListFromCsv(QListWidget* list, const std::wstring& csv);
    std::wstring buildCsvFromList(const QListWidget* list) const;
    /** Trims input, checks for duplicates (case-insensitive), adds if unique.
     *  Shows a warning and returns false on duplicate or empty input. */
    bool    addItemToList(QListWidget* list, QLineEdit* input,
                          const QString& listName);

    AppData*   appData_   { nullptr };
    AppConfig* appConfig_ { nullptr };

    QLineEdit*   shipThresholdEdit_       { nullptr };
    QLineEdit*   dataFilePathEdit_        { nullptr };
    QLineEdit*   reportFolderPathEdit_    { nullptr };
    QComboBox*   uiSizeModeCombo_         { nullptr };
    QComboBox*   mapHexSizeModeCombo_     { nullptr };
    QCheckBox*   onlyLeaderCanTeachCheck_ { nullptr };
    QCheckBox*   leaderMagesCheck_        { nullptr };
    QListWidget* fullMonthOrdersList_     { nullptr };
    QLineEdit*   fullMonthOrdersInput_    { nullptr };
    QListWidget* magicTriggersList_       { nullptr };
    QLineEdit*   magicTriggersInput_      { nullptr };
};
