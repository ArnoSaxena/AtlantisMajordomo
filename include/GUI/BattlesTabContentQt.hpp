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
 * File: BattlesTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include <utility>
#include <vector>

class AppData;
class Battle;
class QComboBox;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QSplitter;

/**
 * @brief Owns and manages the body content for the "Battles" tab (Qt build).
 *
 * Mirrors BattlesTabContent for the Win32 build. Shows a date-filter combo
 * at the top-left, a list of battles below it, and a right pane with a
 * short summary and the full battle report text. A QSplitter divides the
 * left list from the right detail pane.
 */
class BattlesTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit BattlesTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~BattlesTabContentQt() override = default;

    BattlesTabContentQt(const BattlesTabContentQt&) = delete;
    BattlesTabContentQt& operator=(const BattlesTabContentQt&) = delete;
    BattlesTabContentQt(BattlesTabContentQt&&) = delete;
    BattlesTabContentQt& operator=(BattlesTabContentQt&&) = delete;

    void refresh();

    /**
     * @brief Switches to the period that contains a battle at the given region
     *        and selects that battle in the list. Does nothing if no match.
     */
    void focusBattleByRegion(int x, int y, int z, int month, int year);

signals:
    /** Emitted when the user chooses Map for a selected battle. */
    void navigateToMap(int x, int y, int z);

private slots:
    void onDateComboChanged(int index);
    void onBattleSelectionChanged();
    void onBattleContextMenuRequested(const QPoint& pos);

private:
    void updateDateSelector();
    void updateBattleList();
    void updateBattleDetails(int selectedRow);

    AppData*       appData_       { nullptr };
    QComboBox*     dateCombo_     { nullptr };
    QListWidget*   battlesList_   { nullptr };
    QPlainTextEdit* summaryEdit_  { nullptr };
    QPlainTextEdit* fullReportEdit_ { nullptr };

    std::vector<std::pair<int, int>> availablePeriods_;
    std::vector<const Battle*>       visibleBattles_;
    int selectedMonth_ { 0 };
    int selectedYear_  { 0 };
};
