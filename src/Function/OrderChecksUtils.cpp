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
 * File: OrderChecksUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/OrderChecksUtils.hpp"

#include "Data/AppData.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/OrderWarningService.hpp"

#include <algorithm>
#include <vector>

namespace OrderChecksUtils
{

namespace
{
int encodeWarningUnitNumber(const AppData& appData, int unitNumber)
{
  if (appData.unitRepository().findByNumber(unitNumber) != nullptr)
  {
    return unitNumber;
  }

  for (std::size_t index = 0; index < appData.unitNewRepository().size(); ++index)
  {
    const UnitNew& unitNew = appData.unitNewRepository().at(index);
    if (unitNew.getUnitNumber() == unitNumber && !unitNew.getWarnings().empty())
    {
      return -unitNumber;
    }
  }

  return unitNumber;
}
}

void runOrderChecksForMainFaction(AppData& appData,
                                  int selectedUnitNumber,
                                  const std::function<void()>& saveOrders,
                                  const std::function<void()>& populateUnits,
                                  const std::function<void()>& updateWarningsSummary,
                                  const std::function<void(int)>& updateSelectedUnitDetails)
{
  if (saveOrders)
  {
    saveOrders();
  }

  OrderWarningService::runForMainFaction(appData);

  if (populateUnits)
  {
    populateUnits();
  }

  if (updateWarningsSummary)
  {
    updateWarningsSummary();
  }

  if (selectedUnitNumber != 0 && updateSelectedUnitDetails)
  {
    updateSelectedUnitDetails(selectedUnitNumber);
  }
}

int selectPreviousWarningUnitNumber(const AppData& appData, int selectedUnitNumber, bool selectedUnitIsNew)
{
  std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  if (warningUnits.empty())
  {
    return 0;
  }

  std::sort(warningUnits.begin(), warningUnits.end());

  const int selectedReference = selectedUnitIsNew ? -selectedUnitNumber : selectedUnitNumber;
  int selectedIndex = -1;
  for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
  {
    if (encodeWarningUnitNumber(appData, warningUnits[static_cast<std::size_t>(index)]) == selectedReference)
    {
      selectedIndex = index;
      break;
    }
  }

  int targetIndex = selectedIndex - 1;
  if (selectedIndex < 0 || targetIndex < 0)
  {
    targetIndex = static_cast<int>(warningUnits.size()) - 1;
  }

  return encodeWarningUnitNumber(appData, warningUnits[static_cast<std::size_t>(targetIndex)]);
}

int selectNextWarningUnitNumber(const AppData& appData, int selectedUnitNumber, bool selectedUnitIsNew)
{
  std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  if (warningUnits.empty())
  {
    return 0;
  }

  std::sort(warningUnits.begin(), warningUnits.end());

  const int selectedReference = selectedUnitIsNew ? -selectedUnitNumber : selectedUnitNumber;
  int selectedIndex = -1;
  for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
  {
    if (encodeWarningUnitNumber(appData, warningUnits[static_cast<std::size_t>(index)]) == selectedReference)
    {
      selectedIndex = index;
      break;
    }
  }

  int targetIndex = selectedIndex + 1;
  if (selectedIndex < 0 || targetIndex >= static_cast<int>(warningUnits.size()))
  {
    targetIndex = 0;
  }

  return encodeWarningUnitNumber(appData, warningUnits[static_cast<std::size_t>(targetIndex)]);
}

} // namespace OrderChecksUtils
