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
 * File: SkillsTabContent.hpp
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
class Skill;

class SkillsTabContent
{
public:
  static constexpr int kListControlId   = GUI::ControlIds::kSkillsList;
  static constexpr int kSaveButtonId    = GUI::ControlIds::kSkillsSaveButton;
  static constexpr int kLevelDropdownId = GUI::ControlIds::kSkillsLevelDropdown;

  SkillsTabContent() = default;
  ~SkillsTabContent() = default;

  SkillsTabContent(const SkillsTabContent&) = delete;
  SkillsTabContent& operator=(const SkillsTabContent&) = delete;
  SkillsTabContent(SkillsTabContent&&) = delete;
  SkillsTabContent& operator=(SkillsTabContent&&) = delete;

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
  HWND skillsList_ { nullptr };

  HWND tokenLabel_           { nullptr };
  HWND tokenEdit_            { nullptr };
  HWND levelLabel_           { nullptr };
  HWND levelDropdown_        { nullptr };
  HWND nameLabel_            { nullptr };
  HWND nameEdit_             { nullptr };
  HWND studyCostLabel_       { nullptr };
  HWND studyCostEdit_        { nullptr };
  HWND productionCheck_      { nullptr };
  HWND magicCheck_           { nullptr };
  HWND magicFoundationCheck_ { nullptr };
  HWND prerequisitesLabel_   { nullptr };
  HWND prerequisitesEdit_    { nullptr };
  HWND productionItemsLabel_ { nullptr };
  HWND productionItemsEdit_  { nullptr };
  HWND descriptionLabel_     { nullptr };
  HWND descriptionEdit_      { nullptr };
  HWND saveButton_           { nullptr };
  HWND verticalScrollBar_    { nullptr };

  std::wstring selectedSkillToken_;
  int displayedLevel_ { 0 };
  LRESULT notifyResult_ { 0 };
  int dividerAfterRow_ { -1 };
  RECT lastDisplayRect_ { 0, 0, 0, 0 };
  int scrollPosition_ { 0 };
  int maxScrollPosition_ { 0 };

  void updateSkillsList();
  void updateSelectedSkillFromList();
  void populateLevelDropdown(const Skill* skill);
  void loadSkillLevelToFields(const Skill* skill, int level);
  void clearFields();
  void saveSelectedSkill();
};
