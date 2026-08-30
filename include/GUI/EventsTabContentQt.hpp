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
 * File: EventsTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include <utility>
#include <vector>

class AppData;
class QComboBox;
class QTabWidget;
class QTableWidget;

/**
 * @brief Owns and manages the body content for the "Events" tab (Qt build).
 *
 * Mirrors EventsTabContent for the Win32 build. Shows a date-filter combo
 * at the top and a two-column table (Unit Id / Message) below. Error events
 * are highlighted in red.
 */
class EventsTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit EventsTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~EventsTabContentQt() override = default;

    EventsTabContentQt(const EventsTabContentQt&) = delete;
    EventsTabContentQt& operator=(const EventsTabContentQt&) = delete;
    EventsTabContentQt(EventsTabContentQt&&) = delete;
    EventsTabContentQt& operator=(EventsTabContentQt&&) = delete;

    void refresh();

private slots:
    void onDateComboChanged(int index);

private:
    void updateDateCombo();
    void updateEventsList();
    void updateWarningsList();

    AppData*      appData_    { nullptr };
    QTabWidget*   subTabs_    { nullptr };
    QComboBox*    dateCombo_  { nullptr };
    QTableWidget* eventsList_ { nullptr };
    QTableWidget* warningsList_ { nullptr };

    std::vector<std::pair<int, int>> availablePeriods_;
    int selectedMonth_ { 0 };
    int selectedYear_  { 0 };
};
