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
 * File: MapTabContent_Orders.cpp
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

LRESULT CALLBACK MapTabContent::ordersEditorSubclassProc(HWND hwnd,
                                                          UINT msg,
                                                          WPARAM wp,
                                                          LPARAM lp,
                                                          UINT_PTR /*subclassId*/,
                                                          DWORD_PTR refData)
{
  auto* self = reinterpret_cast<MapTabContent*>(refData);
  if (!self)
  {
    return DefSubclassProc(hwnd, msg, wp, lp);
  }

  if (msg == WM_CONTEXTMENU)
  {
    const UINT selected = OrdersEditorUtils::showOrdersEditorMenu(hwnd, lp);
    switch (selected)
    {
      case OrdersEditorUtils::kOrdersUndoCmd:
        SendMessageW(hwnd, EM_UNDO, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersCutCmd:
        SendMessageW(hwnd, WM_CUT, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersCopyCmd:
        SendMessageW(hwnd, WM_COPY, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersPasteCmd:
        SendMessageW(hwnd, WM_PASTE, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersDeleteCmd:
        SendMessageW(hwnd, WM_CLEAR, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersSelectAllCmd:
      {
        const int len = GetWindowTextLengthW(hwnd);
        SendMessageW(hwnd, EM_SETSEL, 0, len);
        break;
      }
      case OrdersEditorUtils::kOrdersFormNewUnitCmd:
      {
        int x = self->selectedRegionX_;
        int y = self->selectedRegionY_;
        int z = self->selectedZ_;
        if (x == 0 && y == 0)
        {
          const Unit* unit = self->appData_ ? self->appData_->unitRepository().findByNumber(self->selectedUnitNumber_) : nullptr;
          if (unit)
          {
            x = unit->getXCoordinate();
            y = unit->getYCoordinate();
            z = unit->getZCoordinate();
          }
        }
        const int newNumber = OrderBusinessLogic::computeNextNewUnitNumber(self->appData_, x, y, z);
        OrdersEditorUtils::insertFormBlockAtEnd(hwnd, newNumber);
        break;
      }
      default:
        break;
    }

    return 0; // suppress default menu
  }

  if (msg == WM_LBUTTONDOWN && self->selectedUnitIsNew_ && hwnd == self->ordersEditor_)
  {
    if (self->focusOriginUnitForSelectedUnitNew())
    {
      return 0;
    }
  }

  if (msg == WM_SETFOCUS && self->selectedUnitIsNew_ && hwnd == self->ordersEditor_)
  {
    const LRESULT nextLineIndex = SendMessageW(hwnd, EM_LINEINDEX, 1, 0);
    if (nextLineIndex >= 0)
    {
      SendMessageW(hwnd, EM_SETSEL, static_cast<WPARAM>(nextLineIndex), static_cast<LPARAM>(nextLineIndex));
    }
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
}


void MapTabContent::saveOrdersToSelectedUnit()
{
  if (!appData_ || selectedUnitNumber_ == 0 || !ordersEditor_)
  {
    return;
  }

  Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
  if (!unit || !canEditOrdersForUnit(unit))
  {
    return;
  }

  const std::wstring text = WinGuiUtils::getWindowText(ordersEditor_);

  const std::vector<std::wstring> orders = StringUtils::splitLines(text);
  unit->setOrders(orders);

  // Keep stale OrderRepository cleanup based on current UnitNew state, then recalc
  // after UnitNew entries are rebuilt from the just-saved FORM/END blocks.
  OrderBusinessLogic::syncOrderRepositoryForSavedUnit(*appData_, selectedUnitNumber_, false);

  // Handle FORM/END blocks: extract new unit numbers and remove previous UnitNew entries
  // originating from this unit, then create new UnitNew snapshot entries.
  appData_->unitNewRepository().removeByOriginUnit(selectedUnitNumber_);

  const std::vector<int> formUnitNumbers =
    OrderParsingUtils::extractFormNewUnitNumbers(orders);

  for (int formUnitNumber : formUnitNumbers)
  {
    // Create a UnitNew snapshot for each newly formed unit.
    // The snapshot will be orderless and marked with the origin unit number.
    const int x = unit->getXCoordinate();
    const int y = unit->getYCoordinate();
    const int z = unit->getZCoordinate();
    const std::wstring formUnitName = L"New Unit";

    appData_->unitNewRepository().add(
      formUnitNumber,
      formUnitName,
      unit->getStructureId(),  // structureId - inherit from origin unit
      x, y, z,
      unit->getFlags(),  // flags
      std::map<std::wstring, int>(),  // itemCounts
      0,  // weight
      0,  // capacityWalk
      0,  // capacityRide
      0,  // capacityFly
      0,  // capacitySwim
      std::map<std::wstring, int>(),  // skills
      unit->getMonth(),  // month
      unit->getYear(),  // year
      selectedUnitNumber_  // originUnit
    );
  }

  CommandSimulationService::recalculateAfterOrdersValues(*appData_);
  OrderWarningService::runForMainFaction(*appData_);

  updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
  populateUnitsForSelectedRegion();
  updateWarningsSummaryLabel();
}


void MapTabContent::appendOrderLineToOrdersEditor(const std::wstring& orderLine)
{
  if (!ordersEditor_)
  {
    return;
  }

  const std::wstring trimmedOrderLine = StringUtils::trimWhitespace(orderLine);
  if (trimmedOrderLine.empty())
  {
    return;
  }

  std::wstring ordersText = WinGuiUtils::getWindowText(ordersEditor_);

  if (!ordersText.empty())
  {
    const wchar_t lastChar = ordersText.back();
    if (lastChar != L'\n' && lastChar != L'\r')
    {
      ordersText += L"\r\n";
    }
    else if (lastChar == L'\r')
    {
      ordersText += L"\n";
    }
  }

  ordersText += trimmedOrderLine;
  if (ordersText.empty() || ordersText.back() != L'\n')
  {
    ordersText += L"\r\n";
  }
  SetWindowTextW(ordersEditor_, ordersText.c_str());
  SendMessageW(ordersEditor_, EM_SETSEL, static_cast<WPARAM>(ordersText.size()), static_cast<LPARAM>(ordersText.size()));
  SetFocus(ordersEditor_);
}


bool MapTabContent::canEditOrdersForUnit(const Unit* unit) const
{
  if (!appData_ || !unit)
  {
    return false;
  }

  const auto& factionRepository = appData_->factionRepository();
  int mainFactionCount = 0;
  int mainFactionNumber = 0;
  for (std::size_t index = 0; index < factionRepository.size(); ++index)
  {
    const Faction& faction = factionRepository.at(index);
    if (faction.isMainFaction())
    {
      ++mainFactionCount;
      mainFactionNumber = faction.getFactionNumber();
    }
  }

  return mainFactionCount == 1 && unit->getFactionNumber() == mainFactionNumber;
}


void MapTabContent::setOrdersEditingEnabled(bool enabled)
{
  if (ordersEditor_)
  {
    // Keep the editor enabled so it can receive focus for unitNew orders,
    // but make it read-only when orders cannot be edited.
    EnableWindow(ordersEditor_, TRUE);
    SendMessageW(ordersEditor_, EM_SETREADONLY, enabled ? FALSE : TRUE, 0);
  }
  if (saveOrdersButton_)
  {
    EnableWindow(saveOrdersButton_, enabled ? TRUE : FALSE);
  }
}

