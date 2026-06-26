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
 * File: MapTabContent_RegionDetails.cpp
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

void MapTabContent::updateRegionDetailsView(const Region* region)
{
  if (!regionDetailsView_)
  {
    return;
  }

  if (!region)
  {
    SetWindowTextW(regionDateLabel_, AppDataUtils::buildDateLabelText(appData_).c_str());
    SetWindowTextW(regionDetailsView_, L"No region selected");
    populateResourcesList(nullptr);
    populateForSaleList(nullptr);
    populateWantedList(nullptr);
    return;
  }

  std::wstring details;
  SetWindowTextW(regionDateLabel_, AppDataUtils::buildDateLabelText(appData_).c_str());
  details += L"Coordinates: " + CoordinateFormattingUtils::formatCoordinates(
    region->getXCoordinate(),
    region->getYCoordinate(),
    region->getZCoordinate()
  ) + L"\r\n";
  details += L"Region Type: " + region->getRegionType() + L"\r\n";
  details += L"Peasants: " + region->getPeasantType() + L"\r\n";
  details += L"Province: " + region->getProvinceName() + L"\r\n";
  if (region->getContainsSettlement())
  {
    details += L"Settlement Type: " + region->getSettlementType() + L"\r\n";
    details += L"Settlement Name: " + region->getSettlementName();
  }

  // Display structure of selected unit if it's in this region
  if (appData_ && selectedUnitNumber_ > 0)
  {
    const Unit* selectedUnit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    const int selectedDisplayStructureId = selectedUnit
      ? selectedUnit->getFutureStructureId()
      : 0;

    if (selectedUnit &&
        selectedUnit->getXCoordinate() == region->getXCoordinate() &&
        selectedUnit->getYCoordinate() == region->getYCoordinate() &&
        selectedUnit->getZCoordinate() == selectedZ_ &&
        selectedDisplayStructureId > 0)
    {
      const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
        selectedDisplayStructureId,
        selectedUnit->getXCoordinate(),
        selectedUnit->getYCoordinate(),
        selectedUnit->getZCoordinate());
      if (structure)
      {
        details += L"\r\nStructure: " + structure->getStructureType() + L" [" + std::to_wstring(selectedDisplayStructureId) + L"]";
        if (!structure->getStructureName().empty())
        {
          details += L" - " + structure->getStructureName();
        }
        const StructInfo* structInfo = appData_->structInfoRepository().findByType(structure->getStructureType());
        if (structInfo && structInfo->getNeeds() > 0)
        {
          details += L", needs " + std::to_wstring(structInfo->getNeeds());
        }

        const auto& fleetItems = structure->getFleetItems();
        if (!fleetItems.empty())
        {
          for (const auto& itemEntry : fleetItems)
          {
            const std::wstring& itemToken = itemEntry.first;
            const int amount = itemEntry.second;
            const StructInfo* itemStructInfo = appData_->structInfoRepository().findByItemIdentifierToken(itemToken);
            const std::wstring itemType = itemStructInfo ? itemStructInfo->getStructureType() : itemToken;
            details += L"\r\n  " + std::to_wstring(amount) + L" " + itemType + L" [" + itemToken + L"]";
          }
        }
      }
    }
  }

  SetWindowTextW(regionDetailsView_, details.c_str());
  populateResourcesList(region);
  populateForSaleList(region);
  populateWantedList(region);
}


void MapTabContent::populateResourcesList(const Region* region)
{
  if (!regionResourcesList_)
  {
    return;
  }

  SendMessageW(regionResourcesList_, LVM_DELETEALLITEMS, 0, 0);

  if (!region)
  {
    return;
  }

  const auto& resources = region->getResources();
  const auto& afterCommandResources = region->getResourcesAfterOrders();
  const int entertainmentAfterCommands = region->getEntertainmentAfterOrders();
  const int taxesAfterCommands = region->getTaxableIncomeAfterOrders();
  const int workWagesAfterCommands = region->getWagesAfterOrders();

  auto insertResourceRow = [this](int rowIndex,
                                  const std::wstring& itemName,
                                  int amount,
                                  int amountAfterCommands)
  {
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = rowIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(itemName.c_str());
    SendMessageW(regionResourcesList_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 1;
    const std::wstring amountStr = std::to_wstring(amount);
    item.pszText = const_cast<LPWSTR>(amountStr.c_str());
    SendMessageW(regionResourcesList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 2;
    const std::wstring amountAfterCommandsStr = std::to_wstring(amountAfterCommands);
    item.pszText = const_cast<LPWSTR>(amountAfterCommandsStr.c_str());
    SendMessageW(regionResourcesList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));
  };

  int itemIndex = 0;

  insertResourceRow(itemIndex++, L"Entertainment", region->getEntertainment(), entertainmentAfterCommands);
  insertResourceRow(itemIndex++, L"Taxes", region->getTaxableIncome(), taxesAfterCommands);
  insertResourceRow(itemIndex++, L"Work wages", region->getWagesMax(), workWagesAfterCommands);

  for (const auto& [token, amount] : resources)
  {
    int amountAfterCommands = amount;
    auto afterIt = afterCommandResources.find(token);
    if (afterIt == afterCommandResources.end())
    {
      std::wstring tokenUpper = token;
      for (wchar_t& ch : tokenUpper)
      {
        ch = static_cast<wchar_t>(towupper(ch));
      }
      afterIt = afterCommandResources.find(tokenUpper);
    }
    if (afterIt != afterCommandResources.end())
    {
      amountAfterCommands = afterIt->second;
    }

    insertResourceRow(itemIndex, token, amount, amountAfterCommands);

    ++itemIndex;
  }
}


void MapTabContent::populateForSaleList(const Region* region)
{
  if (!regionForSaleList_)
  {
    return;
  }

  SendMessageW(regionForSaleList_, LVM_DELETEALLITEMS, 0, 0);

  if (!region)
  {
    return;
  }

  const auto& forSale = region->getForSale();
  const auto& afterCommandForSale = region->getForSaleAfterOrders();

  // Build a sorted list: isMan items first, then the rest, each group in token order.
  std::vector<std::wstring> sortedTokens;
  sortedTokens.reserve(forSale.size());
  for (const auto& [token, amountPrice] : forSale)
  {
    sortedTokens.push_back(token);
  }
  if (appData_)
  {
    std::stable_sort(sortedTokens.begin(), sortedTokens.end(),
      [this](const std::wstring& a, const std::wstring& b)
      {
        const Item* ia = appData_->itemRepository().findByIdentifierToken(a);
        const Item* ib = appData_->itemRepository().findByIdentifierToken(b);
        const bool manA = ia && ia->isMan();
        const bool manB = ib && ib->isMan();
        if (manA != manB)
        {
          return manA > manB;
        }
        return false;
      });
  }

  int itemIndex = 0;
  for (const auto& token : sortedTokens)
  {
    const auto& amountPrice = forSale.at(token);
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(token.c_str());
    SendMessageW(regionForSaleList_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 1;
    const std::wstring amountStr = std::to_wstring(amountPrice.first);
    item.pszText = const_cast<LPWSTR>(amountStr.c_str());
    SendMessageW(regionForSaleList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 2;
    const std::wstring priceStr = std::to_wstring(amountPrice.second);
    item.pszText = const_cast<LPWSTR>(priceStr.c_str());
    SendMessageW(regionForSaleList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 3;
    int amountAfterCommands = amountPrice.first;
    auto afterIt = afterCommandForSale.find(token);
    if (afterIt == afterCommandForSale.end())
    {
      std::wstring tokenUpper = token;
      for (wchar_t& ch : tokenUpper)
      {
        ch = static_cast<wchar_t>(towupper(ch));
      }
      afterIt = afterCommandForSale.find(tokenUpper);
    }
    if (afterIt != afterCommandForSale.end())
    {
      amountAfterCommands = afterIt->second.first;
    }
    const std::wstring amountAfterCommandsStr = std::to_wstring(amountAfterCommands);
    item.pszText = const_cast<LPWSTR>(amountAfterCommandsStr.c_str());
    SendMessageW(regionForSaleList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    ++itemIndex;
  }
}


void MapTabContent::populateWantedList(const Region* region)
{
  if (!regionWantedList_)
  {
    return;
  }

  SendMessageW(regionWantedList_, LVM_DELETEALLITEMS, 0, 0);

  if (!region)
  {
    return;
  }

  const auto& wanted = region->getWanted();
  const auto& afterCommandWanted = region->getWantedAfterOrders();
  int itemIndex = 0;
  for (const auto& [token, amountPrice] : wanted)
  {
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(token.c_str());
    SendMessageW(regionWantedList_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 1;
    const std::wstring amountStr = std::to_wstring(amountPrice.first);
    item.pszText = const_cast<LPWSTR>(amountStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 2;
    const std::wstring priceStr = std::to_wstring(amountPrice.second);
    item.pszText = const_cast<LPWSTR>(priceStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 3;
    int amountAfterCommands = amountPrice.first;
    auto afterIt = afterCommandWanted.find(token);
    if (afterIt == afterCommandWanted.end())
    {
      std::wstring tokenUpper = token;
      for (wchar_t& ch : tokenUpper)
      {
        ch = static_cast<wchar_t>(towupper(ch));
      }
      afterIt = afterCommandWanted.find(tokenUpper);
    }
    if (afterIt != afterCommandWanted.end())
    {
      amountAfterCommands = afterIt->second.first;
    }
    const std::wstring amountAfterCommandsStr = std::to_wstring(amountAfterCommands);
    item.pszText = const_cast<LPWSTR>(amountAfterCommandsStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    ++itemIndex;
  }
}

