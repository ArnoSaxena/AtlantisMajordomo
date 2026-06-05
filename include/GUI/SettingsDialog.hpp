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
 * File: SettingsDialog.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include "GUI/ControlIds.hpp"

class AppData;
class AppConfig;

/**
* @brief Settings dialog for application configuration.
*
* Provides a modal dialog for users to configure application settings.
* Can be extended with additional controls and settings as needed.
*/
class SettingsDialog
{
public:
  SettingsDialog();
  ~SettingsDialog();

  SettingsDialog(const SettingsDialog&) = delete;
  SettingsDialog& operator=(const SettingsDialog&) = delete;
  SettingsDialog(SettingsDialog&&) = delete;
  SettingsDialog& operator=(SettingsDialog&&) = delete;

  /**
  * @brief Shows the settings dialog as a modal dialog.
  * @param[in] parentHwnd  Parent window handle.
  * @param[in] appData  Application data containing settings and structures.
  * @return IDOK if user clicked OK, IDCANCEL if clicked Cancel, -1 on error.
  */
  int showDialog(HWND parentHwnd, AppData& appData, AppConfig& appConfig);

private:
  static constexpr int IDD_SETTINGS = 5001;
  static constexpr int IDC_OK = GUI::ControlIds::kSettingsOk;
  static constexpr int IDC_CANCEL = GUI::ControlIds::kSettingsCancel;
  static constexpr int IDC_SAVE = GUI::ControlIds::kSettingsSave;
  static constexpr int IDC_SHIP_THRESHOLD_EDIT = GUI::ControlIds::kSettingsShipThresholdEdit;
  //static constexpr int IDC_FLYING_SHIPS_LIST = GUI::ControlIds::kSettingsFlyingShipsList;
  static constexpr int IDC_FULL_MONTH_ORDERS_LIST = GUI::ControlIds::kSettingsFullMonthOrdersList;
  static constexpr int IDC_ONLY_LEADER_CAN_TEACH_CHECK = GUI::ControlIds::kSettingsOnlyLeaderCanTeachCheck;
  static constexpr int IDC_MAGIC_TRIGGERS_LIST = GUI::ControlIds::kSettingsMagicTriggersList;
  static constexpr int IDC_FLYING_SHIPS_INPUT = GUI::ControlIds::kSettingsFlyingShipsInput;
  static constexpr int IDC_FULL_MONTH_ORDERS_INPUT = GUI::ControlIds::kSettingsFullMonthOrdersInput;
  static constexpr int IDC_MAGIC_TRIGGERS_INPUT = GUI::ControlIds::kSettingsMagicTriggersInput;
  static constexpr int IDC_DATA_FILE_PATH_INPUT = GUI::ControlIds::kSettingsDataFilePathInput;
  static constexpr int IDC_DATA_FILE_PATH_BROWSE = GUI::ControlIds::kSettingsDataFilePathBrowse;
  static constexpr int IDC_REPORT_FOLDER_PATH_INPUT = GUI::ControlIds::kSettingsReportFolderPathInput;
  static constexpr int IDC_REPORT_FOLDER_PATH_BROWSE = GUI::ControlIds::kSettingsReportFolderBrowse;
  //static constexpr int IDC_FLYING_SHIPS_ADD = GUI::ControlIds::kSettingsFlyingShipsAdd;
  //static constexpr int IDC_FLYING_SHIPS_REMOVE = GUI::ControlIds::kSettingsFlyingShipsRemove;
  static constexpr int IDC_FULL_MONTH_ORDERS_ADD = GUI::ControlIds::kSettingsFullMonthOrdersAdd;
  static constexpr int IDC_FULL_MONTH_ORDERS_REMOVE = GUI::ControlIds::kSettingsFullMonthOrdersRemove;
  static constexpr int IDC_MAGIC_TRIGGERS_ADD = GUI::ControlIds::kSettingsMagicTriggersAdd;
  static constexpr int IDC_MAGIC_TRIGGERS_REMOVE = GUI::ControlIds::kSettingsMagicTriggersRemove;
  static constexpr int IDC_LEADER_MAGES_CHECK = GUI::ControlIds::kSettingsLeaderMagesCheck;

  static LRESULT CALLBACK windowProcedureStatic(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  LRESULT windowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  bool applySettingsFromControls(HWND hwnd, bool closeOnSuccess);
  void addListItem(HWND listHandle, HWND inputHandle, const wchar_t* listNameForWarning);
  void removeSelectedListItem(HWND listHandle);

  AppData* appData_ { nullptr };
  AppConfig* appConfig_ { nullptr };
  HWND hwnd_ { nullptr };
  HWND shipThresholdEdit_ { nullptr };
  HWND dataFilePathEdit_ { nullptr };
  HWND reportFolderPathEdit_ { nullptr };
  //HWND flyingShipsList_ { nullptr };
  HWND fullMonthOrdersList_ { nullptr };
  HWND magicTriggersList_ { nullptr };
  //HWND flyingShipsInput_ { nullptr };
  HWND fullMonthOrdersInput_ { nullptr };
  HWND magicTriggersInput_ { nullptr };
  HWND onlyLeaderCanTeachCheck_ { nullptr };
  HWND leaderMagesCheck_ { nullptr };
  int result_ { -1 };
};
