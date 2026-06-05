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
 * File: ReportsTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include "GUI/ControlIds.hpp"
#include <string>

class AppData;
class AppConfig;

/**
* @brief Owns and manages the body content for the "Loaded Reports" tab.
*/
class ReportsTabContent
{
public:
  static constexpr int kListControlId   = GUI::ControlIds::kReportsList;
  static constexpr int kRemoveButtonId  = GUI::ControlIds::kReportsRemoveButton;
  static constexpr int kContextLoadCmd  = 3001;
  static constexpr int kContextRemoveCmd = 3002;

  ReportsTabContent() = default;
  ~ReportsTabContent() = default;

  ReportsTabContent(const ReportsTabContent&) = delete;
  ReportsTabContent& operator=(const ReportsTabContent&) = delete;
  ReportsTabContent(ReportsTabContent&&) = delete;
  ReportsTabContent& operator=(ReportsTabContent&&) = delete;

  bool create(HWND parentWindow, HINSTANCE instance, AppData& appData, AppConfig& appConfig);
  void resize(const RECT& displayRect);
  void setVisible(bool visible);

  bool handleNotify(const NMHDR* hdr);
  bool handleCommand(int commandId, int notificationCode = 0);
  bool handleMouseMessage(UINT msg, WPARAM wp, LPARAM lp);

  void loadReport(bool syncFactionFromHeader = true);
  void refresh();

private:
  HWND parentWindow_ { nullptr };
  HINSTANCE instance_ { nullptr };
  AppData* appData_ { nullptr };
  AppConfig* appConfig_ { nullptr };

  HWND reportsList_ { nullptr };
  HWND removeReportButton_ { nullptr };
  HWND rightPane_ { nullptr };
  HWND factionLabel_ { nullptr };
  HWND monthLabel_ { nullptr };
  HWND foundRegionsLabel_ { nullptr };
  HWND visitedRegionsLabel_ { nullptr };
  int contextMenuRow_ { -1 };
  int selectedReportRow_ { -1 };
  RECT lastDisplayRect_ { 0, 0, 0, 0 };
  float horizontalSplitRatio_ { 0.5F };
  bool draggingHorizontalSplit_ { false };

  void showContextMenu(POINT screenPoint, int rowIndex);
  void removeReportAt(int rowIndex);
  void clearReports();
  void updateReportsList();
  void updateDetailPane(int selectedRow);
  std::wstring showFileOpenDialog() const;
  RECT getHorizontalSplitterRect() const;
};
