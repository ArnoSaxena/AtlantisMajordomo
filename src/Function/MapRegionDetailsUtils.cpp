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
 * File: MapRegionDetailsUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/MapRegionDetailsUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/StructInfo.hpp"
#include "Data/Structure.hpp"
#include "Data/Unit.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>
#include <map>
#include <vector>

namespace
{
int resolveAfterOrderAmount(const std::map<std::wstring, int>& afterOrders,
                            const std::wstring& token,
                            int fallback)
{
  auto afterIter = afterOrders.find(token);
  if (afterIter != afterOrders.end())
  {
    return afterIter->second;
  }

  const std::wstring upperToken = StringUtils::toUpper(token);
  afterIter = afterOrders.find(upperToken);
  if (afterIter != afterOrders.end())
  {
    return afterIter->second;
  }

  return fallback;
}

int resolveAfterOrderAmountFromPairMap(const std::map<std::wstring, std::pair<int, int>>& afterOrders,
                                       const std::wstring& token,
                                       int fallback)
{
  auto afterIter = afterOrders.find(token);
  if (afterIter != afterOrders.end())
  {
    return afterIter->second.first;
  }

  const std::wstring upperToken = StringUtils::toUpper(token);
  afterIter = afterOrders.find(upperToken);
  if (afterIter != afterOrders.end())
  {
    return afterIter->second.first;
  }

  return fallback;
}
}

namespace MapRegionDetailsUtils
{

std::wstring buildRegionSummaryText(const Region& region,
                                    const AppData* appData,
                                    int selectedUnitNumber,
                                    int selectedZCoordinate,
                                    const wchar_t* lineBreak)
{
  const wchar_t* separator = lineBreak != nullptr ? lineBreak : L"\r\n";

  std::wstring details;
  details += L"Coordinates: " + CoordinateFormattingUtils::formatCoordinates(
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate()
  ) + separator;
  details += L"Region Type: " + region.getRegionType() + separator;
  details += L"Peasants: " + region.getPeasantType() + separator;
  details += L"Province: " + region.getProvinceName() + separator;

  if (region.getContainsSettlement())
  {
    details += L"Settlement Type: " + region.getSettlementType() + separator;
    details += L"Settlement Name: " + region.getSettlementName();
  }

  if (!appData || selectedUnitNumber <= 0)
  {
    return details;
  }

  const Unit* selectedUnit = appData->unitRepository().findByNumber(selectedUnitNumber);
  const int selectedDisplayStructureId = selectedUnit ? selectedUnit->getFutureStructureId() : 0;
  if (!selectedUnit
      || selectedUnit->getXCoordinate() != region.getXCoordinate()
      || selectedUnit->getYCoordinate() != region.getYCoordinate()
      || selectedUnit->getZCoordinate() != selectedZCoordinate
      || selectedDisplayStructureId <= 0)
  {
    return details;
  }

  const Structure* structure = appData->structureRepository().findByIdAndCoordinates(
    selectedDisplayStructureId,
    selectedUnit->getXCoordinate(),
    selectedUnit->getYCoordinate(),
    selectedUnit->getZCoordinate());
  if (!structure)
  {
    return details;
  }

  details += separator;
  details += L"Structure: " + structure->getStructureType() + L" [" + std::to_wstring(selectedDisplayStructureId) + L"]";
  if (!structure->getStructureName().empty())
  {
    details += L" - " + structure->getStructureName();
  }

  const StructInfo* structInfo = appData->structInfoRepository().findByType(structure->getStructureType());
  if (structInfo && structInfo->getNeeds() > 0)
  {
    details += L", needs " + std::to_wstring(structInfo->getNeeds());
  }

  const auto& fleetItems = structure->getFleetItems();
  for (const auto& itemEntry : fleetItems)
  {
    const std::wstring& itemToken = itemEntry.first;
    const int amount = itemEntry.second;
    const StructInfo* itemStructInfo = appData->structInfoRepository().findByItemIdentifierToken(itemToken);
    const std::wstring itemType = itemStructInfo ? itemStructInfo->getStructureType() : itemToken;
    details += separator + std::wstring(L"  ") + std::to_wstring(amount) + L" " + itemType + L" [" + itemToken + L"]";
  }

  return details;
}

std::vector<ResourceRow> buildResourcesRows(const Region& region)
{
  std::vector<ResourceRow> rows;

  rows.push_back(ResourceRow{ L"Entertainment", region.getEntertainment(), region.getEntertainmentAfterOrders() });
  rows.push_back(ResourceRow{ L"Taxes", region.getTaxableIncome(), region.getTaxableIncomeAfterOrders() });
  rows.push_back(ResourceRow{ L"Work wages", region.getWagesMax(), region.getWagesAfterOrders() });

  const auto& resources = region.getResources();
  const auto& afterCommandResources = region.getResourcesAfterOrders();
  for (const auto& entry : resources)
  {
    const std::wstring& token = entry.first;
    const int amount = entry.second;
    rows.push_back(ResourceRow{
      token,
      amount,
      resolveAfterOrderAmount(afterCommandResources, token, 0)
    });
  }

  return rows;
}

std::vector<ForSaleRow> buildForSaleRows(const Region& region, const AppData* appData)
{
  std::vector<std::wstring> sortedTokens;
  const auto& forSale = region.getForSale();
  sortedTokens.reserve(forSale.size());
  for (const auto& entry : forSale)
  {
    sortedTokens.push_back(entry.first);
  }

  if (appData)
  {
    std::stable_sort(sortedTokens.begin(), sortedTokens.end(),
      [appData](const std::wstring& a, const std::wstring& b)
      {
        const Item* ia = appData->itemRepository().findByIdentifierToken(a);
        const Item* ib = appData->itemRepository().findByIdentifierToken(b);
        const bool manA = ia && ia->isMan();
        const bool manB = ib && ib->isMan();
        if (manA != manB)
        {
          return manA > manB;
        }
        return false;
      });
  }

  std::vector<ForSaleRow> rows;
  rows.reserve(sortedTokens.size());

  const auto& afterCommandForSale = region.getForSaleAfterOrders();
  for (const std::wstring& token : sortedTokens)
  {
    const auto& amountPrice = forSale.at(token);
    rows.push_back(ForSaleRow{
      token,
      amountPrice.first,
      amountPrice.second,
      resolveAfterOrderAmountFromPairMap(afterCommandForSale, token, amountPrice.first)
    });
  }

  return rows;
}

std::vector<WantedRow> buildWantedRows(const Region& region)
{
  const auto& wanted = region.getWanted();
  const auto& afterCommandWanted = region.getWantedAfterOrders();

  std::vector<WantedRow> rows;
  rows.reserve(wanted.size());
  for (const auto& entry : wanted)
  {
    const std::wstring& token = entry.first;
    const int amount = entry.second.first;
    const int price = entry.second.second;
    rows.push_back(WantedRow{
      token,
      amount,
      price,
      resolveAfterOrderAmountFromPairMap(afterCommandWanted, token, amount)
    });
  }

  return rows;
}

} // namespace MapRegionDetailsUtils
