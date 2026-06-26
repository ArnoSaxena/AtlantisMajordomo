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
 * File: FactionsTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include <map>
#include <string>

class AppData;
class Faction;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTableWidget;

/**
 * @brief Owns and manages the body content for the "Factions" tab (Qt build).
 *
 * Mirrors FactionsTabContent for the Win32 build. Shows a sorted list of
 * factions on the left (main faction first, then numerically). The right
 * pane is split horizontally: a QScrollArea holds the editable faction-info
 * form and a Save button; an attitudes panel (shown only when the selected
 * faction is the main faction) sits alongside it with a default-attitude
 * QComboBox, an unclaimed-silver read-only field, a command-unit row, and
 * a QTableWidget listing per-faction declared attitudes (right-click to
 * change each one, applied with the command-unit Save button).
 */
class FactionsTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit FactionsTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~FactionsTabContentQt() override = default;

    FactionsTabContentQt(const FactionsTabContentQt&) = delete;
    FactionsTabContentQt& operator=(const FactionsTabContentQt&) = delete;
    FactionsTabContentQt(FactionsTabContentQt&&) = delete;
    FactionsTabContentQt& operator=(FactionsTabContentQt&&) = delete;

    /** Repopulates the factions list and refreshes the detail form. */
    void refresh();

private slots:
    void onFactionSelectionChanged();
    void onDefaultAttitudeComboChanged(int index);
    void onAttitudesContextMenu(const QPoint& pos);
    void onCommandUnitSaveClicked();
    void onSaveClicked();

private:
    void updateFactionsList();
    void updateSelectedFactionFromList();
    void loadFactionToFields(const Faction* faction);
    void updateAttitudesPanel(const Faction* faction);
    void updateAttitudesList(const Faction* faction);
    void captureOriginalAttitudeSnapshot(const Faction* faction);
    void handleDefaultAttitudeSelection(Faction& faction, const std::wstring& selectedAttitudeText);
    void applyAttitudeContextSelection(int targetFactionNumber, const std::wstring& selectedValue);
    void saveAttitudeEdits();
    void clearFields();
    void saveSelectedFaction();

    AppData*       appData_                  { nullptr };

    // Left pane
    QListWidget*   factionsList_             { nullptr };

    // Right pane — faction info form
    QLineEdit*     factionNumberEdit_        { nullptr };   // read-only
    QLineEdit*     factionNameEdit_          { nullptr };
    QCheckBox*     mainFactionCheck_         { nullptr };
    QLineEdit*     monthEdit_               { nullptr };
    QLineEdit*     yearEdit_                { nullptr };
    QLineEdit*     passwordEdit_            { nullptr };
    QLineEdit*     taxedTradedCurrentEdit_   { nullptr };
    QLineEdit*     taxedTradedMaxEdit_       { nullptr };
    QLineEdit*     quartermastersCurrentEdit_ { nullptr };
    QLineEdit*     quartermastersMaxEdit_     { nullptr };
    QLineEdit*     magesCurrentEdit_         { nullptr };
    QLineEdit*     magesMaxEdit_             { nullptr };
    QLineEdit*     apprenticesCurrentEdit_   { nullptr };
    QLineEdit*     apprenticesMaxEdit_       { nullptr };
    QPushButton*   saveButton_              { nullptr };

    // Attitudes panel — only visible when selected faction is main faction
    QWidget*       attitudesWidget_          { nullptr };
    QComboBox*     defaultAttitudeCombo_     { nullptr };
    QLineEdit*     unclaimedSilverEdit_      { nullptr };  // read-only
    QLineEdit*     commandUnitEdit_          { nullptr };
    QPushButton*   commandUnitSaveButton_    { nullptr };
    QTableWidget*  attitudesTable_           { nullptr };

    // Pending attitude edits (committed by the command-unit Save button)
    struct PendingAttitudeEdit
    {
        bool         useDefault   { false };
        std::wstring attitudeText;
    };

    std::map<int, PendingAttitudeEdit>         pendingAttitudeEdits_;
    std::wstring                               originalDefaultAttitudeText_  { L"Neutral" };
    std::map<int, std::wstring>                originalDeclaredAttitudesText_;
    std::map<int, std::wstring>                originalDefaultAttitudeByFactionText_;
    std::map<int, std::map<int, std::wstring>> originalDeclaredAttitudesByFactionText_;

    int            selectedFactionNumber_    { 0 };
};
