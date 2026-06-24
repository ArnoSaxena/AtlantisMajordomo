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
 * File: ReportsTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

class AppData;
class AppConfig;
class QLabel;
class QListWidget;
class QPushButton;
class QSplitter;

/**
 * @brief Owns and manages the body content for the "Loaded Reports" tab (Qt build).
 *
 * Mirrors ReportsTabContent for the Win32 build. The widget shows a list of
 * loaded reports on the left, a detail pane on the right, and a "Clear" button
 * below the list.  A QSplitter lets the user resize the two panes.
 */
class ReportsTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit ReportsTabContentQt(AppData& appData, AppConfig& appConfig,
                                  QWidget* parent = nullptr);
    ~ReportsTabContentQt() override = default;

    ReportsTabContentQt(const ReportsTabContentQt&) = delete;
    ReportsTabContentQt& operator=(const ReportsTabContentQt&) = delete;
    ReportsTabContentQt(ReportsTabContentQt&&) = delete;
    ReportsTabContentQt& operator=(ReportsTabContentQt&&) = delete;

    void loadReport(bool syncFactionFromHeader = true,
                    bool rememberReportImportFolder = true,
                    bool rememberDataFilePath = false);
    void refresh();

private slots:
    void onSelectionChanged();
    void onClearClicked();
    void onContextMenuRequested(const QPoint& pos);

private:
    void updateReportsList();
    void updateDetailPane(int selectedRow);

    AppData*    appData_    { nullptr };
    AppConfig*  appConfig_  { nullptr };

    QSplitter*    splitter_             { nullptr };
    QListWidget*  reportsList_          { nullptr };
    QPushButton*  clearButton_          { nullptr };
    QLabel*       factionLabel_         { nullptr };
    QLabel*       monthLabel_           { nullptr };
    QLabel*       foundRegionsLabel_    { nullptr };
    QLabel*       visitedRegionsLabel_  { nullptr };

    int selectedReportRow_ { -1 };
};
