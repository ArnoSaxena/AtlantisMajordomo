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
 * File: MapTabContent_OrderChecks.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GUI/MapTabContent.hpp"
#include "GUI/MapTabContent_private.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/Faction.hpp"
#include "GUI/WinGuiUtils.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/Skill.hpp"
#include "Data/Structure.hpp"
#include "GUI/ControlIds.hpp"
#include "GUI/OrdersEditorUtils.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/CoordinateUtils.hpp"
#include "Function/MonthUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
#include "Function/SkillFormattingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <commctrl.h>
#include <cwctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <windowsx.h>

void MapTabContent::runOrderChecksForMainFaction()
{
  if (!appData_)
  {
    return;
  }

  saveOrdersToSelectedUnit();
  OrderWarningService::runForMainFaction(*appData_);

  populateUnitsForSelectedRegion();
  updateWarningsSummaryLabel();
  if (selectedUnitNumber_ != 0)
  {
    updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
  }
}


void MapTabContent::updateWarningsSummaryLabel()
{
  if (!warningsCountLabel_ || !appData_)
  {
    return;
  }

  const int warningCount = appData_->unitRepository().countTotalWarnings();

  const std::wstring text = L"Warnings: " + std::to_wstring(warningCount);
  SetWindowTextW(warningsCountLabel_, text.c_str());
}


void MapTabContent::selectPreviousWarningUnit()
{
  if (!appData_)
  {
    return;
  }

  std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(*appData_);

  if (warningUnits.empty())
  {
    return;
  }

  std::sort(warningUnits.begin(), warningUnits.end());

  int selectedIndex = -1;
  for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
  {
    if (warningUnits[static_cast<std::size_t>(index)] == selectedUnitNumber_)
    {
      selectedIndex = index;
      break;
    }
  }

  int targetIndex = selectedIndex - 1;
  if (selectedIndex < 0)
  {
    targetIndex = static_cast<int>(warningUnits.size()) - 1;
  }
  else if (targetIndex < 0)
  {
    targetIndex = static_cast<int>(warningUnits.size()) - 1;
  }

  selectUnitInMap(warningUnits[static_cast<std::size_t>(targetIndex)]);
}


void MapTabContent::selectNextWarningUnit()
{
  if (!appData_)
  {
    return;
  }

  std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(*appData_);

  if (warningUnits.empty())
  {
    return;
  }

  std::sort(warningUnits.begin(), warningUnits.end());

  int selectedIndex = -1;
  for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
  {
    if (warningUnits[static_cast<std::size_t>(index)] == selectedUnitNumber_)
    {
      selectedIndex = index;
      break;
    }
  }

  int targetIndex = selectedIndex + 1;
  if (selectedIndex < 0)
  {
    targetIndex = 0;
  }
  else if (targetIndex >= static_cast<int>(warningUnits.size()))
  {
    targetIndex = 0;
  }

  selectUnitInMap(warningUnits[static_cast<std::size_t>(targetIndex)]);
}


void MapTabContent::clearWarningsForSelectedUnit()
{
  if (!appData_ || selectedUnitNumber_ == 0)
  {
    return;
  }

  Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
  if (!unit)
  {
    return;
  }

  unit->clearWarnings();
  populateUnitsForSelectedRegion();
  updateWarningsSummaryLabel();
  updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
}

