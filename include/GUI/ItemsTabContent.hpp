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
 * File: ItemsTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include "GUI/ControlIds.hpp"

#include <string>

class AppData;
class Item;

class ItemsTabContent
{
public:
  static constexpr int kListControlId = GUI::ControlIds::kItemsList;
  static constexpr int kSaveButtonId = GUI::ControlIds::kItemsSaveButton;

  ItemsTabContent() = default;
  ~ItemsTabContent() = default;

  ItemsTabContent(const ItemsTabContent&) = delete;
  ItemsTabContent& operator=(const ItemsTabContent&) = delete;
  ItemsTabContent(ItemsTabContent&&) = delete;
  ItemsTabContent& operator=(ItemsTabContent&&) = delete;

  bool create(HWND parentWindow, HINSTANCE instance, AppData& appData);
  void resize(const RECT& displayRect);
  void setVisible(bool visible);
  void refresh();
  bool handleNotify(const NMHDR* hdr);
  bool handleCommand(int commandId, int notificationCode = 0);
  bool handleVScroll(WPARAM wp, LPARAM lp);
  LRESULT getNotifyResult() const { return notifyResult_; }

private:
  AppData* appData_ { nullptr };
  HWND itemsList_ { nullptr };

  HWND tokenLabel_ { nullptr };
  HWND tokenEdit_ { nullptr };
  HWND nameLabel_ { nullptr };
  HWND nameEdit_ { nullptr };
  HWND weightLabel_ { nullptr };
  HWND weightEdit_ { nullptr };
  HWND meeleWeaponCheck_ { nullptr };
  HWND rangedWeaponCheck_ { nullptr };
  HWND armourCheck_ { nullptr };
  HWND resourceCheck_ { nullptr };
  HWND mountCheck_ { nullptr };
  HWND movesLabel_ { nullptr };
  HWND movesEdit_ { nullptr };
  HWND walkCapacityLabel_ { nullptr };
  HWND walkCapacityEdit_ { nullptr };
  HWND rideCapacityLabel_ { nullptr };
  HWND rideCapacityEdit_ { nullptr };
  HWND swimCapacityLabel_ { nullptr };
  HWND swimCapacityEdit_ { nullptr };
  HWND flyCapacityLabel_ { nullptr };
  HWND flyCapacityEdit_ { nullptr };
  HWND shipSpeedLabel_ { nullptr };
  HWND shipSpeedEdit_ { nullptr };
  HWND shipSailingSkillLabel_ { nullptr };
  HWND shipSailingSkillEdit_ { nullptr };
  HWND magesStudyLabel_ { nullptr };
  HWND magesStudyEdit_ { nullptr };
  HWND defaultSkillMaxLabel_ { nullptr };
  HWND defaultSkillMaxEdit_ { nullptr };
  HWND manCheck_ { nullptr };
  HWND skillsMaxLabel_ { nullptr };
  HWND skillsMaxEdit_ { nullptr };
  HWND resourcesLabel_ { nullptr };
  HWND resourcesEdit_ { nullptr };
  HWND productionSkillLabel_ { nullptr };
  HWND productionSkillEdit_ { nullptr };
  HWND productionHelpLabel_ { nullptr };
  HWND productionHelpEdit_ { nullptr };
  HWND fullTextLabel_ { nullptr };
  HWND fullTextEdit_ { nullptr };
  HWND saveButton_ { nullptr };
  HWND verticalScrollBar_ { nullptr };

  std::wstring selectedItemToken_;
  LRESULT notifyResult_ { 0 };
  int dividerAfterRow_ { -1 };
  RECT lastDisplayRect_ { 0, 0, 0, 0 };
  int scrollPosition_ { 0 };
  int maxScrollPosition_ { 0 };

  void updateItemsList();
  void updateSelectedItemFromList();
  void loadItemToFields(const Item* item);
  void clearFields();
  void saveSelectedItem();
};
