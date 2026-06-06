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
 * File: MapTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/MapTabContent.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/Faction.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/Skill.hpp"
#include "Data/Structure.hpp"
#include "GUI/ControlIds.hpp"
#include "GUI/OrdersEditorUtils.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "Function/MonthUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
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

namespace
{

constexpr int kMargin = 8;
constexpr int kSplitterThickness = 8;
constexpr int kMinRightPanelWidth = 220;
constexpr int kMinLeftPanelWidth = 260;
constexpr int kMinDetailsWidth = 120;
constexpr int kMinMapWidth = 220;
constexpr int kMinTopHeight = 160;
constexpr int kMinBottomHeight = 140;
constexpr int kZContextMenuBaseId = 4600;

int resolveEffectiveStructureId(int currentStructureId, int futureStructureId)
{
  if (futureStructureId > 0)
  {
    return futureStructureId;
  }

  if (futureStructureId == 0)
  {
    return 0;
  }

  return currentStructureId;
}
constexpr wchar_t kMapCanvasClassName[] = L"WindowsAppMapCanvas";
constexpr UINT_PTR kUnitSearchEditSubclassId = 231001;
constexpr int kOrdersTabIndex = 0;
constexpr int kEventsTabIndex = 1;
constexpr int kErrorsTabIndex = 2;
constexpr int kWarningsTabIndex = 3;
constexpr UINT kSkillStudyContextCommandId = 4711;

std::wstring buildMapDateLabelText(const AppData* appData)
{
  if (!appData)
  {
    return L"Date: -";
  }

  int month = 0;
  int year = 0;

  auto adoptIfLater = [&](int candidateMonth, int candidateYear)
  {
    if (candidateMonth < 1 || candidateMonth > 12 || candidateYear <= 0)
    {
      return;
    }

    if (candidateYear > year || (candidateYear == year && candidateMonth > month))
    {
      month = candidateMonth;
      year = candidateYear;
    }
  };

  const ReportRepository& reportRepository = appData->reportRepository();
  for (std::size_t index = 0; index < reportRepository.size(); ++index)
  {
    const Report& report = reportRepository.at(index);
    adoptIfLater(report.getMonth(), report.getYear());
  }

  int latestBattleMonth = 0;
  int latestBattleYear = 0;
  if (appData->battleRepository().getLatestPeriod(latestBattleMonth, latestBattleYear))
  {
    adoptIfLater(latestBattleMonth, latestBattleYear);
  }

  if (month < 1 || month > 12 || year <= 0)
  {
    return L"Date: -";
  }

  return L"Date: " + MonthUtils::monthNumberToNameOr(month, L"Unknown") + L" " + std::to_wstring(year);
}

// TODO: move int Region class. Rename "isOcean".
bool isOceanRegionType(const std::wstring& regionType)
{
  const std::wstring lower = StringUtils::toLower(regionType);
  return lower == L"ocean";
}

bool isWestDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(direction);
  return normalized == L"w" || normalized == L"west" || normalized == L"nw" ||
        normalized == L"northwest" || normalized == L"sw" || normalized == L"southwest";
}

bool isEastDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(direction);
  return normalized == L"e" || normalized == L"east" || normalized == L"ne" ||
        normalized == L"northeast" || normalized == L"se" || normalized == L"southeast";
}

std::wstring normalizeHexDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(StringUtils::trimWhitespace(direction));
  if (normalized == L"n" || normalized == L"north")
  {
    return L"N";
  }

  if (normalized == L"ne" || normalized == L"northeast")
  {
    return L"NE";
  }

  if (normalized == L"se" || normalized == L"southeast")
  {
    return L"SE";
  }

  if (normalized == L"s" || normalized == L"south")
  {
    return L"S";
  }

  if (normalized == L"sw" || normalized == L"southwest")
  {
    return L"SW";
  }

  if (normalized == L"nw" || normalized == L"northwest")
  {
    return L"NW";
  }

  return L"";
}

std::wstring extractRoadDirectionFromStructure(const Structure& structure)
{
  return StructInfo::extractRoadDirectionFromStructureType(structure.getStructureType());
}

POINT getRoadEndpointForDirection(const std::array<POINT, 6>& polygon, const std::wstring& direction)
{
  auto midpoint = [&polygon](int firstIndex, int secondIndex)
  {
    POINT point {};
    point.x = (polygon[firstIndex].x + polygon[secondIndex].x) / 2;
    point.y = (polygon[firstIndex].y + polygon[secondIndex].y) / 2;
    return point;
  };

  if (direction == L"N")
  {
    return midpoint(1, 2);
  }
  if (direction == L"NE")
  {
    return midpoint(2, 3);
  }
  if (direction == L"SE")
  {
    return midpoint(3, 4);
  }
  if (direction == L"S")
  {
    return midpoint(4, 5);
  }
  if (direction == L"SW")
  {
    return midpoint(5, 0);
  }

  return midpoint(0, 1);
}

std::vector<std::pair<int, int>> calculateMovePathCoordinates(int startX, int startY, const std::vector<std::wstring>& directions)
{
  std::vector<std::pair<int, int>> path;
  path.push_back({startX, startY});

  int x = startX;
  int y = startY;

  for (const auto& direction : directions)
  {
    if (direction == L"N")
    {
      y -= 2;
    }
    else if (direction == L"S")
    {
      y += 2;
    }
    else if (direction == L"NE")
    {
      x += 1;
      y -= 1;
    }
    else if (direction == L"NW")
    {
      x -= 1;
      y -= 1;
    }
    else if (direction == L"SE")
    {
      x += 1;
      y += 1;
    }
    else if (direction == L"SW")
    {
      x -= 1;
      y += 1;
    }

    path.push_back({x, y});
  }

  return path;
}

std::wstring joinCommaSeparated(const std::vector<std::wstring>& values)
{
  std::wstring result;
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    if (index > 0)
    {
      result += L", ";
    }
    result += values[index];
  }
  return result;
}

// TODO: find all formatSkills functions.
//       Make sure they are the same, and 
//       Move to a central locations, to
//       remove duplicated code.
std::wstring formatSkills(const std::map<std::wstring, int>& skills)
{
  std::wstring result;
  bool first = true;
  for (const auto& [skillToken, days] : skills)
  {
    if (!first)
    {
      result += L", ";
    }
    const int level = Skill::trainingDaysToLevel(days);
    result += skillToken + L" [" + skillToken + L"] " + std::to_wstring(level) +
              L" (" + std::to_wstring(days) + L")";
    first = false;
  }
  return result;
}

std::vector<std::wstring> splitLines(const std::wstring& text)
{
  std::vector<std::wstring> lines;
  std::wstringstream stream(text);
  std::wstring line;
  while (std::getline(stream, line))
  {
    if (!line.empty() && line.back() == L'\r')
    {
      line.pop_back();
    }

    const std::size_t first = line.find_first_not_of(L" \t");
    if (first == std::wstring::npos)
    {
      continue;
    }

    const std::size_t last = line.find_last_not_of(L" \t");
    lines.push_back(line.substr(first, last - first + 1));
  }

  return lines;
}

std::vector<std::wstring> extractFormNewUnitBlock(const std::vector<std::wstring>& orders,
                                                  int formUnitNumber)
{
  std::vector<std::wstring> blockLines;
  bool insideFormBlock = false;

  for (const std::wstring& order : orders)
  {
    if (!insideFormBlock)
    {
      int parsedFormUnitNumber = 0;
      if (OrderParsingUtils::tryParseFormNewUnitLine(order, parsedFormUnitNumber) &&
          parsedFormUnitNumber == formUnitNumber)
      {
        insideFormBlock = true;
        blockLines.push_back(order);
      }
      continue;
    }

    blockLines.push_back(order);
    if (StringUtils::toUpper(StringUtils::trimWhitespace(order)) == L"END")
    {
      break;
    }
  }

  return blockLines;
}


// TODO: find all joinLines functions. 
//       Make sure they are all the same,
//       and if so, move to a central helper
//       Same todo for splitLines functions
std::wstring joinLines(const std::vector<std::wstring>& lines)
{
  std::wstring result;
  for (std::size_t index = 0; index < lines.size(); ++index)
  {
    if (index > 0)
    {
      result += L"\r\n";
    }
    result += lines[index];
  }
  result += L"\r\n";
  return result;
}

template <typename T>
T clampValue(T value, T low, T high)
{
  return (std::max)(low, (std::min)(value, high));
}

int wrapMapXCoordinate(int xCoordinate, int minX, int maxX)
{
  const int width = maxX - minX + 1;
  if (width <= 0)
  {
    return xCoordinate;
  }

  int wrappedOffset = (xCoordinate - minX) % width;
  if (wrappedOffset < 0)
  {
    wrappedOffset += width;
  }

  return minX + wrappedOffset;
}

using OrderParsingUtils::tryExtractOrderKeywordUpper;
}

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
        const int newNumber = OrdersEditorUtils::computeNextNewUnitNumber(self->appData_, x, y, z);
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

bool MapTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData, AppConfig& appConfig)
{
  appData_ = &appData;
  appConfig_ = &appConfig;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
  InitCommonControlsEx(&icc);

  WNDCLASSEXW canvasClass {};
  if (!GetClassInfoExW(instance, kMapCanvasClassName, &canvasClass))
  {
    canvasClass.cbSize = sizeof(canvasClass);
    canvasClass.style = CS_HREDRAW | CS_VREDRAW;
    canvasClass.lpfnWndProc = mapCanvasWndProc;
    canvasClass.hInstance = instance;
    canvasClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    canvasClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    canvasClass.lpszClassName = kMapCanvasClassName;
    if (!RegisterClassExW(&canvasClass))
    {
      return false;
    }
  }

  mapCanvas_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    kMapCanvasClassName,
    nullptr,
    WS_CHILD | WS_HSCROLL | WS_VSCROLL,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(GUI::ControlIds::kMapCanvas)),
    instance,
    this
  );

  if (!mapCanvas_)
  {
    return false;
  }

  unitsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitsListControlId)),
    instance,
    nullptr
  );

  if (!unitsList_)
  {
    return false;
  }

  lastWarningButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Last Warning",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    120,
    24,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLastWarningButtonId)),
    instance,
    nullptr
  );
  
  clearWarningButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Clear Warning",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    120,
    24,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kClearWarningButtonId)),
    instance,
    nullptr
  );

  nextWarningButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Next Warning",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    120,
    24,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kNextWarningButtonId)),
    instance,
    nullptr
  );

  warningsCountLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Warnings: 0",
    WS_CHILD | SS_LEFT,
    0,
    0,
    140,
    24,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitWeightLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitCapacitiesLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_OWNERDRAW,
    0,
    0,
    100,
    20,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitCapacitiesLabelControlId)),
    instance,
    nullptr
  );

  unitItemsLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Unit Items",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    16,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!unitItemsLabel_)
  {
    return false;
  }

  unitItemsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitItemsListControlId)),
    instance,
    nullptr
  );

  if (!unitItemsList_)
  {
    return false;
  }

  regionDateLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Date: -",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!regionDateLabel_)
  {
    return false;
  }

  hoverRegionLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Hover: -",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!hoverRegionLabel_)
  {
    return false;
  }

  regionDetailsView_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"No region selected",
    WS_CHILD | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
    0,
    0,
    100,
    100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!regionDetailsView_)
  {
    return false;
  }

  unitSearchLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Unit id:",
    WS_CHILD | SS_LEFT,
    0,
    0,
    60,
    24,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitSearchEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
    0,
    0,
    100,
    24,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitSearchEditControlId)),
    instance,
    nullptr
  );

  unitSearchButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Search",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    80,
    24,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitSearchButtonId)),
    instance,
    nullptr
  );

  if (!unitSearchLabel_ || !unitSearchEdit_ || !unitSearchButton_)
  {
    return false;
  }

  SetWindowSubclass(
    unitSearchEdit_,
    &MapTabContent::unitSearchEditSubclassProc,
    kUnitSearchEditSubclassId,
    reinterpret_cast<DWORD_PTR>(this)
  );

  regionResourcesList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!regionResourcesList_)
  {
    return false;
  }

  // Set up columns for resources list
  LVCOLUMNW listColumn {};
  listColumn.mask = LVCF_TEXT | LVCF_WIDTH;
  listColumn.pszText = const_cast<LPWSTR>(L"Item");
  listColumn.cx = 100;
  SendMessageW(regionResourcesList_, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&listColumn));
  
  listColumn.pszText = const_cast<LPWSTR>(L"Amount");
  listColumn.cx = 80;
  SendMessageW(regionResourcesList_, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&listColumn));

  listColumn.pszText = const_cast<LPWSTR>(L"after com.");
  listColumn.cx = 80;
  SendMessageW(regionResourcesList_, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&listColumn));

  regionResourcesLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Resources",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    16,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  regionForSaleList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!regionForSaleList_)
  {
    return false;
  }

  // Set up columns for for-sale list (Item, Amount, Price)
  listColumn.pszText = const_cast<LPWSTR>(L"Item");
  listColumn.cx = 80;
  SendMessageW(regionForSaleList_, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&listColumn));
  
  listColumn.pszText = const_cast<LPWSTR>(L"Amount");
  listColumn.cx = 70;
  SendMessageW(regionForSaleList_, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&listColumn));
  
  listColumn.pszText = const_cast<LPWSTR>(L"Price");
  listColumn.cx = 60;
  SendMessageW(regionForSaleList_, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&listColumn));

  listColumn.pszText = const_cast<LPWSTR>(L"after com.");
  listColumn.cx = 80;
  SendMessageW(regionForSaleList_, LVM_INSERTCOLUMNW, 3, reinterpret_cast<LPARAM>(&listColumn));

  regionForSaleLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"For Sale",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    16,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  regionWantedList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!regionWantedList_)
  {
    return false;
  }

  // Set up columns for wanted list (Item, Amount, Price)
  listColumn.pszText = const_cast<LPWSTR>(L"Item");
  listColumn.cx = 80;
  SendMessageW(regionWantedList_, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&listColumn));
  
  listColumn.pszText = const_cast<LPWSTR>(L"Amount");
  listColumn.cx = 70;
  SendMessageW(regionWantedList_, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&listColumn));
  
  listColumn.pszText = const_cast<LPWSTR>(L"Price");
  listColumn.cx = 60;
  SendMessageW(regionWantedList_, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&listColumn));

  listColumn.pszText = const_cast<LPWSTR>(L"after com.");
  listColumn.cx = 80;
  SendMessageW(regionWantedList_, LVM_INSERTCOLUMNW, 3, reinterpret_cast<LPARAM>(&listColumn));

  regionWantedLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Wanted",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    16,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  selectedUnitLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitCoordinatesLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitFlagsLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    40,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitWarningLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  unitSkillsLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Unit Skills",
    WS_CHILD | SS_LEFT,
    0,
    0,
    100,
    16,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!unitSkillsLabel_)
  {
    return false;
  }

  unitSkillsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!unitSkillsList_)
  {
    return false;
  }

  unitDetailsTabs_ = CreateWindowExW(
    0,
    WC_TABCONTROLW,
    L"",
    WS_CHILD | WS_CLIPSIBLINGS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitDetailsTabsControlId)),
    instance,
    nullptr
  );

  unitErrorsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitErrorsListControlId)),
    instance,
    nullptr
  );

  unitWarningsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitWarningsListControlId)),
    instance,
    nullptr
  );

  unitEventsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0,
    0,
    100,
    100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnitEventsListControlId)),
    instance,
    nullptr
  );

  if (!unitDetailsTabs_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_)
  {
    return false;
  }

  TCITEMW tabItem {};
  tabItem.mask = TCIF_TEXT;
  tabItem.pszText = const_cast<LPWSTR>(L"Orders");
  TabCtrl_InsertItem(unitDetailsTabs_, kOrdersTabIndex, &tabItem);
  tabItem.pszText = const_cast<LPWSTR>(L"Events");
  TabCtrl_InsertItem(unitDetailsTabs_, kEventsTabIndex, &tabItem);
  tabItem.pszText = const_cast<LPWSTR>(L"Errors");
  TabCtrl_InsertItem(unitDetailsTabs_, kErrorsTabIndex, &tabItem);
  tabItem.pszText = const_cast<LPWSTR>(L"Warnings");
  TabCtrl_InsertItem(unitDetailsTabs_, kWarningsTabIndex, &tabItem);
  TabCtrl_SetCurSel(unitDetailsTabs_, selectedUnitDetailsTab_);

  ordersEditor_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
    0,
    0,
    100,
    80,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  // Install subclass on the orders editor so we can show a custom context menu
  SetWindowSubclass(ordersEditor_, ordersEditorSubclassProc, 0, reinterpret_cast<DWORD_PTR>(this));

  saveOrdersButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Save Orders",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    120,
    30,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveOrdersButtonId)),
    instance,
    nullptr
  );

  checkOrdersButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Check Orders",
    WS_CHILD | BS_PUSHBUTTON,
    0,
    0,
    120,
    30,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCheckOrdersButtonId)),
    instance,
    nullptr
  );

  if (!selectedUnitLabel_ || !unitWeightLabel_ || !unitCapacitiesLabel_ || !unitCoordinatesLabel_ || !unitFlagsLabel_ || !unitWarningLabel_ || !unitItemsLabel_ || !unitSkillsLabel_ || !unitSkillsList_ || !unitDetailsTabs_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_ || !ordersEditor_ || !saveOrdersButton_ || !checkOrdersButton_ || !lastWarningButton_ || !clearWarningButton_ || !nextWarningButton_ || !warningsCountLabel_ || !unitSearchLabel_ || !unitSearchEdit_ || !unitSearchButton_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(
    unitsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP
  );

  HWND unitsTooltip = reinterpret_cast<HWND>(SendMessageW(unitsList_, LVM_GETTOOLTIPS, 0, 0));
  if (unitsTooltip)
  {
    SendMessageW(unitsTooltip, TTM_SETMAXTIPWIDTH, 0, 500);
  }

  ListView_SetExtendedListViewStyle(
    unitItemsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  ListView_SetExtendedListViewStyle(
    unitSkillsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  ListView_SetExtendedListViewStyle(
    unitErrorsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  ListView_SetExtendedListViewStyle(
    unitWarningsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  ListView_SetExtendedListViewStyle(
    unitEventsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  struct Column
  {
    const wchar_t* title;
    int width;
  };

  const Column columns[] = {
    { L"Unit Number", 100 },
    { L"Unit Name", 180 },
    { L"Faction", 90 },
    { L"Faction Name", 150 },
    { L"Structure", 120 },
    { L"Men", 90 },
    { L"Silver", 96 },
    { L"Flags", 240 },
    { L"Skills", 260 },
    { L"!", 28 },
    { L"D", 28 }
  };

  for (int index = 0; index < static_cast<int>(std::size(columns)); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(columns[index].title);
    column.cx = columns[index].width;
    column.iSubItem = index;
    ListView_InsertColumn(unitsList_, index, &column);
  }

  const Column itemColumns[] = {
    { L"Token", 70 },
    { L"Name", 180 },
    { L"Amount", 70 },
    { L"after com.", 80 }
  };

  for (int index = 0; index < static_cast<int>(std::size(itemColumns)); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(itemColumns[index].title);
    column.cx = itemColumns[index].width;
    column.iSubItem = index;
    ListView_InsertColumn(unitItemsList_, index, &column);
  }

  const Column skillColumns[] = {
    { L"Skill ID", 100 },
    { L"Level", 100 },
    { L"After com.", 100 }
  };

  for (int index = 0; index < static_cast<int>(std::size(skillColumns)); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(skillColumns[index].title);
    column.cx = skillColumns[index].width;
    column.iSubItem = index;
    ListView_InsertColumn(unitSkillsList_, index, &column);
  }

  LVCOLUMNW errorColumn {};
  errorColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  errorColumn.pszText = const_cast<LPWSTR>(L"Error");
  errorColumn.cx = 420;
  errorColumn.iSubItem = 0;
  ListView_InsertColumn(unitErrorsList_, 0, &errorColumn);

  LVCOLUMNW warningColumn {};
  warningColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  warningColumn.pszText = const_cast<LPWSTR>(L"Warning");
  warningColumn.cx = 420;
  warningColumn.iSubItem = 0;
  ListView_InsertColumn(unitWarningsList_, 0, &warningColumn);

  LVCOLUMNW eventColumn {};
  eventColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  eventColumn.pszText = const_cast<LPWSTR>(L"Event");
  eventColumn.cx = 420;
  eventColumn.iSubItem = 0;
  ListView_InsertColumn(unitEventsList_, 0, &eventColumn);

  setOrdersEditingEnabled(false);
  clearSelectedUnitDetails();

  refresh();
  return true;
}

void MapTabContent::resize(const RECT& displayRect)
{
  displayRect_ = displayRect;

  if (!mapCanvas_ || !unitsList_ || !unitWeightLabel_ || !unitCapacitiesLabel_ || !unitItemsList_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_ || !unitDetailsTabs_ || !regionDateLabel_ || !regionDetailsView_ || !unitSearchEdit_ || !unitSearchButton_ || !regionResourcesList_ || 
      !regionResourcesLabel_ || !regionForSaleList_ || !regionForSaleLabel_ || !regionWantedList_ || !regionWantedLabel_ ||
      !selectedUnitLabel_ || !unitCoordinatesLabel_ || !unitFlagsLabel_ || !unitWarningLabel_ || !unitSkillsLabel_ || !ordersEditor_ || !saveOrdersButton_ || !checkOrdersButton_)
  {
    return;
  }

  const int width = static_cast<int>(displayRect.right - displayRect.left);
  const int height = static_cast<int>(displayRect.bottom - displayRect.top);
  const int usableWidth = (std::max)(0, width - 2 * kMargin);
  const int usableHeight = (std::max)(0, height - 2 * kMargin);

  const int minLeft = (std::min)(kMinLeftPanelWidth, (std::max)(0, usableWidth - kMinRightPanelWidth - kMargin));
  const int maxLeft = (std::max)(minLeft, usableWidth - (std::min)(kMinRightPanelWidth, (std::max)(0, usableWidth - kMargin)) - kMargin);
  const int leftPanelWidth = clampValue(
    static_cast<int>(leftPanelRatio_ * static_cast<float>(usableWidth)),
    minLeft,
    maxLeft);
  const int rightPanelWidth = (std::max)(0, usableWidth - leftPanelWidth - kMargin);

  const int minTop = (std::min)(kMinTopHeight, (std::max)(0, usableHeight - kMinBottomHeight - kMargin));
  const int maxTop = (std::max)(minTop, usableHeight - (std::min)(kMinBottomHeight, (std::max)(0, usableHeight - kMargin)) - kMargin);
  const int mapHeight = clampValue(
    static_cast<int>(topPanelRatio_ * static_cast<float>(usableHeight)),
    minTop,
    maxTop);
  const int buttonRowHeight = 24;
  const int buttonRowGap = kMargin;
  const int listHeight = (std::max)(0, usableHeight - mapHeight - buttonRowGap - buttonRowHeight - kMargin);

  const int minDetails = (std::min)(kMinDetailsWidth, (std::max)(0, leftPanelWidth - kMinMapWidth - kMargin));
  const int maxDetails = (std::max)(minDetails, leftPanelWidth - (std::min)(kMinMapWidth, (std::max)(0, leftPanelWidth - kMargin)) - kMargin);
  const int detailsWidth = clampValue(
    static_cast<int>(detailsPanelRatio_ * static_cast<float>((std::max)(1, leftPanelWidth))),
    minDetails,
    maxDetails);
  const int mapWidth = (std::max)(0, leftPanelWidth - detailsWidth - kMargin);
  const int rightPanelX = displayRect.left + kMargin + leftPanelWidth + kMargin;

  SetWindowPos(
    mapCanvas_,
    HWND_TOP,
    displayRect.left + kMargin + detailsWidth + kMargin,
    displayRect.top + kMargin,
    mapWidth,
    (std::max)(0, mapHeight),
    SWP_NOACTIVATE
  );

  // Position details and lists in the left panel
  const int detailsX = displayRect.left + kMargin;
  const int detailsStartY = displayRect.top + kMargin;
  const int detailsMaxHeight = (std::max)(0, mapHeight);
  
  const int labelHeight = 16;
  const int dateLabelHeight = 20;
  const int hoverLabelHeight = 20;
  const int headerLineGap = 2;
  const int dateDetailsGap = 6;
  const int listMinHeight = 60;
  
  // Divide the available space
  const int detailsHeight = detailsMaxHeight / 3;
  int listY = detailsStartY + detailsHeight + kMargin;
  int remainingHeight = detailsMaxHeight - detailsHeight - kMargin;
  const int listItemHeight = (remainingHeight > 0) ? remainingHeight / 3 : listMinHeight;

  const int hoverLabelY = detailsStartY + dateLabelHeight + headerLineGap;
  const int headerTotalHeight = dateLabelHeight + headerLineGap + hoverLabelHeight;
  const int detailsBodyY = detailsStartY + headerTotalHeight + dateDetailsGap;
  const int detailsBodyHeight = (std::max)(0, detailsHeight - headerTotalHeight - dateDetailsGap);
  SetWindowPos(regionDetailsView_, HWND_TOP, detailsX, detailsBodyY,
               detailsWidth, detailsBodyHeight, SWP_NOACTIVATE);

  // Keep both header labels in front of the details edit control.
  SetWindowPos(regionDateLabel_, HWND_TOP, detailsX, detailsStartY,
               detailsWidth, dateLabelHeight, SWP_NOACTIVATE);
  SetWindowPos(hoverRegionLabel_, HWND_TOP, detailsX, hoverLabelY,
               detailsWidth, hoverLabelHeight, SWP_NOACTIVATE);

  SetWindowPos(
    regionResourcesLabel_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    labelHeight,
    SWP_NOACTIVATE
  );
  listY += labelHeight + 2;

  SetWindowPos(
    regionResourcesList_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    (std::max)(listMinHeight, listItemHeight - labelHeight - 2),
    SWP_NOACTIVATE
  );
  listY += (std::max)(listMinHeight, listItemHeight - labelHeight - 2) + kMargin;

  SetWindowPos(
    regionForSaleLabel_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    labelHeight,
    SWP_NOACTIVATE
  );
  listY += labelHeight + 2;

  SetWindowPos(
    regionForSaleList_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    (std::max)(listMinHeight, listItemHeight - labelHeight - 2),
    SWP_NOACTIVATE
  );
  listY += (std::max)(listMinHeight, listItemHeight - labelHeight - 2) + kMargin;

  SetWindowPos(
    regionWantedLabel_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    labelHeight,
    SWP_NOACTIVATE
  );
  listY += labelHeight + 2;

  SetWindowPos(
    regionWantedList_,
    HWND_TOP,
    detailsX,
    listY,
    detailsWidth,
    (std::max)(listMinHeight, detailsMaxHeight - (listY - detailsStartY)),
    SWP_NOACTIVATE
  );

  const int mapBottomY = displayRect.top + kMargin + mapHeight;
  const int buttonRowY = mapBottomY + buttonRowGap;
  const int bottomY = buttonRowY + buttonRowHeight + kMargin;
  const int listPanelWidth = leftPanelWidth;
  const int editorPanelX = rightPanelX;
  const int editorPanelWidth = rightPanelWidth;

  const int lineHeight = 20;
  const int checkButtonWidth = 110;
  const int checkButtonHeight = buttonRowHeight;
  const int checkButtonX = displayRect.left + kMargin;
  const int checkButtonY = buttonRowY;
  SetWindowPos(checkOrdersButton_, HWND_TOP, checkButtonX, checkButtonY,
               checkButtonWidth, checkButtonHeight, SWP_NOACTIVATE);

  const int warningButtonWidth = 100;
  const int warningButtonsGap = 6;
  const int lastWarningX = checkButtonX + checkButtonWidth + warningButtonsGap;
  SetWindowPos(lastWarningButton_, HWND_TOP, lastWarningX, buttonRowY,
               warningButtonWidth, buttonRowHeight, SWP_NOACTIVATE);

  const int clearWarningX = lastWarningX + warningButtonWidth + warningButtonsGap;
  SetWindowPos(clearWarningButton_, HWND_TOP, clearWarningX, buttonRowY,
               warningButtonWidth, buttonRowHeight, SWP_NOACTIVATE);

  const int nextWarningX = clearWarningX + warningButtonWidth + warningButtonsGap;
  SetWindowPos(nextWarningButton_, HWND_TOP, nextWarningX, buttonRowY,
               warningButtonWidth, buttonRowHeight, SWP_NOACTIVATE);

  const int searchButtonWidth = 72;
  const int searchFieldGap = 6;
  const int searchLabelWidth = 52;
  const int previousSearchEditWidth = 100;
  const int searchEditWidth = previousSearchEditWidth;
  const int searchButtonX = displayRect.left + kMargin + leftPanelWidth - searchButtonWidth -
                            (previousSearchEditWidth - searchEditWidth);
  const int searchEditX = searchButtonX - searchFieldGap - searchEditWidth;
  const int searchLabelX = searchEditX - searchFieldGap - searchLabelWidth;

  SetWindowPos(unitSearchLabel_, HWND_TOP, searchLabelX, buttonRowY + 3,
               searchLabelWidth, buttonRowHeight, SWP_NOACTIVATE);
  SetWindowPos(unitSearchEdit_, HWND_TOP, searchEditX, buttonRowY,
               searchEditWidth, buttonRowHeight, SWP_NOACTIVATE);
  SetWindowPos(unitSearchButton_, HWND_TOP, searchButtonX, buttonRowY,
               searchButtonWidth, buttonRowHeight, SWP_NOACTIVATE);

  const int warningsLabelX = nextWarningX + warningButtonWidth + 12;
  const int warningsLabelWidth = (std::max)(60, searchEditX - warningsLabelX - 8);
  SetWindowPos(warningsCountLabel_, HWND_TOP, warningsLabelX, buttonRowY + 3,
               warningsLabelWidth, buttonRowHeight, SWP_NOACTIVATE);

  SetWindowPos(unitsList_, HWND_TOP, displayRect.left + kMargin, bottomY,
              listPanelWidth, (std::max)(0, listHeight), SWP_NOACTIVATE);

  const int itemsListTop = displayRect.top + kMargin;
  const int listMargin = 2;
  
  // Layout for right panel selected unit details, items, skills, and errors
  int rightPanelY = itemsListTop;

  SetWindowPos(selectedUnitLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, lineHeight, SWP_NOACTIVATE);
  rightPanelY += lineHeight + listMargin;

  const int unitFlagsHeight = lineHeight * 2;
  SetWindowPos(unitFlagsLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, unitFlagsHeight, SWP_NOACTIVATE);
  rightPanelY += unitFlagsHeight + listMargin;

  SetWindowPos(unitWarningLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, lineHeight, SWP_NOACTIVATE);
  rightPanelY += lineHeight + listMargin;

  SetWindowPos(unitItemsLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, labelHeight, SWP_NOACTIVATE);
  rightPanelY += labelHeight + listMargin;

  // Calculate available height for items, summary labels, skills, and errors
  int availableHeight = bottomY - rightPanelY;
  const int capacitiesLabelHeight = (3 * lineHeight) + 4;
  const int summaryLabelsHeight = lineHeight + capacitiesLabelHeight + listMargin;
  const int availableForLists = (std::max)(0, availableHeight - summaryLabelsHeight - (2 * (labelHeight + listMargin)));
  int itemsListHeight = (std::max)(listMinHeight, availableForLists / 3);
  int skillsListHeight = (std::max)(listMinHeight, availableForLists / 3);

  SetWindowPos(unitItemsList_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, itemsListHeight, SWP_NOACTIVATE);
  rightPanelY += itemsListHeight + listMargin;

  SetWindowPos(unitWeightLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, lineHeight, SWP_NOACTIVATE);
  rightPanelY += lineHeight + listMargin;

  SetWindowPos(unitCapacitiesLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, capacitiesLabelHeight, SWP_NOACTIVATE);
  rightPanelY += capacitiesLabelHeight + listMargin;

  SetWindowPos(unitSkillsLabel_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, labelHeight, SWP_NOACTIVATE);
  rightPanelY += labelHeight + listMargin;

  SetWindowPos(unitSkillsList_, HWND_TOP, editorPanelX, rightPanelY,
              editorPanelWidth, skillsListHeight, SWP_NOACTIVATE);
  rightPanelY += skillsListHeight + listMargin;

  const int tabTop = rightPanelY;
  const int tabBottom = bottomY + listHeight;
  const int tabHeight = (std::max)(0, tabBottom - tabTop);
  SetWindowPos(unitDetailsTabs_, HWND_TOP, editorPanelX, tabTop,
               editorPanelWidth, tabHeight, SWP_NOACTIVATE);

  RECT tabClientRect { 0, 0, editorPanelWidth, tabHeight };
  TabCtrl_AdjustRect(unitDetailsTabs_, FALSE, &tabClientRect);
  const int tabContentX = editorPanelX + static_cast<int>(tabClientRect.left);
  const int tabContentY = tabTop + static_cast<int>(tabClientRect.top);
  const int tabWidthRaw = static_cast<int>(tabClientRect.right - tabClientRect.left);
  const int tabHeightRaw = static_cast<int>(tabClientRect.bottom - tabClientRect.top);
  const int tabContentWidth = (std::max)(0, tabWidthRaw);
  const int tabContentHeight = (std::max)(0, tabHeightRaw);
  const int tabContentPadding = 4;

  const int buttonHeight = 30;
  const int ordersButtonY = tabContentY + (std::max)(0, tabContentHeight - buttonHeight);
  const int ordersEditorHeight = (std::max)(0, tabContentHeight - buttonHeight - tabContentPadding);

  SetWindowPos(ordersEditor_, HWND_TOP,
               tabContentX,
               tabContentY,
               tabContentWidth,
               ordersEditorHeight,
               SWP_NOACTIVATE);
  SetWindowPos(saveOrdersButton_, HWND_TOP,
               tabContentX,
               ordersButtonY,
               120,
               buttonHeight,
               SWP_NOACTIVATE);

  SetWindowPos(unitErrorsList_, HWND_TOP,
               tabContentX,
               tabContentY,
               tabContentWidth,
               tabContentHeight,
               SWP_NOACTIVATE);

  SetWindowPos(unitWarningsList_, HWND_TOP,
               tabContentX,
               tabContentY,
               tabContentWidth,
               tabContentHeight,
               SWP_NOACTIVATE);

  SetWindowPos(unitEventsList_, HWND_TOP,
               tabContentX,
               tabContentY,
               tabContentWidth,
               tabContentHeight,
               SWP_NOACTIVATE);

  updateUnitDetailsTabVisibility();

  recalculateVisibleMap();
  updateMapScrollbars();
  InvalidateRect(mapCanvas_, nullptr, TRUE);
}

bool MapTabContent::handleMouseMessage(UINT msg, WPARAM wp, LPARAM lp)
{
  (void)wp;

  if (!mapCanvas_)
  {
    return false;
  }

  const POINT point {
    GET_X_LPARAM(lp),
    GET_Y_LPARAM(lp)
  };

  switch (msg)
  {
    case WM_LBUTTONDOWN:
    {
      const RECT leftRightRect = getLeftRightSplitterRect();
      const RECT detailsMapRect = getDetailsMapSplitterRect();
      const RECT topBottomRect = getTopBottomSplitterRect();

      if (PtInRect(&leftRightRect, point))
      {
        dragMode_ = DragMode::LeftRightSplit;
        SetCapture(GetParent(mapCanvas_));
        return true;
      }
      if (PtInRect(&detailsMapRect, point))
      {
        dragMode_ = DragMode::DetailsMapSplit;
        SetCapture(GetParent(mapCanvas_));
        return true;
      }
      if (PtInRect(&topBottomRect, point))
      {
        dragMode_ = DragMode::TopBottomSplit;
        SetCapture(GetParent(mapCanvas_));
        return true;
      }
      return false;
    }

    case WM_MOUSEMOVE:
    {
      const int width = (std::max)(0, static_cast<int>(displayRect_.right - displayRect_.left) - 2 * kMargin);
      const int height = (std::max)(0, static_cast<int>(displayRect_.bottom - displayRect_.top) - 2 * kMargin);
      if (dragMode_ == DragMode::LeftRightSplit)
      {
        const int minLeft = (std::min)(kMinLeftPanelWidth, (std::max)(0, width - kMinRightPanelWidth - kMargin));
        const int maxLeft = (std::max)(minLeft, width - (std::min)(kMinRightPanelWidth, (std::max)(0, width - kMargin)) - kMargin);
        int proposedLeft = point.x - (displayRect_.left + kMargin);
        proposedLeft = clampValue(proposedLeft, minLeft, maxLeft);
        if (width > 0)
        {
          leftPanelRatio_ = static_cast<float>(proposedLeft) / static_cast<float>(width);
        }
        resize(displayRect_);
        return true;
      }

      if (dragMode_ == DragMode::DetailsMapSplit)
      {
        const int minLeft = (std::min)(kMinLeftPanelWidth, (std::max)(0, width - kMinRightPanelWidth - kMargin));
        const int maxLeft = (std::max)(minLeft, width - (std::min)(kMinRightPanelWidth, (std::max)(0, width - kMargin)) - kMargin);
        const int leftPanelWidth = clampValue(
          static_cast<int>(leftPanelRatio_ * static_cast<float>(width)),
          minLeft,
          maxLeft);
        const int minDetails = (std::min)(kMinDetailsWidth, (std::max)(0, leftPanelWidth - kMinMapWidth - kMargin));
        const int maxDetails = (std::max)(minDetails, leftPanelWidth - (std::min)(kMinMapWidth, (std::max)(0, leftPanelWidth - kMargin)) - kMargin);
        int proposedDetails = point.x - (displayRect_.left + kMargin);
        proposedDetails = clampValue(proposedDetails, minDetails, maxDetails);
        if (leftPanelWidth > 0)
        {
          detailsPanelRatio_ = static_cast<float>(proposedDetails) / static_cast<float>(leftPanelWidth);
        }
        resize(displayRect_);
        return true;
      }

      if (dragMode_ == DragMode::TopBottomSplit)
      {
        const int minTop = (std::min)(kMinTopHeight, (std::max)(0, height - kMinBottomHeight - kMargin));
        const int maxTop = (std::max)(minTop, height - (std::min)(kMinBottomHeight, (std::max)(0, height - kMargin)) - kMargin);
        int proposedTop = point.y - (displayRect_.top + kMargin);
        proposedTop = clampValue(proposedTop, minTop, maxTop);
        if (height > 0)
        {
          topPanelRatio_ = static_cast<float>(proposedTop) / static_cast<float>(height);
        }
        resize(displayRect_);
        return true;
      }

      const RECT leftRightRect = getLeftRightSplitterRect();
      const RECT detailsMapRect = getDetailsMapSplitterRect();
      const RECT topBottomRect = getTopBottomSplitterRect();
      if (PtInRect(&topBottomRect, point))
      {
        SetCursor(LoadCursorW(nullptr, IDC_SIZENS));
        return true;
      }

      if (PtInRect(&leftRightRect, point) || PtInRect(&detailsMapRect, point))
      {
        SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
        return true;
      }

      return false;
    }

    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
      if (dragMode_ != DragMode::None)
      {
        dragMode_ = DragMode::None;
        if (GetCapture())
        {
          ReleaseCapture();
        }
        return true;
      }
      return false;
    }
  }

  return false;
}

RECT MapTabContent::getLeftRightSplitterRect() const
{
  RECT rect { 0, 0, 0, 0 };
  const int usableWidth = (std::max)(0, static_cast<int>(displayRect_.right - displayRect_.left) - 2 * kMargin);
  const int usableHeight = (std::max)(0, static_cast<int>(displayRect_.bottom - displayRect_.top) - 2 * kMargin);
  const int minLeft = (std::min)(kMinLeftPanelWidth, (std::max)(0, usableWidth - kMinRightPanelWidth - kMargin));
  const int maxLeft = (std::max)(minLeft, usableWidth - (std::min)(kMinRightPanelWidth, (std::max)(0, usableWidth - kMargin)) - kMargin);
  const int leftPanelWidth = clampValue(
    static_cast<int>(leftPanelRatio_ * static_cast<float>(usableWidth)),
    minLeft,
    maxLeft);
  rect.left = displayRect_.left + kMargin + leftPanelWidth - (kSplitterThickness / 2);
  rect.right = rect.left + kSplitterThickness;
  rect.top = displayRect_.top + kMargin;
  rect.bottom = rect.top + usableHeight;
  return rect;
}

RECT MapTabContent::getDetailsMapSplitterRect() const
{
  RECT rect { 0, 0, 0, 0 };
  const int usableWidth = (std::max)(0, static_cast<int>(displayRect_.right - displayRect_.left) - 2 * kMargin);
  const int usableHeight = (std::max)(0, static_cast<int>(displayRect_.bottom - displayRect_.top) - 2 * kMargin);
  const int minLeft = (std::min)(kMinLeftPanelWidth, (std::max)(0, usableWidth - kMinRightPanelWidth - kMargin));
  const int maxLeft = (std::max)(minLeft, usableWidth - (std::min)(kMinRightPanelWidth, (std::max)(0, usableWidth - kMargin)) - kMargin);
  const int leftPanelWidth = clampValue(
    static_cast<int>(leftPanelRatio_ * static_cast<float>(usableWidth)),
    minLeft,
    maxLeft);

  const int minDetails = (std::min)(kMinDetailsWidth, (std::max)(0, leftPanelWidth - kMinMapWidth - kMargin));
  const int maxDetails = (std::max)(minDetails, leftPanelWidth - (std::min)(kMinMapWidth, (std::max)(0, leftPanelWidth - kMargin)) - kMargin);
  const int detailsWidth = clampValue(
    static_cast<int>(detailsPanelRatio_ * static_cast<float>((std::max)(1, leftPanelWidth))),
    minDetails,
    maxDetails);

  const int minTop = (std::min)(kMinTopHeight, (std::max)(0, usableHeight - kMinBottomHeight - kMargin));
  const int maxTop = (std::max)(minTop, usableHeight - (std::min)(kMinBottomHeight, (std::max)(0, usableHeight - kMargin)) - kMargin);
  const int topHeight = clampValue(
    static_cast<int>(topPanelRatio_ * static_cast<float>(usableHeight)),
    minTop,
    maxTop);

  rect.left = displayRect_.left + kMargin + detailsWidth - (kSplitterThickness / 2);
  rect.right = rect.left + kSplitterThickness;
  rect.top = displayRect_.top + kMargin;
  rect.bottom = rect.top + topHeight;
  return rect;
}

RECT MapTabContent::getTopBottomSplitterRect() const
{
  RECT rect { 0, 0, 0, 0 };
  const int usableWidth = (std::max)(0, static_cast<int>(displayRect_.right - displayRect_.left) - 2 * kMargin);
  const int usableHeight = (std::max)(0, static_cast<int>(displayRect_.bottom - displayRect_.top) - 2 * kMargin);
  const int minTop = (std::min)(kMinTopHeight, (std::max)(0, usableHeight - kMinBottomHeight - kMargin));
  const int maxTop = (std::max)(minTop, usableHeight - (std::min)(kMinBottomHeight, (std::max)(0, usableHeight - kMargin)) - kMargin);
  const int topHeight = clampValue(
    static_cast<int>(topPanelRatio_ * static_cast<float>(usableHeight)),
    minTop,
    maxTop);

  rect.left = displayRect_.left + kMargin;
  rect.right = rect.left + usableWidth;
  rect.top = displayRect_.top + kMargin + topHeight - (kSplitterThickness / 2);
  rect.bottom = rect.top + kSplitterThickness;
  return rect;
}

void MapTabContent::setVisible(bool visible)
{
  if (!mapCanvas_ || !unitsList_ || !lastWarningButton_ || !clearWarningButton_ || !nextWarningButton_ || !warningsCountLabel_ || !unitWeightLabel_ || !unitCapacitiesLabel_ || !unitItemsLabel_ || !unitItemsList_ || !unitSkillsLabel_ || !unitSkillsList_ || !unitDetailsTabs_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_ || !regionDateLabel_ || !hoverRegionLabel_ || !regionDetailsView_ || !unitSearchLabel_ || !unitSearchEdit_ || !unitSearchButton_ || !regionResourcesList_ || 
      !regionResourcesLabel_ || !regionForSaleList_ || !regionForSaleLabel_ || !regionWantedList_ || !regionWantedLabel_ ||
      !selectedUnitLabel_ || !unitCoordinatesLabel_ || !unitFlagsLabel_ || !unitWarningLabel_ || !ordersEditor_ || !saveOrdersButton_ || !checkOrdersButton_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  ShowWindow(mapCanvas_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionDateLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(hoverRegionLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionDetailsView_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitSearchLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitSearchEdit_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitSearchButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionResourcesLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionResourcesList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionForSaleLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionForSaleList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionWantedLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(regionWantedList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(lastWarningButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(clearWarningButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(nextWarningButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(warningsCountLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitWeightLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitCapacitiesLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitItemsLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitItemsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitSkillsLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitSkillsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitDetailsTabs_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitErrorsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitWarningsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitEventsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(selectedUnitLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitWeightLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitCapacitiesLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitCoordinatesLabel_, SW_HIDE);
  ShowWindow(unitFlagsLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(unitWarningLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(ordersEditor_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(saveOrdersButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(checkOrdersButton_, visible ? SW_SHOW : SW_HIDE);
  if (visible)
  {
    updateUnitDetailsTabVisibility();
  }
  if (!visible)
  {
    hideHoverTooltip();
  }
}

void MapTabContent::refresh()
{
  recalculateVisibleMap();
  updateMapScrollbars();

  const Region* selectedRegion = nullptr;
  if (appData_ && hasSelectedRegion_)
  {
    selectedRegion = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!selectedRegion)
    {
      selectedRegion = appData_->regionRepository().findByCoordinates(selectedRegionX_, selectedRegionY_);
    }
  }
  updateRegionDetailsView(selectedRegion);

  populateUnitsForSelectedRegion();
  updateWarningsSummaryLabel();
  if (mapCanvas_)
  {
    InvalidateRect(mapCanvas_, nullptr, TRUE);
  }
}

void MapTabContent::commitPendingEdits()
{
  saveOrdersToSelectedUnit();
}

void MapTabContent::refreshItemsForCurrentUnit()
{
  if (!appData_ || selectedUnitNumber_ == 0)
  {
    return;
  }
  updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
}

// TODO: add z display to tab label (display "Map z:<z_coord>")
void MapTabContent::showZSelectionContextMenu(HWND ownerWindow, POINT screenPoint)
{
  if (!appData_)
  {
    return;
  }

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  if (availableZLevels_.empty())
  {
    AppendMenuW(menu, MF_STRING | MF_GRAYED, kZContextMenuBaseId, L"No Z levels");
  }
  else
  {
    for (std::size_t index = 0; index < availableZLevels_.size(); ++index)
    {
      const UINT flags = MF_STRING | (availableZLevels_[index] == selectedZ_ ? MF_CHECKED : 0);
      const UINT id = static_cast<UINT>(kZContextMenuBaseId + static_cast<int>(index));
      const std::wstring text = L"Z = " + std::to_wstring(availableZLevels_[index]);
      AppendMenuW(menu, flags, id, text.c_str());
    }
  }

  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    ownerWindow,
    nullptr
  );

  DestroyMenu(menu);

  if (selectedCommand < static_cast<UINT>(kZContextMenuBaseId))
  {
    return;
  }

  const int selectedIndex = static_cast<int>(selectedCommand) - kZContextMenuBaseId;
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(availableZLevels_.size()))
  {
    return;
  }

  selectedZ_ = availableZLevels_[static_cast<std::size_t>(selectedIndex)];
  hasSelectedRegion_ = false;
  refresh();
}

LRESULT CALLBACK MapTabContent::mapCanvasWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  if (msg == WM_NCCREATE)
  {
    const auto* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lp);
    auto* self = static_cast<MapTabContent*>(createStruct->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }

  auto* self = reinterpret_cast<MapTabContent*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self)
  {
    return DefWindowProcW(hwnd, msg, wp, lp);
  }

  return self->handleMapCanvasMessage(hwnd, msg, wp, lp);
}

LRESULT MapTabContent::handleMapCanvasMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_SIZE:
      updateMapScrollbars();
      return 0;

    case WM_HSCROLL:
    {
      SCROLLINFO info {};
      info.cbSize = sizeof(info);
      info.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_HORZ, &info);
      int nextPosition = info.nPos;

      switch (LOWORD(wp))
      {
        case SB_LINELEFT:   nextPosition -= 20; break;
        case SB_LINERIGHT:  nextPosition += 20; break;
        case SB_PAGELEFT:   nextPosition -= static_cast<int>(info.nPage); break;
        case SB_PAGERIGHT:  nextPosition += static_cast<int>(info.nPage); break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: nextPosition = HIWORD(wp); break;
        default: break;
      }

      nextPosition = (std::max)(info.nMin, (std::min)(nextPosition, info.nMax - static_cast<int>(info.nPage) + 1));
      if (nextPosition != info.nPos)
      {
        info.fMask = SIF_POS;
        info.nPos = nextPosition;
        SetScrollInfo(hwnd, SB_HORZ, &info, TRUE);
        scrollX_ = nextPosition;
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_VSCROLL:
    {
      SCROLLINFO info {};
      info.cbSize = sizeof(info);
      info.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_VERT, &info);
      int nextPosition = info.nPos;

      switch (LOWORD(wp))
      {
        case SB_LINEUP:     nextPosition -= 20; break;
        case SB_LINEDOWN:   nextPosition += 20; break;
        case SB_PAGEUP:     nextPosition -= static_cast<int>(info.nPage); break;
        case SB_PAGEDOWN:   nextPosition += static_cast<int>(info.nPage); break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: nextPosition = HIWORD(wp); break;
        default: break;
      }

      nextPosition = (std::max)(info.nMin, (std::min)(nextPosition, info.nMax - static_cast<int>(info.nPage) + 1));
      if (nextPosition != info.nPos)
      {
        info.fMask = SIF_POS;
        info.nPos = nextPosition;
        SetScrollInfo(hwnd, SB_VERT, &info, TRUE);
        scrollY_ = nextPosition;
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_LBUTTONDOWN:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };
      onMapLeftClick(cursorPoint);
      return 0;
    }

    case WM_MOUSEMOVE:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };

      if (!trackingMouseLeave_)
      {
        TRACKMOUSEEVENT trackEvent {};
        trackEvent.cbSize = sizeof(trackEvent);
        trackEvent.dwFlags = TME_LEAVE;
        trackEvent.hwndTrack = hwnd;
        if (TrackMouseEvent(&trackEvent) != FALSE)
        {
          trackingMouseLeave_ = true;
        }
      }

      const RegionVisual* region = hitTestRegion(cursorPoint);
      if (region && region->region)
      {
        updateHoverTooltip(cursorPoint, *(region->region));
      }
      else
      {
        int xCoordinate = 0;
        int yCoordinate = 0;
        if (hitTestMapCoordinate(cursorPoint, xCoordinate, yCoordinate))
        {
          hoverRegionText_ = L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
            xCoordinate,
            yCoordinate,
            selectedZ_);
          SetWindowTextW(hoverRegionLabel_, hoverRegionText_.c_str());
        }
        else
        {
          hideHoverTooltip();
        }
      }
      return 0;
    }

    case WM_MOUSELEAVE:
      trackingMouseLeave_ = false;
      hideHoverTooltip();
      return 0;

    case WM_RBUTTONDOWN:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };
      onMapRightClick(cursorPoint);
      return 0;
    }

    case WM_PAINT:
    {
      PAINTSTRUCT ps {};
      HDC hdc = BeginPaint(hwnd, &ps);
      paintMap(hdc);
      EndPaint(hwnd, &ps);
      return 0;
    }
  }

  return DefWindowProcW(hwnd, msg, wp, lp);
}

void MapTabContent::recalculateVisibleMap()
{
  visibleRegions_.clear();
  availableZLevels_.clear();
  hasMapBounds_ = false;

  if (!appData_ || !appConfig_)
  {
    return;
  }

  const auto& regionRepository = appData_->regionRepository();
  if (regionRepository.size() == 0)
  {
    contentWidth_ = 0;
    contentHeight_ = 0;
    hasSelectedRegion_ = false;
    return;
  }

  std::set<int> zSet;
  for (std::size_t index = 0; index < regionRepository.size(); ++index)
  {
    zSet.insert(regionRepository.at(index).getZCoordinate());
  }

  availableZLevels_.assign(zSet.begin(), zSet.end());
  if (availableZLevels_.empty())
  {
    selectedZ_ = 1;
  }
  else if (std::find(availableZLevels_.begin(), availableZLevels_.end(), selectedZ_) == availableZLevels_.end())
  {
    selectedZ_ = availableZLevels_.front();
  }

  std::vector<const Region*> zRegions;
  zRegions.reserve(regionRepository.size());

  int minX = 0;
  int maxX = 0;
  int minY = 0;
  int maxY = 0;
  bool firstRegion = true;

  for (std::size_t index = 0; index < regionRepository.size(); ++index)
  {
    const Region& region = regionRepository.at(index);
    if (region.getZCoordinate() != selectedZ_)
    {
      continue;
    }

    zRegions.push_back(&region);
    if (firstRegion)
    {
      minX = maxX = region.getXCoordinate();
      minY = maxY = region.getYCoordinate();
      firstRegion = false;
    }
    else
    {
      minX = (std::min)(minX, region.getXCoordinate());
      maxX = (std::max)(maxX, region.getXCoordinate());
      minY = (std::min)(minY, region.getYCoordinate());
      maxY = (std::max)(maxY, region.getYCoordinate());
    }
  }

  if (zRegions.empty())
  {
    contentWidth_ = 0;
    contentHeight_ = 0;
    hasSelectedRegion_ = false;
    return;
  }

  bool leftRolloverDiscovered = false;
  bool rightRolloverDiscovered = false;
  for (const Region* region : zRegions)
  {
    if (!region)
    {
      continue;
    }

    for (const auto& direction : region->getExitDirections())
    {
      if (isWestDirection(direction) && region->getXCoordinate() <= minX + 1)
      {
        for (const Region* candidate : zRegions)
        {
          if (!candidate)
          {
            continue;
          }
          if (candidate->getXCoordinate() >= maxX - 1 &&
              std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
          {
            leftRolloverDiscovered = true;
            break;
          }
        }
      }

      if (isEastDirection(direction) && region->getXCoordinate() >= maxX - 1)
      {
        for (const Region* candidate : zRegions)
        {
          if (!candidate)
          {
            continue;
          }
          if (candidate->getXCoordinate() <= minX + 1 &&
              std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
          {
            rightRolloverDiscovered = true;
            break;
          }
        }
      }
    }
  }

  const int leftPaddingColumns = leftRolloverDiscovered ? 0 : 3;
  const int rightPaddingColumns = rightRolloverDiscovered ? 0 : 3;

  mapMinX_ = minX;
  mapMaxX_ = maxX;
  mapMinY_ = minY;
  mapMaxY_ = maxY;
  mapLeftPaddingColumns_ = leftPaddingColumns;
  mapRightPaddingColumns_ = rightPaddingColumns;
  hasMapBounds_ = true;

  const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
  const int rowStep = hexHeight;

  int maxCenterY = 0;
  for (const Region* region : zRegions)
  {
    if (!region)
    {
      continue;
    }

    const int mapColumn = (region->getXCoordinate() - minX) + leftPaddingColumns;
    const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

    const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
    const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

    RegionVisual visual;
    visual.region = region;
    visual.center = { centerX, centerY };
    visual.polygon = buildHexagonPolygon(centerX, centerY, hexWidth);
    visibleRegions_.push_back(visual);
    maxCenterY = (std::max)(maxCenterY, centerY);
  }

  // Mirror the right-edge regions into the left padding columns so wrapped map
  // rendering shows real region tiles instead of empty placeholders.
  if (mapLeftPaddingColumns_ > 0)
  {
    for (const Region* region : zRegions)
    {
      if (!region)
      {
        continue;
      }

      const int wrappedLeftX = region->getXCoordinate() - (maxX - minX + 1);
      if (wrappedLeftX < (minX - mapLeftPaddingColumns_) || wrappedLeftX >= minX)
      {
        continue;
      }

      const int mapColumn = (wrappedLeftX - minX) + leftPaddingColumns;
      const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

      const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
      const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

      RegionVisual wrappedVisual;
      wrappedVisual.region = region;
      wrappedVisual.center = { centerX, centerY };
      wrappedVisual.polygon = buildHexagonPolygon(centerX, centerY, hexWidth);
      visibleRegions_.push_back(wrappedVisual);
      maxCenterY = (std::max)(maxCenterY, centerY);
    }
  }

  const int totalColumns = (maxX - minX + 1) + leftPaddingColumns + rightPaddingColumns;
  contentWidth_ = kMargin * 2 + (std::max)(1, totalColumns) * columnStep + hexWidth;
  contentHeight_ = kMargin * 2 + maxCenterY + hexHeight;

  if (hasSelectedRegion_)
  {
    const bool selectedStillVisible = std::any_of(
      visibleRegions_.begin(),
      visibleRegions_.end(),
      [this](const RegionVisual& visual)
      {
        return visual.region != nullptr &&
              visual.region->getXCoordinate() == selectedRegionX_ &&
              visual.region->getYCoordinate() == selectedRegionY_;
      }
    );

    if (!selectedStillVisible)
    {
      hasSelectedRegion_ = false;
    }
  }
}

void MapTabContent::updateMapScrollbars()
{
  if (!mapCanvas_)
  {
    return;
  }

  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);
  const int clientWidthRaw = static_cast<int>(clientRect.right - clientRect.left);
  const int clientHeightRaw = static_cast<int>(clientRect.bottom - clientRect.top);
  const int clientWidth = clientWidthRaw > 0 ? clientWidthRaw : 0;
  const int clientHeight = clientHeightRaw > 0 ? clientHeightRaw : 0;

  const int maxScrollX = (std::max)(0, contentWidth_ - clientWidth);
  const int maxScrollY = (std::max)(0, contentHeight_ - clientHeight);

  scrollX_ = (std::min)(scrollX_, maxScrollX);
  scrollY_ = (std::min)(scrollY_, maxScrollY);

  SCROLLINFO horizontal {};
  horizontal.cbSize = sizeof(horizontal);
  horizontal.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  horizontal.nMin = 0;
  horizontal.nMax = contentWidth_ > 0 ? contentWidth_ - 1 : 0;
  horizontal.nPage = static_cast<UINT>(clientWidth);
  horizontal.nPos = scrollX_;
  SetScrollInfo(mapCanvas_, SB_HORZ, &horizontal, TRUE);

  SCROLLINFO vertical {};
  vertical.cbSize = sizeof(vertical);
  vertical.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  vertical.nMin = 0;
  vertical.nMax = contentHeight_ > 0 ? contentHeight_ - 1 : 0;
  vertical.nPage = static_cast<UINT>(clientHeight);
  vertical.nPos = scrollY_;
  SetScrollInfo(mapCanvas_, SB_VERT, &vertical, TRUE);
}

void MapTabContent::paintMap(HDC hdc) const
{
  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);

  const int paintWidthRaw = static_cast<int>(clientRect.right - clientRect.left);
  const int paintHeightRaw = static_cast<int>(clientRect.bottom - clientRect.top);
  const int paintWidth = paintWidthRaw > 0 ? paintWidthRaw : 1;
  const int paintHeight = paintHeightRaw > 0 ? paintHeightRaw : 1;

  HDC memoryDc = CreateCompatibleDC(hdc);
  HBITMAP bitmap = CreateCompatibleBitmap(hdc, paintWidth, paintHeight);
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

  FillRect(memoryDc, &clientRect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

  HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
  HGDIOBJ oldPen = SelectObject(memoryDc, borderPen);

  if (appConfig_ && hasMapBounds_)
  {
    std::set<std::pair<int, int>> occupiedCoordinates;
    int coordinateParity = 0;
    bool hasCoordinateParity = false;
    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region)
      {
        continue;
      }

      const int regionX = visual.region->getXCoordinate();
      const int regionY = visual.region->getYCoordinate();
      occupiedCoordinates.emplace(regionX, regionY);
      if (!hasCoordinateParity)
      {
        coordinateParity = (regionX + regionY) & 1;
        hasCoordinateParity = true;
      }
    }

    const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
    const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
    HPEN emptyHexPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192)); // colour of empty hex borders
    HGDIOBJ oldEmptyPen = SelectObject(memoryDc, emptyHexPen);
    HGDIOBJ oldEmptyBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));

    for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
    {
      for (int y = mapMinY_; y <= mapMaxY_; ++y)
      {
        if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
        {
          continue;
        }

        if (occupiedCoordinates.find({ x, y }) != occupiedCoordinates.end())
        {
          continue;
        }

        const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
        const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
        const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2) - scrollX_;
        const int centerY = kMargin + static_cast<int>(std::lround(mapRow * (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)))) ) +
          ((std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0))) / 2) - scrollY_;

        const std::array<POINT, 6> emptyPolygon = buildHexagonPolygon(centerX, centerY, hexWidth);
        Polygon(memoryDc, emptyPolygon.data(), static_cast<int>(emptyPolygon.size()));
      }
    }

    SelectObject(memoryDc, oldEmptyBrush);
    SelectObject(memoryDc, oldEmptyPen);
    DeleteObject(emptyHexPen);
  }

  for (const auto& visual : visibleRegions_)
  {
    std::array<POINT, 6> translated = visual.polygon;
    for (auto& point : translated)
    {
      point.x -= scrollX_;
      point.y -= scrollY_;
    }

    HBRUSH fillBrush = CreateSolidBrush(getRegionFillColor(visual.region->getRegionType()));
    HGDIOBJ oldBrush = SelectObject(memoryDc, fillBrush);
    Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
    SelectObject(memoryDc, oldBrush);
    DeleteObject(fillBrush);

    if (appData_ && appConfig_ && visual.region)
    {
      std::set<std::wstring> availableExitDirections;
      for (const auto& exitDirection : visual.region->getExitDirections())
      {
        const std::wstring normalizedDirection = normalizeHexDirection(exitDirection);
        if (!normalizedDirection.empty())
        {
          availableExitDirections.insert(normalizedDirection);
        }
      }

      if (!availableExitDirections.empty())
      {
        std::set<std::wstring> roadDirectionsToDraw;
        const auto& structureRepository = appData_->structureRepository();
        for (std::size_t structureIndex = 0; structureIndex < structureRepository.size(); ++structureIndex)
        {
          const Structure& structure = structureRepository.at(structureIndex);
          if (structure.getXCoordinate() != visual.region->getXCoordinate() ||
              structure.getYCoordinate() != visual.region->getYCoordinate() ||
              structure.getZCoordinate() != selectedZ_)
          {
            continue;
          }

          const std::wstring roadDirection = extractRoadDirectionFromStructure(structure);
          if (roadDirection.empty() || availableExitDirections.find(roadDirection) == availableExitDirections.end())
          {
            continue;
          }

          roadDirectionsToDraw.insert(roadDirection);
        }

        if (!roadDirectionsToDraw.empty())
        {
          const std::array<int, 3> roadColor = appConfig_->getRoadColor();
          HPEN roadPen = CreatePen(
            PS_SOLID,
            4,
            RGB(std::clamp(roadColor[0], 0, 255),
                std::clamp(roadColor[1], 0, 255),
                std::clamp(roadColor[2], 0, 255)));
          HGDIOBJ oldRoadPen = SelectObject(memoryDc, roadPen);

          const int centerX = visual.center.x - scrollX_;
          const int centerY = visual.center.y - scrollY_;
          for (const auto& direction : roadDirectionsToDraw)
          {
            const POINT mapEndpoint = getRoadEndpointForDirection(visual.polygon, direction);
            MoveToEx(memoryDc, centerX, centerY, nullptr);
            LineTo(memoryDc, mapEndpoint.x - scrollX_, mapEndpoint.y - scrollY_);
          }

          SelectObject(memoryDc, oldRoadPen);
          DeleteObject(roadPen);
        }
      }
    }

    if (visual.region->getContainsSettlement())
    {
      const std::wstring settlementType = StringUtils::toLower(visual.region->getSettlementType());
      const int markerDiameter = (std::max)(4, (std::max)(12, appConfig_->getMapHexWidth()) / 4);
      const int coreDiameter = (std::max)(2, markerDiameter / 4);
      const int townRingDiameter = markerDiameter;
      const int cityInnerRingDiameter = (std::max)(coreDiameter + 3, markerDiameter * 2 / 3);
      const int cityOuterRingDiameter = markerDiameter;
      const int centerX = visual.center.x - scrollX_;
      const int centerY = visual.center.y - scrollY_;

      const COLORREF markerColor = RGB(0, 0, 0);
      HPEN markerPen = CreatePen(PS_SOLID, 2, markerColor);
      HBRUSH markerBrush = CreateSolidBrush(markerColor);
      HGDIOBJ oldMarkerPen = SelectObject(memoryDc, markerPen);
      HGDIOBJ oldMarkerBrush = SelectObject(memoryDc, markerBrush);

      if (settlementType == L"village")
      {
        // Draw village as an unfilled ring (outline only, 2 pixels wide).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        Ellipse(memoryDc,
                centerX - markerDiameter / 2,
                centerY - markerDiameter / 2,
                centerX + markerDiameter / 2,
                centerY + markerDiameter / 2);
        // Restore filled brush for subsequent settlement types.
        SelectObject(memoryDc, markerBrush);
      }
      else if (settlementType == L"town")
      {
        // Draw town with two rings: filled core + unfilled outer ring.
        // The filled core circle represents the town center (filled with current brush/pen).
        Ellipse(memoryDc,
          centerX - coreDiameter / 2,
          centerY - coreDiameter / 2,
          centerX + coreDiameter / 2,
          centerY + coreDiameter / 2);

        // Switch to NULL_BRUSH to draw subsequent circles unfilled (outline only).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        
        // Draw outer ring circle to show town's influence/extent area.
        Ellipse(memoryDc,
          centerX - townRingDiameter / 2,
          centerY - townRingDiameter / 2,
          centerX + townRingDiameter / 2,
          centerY + townRingDiameter / 2);
      }
      else if (settlementType == L"city")
      {
        // Draw city with three rings: filled core + two unfilled rings showing influence zones.
        // The filled core circle represents the city center.
        Ellipse(memoryDc,
          centerX - coreDiameter / 2,
          centerY - coreDiameter / 2,
          centerX + coreDiameter / 2,
          centerY + coreDiameter / 2);

        // Switch to NULL_BRUSH for unfilled circles.
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        
        // Draw inner ring (first influence zone).
        Ellipse(memoryDc,
          centerX - cityInnerRingDiameter / 2,
          centerY - cityInnerRingDiameter / 2,
          centerX + cityInnerRingDiameter / 2,
          centerY + cityInnerRingDiameter / 2);
        
        // Draw outer ring (second influence zone).
        Ellipse(memoryDc,
          centerX - cityOuterRingDiameter / 2,
          centerY - cityOuterRingDiameter / 2,
          centerX + cityOuterRingDiameter / 2,
          centerY + cityOuterRingDiameter / 2);
      }
      else
      {
        // Fallback for unknown settlement types: draw as an unfilled ring (like village).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        Ellipse(memoryDc,
                centerX - markerDiameter / 2,
                centerY - markerDiameter / 2,
                centerX + markerDiameter / 2,
                centerY + markerDiameter / 2);
        // Restore filled brush.
        SelectObject(memoryDc, markerBrush);
      }

      SelectObject(memoryDc, oldMarkerBrush);
      SelectObject(memoryDc, oldMarkerPen);
      DeleteObject(markerBrush);
      DeleteObject(markerPen);
    }

    if (appData_ && visual.region)
    {
      const bool hasBattle = appData_->battleRepository().hasBattleInRegionForPeriod(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        visual.region->getMonth(),
        visual.region->getYear()
      );

      if (hasBattle)
      {
        const int centerX = visual.center.x - scrollX_;
        const int centerY = visual.center.y - scrollY_;

        const int hexWidth = (std::max)(12, appConfig_ ? appConfig_->getMapHexWidth() : 12);
        const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
        const int crossSize = (std::max)(4, (((hexHeight * 2) / 5) * 2) / 3);
        const int halfCross = crossSize / 2;
        const int bottomMargin = (std::max)(4, hexHeight / 8);
        const int crossCenterY = centerY + (hexHeight / 2) - bottomMargin - halfCross;

        HPEN battlePen = CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
        HGDIOBJ oldBattlePen = SelectObject(memoryDc, battlePen);

        MoveToEx(memoryDc, centerX - halfCross, crossCenterY - halfCross, nullptr);
        LineTo(memoryDc, centerX + halfCross, crossCenterY + halfCross);
        MoveToEx(memoryDc, centerX - halfCross, crossCenterY + halfCross, nullptr);
        LineTo(memoryDc, centerX + halfCross, crossCenterY - halfCross);

        SelectObject(memoryDc, oldBattlePen);
        DeleteObject(battlePen);
      }
    }
  }

  SelectObject(memoryDc, oldPen);
  DeleteObject(borderPen);

  // Draw selected region border on top in the configured highlight colour.
  if (hasSelectedRegion_ && appConfig_)
  {
    const std::array<int, 3> selColor = appConfig_->getSelectedRegionBorderColor();
    HPEN selPen = CreatePen(PS_SOLID, 3, RGB(selColor[0], selColor[1], selColor[2]));
    HGDIOBJ oldSelPen = SelectObject(memoryDc, selPen);
    HGDIOBJ oldSelBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));

    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region ||
          visual.region->getXCoordinate() != selectedRegionX_ ||
          visual.region->getYCoordinate() != selectedRegionY_)
      {
        continue;
      }

      std::array<POINT, 6> translated = visual.polygon;
      for (auto& point : translated)
      {
        point.x -= scrollX_;
        point.y -= scrollY_;
      }
      Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
      break;
    }

    SelectObject(memoryDc, oldSelBrush);
    SelectObject(memoryDc, oldSelPen);
    DeleteObject(selPen);
  }

  // Draw move path arrows
  if (!movePathCoordinates_.empty() && movePathCoordinates_.size() > 1)
  {
    const COLORREF defaultArrowColor = movePathIsSail_ ? RGB(173, 216, 230) : RGB(144, 238, 144); // Light Blue for SAIL, Light Green for MOVE/ADVANCE
    const COLORREF arrowColor = movePathHasNegativeCapacity_ ? RGB(255, 0, 0) : defaultArrowColor;
    const COLORREF arrowBorderColor = RGB(0, 0, 0); // Black border
    
    HPEN arrowPen = CreatePen(PS_SOLID, 4, arrowColor);
    HPEN arrowBorderPen = CreatePen(PS_SOLID, 8, arrowBorderColor);
    HPEN arrowTipBorderPen = CreatePen(PS_SOLID, 2, arrowBorderColor);
    HBRUSH arrowBrush = CreateSolidBrush(arrowColor);
    
    for (size_t i = 0; i < movePathCoordinates_.size() - 1; ++i)
    {
      const int x1 = movePathCoordinates_[i].first;
      const int y1 = movePathCoordinates_[i].second;
      const int x2 = movePathCoordinates_[i + 1].first;
      const int y2 = movePathCoordinates_[i + 1].second;
      
      // Find visual centers for these coordinates
      POINT startCenter = {0, 0};
      POINT endCenter = {0, 0};
      
      for (const auto& visual : visibleRegions_)
      {
        if (visual.region->getXCoordinate() == x1 && visual.region->getYCoordinate() == y1)
        {
          startCenter = visual.center;
        }
        if (visual.region->getXCoordinate() == x2 && visual.region->getYCoordinate() == y2)
        {
          endCenter = visual.center;
        }
      }
      
      if (startCenter.x == 0 && startCenter.y == 0)
        continue;
      if (endCenter.x == 0 && endCenter.y == 0)
        continue;
      
      // Apply scroll offset
      startCenter.x -= scrollX_;
      startCenter.y -= scrollY_;
      endCenter.x -= scrollX_;
      endCenter.y -= scrollY_;

      // Keep each arrow segment at 50% of center-to-center distance.
      const double dx = endCenter.x - startCenter.x;
      const double dy = endCenter.y - startCenter.y;
      const double length = std::sqrt(dx * dx + dy * dy);

      int adjustedStartX = startCenter.x;
      int adjustedStartY = startCenter.y;
      int adjustedEndX = endCenter.x;
      int adjustedEndY = endCenter.y;

      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double shorteningAmount = (std::max)(1.0, length * 0.25);
        adjustedStartX = static_cast<int>(std::lround(startCenter.x + ux * shorteningAmount));
        adjustedStartY = static_cast<int>(std::lround(startCenter.y + uy * shorteningAmount));
        adjustedEndX = static_cast<int>(std::lround(endCenter.x - ux * shorteningAmount));
        adjustedEndY = static_cast<int>(std::lround(endCenter.y - uy * shorteningAmount));
      }
      
      // Draw border first, then the 4px arrow on top.
      int shaftEndX = adjustedEndX;
      int shaftEndY = adjustedEndY;
      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double arrowLen = 10.0;
        shaftEndX = static_cast<int>(std::lround(adjustedEndX - ux * arrowLen));
        shaftEndY = static_cast<int>(std::lround(adjustedEndY - uy * arrowLen));
      }

      HGDIOBJ oldBorderPen = SelectObject(memoryDc, arrowBorderPen);
      MoveToEx(memoryDc, adjustedStartX, adjustedStartY, nullptr);
      LineTo(memoryDc, shaftEndX, shaftEndY);
      SelectObject(memoryDc, oldBorderPen);

      HGDIOBJ oldArrowPen = SelectObject(memoryDc, arrowPen);
      MoveToEx(memoryDc, adjustedStartX, adjustedStartY, nullptr);
      LineTo(memoryDc, shaftEndX, shaftEndY);
      SelectObject(memoryDc, oldArrowPen);
      
      // Draw arrow head at end
      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double arrowLen = 10;
        const double arrowWidth = 6;
        
        // Arrow tip
        const POINT arrowTip = {adjustedEndX, adjustedEndY};
        
        // Arrow base points
        const POINT arrowBase1 = {
          static_cast<int>(adjustedEndX - ux * arrowLen + uy * arrowWidth),
          static_cast<int>(adjustedEndY - uy * arrowLen - ux * arrowWidth)
        };
        const POINT arrowBase2 = {
          static_cast<int>(adjustedEndX - ux * arrowLen - uy * arrowWidth),
          static_cast<int>(adjustedEndY - uy * arrowLen + ux * arrowWidth)
        };
        
        HGDIOBJ oldBrushArrow = SelectObject(memoryDc, arrowBrush);
        HGDIOBJ oldTipPen = SelectObject(memoryDc, arrowTipBorderPen);
        const POINT arrowPoints[] = {arrowTip, arrowBase1, arrowBase2};
        Polygon(memoryDc, arrowPoints, 3);
        SelectObject(memoryDc, oldTipPen);
        SelectObject(memoryDc, oldBrushArrow);
      }
    }
    
    DeleteObject(arrowPen);
    DeleteObject(arrowBorderPen);
    DeleteObject(arrowTipBorderPen);
    DeleteObject(arrowBrush);
  }

  // Draw structure markers as a final overlay so they remain visible above map symbols.
  if (appData_ && appConfig_)
  {
    const std::array<int, 3> markerColor = appConfig_->getStructureMarkerColor();
    const COLORREF defaultMarkerColor = RGB(
      std::clamp(markerColor[0], 0, 255),
      std::clamp(markerColor[1], 0, 255),
      std::clamp(markerColor[2], 0, 255));
    const COLORREF caravanseraiMarkerColor = RGB(255, 165, 0);
    const COLORREF shipMarkerColor = RGB(173, 216, 230);

    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region)
      {
        continue;
      }

      const auto structuresInRegion = appData_->structureRepository().findByCoordinates(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate());
      if (structuresInRegion.empty())
      {
        continue;
      }

      bool hasNonRoadNonShipStructure = false;
      bool hasShipStructure = false;
      bool hasFlyingShipStructure = false;
      for (const Structure* structure : structuresInRegion)
      {
        if (!structure)
        {
          continue;
        }

        const bool isRoadStructure = !extractRoadDirectionFromStructure(*structure).empty();
        const StructInfo* structInfo = appData_->structInfoRepository().findByType(structure->getStructureType());
        const bool isShipStructure = structInfo && structInfo->isShip();

        if (isShipStructure)
        {
          hasShipStructure = true;
          if (structure->isFlying())
          {
            hasFlyingShipStructure = true;
          }
        }

        if (!isRoadStructure && !isShipStructure)
        {
          hasNonRoadNonShipStructure = true;
        }
      }

      const bool hasCaravanserai = appData_->structureRepository().findByCoordinatesAndType(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        L"caravanserai").size() > 0;

      const bool hasShaft = appData_->structureRepository().findByCoordinatesAndType(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        L"shaft").size() > 0;

      if (!hasNonRoadNonShipStructure && !hasShipStructure)
      {
        continue;
      }

      const POINT northEndpoint = getRoadEndpointForDirection(visual.polygon, L"N");
      const POINT northWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"NW");
      const POINT borderMidpoint {
        (northEndpoint.x + northWestEndpoint.x) / 2,
        (northEndpoint.y + northWestEndpoint.y) / 2
      };
      const POINT southEndpoint = getRoadEndpointForDirection(visual.polygon, L"S");
      const POINT southWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"SW");
      const POINT borderMidpointBottomLeft {
        (southEndpoint.x + southWestEndpoint.x) / 2,
        (southEndpoint.y + southWestEndpoint.y) / 2
      };

      const POINT centerPoint { visual.center.x, visual.center.y };
      const int markerCenterX = borderMidpoint.x + (centerPoint.x - borderMidpoint.x) / 4 - scrollX_;
      const int markerCenterY = borderMidpoint.y + (centerPoint.y - borderMidpoint.y) / 4 - scrollY_;
      const int shipMarkerCenterX = borderMidpointBottomLeft.x + (centerPoint.x - borderMidpointBottomLeft.x) / 4 - scrollX_;
      const int shipMarkerCenterY = borderMidpointBottomLeft.y + (centerPoint.y - borderMidpointBottomLeft.y) / 4 - scrollY_;
      const int markerRadius = 2;

      if (hasNonRoadNonShipStructure)
      {
        const COLORREF topMarkerColor = hasCaravanserai ? caravanseraiMarkerColor : defaultMarkerColor;
        HBRUSH markerBrush = CreateSolidBrush(topMarkerColor);
        HGDIOBJ oldMarkerPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));
        HGDIOBJ oldMarkerBrush = SelectObject(memoryDc, markerBrush);
        Ellipse(memoryDc,
            markerCenterX - markerRadius,
            markerCenterY - markerRadius,
            markerCenterX + markerRadius + 1,
            markerCenterY + markerRadius + 1);
        SelectObject(memoryDc, oldMarkerBrush);
        SelectObject(memoryDc, oldMarkerPen);
        DeleteObject(markerBrush);

        if (hasShaft)
        {
          HPEN shaftBorderPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
          HGDIOBJ oldShaftPen = SelectObject(memoryDc, shaftBorderPen);
          HGDIOBJ oldShaftBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
          Ellipse(memoryDc,
              markerCenterX - markerRadius,
              markerCenterY - markerRadius,
              markerCenterX + markerRadius + 1,
              markerCenterY + markerRadius + 1);
          SelectObject(memoryDc, oldShaftBrush);
          SelectObject(memoryDc, oldShaftPen);
          DeleteObject(shaftBorderPen);
        }
      }

      if (hasShipStructure)
      {
        HBRUSH shipBrush = CreateSolidBrush(shipMarkerColor);
        HPEN shipPen = nullptr;
        HGDIOBJ oldShipPen;
        if (hasFlyingShipStructure)
        {
          shipPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
          oldShipPen = SelectObject(memoryDc, shipPen);
        }
        else
        {
          oldShipPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));
        }
        HGDIOBJ oldShipBrush = SelectObject(memoryDc, shipBrush);
        Ellipse(memoryDc,
            shipMarkerCenterX - markerRadius,
            shipMarkerCenterY - markerRadius,
            shipMarkerCenterX + markerRadius + 1,
            shipMarkerCenterY + markerRadius + 1);
        SelectObject(memoryDc, oldShipBrush);
        SelectObject(memoryDc, oldShipPen);
        DeleteObject(shipBrush);
        if (shipPen)
        {
          DeleteObject(shipPen);
        }
      }
    }
  }

  BitBlt(
    hdc,
    0,
    0,
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
    memoryDc,
    0,
    0,
    SRCCOPY
  );

  SelectObject(memoryDc, oldBitmap);
  DeleteObject(bitmap);
  DeleteDC(memoryDc);
}

const MapTabContent::RegionVisual* MapTabContent::hitTestRegion(POINT pointInMapClient) const
{
  const POINT mapPoint {
    pointInMapClient.x + scrollX_,
    pointInMapClient.y + scrollY_
  };

  for (const auto& visual : visibleRegions_)
  {
    HRGN region = CreatePolygonRgn(visual.polygon.data(), static_cast<int>(visual.polygon.size()), WINDING);
    const bool inside = region != nullptr && PtInRegion(region, mapPoint.x, mapPoint.y) != FALSE;
    if (region)
    {
      DeleteObject(region);
    }

    if (inside)
    {
      return &visual;
    }
  }

  return nullptr;
}

bool MapTabContent::hitTestMapCoordinate(POINT pointInMapClient, int& xCoordinate, int& yCoordinate) const
{
  if (!hasMapBounds_ || !appConfig_)
  {
    return false;
  }

  const POINT mapPoint {
    pointInMapClient.x + scrollX_,
    pointInMapClient.y + scrollY_
  };

  int coordinateParity = 0;
  bool hasCoordinateParity = false;
  for (const auto& visual : visibleRegions_)
  {
    if (!visual.region)
    {
      continue;
    }

    coordinateParity = (visual.region->getXCoordinate() + visual.region->getYCoordinate()) & 1;
    hasCoordinateParity = true;
    break;
  }

  const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
  const int rowStep = hexHeight;

  for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
  {
    for (int y = mapMinY_; y <= mapMaxY_; ++y)
    {
      if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
      {
        continue;
      }

      const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
      const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
      const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
      const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

      const std::array<POINT, 6> polygon = buildHexagonPolygon(centerX, centerY, hexWidth);
      HRGN region = CreatePolygonRgn(polygon.data(), static_cast<int>(polygon.size()), WINDING);
      const bool inside = region != nullptr && PtInRegion(region, mapPoint.x, mapPoint.y) != FALSE;
      if (region)
      {
        DeleteObject(region);
      }

      if (inside)
      {
        xCoordinate = wrapMapXCoordinate(x, mapMinX_, mapMaxX_);
        yCoordinate = y;
        return true;
      }
    }
  }

  return false;
}

std::array<POINT, 6> MapTabContent::buildHexagonPolygon(int centerX, int centerY, int hexWidth) const
{
  const int halfWidth = hexWidth / 2;
  const int quarterWidth = (std::max)(1, hexWidth / 4);
  const int halfHeight = (std::max)(1, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 4.0)));
  std::array<POINT, 6> points {};

  points[0] = { centerX - halfWidth, centerY };
  points[1] = { centerX - quarterWidth, centerY - halfHeight };
  points[2] = { centerX + quarterWidth, centerY - halfHeight };
  points[3] = { centerX + halfWidth, centerY };
  points[4] = { centerX + quarterWidth, centerY + halfHeight };
  points[5] = { centerX - quarterWidth, centerY + halfHeight };

  return points;
}

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
  int latestBattleMonth = 0;
  int latestBattleYear = 0;
  const bool hasLatestBattlePeriod = appData_->battleRepository().getLatestPeriod(latestBattleMonth, latestBattleYear);
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

    const std::wstring unitNumber = std::to_wstring(unit.getUnitNumber());
    std::wstring factionNumber;
    if (unit.getFactionNumber() > 0)
    {
      factionNumber = std::to_wstring(unit.getFactionNumber());
    }

    std::wstring structureDisplay;
    const int displayStructureId = resolveEffectiveStructureId(unit.getStructureId(), unit.getFutureStructureId());
    if (displayStructureId != 0)
    {
      const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
        displayStructureId,
        unit.getXCoordinate(),
        unit.getYCoordinate(),
        unit.getZCoordinate());
      if (structure)
      {
        structureDisplay = structure->getStructureType() + L" [" + std::to_wstring(displayStructureId) + L"]";
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

    const std::map<std::wstring, int>& afterCommandCounts = unit.getItemsAfterOrders();

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
    const std::wstring menText = joinCommaSeparated(menEntries);

    const auto silverCurrentIt = unit.getItems().find(L"SILV");
    const int silverCurrent = silverCurrentIt != unit.getItems().end() ? silverCurrentIt->second : 0;

    const auto silverAfterIt = afterCommandCounts.find(L"SILV");
    const int silverAfter = silverAfterIt != afterCommandCounts.end() ? silverAfterIt->second : 0;
    const std::wstring silverText = std::to_wstring(silverCurrent) + L" (" + std::to_wstring(silverAfter) + L")";

    std::wstring flags = joinCommaSeparated(unit.getFlags());
    std::wstring skills = formatSkills(unit.getSkills());
    const std::wstring warningIndicator = unit.getWarnings().empty() ? L"" : L"!";
    const bool isDamagedInLatestBattle = hasLatestBattlePeriod &&
      appData_->battleRepository().isUnitDamagedInAnyBattleForPeriod(
        unit.getUnitNumber(),
        latestBattleMonth,
        latestBattleYear
      );
    const std::wstring damagedIndicator = isDamagedInLatestBattle ? L"x" : L"";
    ListView_SetItemText(unitsList_, row, 5, const_cast<LPWSTR>(menText.c_str()));
    ListView_SetItemText(unitsList_, row, 6, const_cast<LPWSTR>(silverText.c_str()));
    ListView_SetItemText(unitsList_, row, 7, const_cast<LPWSTR>(flags.c_str()));
    ListView_SetItemText(unitsList_, row, 8, const_cast<LPWSTR>(skills.c_str()));
    ListView_SetItemText(unitsList_, row, 9, const_cast<LPWSTR>(warningIndicator.c_str()));
    ListView_SetItemText(unitsList_, row, 10, const_cast<LPWSTR>(damagedIndicator.c_str()));

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
      const int newDisplayStructureId = resolveEffectiveStructureId(unitNew.getStructureId(), unitNew.getFutureStructureId());
      if (newDisplayStructureId != 0)
      {
        const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
          newDisplayStructureId,
          unitNew.getXCoordinate(),
          unitNew.getYCoordinate(),
          unitNew.getZCoordinate());
        if (structure)
        {
          newStructureDisplay = structure->getStructureType() + L" [" + std::to_wstring(newDisplayStructureId) + L"]";
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

      const auto& newAfterCommandCounts = unitNew.getItemsAfterOrders();

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
      const std::wstring newMenText = joinCommaSeparated(newMenEntries);

      const auto newSilverCurrentIt = unitNew.getItems().find(L"SILV");
      const int newSilverCurrent = newSilverCurrentIt != unitNew.getItems().end() ? newSilverCurrentIt->second : 0;
      const auto newSilverAfterIt = newAfterCommandCounts.find(L"SILV");
      const int newSilverAfter = newSilverAfterIt != newAfterCommandCounts.end() ? newSilverAfterIt->second : 0;
      const std::wstring newSilverText = std::to_wstring(newSilverCurrent) + L" (" + std::to_wstring(newSilverAfter) + L")";

      std::wstring newFlags = joinCommaSeparated(unitNew.getFlags());
      std::wstring newSkills = formatSkills(unitNew.getSkills());
      const std::wstring newWarningIndicator = unitNew.getWarnings().empty() ? L"" : L"!";

      ListView_SetItemText(unitsList_, row, 5, const_cast<LPWSTR>(newMenText.c_str()));
      ListView_SetItemText(unitsList_, row, 6, const_cast<LPWSTR>(newSilverText.c_str()));
      ListView_SetItemText(unitsList_, row, 7, const_cast<LPWSTR>(newFlags.c_str()));
      ListView_SetItemText(unitsList_, row, 8, const_cast<LPWSTR>(newSkills.c_str()));
      ListView_SetItemText(unitsList_, row, 9, const_cast<LPWSTR>(newWarningIndicator.c_str()));

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
    std::wstring structureDisplay = structure->getStructureType() + L" [" + std::to_wstring(structure->getStructureId()) + L"]";
    if (!structure->getStructureName().empty()) {
      structureDisplay += L" - " + structure->getStructureName();
    }
    ListView_SetItemText(unitsList_, row, 4, const_cast<LPWSTR>(structureDisplay.c_str()));

    ++row;
  }
  for (const Structure* structure : regionStructures)
  {
    if (!structure)
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
    std::wstring structureDisplay = structure->getStructureType() + L" [" + std::to_wstring(structure->getStructureId()) + L"]";
    if (!structure->getStructureName().empty()) {
      structureDisplay += L" - " + structure->getStructureName();
    }
    ListView_SetItemText(unitsList_, row, 4, const_cast<LPWSTR>(structureDisplay.c_str()));

    ++row;
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

bool MapTabContent::handleNotify(const NMHDR* hdr)
{
  notifyResult_ = 0;

  if (!hdr)
  {
    return false;
  }

  if (hdr->hwndFrom == unitSkillsList_ && hdr->code == NM_CUSTOMDRAW)
  {
    auto* customDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(const_cast<NMHDR*>(hdr));
    if (!customDraw)
    {
      return false;
    }

    if (customDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
      notifyResult_ = CDRF_NOTIFYITEMDRAW;
      return true;
    }

    if (customDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
      notifyResult_ = CDRF_NOTIFYSUBITEMDRAW;
      return true;
    }

    if (customDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
    {
      const int row = static_cast<int>(customDraw->nmcd.dwItemSpec);
      LVITEMW rowItem {};
      rowItem.mask = LVIF_PARAM;
      rowItem.iItem = row;
      rowItem.iSubItem = 0;
      if (!ListView_GetItem(unitSkillsList_, &rowItem))
      {
        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      if (rowItem.lParam == 1)
      {
        customDraw->clrText = RGB(128, 128, 128);
        customDraw->clrTextBk = CLR_DEFAULT;
        notifyResult_ = CDRF_NEWFONT;
        return true;
      }

      notifyResult_ = CDRF_DODEFAULT;
      return true;
    }

    return false;
  }

  if (hdr->hwndFrom == unitSkillsList_ && hdr->code == NM_RCLICK)
  {
    if (!appData_ || !unitSkillsList_ || !ordersEditor_ || selectedUnitNumber_ == 0)
    {
      return true;
    }

    Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    if (!unit || !canEditOrdersForUnit(unit) || IsWindowEnabled(ordersEditor_) == FALSE)
    {
      return true;
    }

    POINT screenPoint {};
    const DWORD messagePos = GetMessagePos();
    screenPoint.x = GET_X_LPARAM(messagePos);
    screenPoint.y = GET_Y_LPARAM(messagePos);

    POINT clientPoint = screenPoint;
    ScreenToClient(unitSkillsList_, &clientPoint);
    LVHITTESTINFO hitInfo {};
    hitInfo.pt = clientPoint;
    const int hitRow = ListView_SubItemHitTest(unitSkillsList_, &hitInfo);
    if (hitRow < 0)
    {
      return true;
    }

    std::wstring skillToken(128, L'\0');
    LVITEMW skillItem {};
    skillItem.iSubItem = 0;
    skillItem.pszText = skillToken.data();
    skillItem.cchTextMax = static_cast<int>(skillToken.size());
    const int copiedLength = static_cast<int>(SendMessageW(
      unitSkillsList_,
      LVM_GETITEMTEXTW,
      static_cast<WPARAM>(hitRow),
      reinterpret_cast<LPARAM>(&skillItem)));
    if (copiedLength <= 0)
    {
      return true;
    }

    skillToken.resize(static_cast<std::size_t>(copiedLength));
    skillToken = StringUtils::trimWhitespace(skillToken);
    if (skillToken.empty())
    {
      return true;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu)
    {
      return true;
    }

    AppendMenuW(menu, MF_STRING, kSkillStudyContextCommandId, L"study");
    const UINT selectedCommand = TrackPopupMenu(
      menu,
      TPM_RETURNCMD | TPM_RIGHTBUTTON,
      screenPoint.x,
      screenPoint.y,
      0,
      unitSkillsList_,
      nullptr);
    DestroyMenu(menu);

    if (selectedCommand == kSkillStudyContextCommandId)
    {
      appendOrderLineToOrdersEditor(L"study " + skillToken);
    }

    return true;
  }

  if (hdr->idFrom == static_cast<UINT>(kUnitDetailsTabsControlId) && hdr->code == TCN_SELCHANGE)
  {
    if (unitDetailsTabs_)
    {
      selectedUnitDetailsTab_ = TabCtrl_GetCurSel(unitDetailsTabs_);
      if (selectedUnitDetailsTab_ < 0)
      {
        selectedUnitDetailsTab_ = kOrdersTabIndex;
      }
      updateUnitDetailsTabVisibility();
    }
    return true;
  }

  if (hdr->idFrom != static_cast<UINT>(kUnitsListControlId))
  {
    return false;
  }

  if (hdr->code == NM_CUSTOMDRAW)
  {
    auto* customDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(const_cast<NMHDR*>(hdr));
    if (!customDraw)
    {
      return false;
    }

    if (customDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
      notifyResult_ = CDRF_NOTIFYITEMDRAW;
      return true;
    }

    if (customDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
      notifyResult_ = CDRF_NOTIFYSUBITEMDRAW;
      return true;
    }

    if (customDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
    {
      constexpr int kUnitNumberSubItem = 0;
      constexpr int kUnitNameSubItem = 1;
      constexpr int kStructureSubItem = 4;
      constexpr int kWarningsSubItem = 9;
      constexpr int kDamagedSubItem = 10;

      customDraw->clrText = CLR_DEFAULT;
      customDraw->clrTextBk = CLR_DEFAULT;

      const int row = static_cast<int>(customDraw->nmcd.dwItemSpec);
      LVITEMW rowItem {};
      rowItem.mask = LVIF_PARAM;
      rowItem.iItem = row;
      rowItem.iSubItem = 0;
      if (!ListView_GetItem(unitsList_, &rowItem))
      {
        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      const int unitNumber = static_cast<int>(rowItem.lParam);
      const Unit* unit = appData_ ? appData_->unitRepository().findByNumber(unitNumber) : nullptr;

      // Highlight unit number and name cells with light green for on-guard units
      if ((customDraw->iSubItem == kUnitNumberSubItem || customDraw->iSubItem == kUnitNameSubItem) && unit && unit->isOnGuard())
      {
        customDraw->clrTextBk = RGB(144, 238, 144);  // Light green
        customDraw->clrText = RGB(0, 0, 0);
        notifyResult_ = CDRF_NEWFONT;
        return true;
      }

      if (customDraw->iSubItem == kWarningsSubItem)
      {
        if (unit && !unit->getWarnings().empty())
        {
          customDraw->clrTextBk = RGB(255, 204, 128);
          customDraw->clrText = RGB(0, 0, 0);
          notifyResult_ = CDRF_NEWFONT;
          return true;
        }

        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      if (customDraw->iSubItem == kDamagedSubItem)
      {
        int latestBattleMonth = 0;
        int latestBattleYear = 0;
        const bool isDamaged = unit && appData_ &&
          appData_->battleRepository().getLatestPeriod(latestBattleMonth, latestBattleYear) &&
          appData_->battleRepository().isUnitDamagedInAnyBattleForPeriod(unitNumber, latestBattleMonth, latestBattleYear);
        if (isDamaged)
        {
          customDraw->clrText = RGB(200, 0, 0);
          customDraw->clrTextBk = CLR_DEFAULT;
          notifyResult_ = CDRF_NEWFONT;
          return true;
        }

        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      // For columns after the structure column, reset the text color to default
      if (customDraw->iSubItem > kStructureSubItem)
      {
        customDraw->clrText = CLR_DEFAULT;
        customDraw->clrTextBk = CLR_DEFAULT;
        notifyResult_ = CDRF_NEWFONT;
        return true;
      }

      if (customDraw->iSubItem != kStructureSubItem || !appData_)
      {
        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      if (!unit || unit->getStructureId() == 0)
      {
        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
        unit->getStructureId(),
        unit->getXCoordinate(),
        unit->getYCoordinate(),
        unit->getZCoordinate());
      if (!structure || structure->getOwnerUnitId() != unitNumber)
      {
        notifyResult_ = CDRF_DODEFAULT;
        return true;
      }

      customDraw->clrText = RGB(0, 140, 0);
      customDraw->clrTextBk = CLR_DEFAULT;
      notifyResult_ = CDRF_NEWFONT;
      return true;
    }

    return false;
  }

  if (hdr->code == LVN_ITEMCHANGED)
  {
    const auto* listView = reinterpret_cast<const NMLISTVIEW*>(hdr);
    if ((listView->uChanged & LVIF_STATE) != 0 && (listView->uNewState & LVIS_SELECTED) != 0)
    {
      updateSelectedUnitFromList();
    }
    return true;
  }

  if (hdr->code == LVN_GETINFOTIPW)
  {
    auto* infoTip = reinterpret_cast<NMLVGETINFOTIPW*>(const_cast<NMHDR*>(hdr));
    if (!infoTip || !appData_ || !unitsList_ || infoTip->iItem < 0 || !infoTip->pszText || infoTip->cchTextMax <= 0)
    {
      return true;
    }

    POINT screenPos {};
    const DWORD messagePos = GetMessagePos();
    screenPos.x = GET_X_LPARAM(messagePos);
    screenPos.y = GET_Y_LPARAM(messagePos);

    POINT clientPos = screenPos;
    ScreenToClient(unitsList_, &clientPos);

    LVHITTESTINFO hitInfo {};
    hitInfo.pt = clientPos;
    ListView_SubItemHitTest(unitsList_, &hitInfo);

    constexpr int kWarningsSubItem = 6;
    if (hitInfo.iItem < 0 || hitInfo.iSubItem != kWarningsSubItem)
    {
      infoTip->pszText[0] = L'\0';
      return true;
    }

    LVITEMW rowItem {};
    rowItem.mask = LVIF_PARAM;
    rowItem.iItem = hitInfo.iItem;
    rowItem.iSubItem = 0;
    if (!ListView_GetItem(unitsList_, &rowItem))
    {
      infoTip->pszText[0] = L'\0';
      return true;
    }

    const Unit* unit = appData_->unitRepository().findByNumber(static_cast<int>(rowItem.lParam));
    if (!unit || unit->getWarnings().empty())
    {
      infoTip->pszText[0] = L'\0';
      return true;
    }

    std::wstring warningText;
    for (std::size_t index = 0; index < unit->getWarnings().size(); ++index)
    {
      if (index > 0)
      {
        warningText += L"\r\n";
      }
      warningText += unit->getWarnings()[index];
    }

    wcsncpy_s(infoTip->pszText,
              static_cast<std::size_t>(infoTip->cchTextMax),
              warningText.c_str(),
              _TRUNCATE);
    return true;
  }

  return false;
}

bool MapTabContent::handleDrawItem(const DRAWITEMSTRUCT* drawItem)
{
  if (!drawItem || drawItem->CtlID != static_cast<UINT>(kUnitCapacitiesLabelControlId) ||
      drawItem->hwndItem != unitCapacitiesLabel_)
  {
    return false;
  }

  HDC hdc = drawItem->hDC;
  RECT rc = drawItem->rcItem;

  FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
  SetBkMode(hdc, TRANSPARENT);

  if (hasCapacityValues_)
  {
    struct TextPart
    {
      std::wstring text;
      COLORREF color;
    };

    const COLORREF normalColor = RGB(0, 0, 0);
    const COLORREF zeroColor = RGB(200, 0, 0);

    std::vector<TextPart> parts;
    parts.push_back({ L"Walk: ", normalColor });
    parts.push_back({ std::to_wstring(capacityWalkDisplay_), capacityWalkDisplay_ < 0 ? zeroColor : normalColor });

    if (showRideCapacity_)
    {
      parts.push_back({ L" Ride: ", normalColor });
      parts.push_back({ std::to_wstring(capacityRideDisplay_), capacityRideDisplay_ < 0 ? zeroColor : normalColor });
    }

    if (showFlyCapacity_)
    {
      parts.push_back({ L" Fly: ", normalColor });
      parts.push_back({ std::to_wstring(capacityFlyDisplay_), capacityFlyDisplay_ < 0 ? zeroColor : normalColor });
    }

    if (showSwimCapacity_)
    {
      parts.push_back({ L" Swim: ", normalColor });
      parts.push_back({ std::to_wstring(capacitySwimDisplay_), capacitySwimDisplay_ < 0 ? zeroColor : normalColor });
    }

    TEXTMETRICW tm {};
    GetTextMetricsW(hdc, &tm);
    int x = rc.left + 2;
    const int firstLineY = rc.top + 2;

    for (const TextPart& part : parts)
    {
      SetTextColor(hdc, part.color);
      TextOutW(hdc, x, firstLineY, part.text.c_str(), static_cast<int>(part.text.size()));

      SIZE textSize {};
      GetTextExtentPoint32W(hdc, part.text.c_str(), static_cast<int>(part.text.size()), &textSize);
      x += textSize.cx;
    }

    if (hasShipCapacityValues_)
    {
      std::vector<TextPart> shipParts;
      const std::wstring shipLabel = shipIsFlying_ ? L"Flying ship capacity: " : L"Ship capacity: ";
      shipParts.push_back({ shipLabel, normalColor });
      shipParts.push_back({ std::to_wstring(shipCapacityDisplay_), normalColor });
      shipParts.push_back({ L" free: ", normalColor });
      shipParts.push_back({ std::to_wstring(shipFreeCapacityDisplay_), shipFreeCapacityDisplay_ < 0 ? zeroColor : normalColor });

      x = rc.left + 2;
      const int secondLineY = firstLineY + tm.tmHeight + 2;
      for (const TextPart& part : shipParts)
      {
        SetTextColor(hdc, part.color);
        TextOutW(hdc, x, secondLineY, part.text.c_str(), static_cast<int>(part.text.size()));

        SIZE textSize {};
        GetTextExtentPoint32W(hdc, part.text.c_str(), static_cast<int>(part.text.size()), &textSize);
        x += textSize.cx;
      }

      if (hasShipOwnerSkillValues_)
      {
        std::vector<TextPart> skillParts;
        skillParts.push_back({ L"Skill need: ", normalColor });
        skillParts.push_back({ std::to_wstring(shipSkillNeedDisplay_), normalColor });
        skillParts.push_back({ L" Have: ", normalColor });
        skillParts.push_back({ std::to_wstring(shipOwnerSailingDisplay_),
                               shipOwnerSailingDisplay_ < shipSkillNeedDisplay_ ? zeroColor : normalColor });

        x = rc.left + 2;
        const int thirdLineY = secondLineY + tm.tmHeight + 2;
        for (const TextPart& part : skillParts)
        {
          SetTextColor(hdc, part.color);
          TextOutW(hdc, x, thirdLineY, part.text.c_str(), static_cast<int>(part.text.size()));

          SIZE textSize {};
          GetTextExtentPoint32W(hdc, part.text.c_str(), static_cast<int>(part.text.size()), &textSize);
          x += textSize.cx;
        }
      }
    }
  }

  HPEN separatorPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
  HGDIOBJ oldPen = SelectObject(hdc, separatorPen);
  MoveToEx(hdc, rc.left, rc.bottom - 1, nullptr);
  LineTo(hdc, rc.right, rc.bottom - 1);
  SelectObject(hdc, oldPen);
  DeleteObject(separatorPen);

  return true;
}

LRESULT MapTabContent::getNotifyResult() const
{
  return notifyResult_;
}

bool MapTabContent::handleCommand(int commandId, int /*notificationCode*/)
{
  if (commandId == kUnitSearchButtonId)
  {
    searchAndSelectUnitById();
    return true;
  }

  if (commandId == kCheckOrdersButtonId)
  {
    runOrderChecksForMainFaction();
    return true;
  }

  if (commandId == kLastWarningButtonId)
  {
    selectPreviousWarningUnit();
    return true;
  }

  if (commandId == kClearWarningButtonId)
  {
    clearWarningsForSelectedUnit();
    return true;
  }

  if (commandId == kNextWarningButtonId)
  {
    selectNextWarningUnit();
    return true;
  }

  if (commandId == kSaveOrdersButtonId)
  {
    saveOrdersToSelectedUnit();
    return true;
  }

  return false;
}

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

  const std::map<std::wstring, int>& afterCommandCounts = unit->getItemsAfterOrders();

  std::set<std::wstring> itemTokens;
  for (const auto& [itemToken, _] : unit->getItems())
  {
    itemTokens.insert(itemToken);
  }
  for (const auto& [itemToken, _] : afterCommandCounts)
  {
    itemTokens.insert(itemToken);
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
    const auto currentIt = unit->getItems().find(itemToken);
    const int amount = currentIt != unit->getItems().end() ? currentIt->second : 0;

    const auto afterIt = afterCommandCounts.find(itemToken);
    const int amountAfterCommands = afterIt != afterCommandCounts.end() ? afterIt->second : 0;

    std::wstring itemName;
    if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
    {
      itemName = item->getItemName();
    }

    std::wstring amountText = std::to_wstring(amount);
    std::wstring amountAfterCommandsText = std::to_wstring(amountAfterCommands);
    LVITEMW listItem {};
    listItem.mask = LVIF_TEXT;
    listItem.iItem = row;
    listItem.iSubItem = 0;
    listItem.pszText = const_cast<LPWSTR>(itemToken.c_str());
    ListView_InsertItem(unitItemsList_, &listItem);
    ListView_SetItemText(unitItemsList_, row, 1, const_cast<LPWSTR>(itemName.c_str()));
    ListView_SetItemText(unitItemsList_, row, 2, const_cast<LPWSTR>(amountText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 3, const_cast<LPWSTR>(amountAfterCommandsText.c_str()));
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

  const std::map<std::wstring, int>& afterCommandCounts = unitNew->getItemsAfterOrders();

  std::set<std::wstring> itemTokens;
  for (const auto& [itemToken, _] : unitNew->getItems())
  {
    itemTokens.insert(itemToken);
  }
  for (const auto& [itemToken, _] : afterCommandCounts)
  {
    itemTokens.insert(itemToken);
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
    const auto currentIt = unitNew->getItems().find(itemToken);
    const int amount = currentIt != unitNew->getItems().end() ? currentIt->second : 0;

    const auto afterIt = afterCommandCounts.find(itemToken);
    const int amountAfterCommands = afterIt != afterCommandCounts.end() ? afterIt->second : 0;

    std::wstring itemName;
    if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
    {
      itemName = item->getItemName();
    }

    std::wstring amountText = std::to_wstring(amount);
    std::wstring amountAfterCommandsText = std::to_wstring(amountAfterCommands);
    LVITEMW listItem {};
    listItem.mask = LVIF_TEXT;
    listItem.iItem = row;
    listItem.iSubItem = 0;
    listItem.pszText = const_cast<LPWSTR>(itemToken.c_str());
    ListView_InsertItem(unitItemsList_, &listItem);
    ListView_SetItemText(unitItemsList_, row, 1, const_cast<LPWSTR>(itemName.c_str()));
    ListView_SetItemText(unitItemsList_, row, 2, const_cast<LPWSTR>(amountText.c_str()));
    ListView_SetItemText(unitItemsList_, row, 3, const_cast<LPWSTR>(amountAfterCommandsText.c_str()));
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

    std::wstring details = L"Flags: " + joinCommaSeparated(unitNew->getFlags());
    SetWindowTextW(unitFlagsLabel_, details.c_str());
    if (unitWarningLabel_)
    {
      if (!unitNew->getWarnings().empty())
      {
        SetWindowTextW(unitWarningLabel_, unitNew->getWarnings().front().c_str());
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
        const std::vector<std::wstring> formBlock = extractFormNewUnitBlock(originUnit->getOrders(), unitNew->getUnitNumber());
        if (!formBlock.empty())
        {
          SetWindowTextW(ordersEditor_, joinLines(formBlock).c_str());
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

  std::wstring details = L"Flags: " + joinCommaSeparated(unit->getFlags());
  SetWindowTextW(unitFlagsLabel_, details.c_str());
  if (unitWarningLabel_)
  {
    if (!unit->getWarnings().empty())
    {
      SetWindowTextW(unitWarningLabel_, unit->getWarnings().front().c_str());
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

          const std::wstring normalized = normalizeHexDirection(upperToken);
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
          bool isValidShipOwner = false;
          bool isFlyingShip = false;
          if (unit->getStructureId() > 0)
          {
            const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
              unit->getStructureId(), unit->getXCoordinate(), unit->getYCoordinate(), unit->getZCoordinate());
            const StructInfo* structInfo = structure ? appData_->structInfoRepository().findByType(structure->getStructureType()) : nullptr;
            isValidShipOwner = structure && structInfo && structInfo->isShip() && structure->getOwnerUnitId() == unit->getUnitNumber();
            if (isValidShipOwner && structInfo)
            {
              const std::wstring& shipToken = structInfo->getItemIdentifierToken();
              const Item* shipItem = shipToken.empty()
                ? appData_->itemRepository().findByItemName(structure->getStructureName())
                : appData_->itemRepository().findByIdentifierToken(shipToken);
              isFlyingShip = shipItem && shipItem->getFlyCapacity() > 0;
            }
          }

          if (!isValidShipOwner)
          {
            break;
          }

          // Calculate path and evaluate sail-specific colour conditions.
          movePathCoordinates_ = calculateMovePathCoordinates(
            unit->getXCoordinate(),
            unit->getYCoordinate(),
            directions
          );
          movePathIsSail_ = true;

          // Check every segment: each must involve at least one ocean region.
          // Flying ships are exempt from this restriction.
          bool routeInvalid = false;
          if (!isFlyingShip)
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
              const bool startOcean = startReg && isOceanRegionType(startReg->getRegionType());
              const bool endOcean   = endReg   && isOceanRegionType(endReg->getRegionType());
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
          movePathCoordinates_ = calculateMovePathCoordinates(
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

  const std::wstring ordersText = joinLines(unit->getOrders());
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

  // Calculate total weight and capacities
  int walkCapacity = 0;
  int rideCapacity = 0;
  int flyCapacity = 0;
  int swimCapacity = 0;
  bool hasRideCapacitySource = false;
  bool hasFlyCapacitySource = false;
  bool hasSwimCapacitySource = false;

  const auto& itemRepository = appData_->itemRepository();
  const auto& itemCounts = unit->getItems();
  //const int totalWeight = calculateTotalWeight(*appData_, itemCounts);
  const int totalWeight = appData_->itemRepository().calculateTotalWeight(itemCounts);

  for (const auto& itemCountPair : itemCounts)
  {
    const std::wstring& itemToken = itemCountPair.first;
    const int count = itemCountPair.second;

    if (count <= 0)
    {
      continue;
    }

    // Find the item in the repository
    const Item* item = itemRepository.findByIdentifierToken(itemToken);
    if (!item)
    {
      continue;
    }

    const int itemWeight = item->getWeight();

    // For capacity calculation:
    // If isMan: add (walking capacity + weight) per item
    // If mount: add weight to all, and riding capacity to ride only
    if (item->isMan())
    {
      walkCapacity += (item->getWalkCapacity() + itemWeight) * count;
      rideCapacity += item->getRideCapacity() * count;
      flyCapacity += (item->getFlyCapacity() + itemWeight) * count;
      swimCapacity += (item->getSwimCapacity() + itemWeight) * count;
      hasRideCapacitySource = hasRideCapacitySource || item->getRideCapacity() != 0;
      hasFlyCapacitySource = hasFlyCapacitySource || item->getFlyCapacity() != 0;
      hasSwimCapacitySource = hasSwimCapacitySource || item->getSwimCapacity() != 0;
    }
    else if (item->isMount())
    {
      // Mounts add weight to all capacities but only riding to ride capacity
      walkCapacity += itemWeight * count;
      rideCapacity += (item->getRideCapacity() + itemWeight) * count;
      flyCapacity += itemWeight * count;
      swimCapacity += itemWeight * count;
      hasRideCapacitySource = hasRideCapacitySource || item->getRideCapacity() != 0;
      hasFlyCapacitySource = hasFlyCapacitySource || item->getFlyCapacity() != 0;
      hasSwimCapacitySource = hasSwimCapacitySource || item->getSwimCapacity() != 0;
    }
  }

  // Subtract total weight from all capacities
  walkCapacity -= totalWeight;
  rideCapacity -= totalWeight;
  flyCapacity -= totalWeight;
  swimCapacity -= totalWeight;

  capacityWalkDisplay_ = walkCapacity;
  capacityRideDisplay_ = rideCapacity;
  capacityFlyDisplay_ = flyCapacity;
  capacitySwimDisplay_ = swimCapacity;
  showRideCapacity_ = hasRideCapacitySource;
  showFlyCapacity_ = hasFlyCapacitySource;
  showSwimCapacity_ = hasSwimCapacitySource;
  hasCapacityValues_ = true;
  shipCapacityDisplay_ = 0;
  shipFreeCapacityDisplay_ = 0;
  shipSkillNeedDisplay_ = 0;
  shipOwnerSailingDisplay_ = 0;
  hasShipCapacityValues_ = false;
  hasShipOwnerSkillValues_ = false;
  shipIsFlying_ = false;

  if (unit->getStructureId() > 0)
  {
    const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
      unit->getStructureId(), unit->getXCoordinate(), unit->getYCoordinate(), unit->getZCoordinate());
    const StructInfo* structInfo = structure ? appData_->structInfoRepository().findByType(structure->getStructureType()) : nullptr;
    if (structure && structInfo && structInfo->isShip())
    {
      const bool structureIsFlying = structure->isFlying();
      shipIsFlying_ = structureIsFlying;
      const auto& fleetItems = structure->getFleetItems();
      int fleetShipCapacity = 0;
      int fleetSkillNeed = 0;
      bool hasFleetShipValues = false;

      if (!fleetItems.empty())
      {
        for (const auto& fleetItemEntry : fleetItems)
        {
          const std::wstring& itemToken = fleetItemEntry.first;
          const int amount = fleetItemEntry.second;
          if (amount <= 0)
          {
            continue;
          }

          const Item* fleetSubItem = itemRepository.findByIdentifierToken(itemToken);
          if (!fleetSubItem)
          {
            continue;
          }

          fleetShipCapacity += (structureIsFlying ? fleetSubItem->getFlyCapacity() : fleetSubItem->getSwimCapacity()) * amount;
          fleetSkillNeed += fleetSubItem->getShipSailingSkillRequired() * amount;
          hasFleetShipValues = true;
        }
      }

      const std::wstring& shipItemToken = structInfo->getItemIdentifierToken();
      const Item* shipItem = shipItemToken.empty() ? nullptr : itemRepository.findByIdentifierToken(shipItemToken);
      if (!shipItem)
      {
        // Older saved datasets may miss the ship item token; fall back to structure name.
        shipItem = itemRepository.findByItemName(structure->getStructureName());
      }

      auto determineEffectiveStructureId = [](const Unit& candidate) -> int
      {
        const int futureId = candidate.getFutureStructureId();
        if (futureId > 0)
        {
          return futureId;
        }

        if (futureId == 0)
        {
          return 0;
        }

        return candidate.getStructureId();
      };

      int combinedStructureWeight = 0;
      const UnitRepository& unitRepository = appData_->unitRepository();
      for (std::size_t index = 0; index < unitRepository.size(); ++index)
      {
        const Unit& candidate = unitRepository.at(index);
        if (candidate.getXCoordinate() != unit->getXCoordinate() ||
            candidate.getYCoordinate() != unit->getYCoordinate() ||
            candidate.getZCoordinate() != unit->getZCoordinate())
        {
          continue;
        }

        if (determineEffectiveStructureId(candidate) != unit->getStructureId())
        {
          continue;
        }

        const auto& candidateItems = !candidate.getItemsAfterOrders().empty()
          ? candidate.getItemsAfterOrders()
          : candidate.getItems();
        combinedStructureWeight += appData_->itemRepository().calculateTotalWeight(candidateItems);
      }

      const auto& unitNewRepository = appData_->unitNewRepository();
      for (std::size_t index = 0; index < unitNewRepository.size(); ++index)
      {
        const UnitNew& candidate = unitNewRepository.at(index);
        if (candidate.getXCoordinate() != unit->getXCoordinate() ||
            candidate.getYCoordinate() != unit->getYCoordinate() ||
            candidate.getZCoordinate() != unit->getZCoordinate())
        {
          continue;
        }

        const int effectiveStructureId = resolveEffectiveStructureId(candidate.getStructureId(), candidate.getFutureStructureId());
        if (effectiveStructureId != unit->getStructureId())
        {
          continue;
        }

        const auto& candidateItems = !candidate.getItemsAfterOrders().empty()
          ? candidate.getItemsAfterOrders()
          : candidate.getItems();
        combinedStructureWeight += appData_->itemRepository().calculateTotalWeight(candidateItems);
      }

      if (hasFleetShipValues)
      {
        shipCapacityDisplay_ = fleetShipCapacity;
        shipSkillNeedDisplay_ = fleetSkillNeed;
        shipFreeCapacityDisplay_ = shipCapacityDisplay_ - combinedStructureWeight;
        hasShipCapacityValues_ = true;
      }
      else if (shipItem)
      {
        shipCapacityDisplay_ = structureIsFlying ? shipItem->getFlyCapacity() : shipItem->getSwimCapacity();
        shipFreeCapacityDisplay_ = shipCapacityDisplay_ - combinedStructureWeight;
        hasShipCapacityValues_ = true;

        shipSkillNeedDisplay_ = shipItem->getShipSailingSkillRequired();
      }

      if (structure->getOwnerUnitId() == unit->getUnitNumber())
      {
        const int ownerManCount = appData_->itemRepository().calculateManItemCount(unit->getItems());
        const int ownerSailLevel = Skill::trainingDaysToLevel(unit->getSkillDays(L"SAIL"));
        shipOwnerSailingDisplay_ = ownerManCount * ownerSailLevel;
        hasShipOwnerSkillValues_ = true;
      }
    }
  }

  // Format and set the labels
  wchar_t weightText[128];
  swprintf_s(weightText, sizeof(weightText) / sizeof(weightText[0]), L"Weight: %d", totalWeight);
  SetWindowTextW(unitWeightLabel_, weightText);

  std::wstring capacitiesText = L"Walk: " + std::to_wstring(walkCapacity);
  if (showRideCapacity_)
  {
    capacitiesText += L" Ride: " + std::to_wstring(rideCapacity);
  }
  if (showFlyCapacity_)
  {
    capacitiesText += L" Fly: " + std::to_wstring(flyCapacity);
  }
  if (showSwimCapacity_)
  {
    capacitiesText += L" Swim: " + std::to_wstring(swimCapacity);
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

  int walkCapacity = 0;
  int rideCapacity = 0;
  int flyCapacity = 0;
  int swimCapacity = 0;
  bool hasRideCapacitySource = false;
  bool hasFlyCapacitySource = false;
  bool hasSwimCapacitySource = false;

  const auto& itemRepository = appData_->itemRepository();
  const auto& itemCounts = unitNew->getItems();
  const int totalWeight = itemRepository.calculateTotalWeight(itemCounts);

  for (const auto& itemCountPair : itemCounts)
  {
    const std::wstring& itemToken = itemCountPair.first;
    const int count = itemCountPair.second;

    if (count <= 0)
    {
      continue;
    }

    const Item* item = itemRepository.findByIdentifierToken(itemToken);
    if (!item)
    {
      continue;
    }

    const int itemWeight = item->getWeight();
    if (item->isMan())
    {
      walkCapacity += (item->getWalkCapacity() + itemWeight) * count;
      rideCapacity += item->getRideCapacity() * count;
      flyCapacity += (item->getFlyCapacity() + itemWeight) * count;
      swimCapacity += (item->getSwimCapacity() + itemWeight) * count;
      hasRideCapacitySource = hasRideCapacitySource || item->getRideCapacity() != 0;
      hasFlyCapacitySource = hasFlyCapacitySource || item->getFlyCapacity() != 0;
      hasSwimCapacitySource = hasSwimCapacitySource || item->getSwimCapacity() != 0;
    }
    else if (item->isMount())
    {
      walkCapacity += itemWeight * count;
      rideCapacity += (item->getRideCapacity() + itemWeight) * count;
      flyCapacity += itemWeight * count;
      swimCapacity += itemWeight * count;
      hasRideCapacitySource = hasRideCapacitySource || item->getRideCapacity() != 0;
      hasFlyCapacitySource = hasFlyCapacitySource || item->getFlyCapacity() != 0;
      hasSwimCapacitySource = hasSwimCapacitySource || item->getSwimCapacity() != 0;
    }
  }

  walkCapacity -= totalWeight;
  rideCapacity -= totalWeight;
  flyCapacity -= totalWeight;
  swimCapacity -= totalWeight;

  capacityWalkDisplay_ = walkCapacity;
  capacityRideDisplay_ = rideCapacity;
  capacityFlyDisplay_ = flyCapacity;
  capacitySwimDisplay_ = swimCapacity;
  showRideCapacity_ = hasRideCapacitySource;
  showFlyCapacity_ = hasFlyCapacitySource;
  showSwimCapacity_ = hasSwimCapacitySource;
  hasCapacityValues_ = true;
  shipCapacityDisplay_ = 0;
  shipFreeCapacityDisplay_ = 0;
  shipSkillNeedDisplay_ = 0;
  shipOwnerSailingDisplay_ = 0;
  hasShipCapacityValues_ = false;
  hasShipOwnerSkillValues_ = false;
  shipIsFlying_ = false;

  wchar_t weightText[128];
  swprintf_s(weightText, sizeof(weightText) / sizeof(weightText[0]), L"Weight: %d", totalWeight);
  SetWindowTextW(unitWeightLabel_, weightText);

  std::wstring capacitiesText = L"Walk: " + std::to_wstring(walkCapacity);
  if (showRideCapacity_)
  {
    capacitiesText += L" Ride: " + std::to_wstring(rideCapacity);
  }
  if (showFlyCapacity_)
  {
    capacitiesText += L" Fly: " + std::to_wstring(flyCapacity);
  }
  if (showSwimCapacity_)
  {
    capacitiesText += L" Swim: " + std::to_wstring(swimCapacity);
  }
  SetWindowTextW(unitCapacitiesLabel_, capacitiesText.c_str());
  InvalidateRect(unitCapacitiesLabel_, nullptr, TRUE);
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

  const int textLength = GetWindowTextLengthW(ordersEditor_);
  std::wstring buffer(static_cast<std::size_t>(textLength) + 1, L'\0');
  GetWindowTextW(ordersEditor_, buffer.data(), textLength + 1);
  const std::wstring text = buffer.c_str();

  const std::vector<std::wstring> orders = splitLines(text);
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

  const int textLength = GetWindowTextLengthW(ordersEditor_);
  std::wstring buffer(static_cast<std::size_t>(textLength) + 1, L'\0');
  GetWindowTextW(ordersEditor_, buffer.data(), textLength + 1);
  std::wstring ordersText = buffer.c_str();

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

  const int textLength = GetWindowTextLengthW(unitSearchEdit_);
  if (textLength <= 0)
  {
    return;
  }

  std::wstring buffer(static_cast<std::size_t>(textLength) + 1, L'\0');
  GetWindowTextW(unitSearchEdit_, buffer.data(), textLength + 1);
  const std::wstring unitIdText = buffer.c_str();

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

void MapTabContent::selectPreviousWarningUnit()
{
  if (!appData_)
  {
    return;
  }

  std::vector<int> warningUnits = appData_->unitRepository().findUnitNumbersWithWarnings();

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

  std::vector<int> warningUnits = appData_->unitRepository().findUnitNumbersWithWarnings();

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

void MapTabContent::onMapLeftClick(POINT pointInMapClient)
{
  const RegionVisual* region = hitTestRegion(pointInMapClient);
  if (!region || !region->region)
  {
    hasSelectedRegion_ = false;
    updateRegionDetailsView(nullptr);
    clearUnitsList();
    return;
  }

  hasSelectedRegion_ = true;
  selectedRegionX_ = region->region->getXCoordinate();
  selectedRegionY_ = region->region->getYCoordinate();
  updateRegionDetailsView(region->region);
  populateUnitsForSelectedRegion();
  InvalidateRect(mapCanvas_, nullptr, FALSE);
}

void MapTabContent::onMapRightClick(POINT pointInMapClient)
{
  const RegionVisual* region = hitTestRegion(pointInMapClient);
  if (!region || !region->region || !mapCanvas_)
  {
    return;
  }

  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);
  const int clientWidth = clientRect.right - clientRect.left;
  const int clientHeight = clientRect.bottom - clientRect.top;

  const int targetX = region->center.x - clientWidth / 2;
  const int targetY = region->center.y - clientHeight / 2;
  const int maxScrollX = (std::max)(0, contentWidth_ - clientWidth);
  const int maxScrollY = (std::max)(0, contentHeight_ - clientHeight);

  scrollX_ = (std::max)(0, (std::min)(targetX, maxScrollX));
  scrollY_ = (std::max)(0, (std::min)(targetY, maxScrollY));
  updateMapScrollbars();
  InvalidateRect(mapCanvas_, nullptr, TRUE);
}

void MapTabContent::updateHoverTooltip(POINT pointInMapClient, const Region& region)
{
  (void)pointInMapClient;
  if (!hoverRegionLabel_)
  {
    return;
  }

  hoverRegionText_ = L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate()
  );
  SetWindowTextW(hoverRegionLabel_, hoverRegionText_.c_str());
}

void MapTabContent::updateRegionDetailsView(const Region* region)
{
  if (!regionDetailsView_)
  {
    return;
  }

  if (!region)
  {
    SetWindowTextW(regionDateLabel_, buildMapDateLabelText(appData_).c_str());
    SetWindowTextW(regionDetailsView_, L"No region selected");
    populateResourcesList(nullptr);
    populateForSaleList(nullptr);
    populateWantedList(nullptr);
    return;
  }

  std::wstring details;
  SetWindowTextW(regionDateLabel_, buildMapDateLabelText(appData_).c_str());
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
      ? resolveEffectiveStructureId(selectedUnit->getStructureId(), selectedUnit->getFutureStructureId())
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
  int itemIndex = 0;
  for (const auto& [token, amountPrice] : forSale)
  {
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

void MapTabContent::hideHoverTooltip()
{
  if (!hoverRegionLabel_)
  {
    return;
  }

  SetWindowTextW(hoverRegionLabel_, L"Hover: -");
}

COLORREF MapTabContent::getRegionFillColor(const std::wstring& regionType) const
{
  if (!appConfig_)
  {
    return RGB(192, 192, 192);
  }

  const std::array<int, 3> rgb = appConfig_->getRegionColor(regionType);
  const int red = std::clamp(rgb[0], 0, 255);
  const int green = std::clamp(rgb[1], 0, 255);
  const int blue = std::clamp(rgb[2], 0, 255);
  return RGB(red, green, blue);
}
