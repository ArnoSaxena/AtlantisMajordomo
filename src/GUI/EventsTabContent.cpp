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
 * File: EventsTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/EventsTabContent.hpp"
#include "GUI/ControlIds.hpp"

#include "Data/AppData.hpp"
#include "Function/MonthUtils.hpp"

#include <commctrl.h>
#include <algorithm>
#include <string>
#include <vector>

namespace
{
constexpr int kMargin = 8;
constexpr int kDateComboHeight = 280;
constexpr int kDateToListGap = 4;
}

bool EventsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  dateCombo_ = CreateWindowExW(
    0,
    L"COMBOBOX",
    nullptr,
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
    0,
    0,
    240,
    kDateComboHeight,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(GUI::ControlIds::kEventsDateCombo)),
    instance,
    nullptr
  );

  if (!dateCombo_)
  {
    return false;
  }

  eventsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(GUI::ControlIds::kEventsList)),
    instance,
    nullptr
  );

  if (!eventsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(
    eventsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  struct Column
  {
    const wchar_t* title;
    int width;
  };

  const Column columns[] = {
    { L"Unit Id", 90 },
    { L"Message", 720 }
  };

  for (int index = 0; index < static_cast<int>(std::size(columns)); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(columns[index].title);
    column.cx = columns[index].width;
    column.iSubItem = index;
    ListView_InsertColumn(eventsList_, index, &column);
  }

  refresh();
  return true;
}

void EventsTabContent::resize(const RECT& displayRect)
{
  if (!dateCombo_ || !eventsList_)
  {
    return;
  }

  const int x = displayRect.left + kMargin;
  const int y = displayRect.top + kMargin;
  const int width = (displayRect.right - displayRect.left) - 2 * kMargin;
  const int dateWidth = (std::min)(320, (std::max)(120, width));
  const int dateHeight = 220;
  const int listY = y + 24 + kDateToListGap;
  const int listHeight = (displayRect.bottom - displayRect.top) - 2 * kMargin - 24 - kDateToListGap;

  SetWindowPos(
    dateCombo_,
    HWND_TOP,
    x,
    y,
    dateWidth,
    dateHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    eventsList_,
    HWND_TOP,
    x,
    listY,
    width,
    (std::max)(0, listHeight),
    SWP_NOACTIVATE
  );
}

void EventsTabContent::setVisible(bool visible)
{
  if (!dateCombo_ || !eventsList_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  ShowWindow(dateCombo_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(eventsList_, visible ? SW_SHOW : SW_HIDE);
}

void EventsTabContent::refresh()
{
  if (!dateCombo_ || !eventsList_ || !appData_)
  {
    return;
  }

  updateDateDropdown();
  updateEventsList();
}

bool EventsTabContent::handleCommand(int commandId, int notificationCode)
{
  if (!dateCombo_)
  {
    return false;
  }

  if (commandId != GUI::ControlIds::kEventsDateCombo || notificationCode != CBN_SELCHANGE)
  {
    return false;
  }

  updateSelectedPeriodFromDropdown();
  updateEventsList();
  return true;
}

bool EventsTabContent::handleNotify(const NMHDR* hdr)
{
  if (!hdr || !eventsList_)
  {
    return false;
  }

  if (hdr->idFrom != static_cast<UINT>(GUI::ControlIds::kEventsList) || hdr->code != NM_CUSTOMDRAW)
  {
    return false;
  }

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
    const int row = static_cast<int>(customDraw->nmcd.dwItemSpec);
    LVITEMW item {};
    item.mask = LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    if (ListView_GetItem(eventsList_, &item) && item.lParam != 0)
    {
      customDraw->clrText = RGB(200, 0, 0);
    }

    notifyResult_ = CDRF_DODEFAULT;
    return true;
  }

  return false;
}

LRESULT EventsTabContent::getNotifyResult() const
{
  return notifyResult_;
}

void EventsTabContent::updateDateDropdown()
{
  if (!dateCombo_ || !appData_)
  {
    return;
  }

  const int previousMonth = selectedMonth_;
  const int previousYear = selectedYear_;

  availablePeriods_ = appData_->eventRepository().getAvailablePeriods();
  SendMessageW(dateCombo_, CB_RESETCONTENT, 0, 0);

  if (availablePeriods_.empty())
  {
    selectedMonth_ = 0;
    selectedYear_ = 0;
    SendMessageW(dateCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Date: -"));
    SendMessageW(dateCombo_, CB_SETCURSEL, 0, 0);
    EnableWindow(dateCombo_, FALSE);
    return;
  }

  EnableWindow(dateCombo_, TRUE);
  int selectedIndex = 0;
  for (int index = 0; index < static_cast<int>(availablePeriods_.size()); ++index)
  {
    const auto [month, year] = availablePeriods_[static_cast<std::size_t>(index)];
    const std::wstring text = MonthUtils::monthNumberToNameOr(month, L"Unknown") + L" " + std::to_wstring(year);
    SendMessageW(dateCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));

    if (month == previousMonth && year == previousYear)
    {
      selectedIndex = index;
    }
  }

  SendMessageW(dateCombo_, CB_SETCURSEL, static_cast<WPARAM>(selectedIndex), 0);
  updateSelectedPeriodFromDropdown();
}

void EventsTabContent::updateSelectedPeriodFromDropdown()
{
  if (!dateCombo_ || availablePeriods_.empty())
  {
    selectedMonth_ = 0;
    selectedYear_ = 0;
    return;
  }

  int selectedIndex = static_cast<int>(SendMessageW(dateCombo_, CB_GETCURSEL, 0, 0));
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(availablePeriods_.size()))
  {
    selectedIndex = 0;
  }

  const auto [month, year] = availablePeriods_[static_cast<std::size_t>(selectedIndex)];
  selectedMonth_ = month;
  selectedYear_ = year;
}

void EventsTabContent::updateEventsList()
{
  ListView_DeleteAllItems(eventsList_);

  if (selectedMonth_ < 1 || selectedMonth_ > 12 || selectedYear_ <= 0)
  {
    return;
  }

  const std::vector<const Event*> eventsForCurrentPeriod = appData_->eventRepository().findByPeriod(selectedMonth_, selectedYear_);
  int row = 0;
  for (const Event* eventValue : eventsForCurrentPeriod)
  {
    if (!eventValue)
    {
      continue;
    }

    std::wstring unitId = std::to_wstring(eventValue->getUnitId());
    std::wstring message = eventValue->getMessage();

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = unitId.data();
    item.lParam = eventValue->isErrorEvent() ? 1 : 0;
    const int rowIndex = ListView_InsertItem(eventsList_, &item);
    if (rowIndex < 0)
    {
      continue;
    }

    ListView_SetItemText(eventsList_, rowIndex, 1, message.data());
    ++row;
  }
}
