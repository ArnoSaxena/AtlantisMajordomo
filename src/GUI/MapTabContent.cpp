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
 * File: MapTabContent.cpp
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
#include "GUI/WinSizingUtils.hpp"
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
#include <cctype>
#include <cwctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <windowsx.h>

bool MapTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData, AppConfig& appConfig)
{
  appData_ = &appData;
  appConfig_ = &appConfig;

  WNDCLASSEXW canvasClass {};
  if (!GetClassInfoExW(instance, kMapCanvasClassName, &canvasClass))
  {
    canvasClass.cbSize = sizeof(canvasClass);
    canvasClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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
    { L"#", 50 },
    { L"Name", 180 },
    { L"Faction", 50 },
    { L"Faction Name", 120 },
    { L"Structure", 150 },
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
    { L"Name", 100 },
    { L"Amount", 60 },
    { L"after com.", 60 }
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
    { L"Skill ID", 60 },
    { L"Level", 60 },
    { L"After com.", 60 }
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

  applyListColumnWidths(resolveUiMetrics(), 420, 420, 220);

  refresh();
  return true;
}


void MapTabContent::setVisible(bool visible)
{
  if (!mapCanvas_ || !unitsList_ || !lastWarningButton_ || !clearWarningButton_ || !nextWarningButton_ || !warningsCountLabel_ || !unitWeightLabel_ || !unitCapacitiesLabel_ || !unitItemsLabel_ || !unitItemsList_ || !unitSkillsLabel_ || !unitSkillsList_ || !unitDetailsTabs_ || !unitErrorsList_ || !unitWarningsList_ || !unitEventsList_ || !regionDateLabel_ || !hoverRegionLabel_ || !regionDetailsView_ || !unitSearchLabel_ || !unitSearchEdit_ || !unitSearchButton_ || !regionResourcesList_ || 
      !regionResourcesLabel_ || !regionForSaleList_ || !regionForSaleLabel_ || !regionWantedList_ || !regionWantedLabel_ ||
      !selectedUnitLabel_ || !unitCoordinatesLabel_ || !unitFlagsLabel_ || !unitWarningLabel_ || !ordersEditor_ || !saveOrdersButton_ || !checkOrdersButton_)
  {
    return;
  }

  const bool wasVisible = isVisible_;
  isVisible_ = visible;

  if (visible && !wasVisible)
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

UiSizeProfile::Profile MapTabContent::resolveRequestedUiProfile() const
{
  if (appConfig_ == nullptr)
  {
    return UiSizeProfile::Profile::Auto;
  }

  std::wstring mode = appConfig_->getUiSizeMode();
  for (wchar_t& ch : mode)
  {
    ch = static_cast<wchar_t>(std::towlower(ch));
  }

  if (mode == L"compact")
  {
    return UiSizeProfile::Profile::Compact;
  }
  if (mode == L"standard")
  {
    return UiSizeProfile::Profile::Standard;
  }
  if (mode == L"large")
  {
    return UiSizeProfile::Profile::Large;
  }

  return UiSizeProfile::Profile::Auto;
}

UiSizeProfile::MapHexProfile MapTabContent::resolveRequestedMapHexProfile() const
{
  if (appConfig_ == nullptr)
  {
    return UiSizeProfile::MapHexProfile::Medium;
  }

  std::wstring mode = appConfig_->getMapHexSizeMode();
  for (wchar_t& ch : mode)
  {
    ch = static_cast<wchar_t>(std::towlower(ch));
  }

  if (mode == L"small")
  {
    return UiSizeProfile::MapHexProfile::Small;
  }
  if (mode == L"large")
  {
    return UiSizeProfile::MapHexProfile::Large;
  }

  return UiSizeProfile::MapHexProfile::Medium;
}

UiSizeProfile::Metrics MapTabContent::resolveUiMetrics() const
{
  const HWND referenceWindow = mapCanvas_ != nullptr ? mapCanvas_ : GetDesktopWindow();
  const UiSizeProfile::DisplayInfo displayInfo = UiSizeProfile::queryDisplayInfoForWindow(referenceWindow);
  const UiSizeProfile::Profile effectiveProfile = UiSizeProfile::resolveProfile(resolveRequestedUiProfile(), displayInfo);
  return UiSizeProfile::getMetrics(effectiveProfile);
}

int MapTabContent::resolveScaledMapHexWidth() const
{
  const int baseHexWidth = (appConfig_ != nullptr) ? appConfig_->getMapHexWidth() : 40;
  const int clampedBase = (std::max)(12, baseHexWidth);
  const UiSizeProfile::MapHexMetrics mapHexMetrics = UiSizeProfile::getMapHexMetrics(resolveRequestedMapHexProfile());
  const int scaled = static_cast<int>(std::lround(static_cast<double>(clampedBase) * mapHexMetrics.mapHexWidthScale));
  return (std::max)(12, scaled);
}

void MapTabContent::applyListColumnWidths(const UiSizeProfile::Metrics& metrics,
                                          int leftPanelWidth,
                                          int rightPanelWidth,
                                          int detailsWidth)
{
  (void)leftPanelWidth;
  const int safeRightPanelWidth = (std::max)(160, rightPanelWidth);
  const int safeDetailsWidth = (std::max)(140, detailsWidth);

  if (unitsList_ != nullptr)
  {
    struct UnitsColumnBase
    {
      int index;
      int width;
    };

    const UnitsColumnBase unitColumns[] = {
      { 0, 50 },
      { 1, 180 },
      { 2, 50 },
      { 3, 120 },
      { 4, 150 },
      { 5, 90 },
      { 6, 96 },
      { 7, 240 },
      { 8, 260 },
      { 9, 28 },
      { 10, 28 },
    };

    for (const UnitsColumnBase& column : unitColumns)
    {
      ListView_SetColumnWidth(unitsList_,
                              column.index,
                              WinSizingUtils::scalePx(column.width, metrics));
    }
  }

  if (unitItemsList_ != nullptr)
  {
    const int tokenCol = (std::max)(48, safeRightPanelWidth * 20 / 100);
    const int nameCol = (std::max)(80, safeRightPanelWidth * 42 / 100);
    const int amountCol = (std::max)(56, safeRightPanelWidth * 16 / 100);
    const int afterComCol = (std::max)(70, safeRightPanelWidth * 18 / 100);
    ListView_SetColumnWidth(unitItemsList_, 0, tokenCol);
    ListView_SetColumnWidth(unitItemsList_, 1, nameCol);
    ListView_SetColumnWidth(unitItemsList_, 2, amountCol);
    ListView_SetColumnWidth(unitItemsList_, 3, afterComCol);
  }

  if (unitSkillsList_ != nullptr)
  {
    const int skillIdCol = (std::max)(56, safeRightPanelWidth * 38 / 100);
    const int levelCol = (std::max)(52, safeRightPanelWidth * 20 / 100);
    const int afterComCol = (std::max)(70, safeRightPanelWidth * 34 / 100);
    ListView_SetColumnWidth(unitSkillsList_, 0, skillIdCol);
    ListView_SetColumnWidth(unitSkillsList_, 1, levelCol);
    ListView_SetColumnWidth(unitSkillsList_, 2, afterComCol);
  }

  if (regionResourcesList_ != nullptr)
  {
    const int tokenCol = (std::max)(56, safeDetailsWidth * 44 / 100);
    const int amountCol = (std::max)(52, safeDetailsWidth * 24 / 100);
    const int afterComCol = (std::max)(56, safeDetailsWidth * 26 / 100);
    ListView_SetColumnWidth(regionResourcesList_, 0, tokenCol);
    ListView_SetColumnWidth(regionResourcesList_, 1, amountCol);
    ListView_SetColumnWidth(regionResourcesList_, 2, afterComCol);
  }

  if (regionForSaleList_ != nullptr)
  {
    const int tokenCol = (std::max)(54, safeDetailsWidth * 36 / 100);
    const int amountCol = (std::max)(48, safeDetailsWidth * 20 / 100);
    const int priceCol = (std::max)(46, safeDetailsWidth * 16 / 100);
    const int afterComCol = (std::max)(56, safeDetailsWidth * 24 / 100);
    ListView_SetColumnWidth(regionForSaleList_, 0, tokenCol);
    ListView_SetColumnWidth(regionForSaleList_, 1, amountCol);
    ListView_SetColumnWidth(regionForSaleList_, 2, priceCol);
    ListView_SetColumnWidth(regionForSaleList_, 3, afterComCol);
  }

  if (regionWantedList_ != nullptr)
  {
    const int tokenCol = (std::max)(54, safeDetailsWidth * 36 / 100);
    const int amountCol = (std::max)(48, safeDetailsWidth * 20 / 100);
    const int priceCol = (std::max)(46, safeDetailsWidth * 16 / 100);
    const int afterComCol = (std::max)(56, safeDetailsWidth * 24 / 100);
    ListView_SetColumnWidth(regionWantedList_, 0, tokenCol);
    ListView_SetColumnWidth(regionWantedList_, 1, amountCol);
    ListView_SetColumnWidth(regionWantedList_, 2, priceCol);
    ListView_SetColumnWidth(regionWantedList_, 3, afterComCol);
  }

  const int detailTextWidth = (std::max)(140, safeRightPanelWidth - WinSizingUtils::scalePx(24, metrics));
  if (unitErrorsList_ != nullptr)
  {
    ListView_SetColumnWidth(unitErrorsList_, 0, detailTextWidth);
  }
  if (unitWarningsList_ != nullptr)
  {
    ListView_SetColumnWidth(unitWarningsList_, 0, detailTextWidth);
  }
  if (unitEventsList_ != nullptr)
  {
    ListView_SetColumnWidth(unitEventsList_, 0, detailTextWidth);
  }
}

// TODO: add z display to tab label (display "Map z:<z_coord>")
