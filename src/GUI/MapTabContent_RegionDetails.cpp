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
#include "Function/MapRegionDetailsUtils.hpp"
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
#include <richedit.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <windowsx.h>

namespace
{
void applyColorizedRange(HWND richEditHwnd,
                        const std::wstring& fullText,
                        const std::wstring& label,
                        COLORREF color)
{
  if (!richEditHwnd || fullText.empty())
  {
    return;
  }

  wchar_t className[32] {};
  if (GetClassNameW(richEditHwnd, className, static_cast<int>(std::size(className))) == 0)
  {
    return;
  }

  if (std::wstring(className) != RICHEDIT_CLASS)
  {
    return;
  }

  const std::wstring lowerText = StringUtils::toLower(fullText);
  const std::wstring lowerLabel = StringUtils::toLower(label);
  const std::size_t labelPos = lowerText.find(lowerLabel);
  if (labelPos == std::wstring::npos)
  {
    return;
  }

  const std::size_t valueStart = labelPos + label.size();
  const std::size_t lineBreakPos = fullText.find(L"\r\n", valueStart);
  const std::size_t valueEnd = lineBreakPos == std::wstring::npos ? fullText.size() : lineBreakPos;

  if (valueEnd <= valueStart)
  {
    return;
  }

  const LONG start = static_cast<LONG>(valueStart);
  const LONG end = static_cast<LONG>(valueEnd);
  SendMessageW(richEditHwnd, EM_SETSEL, start, end);

  CHARFORMAT2W charFormat {};
  charFormat.cbSize = sizeof(charFormat);
  charFormat.dwMask = CFM_COLOR;
  charFormat.crTextColor = color;
  SendMessageW(richEditHwnd, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&charFormat));
}

void applyShipWarningColors(HWND richEditHwnd, const std::wstring& fullText)
{
  if (!richEditHwnd || fullText.empty())
  {
    return;
  }

  static const std::wstring kCapacityLabel = L"Capacity: ";
  static const std::wstring kSkillLabel = L"Sailing skill: ";

  const std::wstring lowerText = StringUtils::toLower(fullText);
  const std::size_t capacityPos = lowerText.find(StringUtils::toLower(kCapacityLabel));
  const std::size_t skillPos = lowerText.find(StringUtils::toLower(kSkillLabel));

  if (capacityPos != std::wstring::npos)
  {
    const std::size_t valueStart = capacityPos + kCapacityLabel.size();
    const std::size_t lineBreakPos = fullText.find(L"\r\n", valueStart);
    const std::size_t valueEnd = lineBreakPos == std::wstring::npos ? fullText.size() : lineBreakPos;
    if (valueEnd > valueStart)
    {
      const std::wstring value = fullText.substr(valueStart, valueEnd - valueStart);
      const std::size_t slashPos = value.find(L"/");
      if (slashPos != std::wstring::npos)
      {
        const std::wstring currentLoadText = StringUtils::trimWhitespace(value.substr(slashPos + 1));
        const std::wstring capacityText = StringUtils::trimWhitespace(value.substr(0, slashPos));
        const int currentLoad = StringUtils::parseIntSafe(currentLoadText);
        const int capacity = StringUtils::parseIntSafe(capacityText);
        if (currentLoad > capacity)
        {
          applyColorizedRange(richEditHwnd, fullText, kCapacityLabel, RGB(200, 0, 0));
        }
      }
    }
  }

  if (skillPos != std::wstring::npos)
  {
    const std::size_t valueStart = skillPos + kSkillLabel.size();
    const std::size_t lineBreakPos = fullText.find(L"\r\n", valueStart);
    const std::size_t valueEnd = lineBreakPos == std::wstring::npos ? fullText.size() : lineBreakPos;
    if (valueEnd > valueStart)
    {
      const std::wstring value = fullText.substr(valueStart, valueEnd - valueStart);
      const std::size_t slashPos = value.find(L"/");
      if (slashPos != std::wstring::npos)
      {
        const std::wstring requiredText = StringUtils::trimWhitespace(value.substr(0, slashPos));
        const std::wstring currentText = StringUtils::trimWhitespace(value.substr(slashPos + 1));
        const int required = StringUtils::parseIntSafe(requiredText);
        const int current = StringUtils::parseIntSafe(currentText);
        if (current < required)
        {
          applyColorizedRange(richEditHwnd, fullText, kSkillLabel, RGB(200, 0, 0));
        }
      }
    }
  }
}
}

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

  SetWindowTextW(regionDateLabel_, AppDataUtils::buildDateLabelText(appData_).c_str());
  const std::wstring details = MapRegionDetailsUtils::buildRegionSummaryText(
    *region,
    appData_,
    selectedUnitNumber_,
    selectedZ_,
    L"\r\n");

  SetWindowTextW(regionDetailsView_, details.c_str());
  applyShipWarningColors(regionDetailsView_, details);
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

  const std::vector<MapRegionDetailsUtils::ResourceRow> rows =
    MapRegionDetailsUtils::buildResourcesRows(*region);
  int itemIndex = 0;
  for (const MapRegionDetailsUtils::ResourceRow& row : rows)
  {
    insertResourceRow(itemIndex++, row.token, row.amount, row.amountAfterOrders);
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

  const std::vector<MapRegionDetailsUtils::ForSaleRow> rows =
    MapRegionDetailsUtils::buildForSaleRows(*region, appData_);

  int itemIndex = 0;
  for (const MapRegionDetailsUtils::ForSaleRow& row : rows)
  {
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(row.token.c_str());
    SendMessageW(regionForSaleList_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 1;
    const std::wstring amountStr = std::to_wstring(row.amount);
    item.pszText = const_cast<LPWSTR>(amountStr.c_str());
    SendMessageW(regionForSaleList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 2;
    const std::wstring priceStr = std::to_wstring(row.price);
    item.pszText = const_cast<LPWSTR>(priceStr.c_str());
    SendMessageW(regionForSaleList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 3;
    const std::wstring amountAfterCommandsStr = std::to_wstring(row.amountAfterOrders);
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

  const std::vector<MapRegionDetailsUtils::WantedRow> rows =
    MapRegionDetailsUtils::buildWantedRows(*region);
  int itemIndex = 0;
  for (const MapRegionDetailsUtils::WantedRow& row : rows)
  {
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(row.token.c_str());
    SendMessageW(regionWantedList_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 1;
    const std::wstring amountStr = std::to_wstring(row.amount);
    item.pszText = const_cast<LPWSTR>(amountStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 2;
    const std::wstring priceStr = std::to_wstring(row.price);
    item.pszText = const_cast<LPWSTR>(priceStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    item.iSubItem = 3;
    const std::wstring amountAfterCommandsStr = std::to_wstring(row.amountAfterOrders);
    item.pszText = const_cast<LPWSTR>(amountAfterCommandsStr.c_str());
    SendMessageW(regionWantedList_, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    ++itemIndex;
  }
}

