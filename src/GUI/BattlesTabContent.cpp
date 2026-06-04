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
 * File: BattlesTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/BattlesTabContent.hpp"

#include "Data/AppData.hpp"
#include "Data/Battle.hpp"
#include "Function/CoordinateFormattingUtils.hpp"

#include <commctrl.h>
#include <sstream>
#include <string>

namespace
{
constexpr int kMargin = 8;
constexpr int kPaneGap = 10;
constexpr int kDateComboWidth = 180;

std::wstring formatPeriod(int month, int year)
{
  std::wstringstream stream;
  if (month < 10)
  {
    stream << L"0";
  }
  stream << month << L"/" << year;
  return stream.str();
}

std::wstring formatBattleCoordinates(const Battle& battle)
{
  return CoordinateFormattingUtils::formatCoordinates(
    battle.getRegionXCoordinate(),
    battle.getRegionYCoordinate(),
    battle.getRegionZCoordinate()
  );
}

std::wstring formatBattleListEntry(const Battle& battle)
{
  std::wstringstream stream;
  stream << battle.getAttackerUnitName() << L" (" << battle.getAttackerUnitId() << L") attacks "
         << battle.getDefenderUnitName() << L" (" << battle.getDefenderUnitId() << L") in "
         << formatBattleCoordinates(battle);
  return stream.str();
}

std::wstring joinDamaged(const std::vector<int>& damagedUnitIds)
{
  if (damagedUnitIds.empty())
  {
    return L"none";
  }

  std::wstringstream stream;
  for (std::size_t index = 0; index < damagedUnitIds.size(); ++index)
  {
    if (index != 0)
    {
      stream << L", ";
    }
    stream << damagedUnitIds[index];
  }

  return stream.str();
}

std::wstring formatSpoils(const Battle& battle)
{
  if (battle.getSpoils().empty())
  {
    return L"none";
  }

  std::wstringstream stream;
  for (std::size_t index = 0; index < battle.getSpoils().size(); ++index)
  {
    if (index != 0)
    {
      stream << L", ";
    }

    const BattleSpoil& spoil = battle.getSpoils()[index];
    stream << spoil.amount << L" [" << spoil.token << L"]";
  }

  return stream.str();
}

std::wstring formatSummary(const Battle& battle)
{
  std::wstringstream stream;
  stream << L"Attacker " << battle.getAttackerUnitName() << L" (" << battle.getAttackerUnitId() << L") loses "
         << battle.getAttackerLosses() << L". Damaged units: " << joinDamaged(battle.getAttackerDamagedUnitIds()) << L"\r\n"
         << L"Defender " << battle.getDefenderUnitName() << L" (" << battle.getDefenderUnitId() << L") loses "
         << battle.getDefenderLosses() << L". Damaged units: " << joinDamaged(battle.getDefenderDamagedUnitIds()) << L"\r\n"
         << L"Spoils: " << formatSpoils(battle);
  return stream.str();
}

} // namespace

bool BattlesTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  dateLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Date",
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!dateLabel_)
  {
    return false;
  }

  dateCombo_ = CreateWindowExW(
    0,
    L"COMBOBOX",
    nullptr,
    WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
    0, 0, 100, 200,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDateComboControlId)),
    instance,
    nullptr
  );

  if (!dateCombo_)
  {
    return false;
  }

  battlesList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBattleListControlId)),
    instance,
    nullptr
  );

  if (!battlesList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(
    battlesList_,
    LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER
  );

  LVCOLUMNW column {};
  column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  column.pszText = const_cast<LPWSTR>(L"Battle");
  column.cx = 520;
  column.iSubItem = 0;
  ListView_InsertColumn(battlesList_, 0, &column);

  summaryEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    nullptr,
    WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
    0, 0, 100, 70,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!summaryEdit_)
  {
    return false;
  }

  fullReportEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    nullptr,
    WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
    0, 0, 100, 100,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!fullReportEdit_)
  {
    return false;
  }

  refresh();
  return true;
}

void BattlesTabContent::resize(const RECT& displayRect)
{
  if (!dateLabel_ || !dateCombo_ || !battlesList_ || !summaryEdit_ || !fullReportEdit_)
  {
    return;
  }

  const int contentWidth = static_cast<int>(displayRect.right - displayRect.left) - 2 * kMargin;
  const int contentHeight = static_cast<int>(displayRect.bottom - displayRect.top) - 2 * kMargin;
  const int leftPaneWidth = contentWidth > 0 ? contentWidth / 2 : 0;
  const int rightPaneWidth = contentWidth - leftPaneWidth - kPaneGap;

  const int leftPaneX = displayRect.left + kMargin;
  const int topY = displayRect.top + kMargin;
  const int rightPaneX = leftPaneX + leftPaneWidth + kPaneGap;

  SetWindowPos(dateLabel_, HWND_TOP, leftPaneX, topY + 3, 50, 20, SWP_NOACTIVATE);
  SetWindowPos(dateCombo_, HWND_TOP, leftPaneX + 56, topY, kDateComboWidth, 300, SWP_NOACTIVATE);

  const int listTop = topY + 30;
  const int listHeight = (contentHeight > 0) ? contentHeight - 30 : 0;
  SetWindowPos(
    battlesList_,
    HWND_TOP,
    leftPaneX,
    listTop,
    leftPaneWidth,
    listHeight,
    SWP_NOACTIVATE
  );

  const int summaryHeight = 82;
  SetWindowPos(
    summaryEdit_,
    HWND_TOP,
    rightPaneX,
    topY,
    rightPaneWidth,
    summaryHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    fullReportEdit_,
    HWND_TOP,
    rightPaneX,
    topY + summaryHeight + kPaneGap,
    rightPaneWidth,
    contentHeight - summaryHeight - kPaneGap,
    SWP_NOACTIVATE
  );

  const int listClientWidth = (leftPaneWidth > 6) ? leftPaneWidth - 6 : leftPaneWidth;
  ListView_SetColumnWidth(battlesList_, 0, (listClientWidth > 100) ? listClientWidth : 100);
}

void BattlesTabContent::setVisible(bool visible)
{
  if (!dateLabel_ || !dateCombo_ || !battlesList_ || !summaryEdit_ || !fullReportEdit_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  ShowWindow(dateLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(dateCombo_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(battlesList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(summaryEdit_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(fullReportEdit_, visible ? SW_SHOW : SW_HIDE);
}

void BattlesTabContent::refresh()
{
  if (!appData_)
  {
    return;
  }

  updateDateSelector();
  updateBattleList();
  updateBattleDetailsFromSelection();
}

bool BattlesTabContent::handleNotify(const NMHDR* hdr)
{
  if (!hdr)
  {
    return false;
  }

  if (hdr->idFrom == static_cast<UINT>(kBattleListControlId) && hdr->code == LVN_ITEMCHANGED)
  {
    const auto* listView = reinterpret_cast<const NMLISTVIEW*>(hdr);
    if ((listView->uChanged & LVIF_STATE) != 0 && (listView->uNewState & LVIS_SELECTED) != 0)
    {
      updateBattleDetailsFromSelection();
    }
    return true;
  }

  return false;
}

bool BattlesTabContent::handleCommand(int commandId, int notificationCode)
{
  if (commandId != kDateComboControlId)
  {
    return false;
  }

  if (notificationCode == CBN_SELCHANGE)
  {
    const int selectedIndex = static_cast<int>(SendMessageW(dateCombo_, CB_GETCURSEL, 0, 0));
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(availablePeriods_.size()))
    {
      selectedMonth_ = availablePeriods_[static_cast<std::size_t>(selectedIndex)].first;
      selectedYear_ = availablePeriods_[static_cast<std::size_t>(selectedIndex)].second;
      updateBattleList();
      updateBattleDetailsFromSelection();
    }
    return true;
  }

  return false;
}

void BattlesTabContent::updateDateSelector()
{
  if (!dateCombo_ || !appData_)
  {
    return;
  }

  const auto& battleRepository = appData_->battleRepository();
  availablePeriods_ = battleRepository.getAvailablePeriodsDescending();

  SendMessageW(dateCombo_, CB_RESETCONTENT, 0, 0);

  for (const auto& [month, year] : availablePeriods_)
  {
    const std::wstring periodText = formatPeriod(month, year);
    SendMessageW(dateCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(periodText.c_str()));
  }

  if (availablePeriods_.empty())
  {
    selectedMonth_ = 0;
    selectedYear_ = 0;
    return;
  }

  int desiredIndex = 0;
  if (selectedMonth_ > 0 && selectedYear_ > 0)
  {
    for (int index = 0; index < static_cast<int>(availablePeriods_.size()); ++index)
    {
      const auto& period = availablePeriods_[static_cast<std::size_t>(index)];
      if (period.first == selectedMonth_ && period.second == selectedYear_)
      {
        desiredIndex = index;
        break;
      }
    }
  }

  selectedMonth_ = availablePeriods_[static_cast<std::size_t>(desiredIndex)].first;
  selectedYear_ = availablePeriods_[static_cast<std::size_t>(desiredIndex)].second;
  SendMessageW(dateCombo_, CB_SETCURSEL, static_cast<WPARAM>(desiredIndex), 0);
}

void BattlesTabContent::updateBattleList()
{
  if (!battlesList_ || !appData_)
  {
    return;
  }

  ListView_DeleteAllItems(battlesList_);
  visibleBattles_.clear();

  if (selectedMonth_ <= 0 || selectedYear_ <= 0)
  {
    return;
  }

  visibleBattles_ = appData_->battleRepository().findByPeriod(selectedMonth_, selectedYear_);
  for (int row = 0; row < static_cast<int>(visibleBattles_.size()); ++row)
  {
    const Battle* battle = visibleBattles_[static_cast<std::size_t>(row)];
    if (!battle)
    {
      continue;
    }

    const std::wstring battleText = formatBattleListEntry(*battle);

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(battleText.c_str());
    item.lParam = row;
    ListView_InsertItem(battlesList_, &item);
  }

  if (!visibleBattles_.empty())
  {
    ListView_SetItemState(battlesList_, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(battlesList_, 0, FALSE);
  }
}

void BattlesTabContent::updateBattleDetailsFromSelection()
{
  if (!summaryEdit_ || !fullReportEdit_)
  {
    return;
  }

  const int selectedRow = ListView_GetNextItem(battlesList_, -1, LVNI_SELECTED);
  if (selectedRow < 0 || selectedRow >= static_cast<int>(visibleBattles_.size()))
  {
    SetWindowTextW(summaryEdit_, L"");
    SetWindowTextW(fullReportEdit_, L"");
    return;
  }

  const Battle* battle = visibleBattles_[static_cast<std::size_t>(selectedRow)];
  if (!battle)
  {
    SetWindowTextW(summaryEdit_, L"");
    SetWindowTextW(fullReportEdit_, L"");
    return;
  }

  const std::wstring summaryText = formatSummary(*battle);
  SetWindowTextW(summaryEdit_, summaryText.c_str());
  SetWindowTextW(fullReportEdit_, battle->getFullText().c_str());
}
