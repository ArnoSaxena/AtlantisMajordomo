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
 * File: MainWindow.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "AppConfig.hpp"

#include <windows.h>
#include <memory>

class AppData;
class EventsTabContent;
class ReportsTabContent;
class ItemsTabContent;
class SkillsTabContent;
class FactionsTabContent;
class BattlesTabContent;
class MapTabContent;
class TabView;

/**
* @brief Top-level application window.
*
* Owns the main HWND and a TabView. The application data model is provided
* externally so the GUI does not own domain objects directly.
*/
class MainWindow
{
public:
  MainWindow();
  ~MainWindow();

  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;
  MainWindow(MainWindow&&) = delete;
  MainWindow& operator=(MainWindow&&) = delete;

  static constexpr wchar_t kAboutAppName[] = L"Atlantis Majordomo";
  static constexpr wchar_t kAboutLoremIpsum[] =
    L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
  static constexpr wchar_t kAboutVersion[] = L"1.0.20";

  /**
  * @brief Registers the window class and creates the main window.
  * @param[in] instance  Application instance handle.
    * @param[in] appData   Root application data model.
  * @return true on success.
  */
    bool create(HINSTANCE instance, AppData& appData);

  /**
  * @brief Shows the window and enters the message loop.
  * @param[in] showCmd  nCmdShow value from WinMain.
  * @return Exit code from the message loop.
  */
  int run(int showCmd);

  /**
  * @brief Returns the TabView so callers can add tabs or populate panels.
  * @return Reference to the owned TabView.
  */
  TabView& getTabView();

private:
  HWND                     hwnd_    { nullptr };
  HINSTANCE                instance_{ nullptr };
  AppData*                 appData_ { nullptr };
  AppConfig                appConfig_;
  std::unique_ptr<TabView> tabView_;
  std::unique_ptr<ReportsTabContent> reportsTabContent_;
  std::unique_ptr<ItemsTabContent> itemsTabContent_;
  std::unique_ptr<SkillsTabContent> skillsTabContent_;
  std::unique_ptr<EventsTabContent> eventsTabContent_;
  std::unique_ptr<MapTabContent> mapTabContent_;
  std::unique_ptr<FactionsTabContent> factionsTabContent_;
  std::unique_ptr<BattlesTabContent> battlesTabContent_;
  int reportsTabIndex_ { -1 };
  int mapTabIndex_ { -1 };
  int eventsTabIndex_ { -1 };
  int itemsTabIndex_ { -1 };
  int skillsTabIndex_ { -1 };
  int factionsTabIndex_ { -1 };
  int battlesTabIndex_ { -1 };

  static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);
  void initializeTabs();
  void createMenu();
  void refreshAllTabContents();
  void updateReportsTabVisibility();
  void showLoadedReportsTabContextMenu(POINT screenPoint);
  void persistWindowSizeToConfig();
};
