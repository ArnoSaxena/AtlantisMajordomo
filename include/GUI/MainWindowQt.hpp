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
 * File: MainWindowQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "AppConfig.hpp"

#include <QMainWindow>

class AppData;
class QTabWidget;
class BattlesTabContentQt;
class EventsTabContentQt;
class FactionsTabContentQt;
class ItemsTabContentQt;
class MapTabContentQt;
class ReportsTabContentQt;
class SkillsTabContentQt;

/**
 * @brief Top-level application window for the Qt / Linux build.
 *
 * Mirrors the responsibilities of MainWindowWin: owns the tab widget,
 * drives AppConfig, implements all menus, and manages the startup
 * auto-load sequence. Tab content widgets are currently placeholders
 * and will be replaced with real Qt implementations over time.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(AppData& appData, QWidget* parent = nullptr);
    ~MainWindow() override;

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    static constexpr const char* kAboutAppName     = "Atlantis Majordomo";
    static constexpr const char* kAboutDescription = "Yet another Atlantis Pbem player client.";
    static constexpr const char* kAboutVersion     = "1.2.23";

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileLoadReport();
    void onFileImportData();
    void onFileExportOrders();
    void onSettingsOptions();
    void onHelpDescription();
    void onHelpAbout();
    void onNavigateToBattle(int x, int y, int z, int month, int year);
    void onNavigateToSkill(const QString& skillToken);
    void onNavigateToItem(const QString& itemToken);

private:
    void setupMenus();
    void setupTabs();
    void deferredInit();
    void autoLoad();
    void refreshAllTabs();
    void applyConfigToAppData();
    void applyQtUiSizing();

    AppData&    appData_;
    AppConfig   appConfig_;

    QTabWidget*           tabWidget_           { nullptr };
    ReportsTabContentQt*  reportsTabContent_   { nullptr };
    EventsTabContentQt*   eventsTabContent_    { nullptr };
    BattlesTabContentQt*  battlesTabContent_   { nullptr };
    FactionsTabContentQt* factionsTabContent_  { nullptr };
    ItemsTabContentQt*    itemsTabContent_     { nullptr };
    SkillsTabContentQt*   skillsTabContent_    { nullptr };
    MapTabContentQt*      mapTabContent_       { nullptr };

    QWidget*    reportsTab_  { nullptr };
    QWidget*    mapTab_      { nullptr };
    QWidget*    eventsTab_   { nullptr };
    QWidget*    itemsTab_    { nullptr };
    QWidget*    skillsTab_   { nullptr };
    QWidget*    factionsTab_ { nullptr };
    QWidget*    battlesTab_  { nullptr };
};

