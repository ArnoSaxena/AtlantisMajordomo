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
 * File: MapTabContent_UnitDetails.cpp
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
#include "Data/Order.hpp"
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
#include "Function/OrderItemTokenUtils.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
#include "Function/UnitCapacityUtils.hpp"
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

void MapTabContent::populateUnitsForSelectedRegion()
{
  if (!unitsList_)
  {
    return;
  }

  const int previousSelectedUnitNumber = selectedUnitNumber_;
  ListView_DeleteAllItems(unitsList_);

  if (!appData_ || !hasSelectedRegion_)
  {
    selectedUnitNumber_ = 0;
    clearSelectedUnitDetails();
    return;
  }

  const auto& unitRepository = appData_->unitRepository();
  const auto& unitNewRepository = appData_->unitNewRepository();

  // Compute the latest report period to filter out stale units.
  int latestMonth = 0;
  int latestYear = 0;
  {
    const auto& reportRepository = appData_->reportRepository();
    for (std::size_t i = 0; i < reportRepository.size(); ++i)
    {
      const Report& report = reportRepository.at(i);
      const int rm = report.getMonth();
      const int ry = report.getYear();
      if (rm >= 1 && rm <= 12 && ry > 0)
      {
        if (ry > latestYear || (ry == latestYear && rm > latestMonth))
        {
          latestMonth = rm;
          latestYear = ry;
        }
      }
    }
  }
  const bool hasLatestPeriod = (latestMonth >= 1 && latestMonth <= 12 && latestYear > 0);

  // Battle indicators must reflect the currently displayed turn, not the
  // repository's globally latest battle period (which may be an older turn).
  const int latestBattleMonth = latestMonth;
  const int latestBattleYear = latestYear;
  const bool hasLatestBattlePeriod = hasLatestPeriod;
  int row = 0;

  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const auto& unit = unitRepository.at(index);
    if (unit.getXCoordinate() != selectedRegionX_ ||
        unit.getYCoordinate() != selectedRegionY_ ||
        unit.getZCoordinate() != selectedZ_)
    {
      continue;
    }

    if (hasLatestPeriod && (unit.getMonth() != latestMonth || unit.getYear() != latestYear))
    {
      continue;
    }

    const std::wstring unitNumber = std::to_wstring(unit.getUnitNumber());
    std::wstring factionNumber;
    if (unit.getFactionNumber() > 0)
    {
      factionNumber = std::to_wstring(unit.getFactionNumber());
    }

    std::wstring structureDisplay;
    const int displayStructureId = unit.getFutureStructureId();
    if (displayStructureId != 0)
    {
      const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
        displayStructureId,
        unit.getXCoordinate(),
        unit.getYCoordinate(),
        unit.getZCoordinate());
      if (structure)
      {
        structureDisplay = L"[" + std::to_wstring(displayStructureId) + L"] " + structure->getStructureType();
        if (!structure->getStructureName().empty())
        {
          structureDisplay += L" - " + structure->getStructureName();
        }
      }
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(unitNumber.c_str());
    item.lParam = unit.getUnitNumber();
    ListView_InsertItem(unitsList_, &item);

    ListView_SetItemText(unitsList_, row, 1, const_cast<LPWSTR>(unit.getUnitNameAfterOrders().c_str()));
    ListView_SetItemText(unitsList_, row, 2, const_cast<LPWSTR>(factionNumber.c_str()));

    std::wstring factionName;
    if (unit.getFactionNumber() > 0)
    {
      const Faction* faction = appData_->factionRepository().findByNumber(unit.getFactionNumber());
      if (faction)
      {
        factionName = faction->getName();
      }
    }
    ListView_SetItemText(unitsList_, row, 3, const_cast<LPWSTR>(factionName.c_str()));
    ListView_SetItemText(unitsList_, row, 4, const_cast<LPWSTR>(structureDisplay.c_str()));

    const std::map<std::wstring, int> afterCommandCounts =
      Commands::calculateAfterCommandItemCountsForUnit(*appData_, unit);

    std::vector<std::wstring> menEntries;
    for (const auto& [itemToken, amount] : afterCommandCounts)
    {
      if (amount <= 0)
      {
        continue;
      }

      const Item* itemDefinition = appData_->itemRepository().findByIdentifierToken(itemToken);
      if (itemDefinition && itemDefinition->isMan())
      {
        menEntries.push_back(itemToken + L" (" + std::to_wstring(amount) + L")");
      }
    }
    const std::wstring menText = StringUtils::joinLines(menEntries, L", ");

    const auto silverCurrentIt = unit.getItems().find(L"SILV");
    const int silverCurrent = silverCurrentIt != unit.getItems().end() ? silverCurrentIt->second : 0;

    const auto silverAfterIt = afterCommandCounts.find(L"SILV");
    const int silverAfter = silverAfterIt != afterCommandCounts.end() ? silverAfterIt->second : 0;
    const std::wstring silverText = std::to_wstring(silverCurrent) + L" (" + std::to_wstring(silverAfter) + L")";

    std::wstring flags = StringUtils::joinLines(unit.getFlags(), L", ");
    std::wstring skills = SkillFormattingUtils::formatSkills(unit.getSkills());
    const std::wstring monthLongOrder = OrderParsingUtils::findMonthLongOrderText(unit.getOrders());
    const std::wstring warningIndicator = unit.getWarnings().empty() ? L"" : L"!";
    const bool isDamagedInLatestBattle = hasLatestBattlePeriod &&
      appData_->battleRepository().isUnitDamagedInAnyBattleForPeriod(
        unit.getUnitNumber(),
        latestBattleMonth,
        latestBattleYear
      );
    const bool isParticipantInLatestBattle = hasLatestBattlePeriod && appData_->battleRepository().isParticipantInAnyBattleForPeriod(
        unit.getUnitNumber(),
        latestBattleMonth,
        latestBattleYear
      );
    const std::wstring damagedIndicator = isDamagedInLatestBattle ? L"x" : L"";
    const std::wstring battleIndicator = isParticipantInLatestBattle ? L"x" : L"";

    // Units-list column maintenance:
    // 1. Add the title and width to kUnitsListColumns in MapTabContent_private.hpp.
    //    MapTabContent.cpp then creates and resizes the Win32 column automatically.
    // 2. Add the normal-unit value here. Update the UnitNew and empty-structure rows
    //    below when they need a value or an explicit blank cell.
    // 3. Update column-specific Win32 behavior and indexes in MapTabContent_Events.cpp,
    //    including custom drawing and tooltips.
    // 4. Keep Qt in sync: update the column count, header labels, and widths in
    //    MapTabContentQt_Layout.cpp; then update appendUnitRow, each appendUnitRow call,
    //    and setItem indexes in MapTabContentQt_UnitDetails.cpp.
    ListView_SetItemText(unitsList_, row, 5, const_cast<LPWSTR>(menText.c_str()));
    ListView_SetItemText(unitsList_, row, 6, const_cast<LPWSTR>(silverText.c_str()));
    ListView_SetItemText(unitsList_, row, 7, const_cast<LPWSTR>(flags.c_str()));
    ListView_SetItemText(unitsList_, row, 8, const_cast<LPWSTR>(skills.c_str()));
    ListView_SetItemText(unitsList_, row, 9, const_cast<LPWSTR>(monthLongOrder.c_str()));
    ListView_SetItemText(unitsList_, row, 10, const_cast<LPWSTR>(warningIndicator.c_str()));
    ListView_SetItemText(unitsList_, row, 11, const_cast<LPWSTR>(battleIndicator.c_str()));
    ListView_SetItemText(unitsList_, row, 12, const_cast<LPWSTR>(damagedIndicator.c_str()));

    if (previousSelectedUnitNumber != 0 && unit.getUnitNumber() == previousSelectedUnitNumber)
    {
      ListView_SetItemState(unitsList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
      ListView_EnsureVisible(unitsList_, row, FALSE);
    }

    ++row;

    // Display UnitNew entries with this unit as origin
    for (std::size_t newIndex = 0; newIndex < unitNewRepository.size(); ++newIndex)
    {
      const auto& unitNew = unitNewRepository.at(newIndex);
      if (unitNew.getOriginUnit() != unit.getUnitNumber() ||
          unitNew.getXCoordinate() != selectedRegionX_ ||
          unitNew.getYCoordinate() != selectedRegionY_ ||
          unitNew.getZCoordinate() != selectedZ_)
      {
        continue;
      }

      if (hasLatestPeriod && (unitNew.getMonth() != latestMonth || unitNew.getYear() != latestYear))
      {
        continue;
      }

      const std::wstring newUnitNumber = L"New " + std::to_wstring(unitNew.getUnitNumber());
      std::wstring newFactionNumber;
      int newUnitFactionNumber = unitNew.getFactionNumber();
      if (newUnitFactionNumber <= 0 && appData_ != nullptr)
      {
        const Unit* originUnit = appData_->unitRepository().findByNumber(unitNew.getOriginUnit());
        if (originUnit)
        {
          newUnitFactionNumber = originUnit->getFactionNumber();
        }
      }
      if (newUnitFactionNumber > 0)
      {
        newFactionNumber = std::to_wstring(newUnitFactionNumber);
      }

      std::wstring newStructureDisplay;
      const int newDisplayStructureId = unitNew.getFutureStructureId();
      if (newDisplayStructureId != 0)
      {
        const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
          newDisplayStructureId,
          unitNew.getXCoordinate(),
          unitNew.getYCoordinate(),
          unitNew.getZCoordinate());
        if (structure)
        {
          newStructureDisplay = L"[" + std::to_wstring(newDisplayStructureId) + L"] " + structure->getStructureType();
          if (!structure->getStructureName().empty())
          {
            newStructureDisplay += L" - " + structure->getStructureName();
          }
        }
      }

      LVITEMW newItem {};
      newItem.mask = LVIF_TEXT | LVIF_PARAM;
      newItem.iItem = row;
      newItem.iSubItem = 0;
      newItem.pszText = const_cast<LPWSTR>(newUnitNumber.c_str());
      newItem.lParam = -unitNew.getUnitNumber();
      ListView_InsertItem(unitsList_, &newItem);

      ListView_SetItemText(unitsList_, row, 1, const_cast<LPWSTR>(unitNew.getUnitNameAfterOrders().c_str()));
      ListView_SetItemText(unitsList_, row, 2, const_cast<LPWSTR>(newFactionNumber.c_str()));

      std::wstring newFactionName;
      if (newUnitFactionNumber > 0)
      {
        const Faction* faction = appData_->factionRepository().findByNumber(newUnitFactionNumber);
        if (faction)
        {
          newFactionName = faction->getName();
        }
      }
      ListView_SetItemText(unitsList_, row, 3, const_cast<LPWSTR>(newFactionName.c_str()));
      ListView_SetItemText(unitsList_, row, 4, const_cast<LPWSTR>(newStructureDisplay.c_str()));

      const auto newAfterCommandCounts =
        Commands::calculateAfterCommandItemCountsForUnitNew(*appData_, unitNew);

      std::vector<std::wstring> newMenEntries;
      for (const auto& [itemToken, amount] : newAfterCommandCounts)
      {
        if (amount <= 0)
        {
          continue;
        }

        const Item* itemDefinition = appData_->itemRepository().findByIdentifierToken(itemToken);
        if (itemDefinition && itemDefinition->isMan())
        {
          newMenEntries.push_back(itemToken + L" (" + std::to_wstring(amount) + L")");
        }
      }
      const std::wstring newMenText = StringUtils::joinLines(newMenEntries, L", ");

      const auto newSilverCurrentIt = unitNew.getItems().find(L"SILV");
      const int newSilverCurrent = newSilverCurrentIt != unitNew.getItems().end() ? newSilverCurrentIt->second : 0;
      const auto newSilverAfterIt = newAfterCommandCounts.find(L"SILV");
      const int newSilverAfter = newSilverAfterIt != newAfterCommandCounts.end() ? newSilverAfterIt->second : 0;
      const std::wstring newSilverText = std::to_wstring(newSilverCurrent) + L" (" + std::to_wstring(newSilverAfter) + L")";

      std::wstring newFlags = StringUtils::joinLines(unitNew.getFlags(), L", ");
      std::wstring newSkills = SkillFormattingUtils::formatSkills(unitNew.getSkills());
      const Unit* originUnit = appData_->unitRepository().findByNumber(unitNew.getOriginUnit());
      const std::wstring newMonthLongOrder = originUnit
        ? OrderParsingUtils::findMonthLongOrderText(OrderParsingUtils::extractFormNewUnitBlock(originUnit->getOrders(), unitNew.getUnitNumber()))
        : L"";
      const std::wstring newWarningIndicator = unitNew.getWarnings().empty() ? L"" : L"!";

      ListView_SetItemText(unitsList_, row, 5, const_cast<LPWSTR>(newMenText.c_str()));
      ListView_SetItemText(unitsList_, row, 6, const_cast<LPWSTR>(newSilverText.c_str()));
      ListView_SetItemText(unitsList_, row, 7, const_cast<LPWSTR>(newFlags.c_str()));
      ListView_SetItemText(unitsList_, row, 8, const_cast<LPWSTR>(newSkills.c_str()));
      ListView_SetItemText(unitsList_, row, 9, const_cast<LPWSTR>(newMonthLongOrder.c_str()));
      ListView_SetItemText(unitsList_, row, 10, const_cast<LPWSTR>(newWarningIndicator.c_str()));

      if (previousSelectedUnitNumber != 0 && unitNew.getUnitNumber() == previousSelectedUnitNumber)
      {
        ListView_SetItemState(unitsList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(unitsList_, row, FALSE);
      }

      ++row;
    }
  }

  const auto regionStructures = appData_->structureRepository().findByCoordinates(
    selectedRegionX_,
    selectedRegionY_,
    selectedZ_
  );
  for (const Structure* structure : regionStructures)
  {
    if (!structure)
    {
      continue;
    }

    if (hasLatestPeriod && (structure->getMonth() != latestMonth || structure->getYear() != latestYear))
    {
      continue;
    }

    if (appData_->unitRepository().hasUnitInStructureAtCoordinates(
      structure->getStructureId(),
      structure->getXCoordinate(),
      structure->getYCoordinate(),
      structure->getZCoordinate()))
    {
      continue;
    }

    LVITEMW emptyStructureItem {};
    emptyStructureItem.mask = LVIF_TEXT | LVIF_PARAM;
    emptyStructureItem.iItem = row;
    emptyStructureItem.iSubItem = 0;
    emptyStructureItem.pszText = nullptr; // Unit Number column left empty
    emptyStructureItem.lParam = 0;
    ListView_InsertItem(unitsList_, &emptyStructureItem);

    // Unit Name column left empty (column 1)

    // Structure column (column 4): same format as for units
    std::wstring structureDisplay = L"[" + std::to_wstring(structure->getStructureId()) + L"] " + structure->getStructureType();
    if (!structure->getStructureName().empty()) {
      structureDisplay += L" - " + structure->getStructureName();
    }
    ListView_SetItemText(unitsList_, row, 4, const_cast<LPWSTR>(structureDisplay.c_str()));

    ++row;
  }

  if (unitsListSortColumn_ >= 0)
  {
    sortUnitsListByColumn(unitsListSortColumn_, unitsListSortAscending_);
  }

  updateSelectedUnitFromList();
}

void MapTabContent::clearUnitsList()
{
  if (unitsList_)
  {
    ListView_DeleteAllItems(unitsList_);
  }
  selectedUnitNumber_ = 0;
  clearSelectedUnitDetails();
}


void MapTabContent::updateSelectedUnitFromList()
{
  if (!unitsList_)
  {
    selectedUnitNumber_ = 0;
    clearSelectedUnitDetails();
    return;
  }

  const int selectedRow = ListView_GetNextItem(unitsList_, -1, LVNI_SELECTED);
  if (selectedRow < 0)
  {
    selectedUnitNumber_ = 0;
    clearSelectedUnitDetails();
    return;
  }

  LVITEMW item {};
  item.mask = LVIF_PARAM;
  item.iItem = selectedRow;
  item.iSubItem = 0;
  if (!ListView_GetItem(unitsList_, &item))
  {
    selectedUnitNumber_ = 0;
    clearSelectedUnitDetails();
    return;
  }

  const int itemValue = static_cast<int>(item.lParam);
  if (itemValue == 0)
  {
    selectedUnitNumber_ = 0;
    selectedUnitIsNew_ = false;
    clearSelectedUnitDetails();
  }
  else
  {
    selectedUnitIsNew_ = itemValue < 0;
    selectedUnitNumber_ = selectedUnitIsNew_ ? -itemValue : itemValue;
    updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
  }
  
  // Update region details to show structure of selected unit
  if (appData_ && hasSelectedRegion_)
  {
    const Region* region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!region)
    {
      region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_);
    }
    if (region)
    {
      updateRegionDetailsView(region);
    }
  }
}


void MapTabContent::populateItemsForSelectedUnit(const Unit* unit)
{
  if (!unitItemsList_)
  {
    return;
  }

  ListView_DeleteAllItems(unitItemsList_);
  if (!appData_ || !unit)
  {
    return;
  }

  const std::map<std::wstring, int> afterCommandCounts =
    Commands::calculateAfterCommandItemCountsForUnit(*appData_, *unit);

  std::map<std::wstring, int> normalizedCurrentCounts;
  for (const auto& [itemToken, amount] : unit->getItems())
  {
    if (amount <= 0)
    {
      continue;
    }

    const std::wstring normalized = StringUtils::normalizeToken(itemToken);
    if (normalized.empty())
    {
      continue;
    }

    normalizedCurrentCounts[normalized] += amount;
  }

  std::map<std::wstring, int> normalizedAfterCounts;
  for (const auto& [itemToken, amount] : afterCommandCounts)
  {
    if (amount <= 0)
    {
      continue;
    }

    const std::wstring normalized = StringUtils::normalizeToken(itemToken);
    if (normalized.empty())
    {
      continue;
    }

    normalizedAfterCounts[normalized] += amount;
  }

  std::set<std::wstring> itemTokens;
  for (const auto& [itemToken, _] : normalizedCurrentCounts)
  {
    itemTokens.insert(itemToken);
  }
  for (const auto& [itemToken, _] : normalizedAfterCounts)
  {
    itemTokens.insert(itemToken);
  }
  for (const std::wstring& touchedToken : OrderItemTokenUtils::collectTouchedItemTokensForUnit(*appData_, unit->getUnitNumber(), false))
  {
    itemTokens.insert(touchedToken);
  }

  std::vector<std::wstring> sortedItemTokens(itemTokens.begin(), itemTokens.end());
  std::sort(sortedItemTokens.begin(), sortedItemTokens.end(),
            [this](const std::wstring& leftToken, const std::wstring& rightToken)
            {
              const Item* leftItem = appData_ ? appData_->itemRepository().findByIdentifierToken(leftToken) : nullptr;
              const Item* rightItem = appData_ ? appData_->itemRepository().findByIdentifierToken(rightToken) : nullptr;

              const bool leftIsMan = leftItem && leftItem->isMan();
              const bool rightIsMan = rightItem && rightItem->isMan();
              if (leftIsMan != rightIsMan)
              {
                return leftIsMan > rightIsMan;
              }

              return leftToken < rightToken;
            });

  int row = 0;
  for (const std::wstring& itemToken : sortedItemTokens)
  {
    const auto currentIt = normalizedCurrentCounts.find(itemToken);
    const int amount = currentIt != normalizedCurrentCounts.end() ? currentIt->second : 0;

    const auto afterIt = normalizedAfterCounts.find(itemToken);
    const int amountAfterCommands = afterIt != normalizedAfterCounts.end() ? afterIt->second : 0;

    const std::map<std::wstring, int> afterGiveCounts =
      Commands::calculateAfterGiveTransfersForUnit(*appData_, *unit);
    const auto afterGiveIt = afterGiveCounts.find(itemToken);
    const int amountAfterGive = afterGiveIt != afterGiveCounts.end() ? afterGiveIt->second : amount;

    std::wstring itemName;
    if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
    {
      itemName = item->getItemName();
    }

    std::wstring amountText = std::to_wstring(amount);
    std::wstring amountAfterGiveText = std::to_wstring(amountAfterGive);
    std::wstring amountAfterCommandsText = std::to_wstring(amountAfterCommands);
    LVITEMW listItem {};
    listItem.mask = LVIF_TEXT;
    listItem.iItem = row;
    listItem.iSubItem = 0;
    listItem.pszText = const_cast<LPWSTR>(itemToken.c_str());
    ListView_InsertItem(unitItemsList_, &listItem);
    ListView_SetItemText(unitItemsList_, row, 1, const_cast<LPWSTR>(itemName.c_str()));
    ListView_SetItemText(unitItemsList_, row, 2, const_cast<LPWSTR>(amountText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 3, const_cast<LPWSTR>(amountAfterGiveText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 4, const_cast<LPWSTR>(amountAfterCommandsText.c_str()));
    ++row;
  }
}


void MapTabContent::populateItemsForSelectedUnit(const UnitNew* unitNew)
{
  if (!unitItemsList_)
  {
    return;
  }

  ListView_DeleteAllItems(unitItemsList_);
  if (!appData_ || !unitNew)
  {
    return;
  }

  const std::map<std::wstring, int> afterCommandCounts =
    Commands::calculateAfterCommandItemCountsForUnitNew(*appData_, *unitNew);

  std::map<std::wstring, int> normalizedCurrentCounts;
  for (const auto& [itemToken, amount] : unitNew->getItems())
  {
    if (amount <= 0)
    {
      continue;
    }

    const std::wstring normalized = StringUtils::normalizeToken(itemToken);
    if (normalized.empty())
    {
      continue;
    }

    normalizedCurrentCounts[normalized] += amount;
  }

  std::map<std::wstring, int> normalizedAfterCounts;
  for (const auto& [itemToken, amount] : afterCommandCounts)
  {
    if (amount <= 0)
    {
      continue;
    }

    const std::wstring normalized = StringUtils::normalizeToken(itemToken);
    if (normalized.empty())
    {
      continue;
    }

    normalizedAfterCounts[normalized] += amount;
  }

  std::set<std::wstring> itemTokens;
  for (const auto& [itemToken, _] : normalizedCurrentCounts)
  {
    itemTokens.insert(itemToken);
  }
  for (const auto& [itemToken, _] : normalizedAfterCounts)
  {
    itemTokens.insert(itemToken);
  }
  for (const std::wstring& touchedToken : OrderItemTokenUtils::collectTouchedItemTokensForUnit(*appData_, unitNew->getUnitNumber(), true))
  {
    itemTokens.insert(touchedToken);
  }

  std::vector<std::wstring> sortedItemTokens(itemTokens.begin(), itemTokens.end());
  std::sort(sortedItemTokens.begin(), sortedItemTokens.end(),
            [this](const std::wstring& leftToken, const std::wstring& rightToken)
            {
              const Item* leftItem = appData_ ? appData_->itemRepository().findByIdentifierToken(leftToken) : nullptr;
              const Item* rightItem = appData_ ? appData_->itemRepository().findByIdentifierToken(rightToken) : nullptr;

              const bool leftIsMan = leftItem && leftItem->isMan();
              const bool rightIsMan = rightItem && rightItem->isMan();
              if (leftIsMan != rightIsMan)
              {
                return leftIsMan > rightIsMan;
              }

              return leftToken < rightToken;
            });

  int row = 0;
  for (const std::wstring& itemToken : sortedItemTokens)
  {
    const auto currentIt = normalizedCurrentCounts.find(itemToken);
    const int amount = currentIt != normalizedCurrentCounts.end() ? currentIt->second : 0;

    const auto afterIt = normalizedAfterCounts.find(itemToken);
    const int amountAfterCommands = afterIt != normalizedAfterCounts.end() ? afterIt->second : 0;

    const std::map<std::wstring, int> afterGiveCounts =
      Commands::calculateAfterGiveTransfersForUnitNew(*appData_, *unitNew);
    const auto afterGiveIt = afterGiveCounts.find(itemToken);
    const int amountAfterGive = afterGiveIt != afterGiveCounts.end() ? afterGiveIt->second : amount;

    std::wstring itemName;
    if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
    {
      itemName = item->getItemName();
    }

    std::wstring amountText = std::to_wstring(amount);
    std::wstring amountAfterGiveText = std::to_wstring(amountAfterGive);
    std::wstring amountAfterCommandsText = std::to_wstring(amountAfterCommands);
    LVITEMW listItem {};
    listItem.mask = LVIF_TEXT;
    listItem.iItem = row;
    listItem.iSubItem = 0;
    listItem.pszText = const_cast<LPWSTR>(itemToken.c_str());
    ListView_InsertItem(unitItemsList_, &listItem);
    ListView_SetItemText(unitItemsList_, row, 1, const_cast<LPWSTR>(itemName.c_str()));
    ListView_SetItemText(unitItemsList_, row, 2, const_cast<LPWSTR>(amountText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 3, const_cast<LPWSTR>(amountAfterGiveText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 4, const_cast<LPWSTR>(amountAfterCommandsText.c_str()));
    ++row;
  }
}


void MapTabContent::populateSkillsList(const Unit* unit)
{
  if (!unitSkillsList_)
  {
    return;
  }

  ListView_DeleteAllItems(unitSkillsList_);
  if (!unit)
  {
    return;
  }

  const auto& skills = unit->getSkills();
  const auto& afterCommandSkills = unit->getSkillsAfterOrders();
  
  int row = 0;
  
  // First, display all currently-known skills with their before and after values
  for (const auto& [skillToken, days] : skills)
  {
    const int level = Skill::trainingDaysToLevel(days);

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(skillToken.c_str());
    item.lParam = 0;
    ListView_InsertItem(unitSkillsList_, &item);

    // Format level with days in brackets
    const std::wstring levelText = std::to_wstring(level) + L" [" + std::to_wstring(days) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(levelText.c_str()));

    // Get after-command skill days and calculate level
    const auto afterIt = afterCommandSkills.find(skillToken);
    const int afterDays = afterIt != afterCommandSkills.end() ? afterIt->second : days;
    const int afterLevel = Skill::trainingDaysToLevel(afterDays);
    
    // Format after-command level with days in brackets
    const std::wstring afterLevelText = std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(afterLevelText.c_str()));

    ++row;
  }
  
  // Then, display newly-studied skills that don't exist in current skills
  for (const auto& [skillToken, afterDays] : afterCommandSkills)
  {
    if (skills.find(skillToken) != skills.end())
    {
      // Skip skills that were already displayed above
      continue;
    }

    const int afterLevel = Skill::trainingDaysToLevel(afterDays);

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(skillToken.c_str());
    item.lParam = 0;
    ListView_InsertItem(unitSkillsList_, &item);

    // Current level: 0 with 0 days
    const std::wstring levelText = L"0 [0]";
    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(levelText.c_str()));

    // After-command level with days in brackets
    const std::wstring afterLevelText = std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(afterLevelText.c_str()));

    ++row;
  }

  // Finally, append potential study tokens at the bottom in gray.
  const auto& canStudySkillTokens = unit->getCanStudySkillTokens();
  for (const std::wstring& canStudyToken : canStudySkillTokens)
  {
    if (canStudyToken.empty())
    {
      continue;
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(canStudyToken.c_str());
    item.lParam = 1;
    ListView_InsertItem(unitSkillsList_, &item);

    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(L"can study"));
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(L""));

    ++row;
  }
}


void MapTabContent::populateSkillsList(const UnitNew* unitNew)
{
  if (!unitSkillsList_)
  {
    return;
  }

  ListView_DeleteAllItems(unitSkillsList_);
  if (!unitNew)
  {
    return;
  }

  const auto& skills = unitNew->getSkills();
  const auto& afterCommandSkills = unitNew->getSkillsAfterOrders();
  int row = 0;

  // First, display all currently-known skills with their before and after values.
  for (const auto& [skillToken, days] : skills)
  {
    const int level = Skill::trainingDaysToLevel(days);

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(skillToken.c_str());
    item.lParam = 0;
    ListView_InsertItem(unitSkillsList_, &item);

    const std::wstring levelText = std::to_wstring(level) + L" [" + std::to_wstring(days) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(levelText.c_str()));

    const auto afterIt = afterCommandSkills.find(skillToken);
    const int afterDays = afterIt != afterCommandSkills.end() ? afterIt->second : days;
    const int afterLevel = Skill::trainingDaysToLevel(afterDays);
    const std::wstring afterLevelText = std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(afterLevelText.c_str()));

    ++row;
  }

  // Then, display newly-studied skills that are absent from current skills.
  for (const auto& [skillToken, afterDays] : afterCommandSkills)
  {
    if (skills.find(skillToken) != skills.end())
    {
      continue;
    }

    const int afterLevel = Skill::trainingDaysToLevel(afterDays);

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(skillToken.c_str());
    item.lParam = 0;
    ListView_InsertItem(unitSkillsList_, &item);

    const std::wstring levelText = L"0 [0]";
    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(levelText.c_str()));

    const std::wstring afterLevelText = std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(afterLevelText.c_str()));

    ++row;
  }

  const auto& canStudySkillTokens = unitNew->getCanStudySkillTokens();
  for (const std::wstring& canStudyToken : canStudySkillTokens)
  {
    if (canStudyToken.empty())
    {
      continue;
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(canStudyToken.c_str());
    item.lParam = 1;
    ListView_InsertItem(unitSkillsList_, &item);

    ListView_SetItemText(unitSkillsList_, row, 1, const_cast<LPWSTR>(L"can study"));
    ListView_SetItemText(unitSkillsList_, row, 2, const_cast<LPWSTR>(L""));

    ++row;
  }
}


int MapTabContent::populateErrorsList(const Unit* unit)
{
  if (!unitErrorsList_)
  {
    return 0;
  }

  ListView_DeleteAllItems(unitErrorsList_);
  if (!unit || !appData_)
  {
    return 0;
  }

  const auto& eventRepository = appData_->eventRepository();
  const std::vector<const Event*> unitErrors = eventRepository.findErrorsByUnitId(unit->getUnitNumber());
  int row = 0;
  for (const Event* eventValue : unitErrors)
  {
    if (!eventValue)
    {
      continue;
    }

    std::wstring message = eventValue->getMessage();
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = message.data();
    ListView_InsertItem(unitErrorsList_, &item);
    ++row;
  }

  return row;
}


int MapTabContent::populateWarningsList(const Unit* unit)
{
  if (!unitWarningsList_)
  {
    return 0;
  }

  ListView_DeleteAllItems(unitWarningsList_);
  if (!unit)
  {
    return 0;
  }

  int row = 0;
  for (const std::wstring& warning : unit->getWarnings())
  {
    if (warning.empty())
    {
      continue;
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(warning.c_str());
    ListView_InsertItem(unitWarningsList_, &item);
    ++row;
  }

  return row;
}


int MapTabContent::populateWarningsList(const UnitNew* unitNew)
{
  if (!unitWarningsList_)
  {
    return 0;
  }

  ListView_DeleteAllItems(unitWarningsList_);
  if (!unitNew)
  {
    return 0;
  }

  int row = 0;
  for (const std::wstring& warning : unitNew->getWarnings())
  {
    if (warning.empty())
    {
      continue;
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(warning.c_str());
    ListView_InsertItem(unitWarningsList_, &item);
    ++row;
  }

  return row;
}


int MapTabContent::populateUnitEventsList(const Unit* unit)
{
  if (!unitEventsList_)
  {
    return 0;
  }

  ListView_DeleteAllItems(unitEventsList_);

  if (!unit)
  {
    return 0;
  }

  const auto& eventRepository = appData_->eventRepository();
  std::vector<const Event*> unitEvents = eventRepository.findLatestEventsByUnitId(unit->getUnitNumber());

  std::sort(unitEvents.begin(), unitEvents.end(),
      [](const Event* a, const Event* b)
      {
          return a->getEventId() < b->getEventId();
      }
  );

  int row = 0;
  for (const Event* eventValue : unitEvents)
  {
    if (!eventValue)
    {
      continue;
    }

    std::wstring message = eventValue->getMessage();
    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = message.data();
    item.lParam = eventValue->isErrorEvent() ? 1 : 0;
    ListView_InsertItem(unitEventsList_, &item);
    ++row;
  }

  // TODO: sort unitEventsList by eventId


  return row;
}


void MapTabContent::updateUnitDetailsTabCaptions(int errorCount, int warningCount, int eventCount)
{
  if (!unitDetailsTabs_)
  {
    return;
  }

  TCITEMW tabItem {};
  tabItem.mask = TCIF_TEXT;

  wchar_t ordersLabel[] = L"Orders";
  tabItem.pszText = ordersLabel;
  TabCtrl_SetItem(unitDetailsTabs_, kOrdersTabIndex, &tabItem);

  wchar_t errorsLabel[] = L"Errors";
  wchar_t errorsWithAsterisk[] = L"Errors*";
  tabItem.pszText = errorCount > 0 ? errorsWithAsterisk : errorsLabel;
  TabCtrl_SetItem(unitDetailsTabs_, kErrorsTabIndex, &tabItem);

  wchar_t warningsLabel[] = L"Warnings";
  wchar_t warningsWithAsterisk[] = L"*Warnings";
  tabItem.pszText = warningCount > 0 ? warningsWithAsterisk : warningsLabel;
  TabCtrl_SetItem(unitDetailsTabs_, kWarningsTabIndex, &tabItem);

  wchar_t eventsLabel[] = L"Events";
  wchar_t eventsWithAsterisk[] = L"Events*";
  tabItem.pszText = eventCount > 0 ? eventsWithAsterisk : eventsLabel;
  TabCtrl_SetItem(unitDetailsTabs_, kEventsTabIndex, &tabItem);
}


void MapTabContent::updateUnitDetailsTabVisibility()
{
  if (!unitDetailsTabs_ || !ordersEditor_ || !saveOrdersButton_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_)
  {
    return;
  }

  if (selectedUnitDetailsTab_ == kErrorsTabIndex)
  {
    ShowWindow(ordersEditor_, SW_HIDE);
    ShowWindow(saveOrdersButton_, SW_HIDE);
    ShowWindow(unitErrorsList_, SW_SHOW);
    ShowWindow(unitWarningsList_, SW_HIDE);
    ShowWindow(unitEventsList_, SW_HIDE);
    return;
  }

  if (selectedUnitDetailsTab_ == kWarningsTabIndex)
  {
    ShowWindow(ordersEditor_, SW_HIDE);
    ShowWindow(saveOrdersButton_, SW_HIDE);
    ShowWindow(unitErrorsList_, SW_HIDE);
    ShowWindow(unitWarningsList_, SW_SHOW);
    ShowWindow(unitEventsList_, SW_HIDE);
    return;
  }

  if (selectedUnitDetailsTab_ == kEventsTabIndex)
  {
    ShowWindow(ordersEditor_, SW_HIDE);
    ShowWindow(saveOrdersButton_, SW_HIDE);
    ShowWindow(unitErrorsList_, SW_HIDE);
    ShowWindow(unitWarningsList_, SW_HIDE);
    ShowWindow(unitEventsList_, SW_SHOW);
    return;
  }

  // default to orders tab
  ShowWindow(ordersEditor_, SW_SHOW);
  ShowWindow(saveOrdersButton_, SW_SHOW);
  ShowWindow(unitErrorsList_, SW_HIDE);
  ShowWindow(unitWarningsList_, SW_HIDE);
  ShowWindow(unitEventsList_, SW_HIDE);
}


void MapTabContent::updateSelectedUnitDetailsByNumber(int unitNumber)
{
  if (!appData_)
  {
    selectedUnitIsNew_ = false;
    clearSelectedUnitDetails();
    return;
  }

  if (selectedUnitIsNew_)
  {
    const UnitNew* unitNew = appData_->unitNewRepository().findByNumberAndCoordinates(
      unitNumber, selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!unitNew)
    {
      selectedUnitIsNew_ = false;
      clearSelectedUnitDetails();
      return;
    }

    std::wstring title =
      unitNew->getUnitNameAfterOrders() +
      L", [New " + std::to_wstring(unitNew->getUnitNumber()) +
      L"] (origin unit: " + std::to_wstring(unitNew->getOriginUnit()) + L")";
    SetWindowTextW(selectedUnitLabel_, title.c_str());

    std::wstring details = L"Flags: " + StringUtils::joinLines(unitNew->getFlags(), L", ");
    SetWindowTextW(unitFlagsLabel_, details.c_str());
    if (unitWarningLabel_)
    {
      if (!unitNew->getWarnings().empty())
      {
        SetWindowTextW(unitWarningLabel_, StringUtils::joinLines(unitNew->getWarnings(), L" | ").c_str());
      }
      else
      {
        SetWindowTextW(unitWarningLabel_, L"");
      }
    }

    populateItemsForSelectedUnit(unitNew);
    populateSkillsList(unitNew);
    const int errorCount = populateErrorsList(nullptr);
    const int warningCount = populateWarningsList(unitNew);
    const int eventCount = populateUnitEventsList(nullptr);
    updateUnitDetailsTabCaptions(errorCount, warningCount, eventCount);
    updateUnitWeightAndCapacities(unitNew);

    if (ordersEditor_)
    {
      const Unit* originUnit = appData_->unitRepository().findByNumber(unitNew->getOriginUnit());
      if (originUnit)
      {
        const std::vector<std::wstring> formBlock = OrderParsingUtils::extractFormNewUnitBlock(originUnit->getOrders(), unitNew->getUnitNumber());
        if (!formBlock.empty())
        {
          SetWindowTextW(ordersEditor_, (StringUtils::joinLines(formBlock) + L"\r\n").c_str());
        }
        else
        {
          SetWindowTextW(ordersEditor_, L"");
        }
      }
      else
      {
        SetWindowTextW(ordersEditor_, L"");
      }
    }
    setOrdersEditingEnabled(false);
    updateUnitDetailsTabVisibility();

    if (mapCanvas_)
    {
      InvalidateRect(mapCanvas_, nullptr, FALSE);
    }
    return;
  }

  const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
  if (!unit)
  {
    clearSelectedUnitDetails();
    return;
  }

  std::wstring title = unit->getUnitNameAfterOrders() + L" [" + std::to_wstring(unit->getUnitNumber()) + L"]";
  SetWindowTextW(selectedUnitLabel_, title.c_str());

  std::wstring details = L"Flags: " + StringUtils::joinLines(unit->getFlags(), L", ");
  SetWindowTextW(unitFlagsLabel_, details.c_str());
  if (unitWarningLabel_)
  {
    if (!unit->getWarnings().empty())
    {
      SetWindowTextW(unitWarningLabel_, StringUtils::joinLines(unit->getWarnings(), L" | ").c_str());
    }
    else
    {
      SetWindowTextW(unitWarningLabel_, L"");
    }
  }
  populateItemsForSelectedUnit(unit);
  populateSkillsList(unit);
  const int errorCount = populateErrorsList(unit);
  const int warningCount = populateWarningsList(unit);
  const int eventCount = populateUnitEventsList(unit);
  updateUnitDetailsTabCaptions(errorCount, warningCount, eventCount);
  updateUnitWeightAndCapacities(unit);

  // Parse MOVE/ADVANCE/SAIL command from orders and calculate path
  movePathCoordinates_.clear();
  movePathIsSail_ = false;
  movePathHasNegativeCapacity_ = false;
  movePathSailRouteInvalid_ = false;
  
  const auto& orders = unit->getOrders();
  for (const auto& order : orders)
  {
    const std::wstring trimmedOrder = StringUtils::trimWhitespace(order);
    std::wstring upperOrder = StringUtils::toLower(trimmedOrder);
    
    // Check if this is a MOVE, ADVANCE, or SAIL command.
    std::size_t movePos = upperOrder.find(L"move");
    std::size_t advancePos = upperOrder.find(L"advance");
    std::size_t sailPos = upperOrder.find(L"sail");
    if ((movePos != std::wstring::npos && movePos <= 2) ||
        (advancePos != std::wstring::npos && advancePos <= 2) ||
        (sailPos != std::wstring::npos && sailPos <= 2))
    {
      // Extract directions from the order
      std::vector<std::wstring> directions;
      std::wstringstream stream(trimmedOrder);
      std::wstring token;
      bool foundMoveLikeCommand = false;
      bool isSailCommand = false;
      // Track whether the unit is currently inside a structure during path parsing.
      // A structure entry token (numeric ID) transitions the unit into a structure.
      // A hex direction move transitions the unit out of any structure.
      bool inStructure = (unit->getStructureId() > 0);

      while (stream >> token)
      {
        const std::wstring upperToken = StringUtils::toLower(token);
        if (upperToken == L"move" || upperToken == L"advance" || upperToken == L"sail" ||
            (!upperToken.empty() && upperToken[0] == L'@' &&
             (upperToken.find(L"move") != std::wstring::npos ||
              upperToken.find(L"advance") != std::wstring::npos ||
              upperToken.find(L"sail") != std::wstring::npos)))
        {
          foundMoveLikeCommand = true;
          isSailCommand = (upperToken.find(L"sail") != std::wstring::npos);
        }
        else if (foundMoveLikeCommand)
        {
          // A numeric token is a structure entry ID: the unit enters that structure.
          const bool isNumericToken = !token.empty() &&
            std::all_of(token.begin(), token.end(), [](wchar_t c) { return iswdigit(c); });
          if (isNumericToken)
          {
            inStructure = true;
            continue;
          }

          // "IN" is a valid direction only when the unit is inside a structure.
          // It has no effect on map coordinates; just consume it.
          if (upperToken == L"in")
          {
            // IN is only valid inside a structure; ignore it if not in one.
            continue;
          }

          const std::wstring normalized = HexDirectionUtils::normalizeHexDirection(upperToken);
          if (!normalized.empty())
          {
            directions.push_back(normalized);
            inStructure = false; // Moving to a new region exits any structure.
          }
        }
      }
      
      if (!directions.empty())
      {
        if (isSailCommand)
        {
          // SAIL is valid only for the owner of a ship.
          // Both flags are already populated by updateUnitWeightAndCapacities()
          // via UnitCapacityUtils — no GUI-layer ship inspection needed here.
          if (!hasShipOwnerSkillValues_)
          {
            break;
          }

          // Calculate path and evaluate sail-specific colour conditions.
          movePathCoordinates_ = HexDirectionUtils::calculateMovePathCoordinates(
            unit->getXCoordinate(),
            unit->getYCoordinate(),
            directions
          );
          movePathIsSail_ = true;

          // Check every segment: each must involve at least one ocean region.
          // Flying ships are exempt from this restriction.
          bool routeInvalid = false;
          if (!shipIsCapableOfFlying_)
          {
            const RegionRepository& regionRepo = appData_->regionRepository();
            const int unitZ = unit->getZCoordinate();
            for (std::size_t si = 0; si + 1 < movePathCoordinates_.size(); ++si)
            {
              const int sx = movePathCoordinates_[si].first;
              const int sy = movePathCoordinates_[si].second;
              const int ex = movePathCoordinates_[si + 1].first;
              const int ey = movePathCoordinates_[si + 1].second;
              const Region* startReg = regionRepo.findByCoordinates(sx, sy, unitZ);
              const Region* endReg   = regionRepo.findByCoordinates(ex, ey, unitZ);
              const bool startOcean = startReg && startReg->isOcean();
              const bool endOcean   = endReg   && endReg->isOcean();
              if (!startOcean && !endOcean)
              {
                routeInvalid = true;
                break;
              }
            }
          }
          movePathSailRouteInvalid_ = routeInvalid;

          const bool shipSkillInsufficient = hasShipOwnerSkillValues_ && shipOwnerSailingDisplay_ < shipSkillNeedDisplay_;
          movePathHasNegativeCapacity_ = routeInvalid || (shipFreeCapacityDisplay_ < 0) || shipSkillInsufficient;
        }
        else
        {
          // Calculate path for MOVE/ADVANCE.
          movePathCoordinates_ = HexDirectionUtils::calculateMovePathCoordinates(
            unit->getXCoordinate(),
            unit->getYCoordinate(),
            directions
          );
          movePathIsSail_ = false;
          // Red only when walking free capacity is negative (0 is valid).
          movePathHasNegativeCapacity_ = (capacityWalkDisplay_ < 0);
        }
      }
      break; // Only process first MOVE/ADVANCE/SAIL command
    }
  }

  const std::wstring ordersText = StringUtils::joinLines(unit->getOrders()) + L"\r\n";
  SetWindowTextW(ordersEditor_, ordersText.c_str());
  setOrdersEditingEnabled(canEditOrdersForUnit(unit));
  updateUnitDetailsTabVisibility();

  if (mapCanvas_)
  {
    InvalidateRect(mapCanvas_, nullptr, FALSE);
  }

  // Update region details to show structure of selected unit
  if (appData_ && hasSelectedRegion_)
  {
    const Region* region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!region)
    {
      region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_);
    }
    if (region)
    {
      updateRegionDetailsView(region);
    }
  }
}


void MapTabContent::clearSelectedUnitDetails()
{
  if (selectedUnitLabel_)
  {
    SetWindowTextW(selectedUnitLabel_, L"");
  }
  if (unitWeightLabel_)
  {
    SetWindowTextW(unitWeightLabel_, L"");
  }
  if (unitCapacitiesLabel_)
  {
    SetWindowTextW(unitCapacitiesLabel_, L"");
  }
  hasCapacityValues_ = false;
  capacityWalkDisplay_ = 0;
  capacityRideDisplay_ = 0;
  capacityFlyDisplay_ = 0;
  capacitySwimDisplay_ = 0;
  shipCapacityDisplay_ = 0;
  shipFreeCapacityDisplay_ = 0;
  shipSkillNeedDisplay_ = 0;
  shipOwnerSailingDisplay_ = 0;
  showRideCapacity_ = false;
  showFlyCapacity_ = false;
  showSwimCapacity_ = false;
  hasShipCapacityValues_ = false;
  hasShipOwnerSkillValues_ = false;
  shipIsCapableOfFlying_ = false;
  movePathCoordinates_.clear();
  movePathIsSail_ = false;
  movePathHasNegativeCapacity_ = false;
  movePathSailRouteInvalid_ = false;
  if (unitCapacitiesLabel_)
  {
    InvalidateRect(unitCapacitiesLabel_, nullptr, TRUE);
  }
  if (unitCoordinatesLabel_)
  {
    SetWindowTextW(unitCoordinatesLabel_, L"");
  }
  if (unitFlagsLabel_)
  {
    SetWindowTextW(unitFlagsLabel_, L"");
  }
  if (unitWarningLabel_)
  {
    SetWindowTextW(unitWarningLabel_, L"");
  }
  if (unitItemsList_)
  {
    ListView_DeleteAllItems(unitItemsList_);
  }
  if (unitSkillsList_)
  {
    ListView_DeleteAllItems(unitSkillsList_);
  }
  if (unitErrorsList_)
  {
    ListView_DeleteAllItems(unitErrorsList_);
  }
  if (unitWarningsList_)
  {
    ListView_DeleteAllItems(unitWarningsList_);
  }
  if (unitEventsList_)
  {
    ListView_DeleteAllItems(unitEventsList_);
  }
  if (ordersEditor_)
  {
    SetWindowTextW(ordersEditor_, L"\r\n");
  }
  setOrdersEditingEnabled(false);
  updateUnitDetailsTabCaptions(0, 0, 0);
  updateUnitDetailsTabVisibility();

  // Update region details to remove structure info when unit is deselected
  if (appData_ && hasSelectedRegion_)
  {
    const Region* region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!region)
    {
      region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_);
    }
    if (region)
    {
      updateRegionDetailsView(region);
    }
  }
}


void MapTabContent::updateUnitWeightAndCapacities(const Unit* unit)
{
  if (!unit || !appData_ || !unitWeightLabel_ || !unitCapacitiesLabel_)
  {
    return;
  }

  const UnitCapacityUtils::UnitCapacities caps =
    UnitCapacityUtils::getUnitCapacities(*unit, *appData_);
  const UnitCapacityUtils::ShipCapacities ship =
    UnitCapacityUtils::getShipCapacities(*unit, *appData_);

  capacityWalkDisplay_     = caps.walkCapacity;
  capacityRideDisplay_     = caps.rideCapacity;
  capacityFlyDisplay_      = caps.flyCapacity;
  capacitySwimDisplay_     = caps.swimCapacity;
  showRideCapacity_        = caps.hasRideSource;
  showFlyCapacity_         = caps.hasFlySource;
  showSwimCapacity_        = caps.hasSwimSource;
  hasCapacityValues_       = true;
  shipCapacityDisplay_     = ship.shipCapacity;
  shipFreeCapacityDisplay_ = ship.shipFreeCapacity;
  shipSkillNeedDisplay_    = ship.shipSkillNeed;
  shipOwnerSailingDisplay_ = ship.ownerSailContrib;
  hasShipCapacityValues_   = ship.hasCapacityValues;
  hasShipOwnerSkillValues_ = ship.hasOwnerSkillValues;
  shipIsFlying_            = ship.isFlying;
  shipIsCapableOfFlying_   = ship.isCapableOfFlying;

  wchar_t weightText[128];
  swprintf_s(weightText, sizeof(weightText) / sizeof(weightText[0]), L"Weight: %d", caps.totalWeight);
  SetWindowTextW(unitWeightLabel_, weightText);

  std::wstring capacitiesText = L"Walk: " + std::to_wstring(caps.walkCapacity);
  if (showRideCapacity_)
  {
    capacitiesText += L" Ride: " + std::to_wstring(caps.rideCapacity);
  }
  if (showFlyCapacity_)
  {
    capacitiesText += L" Fly: " + std::to_wstring(caps.flyCapacity);
  }
  if (showSwimCapacity_)
  {
    capacitiesText += L" Swim: " + std::to_wstring(caps.swimCapacity);
  }
  SetWindowTextW(unitCapacitiesLabel_, capacitiesText.c_str());
  InvalidateRect(unitCapacitiesLabel_, nullptr, TRUE);
}


void MapTabContent::updateUnitWeightAndCapacities(const UnitNew* unitNew)
{
  if (!unitNew || !appData_ || !unitWeightLabel_ || !unitCapacitiesLabel_)
  {
    return;
  }

  const UnitCapacityUtils::UnitCapacities caps =
    UnitCapacityUtils::getUnitCapacities(*unitNew, *appData_);
  const UnitCapacityUtils::ShipCapacities ship =
    UnitCapacityUtils::getShipCapacities(*unitNew, *appData_);

  capacityWalkDisplay_     = caps.walkCapacity;
  capacityRideDisplay_     = caps.rideCapacity;
  capacityFlyDisplay_      = caps.flyCapacity;
  capacitySwimDisplay_     = caps.swimCapacity;
  showRideCapacity_        = caps.hasRideSource;
  showFlyCapacity_         = caps.hasFlySource;
  showSwimCapacity_        = caps.hasSwimSource;
  hasCapacityValues_       = true;
  shipCapacityDisplay_     = ship.shipCapacity;
  shipFreeCapacityDisplay_ = ship.shipFreeCapacity;
  shipSkillNeedDisplay_    = ship.shipSkillNeed;
  shipOwnerSailingDisplay_ = ship.ownerSailContrib;
  hasShipCapacityValues_   = ship.hasCapacityValues;
  hasShipOwnerSkillValues_ = ship.hasOwnerSkillValues;
  shipIsFlying_            = ship.isFlying;

  wchar_t weightText[128];
  swprintf_s(weightText, sizeof(weightText) / sizeof(weightText[0]), L"Weight: %d", caps.totalWeight);
  SetWindowTextW(unitWeightLabel_, weightText);

  std::wstring capacitiesText = L"Walk: " + std::to_wstring(caps.walkCapacity);
  if (showRideCapacity_)
  {
    capacitiesText += L" Ride: " + std::to_wstring(caps.rideCapacity);
  }
  if (showFlyCapacity_)
  {
    capacitiesText += L" Fly: " + std::to_wstring(caps.flyCapacity);
  }
  if (showSwimCapacity_)
  {
    capacitiesText += L" Swim: " + std::to_wstring(caps.swimCapacity);
  }
  SetWindowTextW(unitCapacitiesLabel_, capacitiesText.c_str());
  InvalidateRect(unitCapacitiesLabel_, nullptr, TRUE);
}

