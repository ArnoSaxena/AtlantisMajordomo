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
 * File: ControlIds.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <array>
#include <cstddef>

namespace GUI::ControlIds
{
constexpr int kMainTabView = 100;

constexpr int kReportsList = 2001;
constexpr int kReportsRemoveButton = 2002;

constexpr int kMapCanvas = 2300;
constexpr int kMapUnitsList = 2301;
constexpr int kMapSaveOrdersButton = 2302;
constexpr int kMapUnitItemsList = 2303;
constexpr int kMapUnitCapacitiesLabel = 2304;
constexpr int kMapCheckOrdersButton = 2305;
constexpr int kMapLastWarningButton = 2306;
constexpr int kMapClearWarningButton = 2307;
constexpr int kMapNextWarningButton = 2308;
constexpr int kMapUnitSearchButton = 2309;
constexpr int kMapUnitSearchEdit = 2310;
constexpr int kMapUnitErrorsList = 2311;
constexpr int kMapUnitDetailsTabs = 2312;
constexpr int kMapUnitWarningsList = 2313;
constexpr int kMapUnitEventsList = 2314;

constexpr int kItemsList = 2401;
constexpr int kItemsSaveButton = 2402;

constexpr int kSkillsList = 2501;
constexpr int kSkillsSaveButton = 2502;
constexpr int kSkillsLevelDropdown = 2503;

constexpr int kFactionsList = 2601;
constexpr int kFactionsSaveButton = 2602;
constexpr int kFactionsDefaultAttitudeCombo = 2603;
constexpr int kFactionsAttitudesList = 2604;
constexpr int kFactionsCommandUnitSaveButton = 2605;

constexpr int kBattlesDateCombo = 2701;
constexpr int kBattlesList = 2702;

constexpr int kEventsList = 2800;
constexpr int kEventsDateCombo = 2801;

constexpr int kSettingsOk = 5002;
constexpr int kSettingsCancel = 5003;
constexpr int kSettingsShipThresholdEdit = 5004;
constexpr int kSettingsFlyingShipsList = 5005;
constexpr int kSettingsFullMonthOrdersList = 5006;
constexpr int kSettingsOnlyLeaderCanTeachCheck = 5007;
constexpr int kSettingsSave = 5008;
constexpr int kSettingsMagicTriggersList = 5009;
constexpr int kSettingsFlyingShipsInput = 5010;
constexpr int kSettingsFullMonthOrdersInput = 5011;
constexpr int kSettingsMagicTriggersInput = 5012;
constexpr int kSettingsDataFilePathInput = 5020;
constexpr int kSettingsDataFilePathBrowse = 5021;
constexpr int kSettingsReportFolderPathInput = 5022;
constexpr int kSettingsReportFolderBrowse = 5023;
constexpr int kSettingsFlyingShipsAdd = 5013;
constexpr int kSettingsFlyingShipsRemove = 5014;
constexpr int kSettingsFullMonthOrdersAdd = 5015;
constexpr int kSettingsFullMonthOrdersRemove = 5016;
constexpr int kSettingsMagicTriggersAdd = 5017;
constexpr int kSettingsMagicTriggersRemove = 5018;
constexpr int kSettingsLeaderMagesCheck = 5019;

constexpr std::array<int, 54> kAll = {
  kMainTabView,
  kReportsList,
  kReportsRemoveButton,
  kMapCanvas,
  kMapUnitsList,
  kMapSaveOrdersButton,
  kMapUnitItemsList,
  kMapUnitCapacitiesLabel,
  kMapCheckOrdersButton,
  kMapLastWarningButton,
  kMapClearWarningButton,
  kMapNextWarningButton,
  kMapUnitSearchButton,
  kMapUnitSearchEdit,
  kMapUnitErrorsList,
  kMapUnitDetailsTabs,
  kMapUnitWarningsList,
  kMapUnitEventsList,
  kItemsList,
  kItemsSaveButton,
  kSkillsList,
  kSkillsSaveButton,
  kSkillsLevelDropdown,
  kFactionsList,
  kFactionsSaveButton,
  kFactionsDefaultAttitudeCombo,
  kFactionsAttitudesList,
  kFactionsCommandUnitSaveButton,
  kBattlesDateCombo,
  kBattlesList,
  kEventsList,
  kEventsDateCombo,
  kSettingsOk,
  kSettingsCancel,
  kSettingsShipThresholdEdit,
  kSettingsFlyingShipsList,
  kSettingsFullMonthOrdersList,
  kSettingsOnlyLeaderCanTeachCheck,
  kSettingsSave,
  kSettingsMagicTriggersList,
  kSettingsFlyingShipsInput,
  kSettingsFullMonthOrdersInput,
  kSettingsMagicTriggersInput,
  kSettingsDataFilePathInput,
  kSettingsDataFilePathBrowse,
  kSettingsReportFolderPathInput,
  kSettingsReportFolderBrowse,
  kSettingsFlyingShipsAdd,
  kSettingsFlyingShipsRemove,
  kSettingsFullMonthOrdersAdd,
  kSettingsFullMonthOrdersRemove,
  kSettingsMagicTriggersAdd,
  kSettingsMagicTriggersRemove,
  kSettingsLeaderMagesCheck,
};

constexpr bool hasDuplicateControlIds()
{
  for (std::size_t i = 0; i < kAll.size(); ++i)
  {
    for (std::size_t j = i + 1; j < kAll.size(); ++j)
    {
      if (kAll[i] == kAll[j])
      {
        return true;
      }
    }
  }
  return false;
}

static_assert(!hasDuplicateControlIds(), "Duplicate GUI control IDs in GUI::ControlIds");
} // namespace GUI::ControlIds