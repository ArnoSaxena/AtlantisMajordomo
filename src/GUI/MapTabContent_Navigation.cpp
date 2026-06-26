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
 * File: MapTabContent_Navigation.cpp
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

LRESULT CALLBACK MapTabContent::unitSearchEditSubclassProc(HWND hwnd,
                                                           UINT msg,
                                                           WPARAM wp,
                                                           LPARAM lp,
                                                           UINT_PTR subclassId,
                                                           DWORD_PTR refData)
{
  (void)subclassId;

  auto* self = reinterpret_cast<MapTabContent*>(refData);
  if (!self)
  {
    return DefSubclassProc(hwnd, msg, wp, lp);
  }

  if (msg == WM_KEYDOWN && wp == VK_RETURN)
  {
    self->searchAndSelectUnitById();
    return 0;
  }

  if (msg == WM_CHAR && wp == VK_RETURN)
  {
    return 0;
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
}


void MapTabContent::navigateToSkillList(const std::wstring& skillToken)
{
  if (skillToken.empty() || !navigationCallback_)
  {
    return;
  }
  navigationCallback_(NavigationRequest{
    NavigationTarget::Skills,
    SkillNavigationPayload{ skillToken }
  });
}


void MapTabContent::selectUnitInMap(int unitNumber)
{
  if (!appData_)
  {
    return;
  }

  const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
  if (!unit)
  {
    std::wstring message = L"Unit " + std::to_wstring(unitNumber) + L" was not found in the database.";
    MessageBoxW(
      unitSearchEdit_,
      message.c_str(),
      L"Unit Not Found",
      MB_OK | MB_ICONWARNING
    );
    return;
  }

  selectedZ_ = unit->getZCoordinate();
  hasSelectedRegion_ = true;
  selectedRegionX_ = unit->getXCoordinate();
  selectedRegionY_ = unit->getYCoordinate();

  refresh();

  const Region* region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_, selectedZ_);
  if (!region)
  {
    region = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_);
  }
  updateRegionDetailsView(region);

  if (unitsList_)
  {
    ListView_SetItemState(unitsList_, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    const int itemCount = ListView_GetItemCount(unitsList_);
    for (int row = 0; row < itemCount; ++row)
    {
      LVITEMW item {};
      item.mask = LVIF_PARAM;
      item.iItem = row;
      item.iSubItem = 0;
      if (!ListView_GetItem(unitsList_, &item))
      {
        continue;
      }

      if (static_cast<int>(item.lParam) == unitNumber)
      {
        ListView_SetItemState(unitsList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(unitsList_, row, FALSE);
        SetFocus(unitsList_);
        updateSelectedUnitFromList();
        break;
      }
    }
  }

  if (mapCanvas_)
  {
    RECT clientRect {};
    GetClientRect(mapCanvas_, &clientRect);
    const int clientWidth = clientRect.right - clientRect.left;
    const int clientHeight = clientRect.bottom - clientRect.top;
    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region)
      {
        continue;
      }

      if (visual.region->getXCoordinate() == selectedRegionX_ && visual.region->getYCoordinate() == selectedRegionY_)
      {
        const int targetX = visual.center.x - clientWidth / 2;
        const int targetY = visual.center.y - clientHeight / 2;
        const int maxScrollX = (std::max)(0, contentWidth_ - clientWidth);
        const int maxScrollY = (std::max)(0, contentHeight_ - clientHeight);
        scrollX_ = (std::max)(0, (std::min)(targetX, maxScrollX));
        scrollY_ = (std::max)(0, (std::min)(targetY, maxScrollY));
        updateMapScrollbars();
        InvalidateRect(mapCanvas_, nullptr, TRUE);
        break;
      }
    }
  }
}


bool MapTabContent::focusOriginUnitForSelectedUnitNew()
{
  if (!appData_ || !selectedUnitIsNew_ || selectedUnitNumber_ == 0)
  {
    return false;
  }

  const UnitNew* unitNew = appData_->unitNewRepository().findByNumberAndCoordinates(
    selectedUnitNumber_, selectedRegionX_, selectedRegionY_, selectedZ_);
  if (!unitNew)
  {
    return false;
  }

  const int originUnitNumber = unitNew->getOriginUnit();
  if (originUnitNumber <= 0)
  {
    return false;
  }

  selectedUnitIsNew_ = false;
  selectUnitInMap(originUnitNumber);
  if (ordersEditor_)
  {
    SetFocus(ordersEditor_);
  }
  return true;
}


void MapTabContent::searchAndSelectUnitById()
{
  if (!appData_ || !unitSearchEdit_)
  {
    return;
  }

  const std::wstring unitIdText = WinGuiUtils::getWindowText(unitSearchEdit_);
  if (unitIdText.empty())
  {
    return;
  }

  int unitNumber = 0;
  try
  {
    unitNumber = std::stoi(unitIdText);
  }
  catch (...)
  {
    return;
  }

  if (unitNumber <= 0)
  {
    return;
  }

  const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
  if (!unit)
  {
    const std::wstring message = L"Unit " + std::to_wstring(unitNumber) + L" was not found in the database.";
    MessageBoxW(
      unitSearchEdit_,
      message.c_str(),
      L"Unit Not Found",
      MB_OK | MB_ICONWARNING
    );
    return;
  }

  selectUnitInMap(unitNumber);
}


void MapTabContent::setNavigationCallback(std::function<void(const NavigationRequest&)> callback)
{
  navigationCallback_ = std::move(callback);
}

