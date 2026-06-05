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
 * File: EventsTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <string>
#include <utility>
#include <vector>

class AppData;

class EventsTabContent
{
public:
  EventsTabContent() = default;
  ~EventsTabContent() = default;

  EventsTabContent(const EventsTabContent&) = delete;
  EventsTabContent& operator=(const EventsTabContent&) = delete;
  EventsTabContent(EventsTabContent&&) = delete;
  EventsTabContent& operator=(EventsTabContent&&) = delete;

  bool create(HWND parentWindow, HINSTANCE instance, AppData& appData);
  void resize(const RECT& displayRect);
  void setVisible(bool visible);
  void refresh();
  bool handleNotify(const NMHDR* hdr);
  bool handleCommand(int commandId, int notificationCode = 0);
  LRESULT getNotifyResult() const;

private:
  AppData* appData_ { nullptr };
  HWND     dateCombo_ { nullptr };
  HWND     eventsList_ { nullptr };
  LRESULT  notifyResult_ { 0 };
  std::vector<std::pair<int, int>> availablePeriods_;
  int selectedMonth_ { 0 };
  int selectedYear_ { 0 };

  void updateDateDropdown();
  void updateSelectedPeriodFromDropdown();
  void updateEventsList();
};
