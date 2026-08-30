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
#include "GUI/UiSizeProfile.hpp"
#include "GUI/WinSizingUtils.hpp"

#include "Data/AppData.hpp"
#include "Function/AppDataUtils.hpp"
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

UiSizeProfile::Metrics resolveUiMetrics(HWND referenceWindow)
{
  const UiSizeProfile::DisplayInfo displayInfo = UiSizeProfile::queryDisplayInfoForWindow(
    referenceWindow != nullptr ? referenceWindow : GetDesktopWindow());
  const UiSizeProfile::Profile effectiveProfile = UiSizeProfile::resolveProfile(UiSizeProfile::Profile::Auto, displayInfo);
  return UiSizeProfile::getMetrics(effectiveProfile);
}
}

bool EventsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;
  const UiSizeProfile::Metrics metrics = resolveUiMetrics(parentWindow);

  subTab_ = CreateWindowExW(
    0,
    WC_TABCONTROLW,
    nullptr,
    WS_CHILD | WS_CLIPSIBLINGS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(GUI::ControlIds::kEventsSubTab)),
    instance,
    nullptr
  );

  if (!subTab_)
  {
    return false;
  }

  TCITEMW tabItem {};
  tabItem.mask = TCIF_TEXT;
  tabItem.pszText = const_cast<LPWSTR>(L"Events");
  TabCtrl_InsertItem(subTab_, 0, &tabItem);
  tabItem.pszText = const_cast<LPWSTR>(L"Warnings");
  TabCtrl_InsertItem(subTab_, 1, &tabItem);

  dateCombo_ = CreateWindowExW(
    0,
    L"COMBOBOX",
    nullptr,
    WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
    0,
    0,
    240,
    WinSizingUtils::scalePx(kDateComboHeight, metrics),
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

  warningsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(GUI::ControlIds::kWarningsList)),
    instance,
    nullptr
  );

  if (!warningsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(
    warningsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );
  WinSizingUtils::listViewApplyDensity(warningsList_, metrics, nullptr, nullptr);

  ListView_SetExtendedListViewStyle(
    eventsList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );
  WinSizingUtils::listViewApplyDensity(eventsList_, metrics, nullptr, nullptr);

  struct Column
  {
    const wchar_t* title;
    int width;
  };

  const Column columns[] = {
    { L"Unit Id", WinSizingUtils::scalePx(90, metrics) },
    { L"Message", WinSizingUtils::scalePx(720, metrics) }
  };

  for (int index = 0; index < static_cast<int>(std::size(columns)); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(columns[index].title);
    column.cx = columns[index].width;
    column.iSubItem = index;
    ListView_InsertColumn(eventsList_, index, &column);
    ListView_InsertColumn(warningsList_, index, &column);
  }

  updateVisibleSubTab();
  refresh();
  return true;
}

void EventsTabContent::resize(const RECT& displayRect)
{
  if (!dateCombo_ || !subTab_ || !eventsList_ || !warningsList_)
  {
    return;
  }

  const UiSizeProfile::Metrics metrics = resolveUiMetrics(dateCombo_);
  const int margin = WinSizingUtils::scalePx(kMargin, metrics);
  const int dateToListGap = WinSizingUtils::scalePx(kDateToListGap, metrics);
  const int x = displayRect.left + margin;
  const int y = displayRect.top + margin;
  const int width = (displayRect.right - displayRect.left) - 2 * margin;
  const int dateWidth = (std::min)(WinSizingUtils::scalePx(320, metrics), (std::max)(WinSizingUtils::scalePx(120, metrics), width));
  const int dateCollapsedHeight = (std::max)(metrics.buttonHeight, WinSizingUtils::scalePx(24, metrics));
  const int dateDropHeight = WinSizingUtils::scalePx(220, metrics);
  const int tabHeight = (std::max)(0, static_cast<int>(displayRect.bottom - displayRect.top) - 2 * margin);

  SetWindowPos(
    subTab_,
    HWND_TOP,
    x,
    y,
    width,
    tabHeight,
    SWP_NOACTIVATE
  );

  RECT tabDisplayRect { 0, 0, width, tabHeight };
  TabCtrl_AdjustRect(subTab_, FALSE, &tabDisplayRect);

  SetWindowPos(
    dateCombo_,
    HWND_TOP,
    x + tabDisplayRect.left,
    y + tabDisplayRect.top,
    dateWidth,
    dateDropHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    eventsList_,
    HWND_TOP,
    x + tabDisplayRect.left,
    y + tabDisplayRect.top + dateCollapsedHeight + dateToListGap,
    tabDisplayRect.right - tabDisplayRect.left,
    (std::max)(0, static_cast<int>(tabDisplayRect.bottom - tabDisplayRect.top) - dateCollapsedHeight - dateToListGap),
    SWP_NOACTIVATE
  );

  SetWindowPos(
    warningsList_,
    HWND_TOP,
    x + tabDisplayRect.left,
    y + tabDisplayRect.top,
    tabDisplayRect.right - tabDisplayRect.left,
    (std::max)(0, static_cast<int>(tabDisplayRect.bottom - tabDisplayRect.top)),
    SWP_NOACTIVATE
  );

  const int listClientWidth = (std::max)(0, static_cast<int>(tabDisplayRect.right - tabDisplayRect.left) - 6);
  const int unitIdWidth = (std::max)(WinSizingUtils::scalePx(80, metrics), listClientWidth / 5);
  ListView_SetColumnWidth(eventsList_, 0, unitIdWidth);
  ListView_SetColumnWidth(eventsList_, 1, (std::max)(WinSizingUtils::scalePx(120, metrics), listClientWidth - unitIdWidth));
  ListView_SetColumnWidth(warningsList_, 0, unitIdWidth);
  ListView_SetColumnWidth(warningsList_, 1, (std::max)(WinSizingUtils::scalePx(120, metrics), listClientWidth - unitIdWidth));
  updateVisibleSubTab();
}

void EventsTabContent::setVisible(bool visible)
{
  if (!dateCombo_ || !subTab_ || !eventsList_ || !warningsList_)
  {
    return;
  }

  visible_ = visible;

  if (visible)
  {
    refresh();
  }

  ShowWindow(dateCombo_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(eventsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(warningsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(subTab_, visible ? SW_SHOW : SW_HIDE);
  updateVisibleSubTab();
}

void EventsTabContent::refresh()
{
  if (!dateCombo_ || !eventsList_ || !warningsList_ || !appData_)
  {
    return;
  }

  updateDateDropdown();
  updateEventsList();
  updateWarningsList();
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
  if (!hdr || !subTab_ || !eventsList_ || !warningsList_)
  {
    return false;
  }

  if (hdr->idFrom == static_cast<UINT>(GUI::ControlIds::kEventsSubTab) && hdr->code == TCN_SELCHANGE)
  {
    updateVisibleSubTab();
    return true;
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

void EventsTabContent::updateWarningsList()
{
  ListView_DeleteAllItems(warningsList_);

  const std::vector<AppDataUtils::WarningRow> warnings =
    AppDataUtils::getWarningsForLatestPeriod(*appData_);
  int row = 0;
  for (const auto& warning : warnings)
  {
    std::wstring unitId = warning.isNewUnit
      ? L"New " + std::to_wstring(warning.unitNumber) + L", "
        + std::to_wstring(warning.xCoordinate) + L" "
        + std::to_wstring(warning.yCoordinate) + L" "
        + std::to_wstring(warning.zCoordinate)
      : std::to_wstring(warning.unitNumber);
    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.pszText = unitId.data();
    const int rowIndex = ListView_InsertItem(warningsList_, &item);
    if (rowIndex >= 0)
    {
      ListView_SetItemText(warningsList_, rowIndex, 1, const_cast<LPWSTR>(warning.text.c_str()));
      ++row;
    }
  }
}

void EventsTabContent::updateVisibleSubTab()
{
  if (!subTab_ || !dateCombo_ || !eventsList_ || !warningsList_)
  {
    return;
  }

  const bool showEvents = visible_ && TabCtrl_GetCurSel(subTab_) != 1;
  ShowWindow(dateCombo_, visible_ && showEvents ? SW_SHOW : SW_HIDE);
  ShowWindow(eventsList_, showEvents ? SW_SHOW : SW_HIDE);
  ShowWindow(warningsList_, visible_ && !showEvents ? SW_SHOW : SW_HIDE);
}
