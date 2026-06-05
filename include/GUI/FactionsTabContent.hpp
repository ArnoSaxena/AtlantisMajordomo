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
 * File: FactionsTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include "GUI/ControlIds.hpp"

#include <map>
#include <string>

class AppData;
class Faction;

class FactionsTabContent
{
public:
  static constexpr int kListControlId = GUI::ControlIds::kFactionsList;
  static constexpr int kSaveButtonId = GUI::ControlIds::kFactionsSaveButton;
  static constexpr int kDefaultAttitudeComboId = GUI::ControlIds::kFactionsDefaultAttitudeCombo;
  static constexpr int kAttitudesListControlId = GUI::ControlIds::kFactionsAttitudesList;
  static constexpr int kCommandUnitSaveButtonId = GUI::ControlIds::kFactionsCommandUnitSaveButton;

  FactionsTabContent() = default;
  ~FactionsTabContent() = default;

  FactionsTabContent(const FactionsTabContent&) = delete;
  FactionsTabContent& operator=(const FactionsTabContent&) = delete;
  FactionsTabContent(FactionsTabContent&&) = delete;
  FactionsTabContent& operator=(FactionsTabContent&&) = delete;

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
  HWND factionsList_ { nullptr };

  HWND factionNumberLabel_ { nullptr };
  HWND factionNumberEdit_ { nullptr };
  HWND factionNameLabel_ { nullptr };
  HWND factionNameEdit_ { nullptr };
  HWND mainFactionCheck_ { nullptr };
  HWND monthLabel_ { nullptr };
  HWND monthEdit_ { nullptr };
  HWND yearLabel_ { nullptr };
  HWND yearEdit_ { nullptr };
  HWND passwordLabel_ { nullptr };
  HWND passwordEdit_ { nullptr };
  HWND taxedTradedCurrentLabel_ { nullptr };
  HWND taxedTradedCurrentEdit_ { nullptr };
  HWND taxedTradedMaxLabel_ { nullptr };
  HWND taxedTradedMaxEdit_ { nullptr };
  HWND quartermastersCurrentLabel_ { nullptr };
  HWND quartermastersCurrentEdit_ { nullptr };
  HWND quartermastersMaxLabel_ { nullptr };
  HWND quartermastersMaxEdit_ { nullptr };
  HWND magesCurrentLabel_ { nullptr };
  HWND magesCurrentEdit_ { nullptr };
  HWND magesMaxLabel_ { nullptr };
  HWND magesMaxEdit_ { nullptr };
  HWND apprenticesCurrentLabel_ { nullptr };
  HWND apprenticesCurrentEdit_ { nullptr };
  HWND apprenticesMaxLabel_ { nullptr };
  HWND apprenticesMaxEdit_ { nullptr };
  HWND unclaimedSilverLabel_ { nullptr };
  HWND unclaimedSilverEdit_ { nullptr };
  HWND defaultAttitudeLabel_ { nullptr };
  HWND defaultAttitudeCombo_ { nullptr };
  HWND attitudesList_ { nullptr };
  HWND commandUnitLabel_ { nullptr };
  HWND commandUnitEdit_ { nullptr };
  HWND commandUnitSaveButton_ { nullptr };
  HWND saveButton_ { nullptr };
  HWND verticalScrollBar_ { nullptr };

  struct PendingAttitudeEdit
  {
    bool useDefault { false };
    std::wstring attitudeText;
  };

  std::map<int, PendingAttitudeEdit> pendingAttitudeEdits_;
  std::wstring originalDefaultAttitudeText_ { L"Neutral" };
  std::map<int, std::wstring> originalDeclaredAttitudesText_;
  std::map<int, std::wstring> originalDefaultAttitudeByFactionText_;
  std::map<int, std::map<int, std::wstring>> originalDeclaredAttitudesByFactionText_;

  int selectedFactionNumber_ { 0 };
  LRESULT notifyResult_ { 0 };
  RECT lastDisplayRect_ { 0, 0, 0, 0 };
  int scrollPosition_ { 0 };
  int maxScrollPosition_ { 0 };

  void updateFactionsList();
  void updateSelectedFactionFromList();
  void loadFactionToFields(const Faction* faction);
  void updateAttitudesPanel(const Faction* faction);
  void updateAttitudesList(const Faction* faction);
  void setAttitudesPanelVisible(bool visible);
  void handleDefaultAttitudeSelection(Faction& faction, const std::wstring& selectedAttitudeText);
  void captureOriginalAttitudeSnapshot(const Faction* faction);
  void showAttitudeContextMenu();
  void applyAttitudeContextSelection(int targetFactionNumber, const std::wstring& selectedValue);
  void saveAttitudeEdits();
  void clearFields();
  void saveSelectedFaction();
};
