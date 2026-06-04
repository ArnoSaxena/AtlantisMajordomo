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
 * File: FactionsTabContent.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/FactionsTabContent.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/Unit.hpp"
#include "Function/FactionAttitudeUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/StringUtils.hpp"

#include <commctrl.h>
#include <windowsx.h>
#include <algorithm>
#include <cwctype>
#include <map>
#include <string>
#include <vector>
#include <limits>

namespace
{
constexpr int kMargin = 8;
constexpr int kListWidth = 220;
constexpr int kComboDropHeight = 140;
constexpr UINT kAttitudeMenuDefaultCommand = 6101;
constexpr UINT kAttitudeMenuHostileCommand = 6102;
constexpr UINT kAttitudeMenuUnfriendlyCommand = 6103;
constexpr UINT kAttitudeMenuNeutralCommand = 6104;
constexpr UINT kAttitudeMenuFriendlyCommand = 6105;
constexpr UINT kAttitudeMenuAllyCommand = 6106;

HWND createLabel(HWND parent, HINSTANCE instance, const wchar_t* text)
{
  return CreateWindowExW(
    0,
    L"STATIC",
    text,
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parent,
    nullptr,
    instance,
    nullptr);
}

HWND createEdit(HWND parent, HINSTANCE instance, DWORD extraStyle = 0)
{
  return CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | extraStyle,
    0, 0, 100, 22,
    parent,
    nullptr,
    instance,
    nullptr);
}

std::wstring getWindowText(HWND control)
{
  if (!control)
  {
    return L"";
  }

  const int length = GetWindowTextLengthW(control);
  if (length <= 0)
  {
    return L"";
  }

  std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(control, text.data(), length + 1);
  return text.c_str();
}

void populateAttitudeCombo(HWND combo)
{
  if (!combo)
  {
    return;
  }

  ComboBox_ResetContent(combo);
  ComboBox_AddString(combo, L"Hostile");
  ComboBox_AddString(combo, L"Unfriendly");
  ComboBox_AddString(combo, L"Neutral");
  ComboBox_AddString(combo, L"Friendly");
  ComboBox_AddString(combo, L"Ally");
}

}

bool FactionsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  factionsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kListControlId)),
    instance,
    nullptr
  );

  if (!factionsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(factionsList_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

  LVCOLUMNW column {};
  column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  column.pszText = const_cast<LPWSTR>(L"Faction");
  column.cx = 190;
  column.iSubItem = 0;
  ListView_InsertColumn(factionsList_, 0, &column);

  factionNumberLabel_ = createLabel(parentWindow, instance, L"Faction Number");
  factionNumberEdit_ = createEdit(parentWindow, instance, ES_READONLY);
  factionNameLabel_ = createLabel(parentWindow, instance, L"Name");
  factionNameEdit_ = createEdit(parentWindow, instance);
  mainFactionCheck_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Main Faction",
    WS_CHILD | BS_AUTOCHECKBOX,
    0, 0, 140, 20,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );
  monthLabel_ = createLabel(parentWindow, instance, L"Month");
  monthEdit_ = createEdit(parentWindow, instance);
  yearLabel_ = createLabel(parentWindow, instance, L"Year");
  yearEdit_ = createEdit(parentWindow, instance);
  passwordLabel_ = createLabel(parentWindow, instance, L"Password");
  passwordEdit_ = createEdit(parentWindow, instance);

  taxedTradedCurrentLabel_ = createLabel(parentWindow, instance, L"Taxed/Traded Regions Current");
  taxedTradedCurrentEdit_ = createEdit(parentWindow, instance);
  taxedTradedMaxLabel_ = createLabel(parentWindow, instance, L"Taxed/Traded Regions Max");
  taxedTradedMaxEdit_ = createEdit(parentWindow, instance);

  quartermastersCurrentLabel_ = createLabel(parentWindow, instance, L"Quartermasters Current");
  quartermastersCurrentEdit_ = createEdit(parentWindow, instance);
  quartermastersMaxLabel_ = createLabel(parentWindow, instance, L"Quartermasters Max");
  quartermastersMaxEdit_ = createEdit(parentWindow, instance);

  magesCurrentLabel_ = createLabel(parentWindow, instance, L"Mages Current");
  magesCurrentEdit_ = createEdit(parentWindow, instance);
  magesMaxLabel_ = createLabel(parentWindow, instance, L"Mages Max");
  magesMaxEdit_ = createEdit(parentWindow, instance);

  apprenticesCurrentLabel_ = createLabel(parentWindow, instance, L"Apprentices Current");
  apprenticesCurrentEdit_ = createEdit(parentWindow, instance);
  apprenticesMaxLabel_ = createLabel(parentWindow, instance, L"Apprentices Max");
  apprenticesMaxEdit_ = createEdit(parentWindow, instance);
  unclaimedSilverLabel_ = createLabel(parentWindow, instance, L"Unclaimed Silver (after com)");
  unclaimedSilverEdit_ = createEdit(parentWindow, instance, ES_READONLY);

  defaultAttitudeLabel_ = createLabel(parentWindow, instance, L"Default Attitude");
  defaultAttitudeCombo_ = CreateWindowExW(
    0,
    WC_COMBOBOXW,
    nullptr,
    WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
    0, 0, 100, 120,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDefaultAttitudeComboId)),
    instance,
    nullptr
  );
  attitudesList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAttitudesListControlId)),
    instance,
    nullptr
  );

  commandUnitLabel_ = createLabel(parentWindow, instance, L"Faction Command Unit:");
  commandUnitEdit_ = createEdit(parentWindow, instance);
  commandUnitSaveButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Save",
    WS_CHILD | BS_PUSHBUTTON,
    0, 0, 70, 22,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommandUnitSaveButtonId)),
    instance,
    nullptr
  );

  populateAttitudeCombo(defaultAttitudeCombo_);

  if (attitudesList_)
  {
    ListView_SetExtendedListViewStyle(attitudesList_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    LVCOLUMNW attitudesFactionColumn {};
    attitudesFactionColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    attitudesFactionColumn.pszText = const_cast<LPWSTR>(L"Faction");
    attitudesFactionColumn.cx = 220;
    attitudesFactionColumn.iSubItem = 0;
    ListView_InsertColumn(attitudesList_, 0, &attitudesFactionColumn);

    LVCOLUMNW attitudesValueColumn {};
    attitudesValueColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    attitudesValueColumn.pszText = const_cast<LPWSTR>(L"Attitude");
    attitudesValueColumn.cx = 110;
    attitudesValueColumn.iSubItem = 1;
    ListView_InsertColumn(attitudesList_, 1, &attitudesValueColumn);
  }

  saveButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Save",
    WS_CHILD | BS_PUSHBUTTON,
    0, 0, 120, 30,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveButtonId)),
    instance,
    nullptr
  );

  verticalScrollBar_ = CreateWindowExW(
    0,
    L"SCROLLBAR",
    nullptr,
    WS_CHILD | SBS_VERT,
    0, 0, 0, 0,
    parentWindow,
    nullptr,
    instance,
    nullptr
  );

  if (!factionNumberLabel_ || !factionNumberEdit_ || !factionNameLabel_ || !factionNameEdit_ || !mainFactionCheck_
      || !monthLabel_ || !monthEdit_ || !yearLabel_ || !yearEdit_ || !passwordLabel_ || !passwordEdit_
      || !taxedTradedCurrentLabel_ || !taxedTradedCurrentEdit_ || !taxedTradedMaxLabel_ || !taxedTradedMaxEdit_
      || !quartermastersCurrentLabel_ || !quartermastersCurrentEdit_ || !quartermastersMaxLabel_ || !quartermastersMaxEdit_
      || !magesCurrentLabel_ || !magesCurrentEdit_ || !magesMaxLabel_ || !magesMaxEdit_
      || !apprenticesCurrentLabel_ || !apprenticesCurrentEdit_ || !apprenticesMaxLabel_ || !apprenticesMaxEdit_
      || !unclaimedSilverLabel_ || !unclaimedSilverEdit_
      || !defaultAttitudeLabel_ || !defaultAttitudeCombo_ || !attitudesList_
      || !commandUnitLabel_ || !commandUnitEdit_ || !commandUnitSaveButton_
      || !saveButton_ || !verticalScrollBar_)
  {
    return false;
  }

  refresh();
  return true;
}

void FactionsTabContent::resize(const RECT& displayRect)
{
  if (!factionsList_)
  {
    return;
  }

  lastDisplayRect_ = displayRect;

  const int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
  const int contentTop = displayRect.top + kMargin;
  const int contentHeight = (std::max)(0, static_cast<int>(displayRect.bottom - displayRect.top) - 2 * kMargin);

  if (verticalScrollBar_)
  {
    SetWindowPos(
      verticalScrollBar_,
      HWND_TOP,
      displayRect.right - kMargin - scrollBarWidth,
      contentTop,
      scrollBarWidth,
      contentHeight,
      SWP_NOACTIVATE
    );
  }

  const int usableWidth = (std::max)(0, static_cast<int>(displayRect.right - displayRect.left) - 2 * kMargin - scrollBarWidth - 4);
  const int leftX = displayRect.left + kMargin;
  const int rightX = leftX + kListWidth + kMargin;
  const int rightWidth = (std::max)(0, usableWidth - kListWidth - kMargin);
  const int attitudesPanelGap = 10;
  const int editorWidth = (std::max)(220, static_cast<int>(rightWidth * 55 / 100));
  const int attitudesX = rightX + editorWidth + attitudesPanelGap;
  const int attitudesWidth = (std::max)(0, rightWidth - editorWidth - attitudesPanelGap);
  int &scrollPosition = scrollPosition_;

  auto layoutControls = [&, this]()
  {
    SetWindowPos(factionsList_, HWND_TOP, leftX, contentTop, kListWidth, contentHeight, SWP_NOACTIVATE);

    int editorLogicalY = contentTop;
    const int labelHeight = 18;
    const int editHeight = 22;
    const int lineGap = 4;
    const int viewBottom = contentTop + contentHeight;

    auto setCtrlPos = [&](HWND ctrl, int x, int y, int w, int h)
    {
      const bool inView = (y >= contentTop) && (y + h <= viewBottom);
      ShowWindow(ctrl, inView ? SW_SHOW : SW_HIDE);
      SetWindowPos(ctrl, HWND_TOP, x, y, w, h, SWP_NOACTIVATE);
    };

    auto placePair = [&](HWND label, HWND edit)
    {
      setCtrlPos(label, rightX, editorLogicalY - scrollPosition, editorWidth, labelHeight);
      editorLogicalY += labelHeight + 2;
      setCtrlPos(edit, rightX, editorLogicalY - scrollPosition, editorWidth, editHeight);
      editorLogicalY += editHeight + lineGap;
    };

    placePair(factionNumberLabel_, factionNumberEdit_);
    placePair(factionNameLabel_, factionNameEdit_);
    setCtrlPos(mainFactionCheck_, rightX, editorLogicalY - scrollPosition_, 160, 20);
    editorLogicalY += 24;

    placePair(monthLabel_, monthEdit_);
    placePair(yearLabel_, yearEdit_);
    placePair(passwordLabel_, passwordEdit_);
    placePair(taxedTradedCurrentLabel_, taxedTradedCurrentEdit_);
    placePair(taxedTradedMaxLabel_, taxedTradedMaxEdit_);
    placePair(quartermastersCurrentLabel_, quartermastersCurrentEdit_);
    placePair(quartermastersMaxLabel_, quartermastersMaxEdit_);
    placePair(magesCurrentLabel_, magesCurrentEdit_);
    placePair(magesMaxLabel_, magesMaxEdit_);
    placePair(apprenticesCurrentLabel_, apprenticesCurrentEdit_);
    placePair(apprenticesMaxLabel_, apprenticesMaxEdit_);
    placePair(unclaimedSilverLabel_, unclaimedSilverEdit_);

    const int saveButtonHeight = 30;
    setCtrlPos(saveButton_, rightX + editorWidth - 120, editorLogicalY - scrollPosition_, 120, saveButtonHeight);
    editorLogicalY += saveButtonHeight;

    // Attitudes panel: fixed position, no scroll offset
    int attitudesLogicalY = contentTop;
    setCtrlPos(defaultAttitudeLabel_, attitudesX, attitudesLogicalY, attitudesWidth, labelHeight);
    attitudesLogicalY += labelHeight + 2;
    setCtrlPos(defaultAttitudeCombo_, attitudesX, attitudesLogicalY, attitudesWidth, kComboDropHeight);
    attitudesLogicalY += editHeight + lineGap;

    const int attitudesListHeight = 280;
    setCtrlPos(attitudesList_, attitudesX, attitudesLogicalY, attitudesWidth, attitudesListHeight);
    attitudesLogicalY += attitudesListHeight + lineGap;

    const int commandSaveButtonWidth = 70;
    const int commandGap = 6;
    const int commandEditWidth = (std::max)(80, attitudesWidth - commandSaveButtonWidth - commandGap);
    setCtrlPos(commandUnitLabel_, attitudesX, attitudesLogicalY, attitudesWidth, labelHeight);
    attitudesLogicalY += labelHeight + 2;
    setCtrlPos(commandUnitEdit_, attitudesX, attitudesLogicalY, commandEditWidth, editHeight);
    setCtrlPos(commandUnitSaveButton_, attitudesX + commandEditWidth + commandGap,
           attitudesLogicalY, commandSaveButtonWidth, editHeight);

    return editorLogicalY;
  };

  const int contentBottom = layoutControls();
  maxScrollPosition_ = (std::max)(0, contentBottom - (contentTop + contentHeight));
  scrollPosition_ = (std::clamp)(scrollPosition_, 0, maxScrollPosition_);

  if (verticalScrollBar_)
  {
    SCROLLINFO scrollInfo {};
    scrollInfo.cbSize = sizeof(scrollInfo);
    scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    scrollInfo.nMin = 0;
    scrollInfo.nMax = (std::max)(0, maxScrollPosition_ + contentHeight - 1);
    scrollInfo.nPage = static_cast<UINT>(contentHeight);
    scrollInfo.nPos = scrollPosition_;
    SetScrollInfo(verticalScrollBar_, SB_CTL, &scrollInfo, TRUE);
  }

  layoutControls();
}

void FactionsTabContent::setVisible(bool visible)
{
  if (!factionsList_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  const HWND controls[] = {
    factionsList_,
    factionNumberLabel_, factionNumberEdit_,
    factionNameLabel_, factionNameEdit_,
    mainFactionCheck_,
    monthLabel_, monthEdit_,
    yearLabel_, yearEdit_,
    passwordLabel_, passwordEdit_,
    taxedTradedCurrentLabel_, taxedTradedCurrentEdit_,
    taxedTradedMaxLabel_, taxedTradedMaxEdit_,
    quartermastersCurrentLabel_, quartermastersCurrentEdit_,
    quartermastersMaxLabel_, quartermastersMaxEdit_,
    magesCurrentLabel_, magesCurrentEdit_,
    magesMaxLabel_, magesMaxEdit_,
    apprenticesCurrentLabel_, apprenticesCurrentEdit_,
    apprenticesMaxLabel_, apprenticesMaxEdit_,
    unclaimedSilverLabel_, unclaimedSilverEdit_,
    saveButton_,
    verticalScrollBar_
  };

  for (HWND control : controls)
  {
    if (control)
    {
      ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
    }
  }

  if (visible)
  {
    const Faction* faction = appData_ ? appData_->factionRepository().findByNumber(selectedFactionNumber_) : nullptr;
    updateAttitudesPanel(faction);
  }
  else
  {
    setAttitudesPanelVisible(false);
  }

  if (visible && lastDisplayRect_.right > lastDisplayRect_.left)
  {
    resize(lastDisplayRect_);
  }
}

void FactionsTabContent::refresh()
{
  if (!appData_ || !factionsList_)
  {
    return;
  }

  updateFactionsList();
}

bool FactionsTabContent::handleNotify(const NMHDR* hdr)
{
  notifyResult_ = 0;

  if (!hdr)
  {
    return false;
  }

  if (hdr->idFrom == static_cast<UINT>(kAttitudesListControlId) && hdr->code == NM_RCLICK)
  {
    showAttitudeContextMenu();
    return true;
  }

  if (hdr->idFrom != static_cast<UINT>(kListControlId))
  {
    return false;
  }

  if (hdr->code == LVN_ITEMCHANGED)
  {
    const auto* listView = reinterpret_cast<const NMLISTVIEW*>(hdr);
    if ((listView->uChanged & LVIF_STATE) != 0 && (listView->uNewState & LVIS_SELECTED) != 0)
    {
      updateSelectedFactionFromList();
    }
    return true;
  }

  return false;
}

bool FactionsTabContent::handleCommand(int commandId, int notificationCode)
{
  if (commandId == kDefaultAttitudeComboId && notificationCode == CBN_SELCHANGE)
  {
    Faction* faction = appData_ ? appData_->factionRepository().findByNumber(selectedFactionNumber_) : nullptr;
    if (faction && faction->isMainFaction())
    {
      const int selectedIndex = ComboBox_GetCurSel(defaultAttitudeCombo_);
      if (selectedIndex >= 0)
      {
        wchar_t buffer[64] {};
        ComboBox_GetLBText(defaultAttitudeCombo_, selectedIndex, buffer);
        handleDefaultAttitudeSelection(*faction, buffer);
      }
    }
    return true;
  }

  if (commandId == kCommandUnitSaveButtonId)
  {
    Faction* faction = appData_ ? appData_->factionRepository().findByNumber(selectedFactionNumber_) : nullptr;
    if (faction)
    {
      const int requestedCommandUnitNumber = StringUtils::parseIntSafe(getWindowText(commandUnitEdit_));
      const auto resolution = OrderBusinessLogic::resolveFactionCommandUnit(
        *appData_,
        faction->getFactionNumber(),
        requestedCommandUnitNumber
      );

      if (resolution.usedFallback)
      {
        MessageBoxW(
          commandUnitEdit_,
          L"Selected command unit is not part of the main faction. The default command unit (smallest unit number) was selected.",
          L"Invalid Command Unit",
          MB_OK | MB_ICONWARNING
        );
      }

      faction->setCommandUnitNumber(resolution.commandUnitNumber);
      SetWindowTextW(commandUnitEdit_,
                     resolution.commandUnitNumber > 0 ? std::to_wstring(resolution.commandUnitNumber).c_str() : L"");

      saveAttitudeEdits();

      OrderBusinessLogic::rewriteFactionDeclareOrders(
        *appData_,
        faction->getFactionNumber(),
        resolution.commandUnitNumber,
        originalDefaultAttitudeText_,
        originalDeclaredAttitudesText_
      );
      updateAttitudesList(faction);
    }
    return true;
  }

  if (commandId == kSaveButtonId)
  {
    saveSelectedFaction();
    return true;
  }

  return false;
}

bool FactionsTabContent::handleVScroll(WPARAM wp, LPARAM lp)
{
  if (reinterpret_cast<HWND>(lp) != verticalScrollBar_ || !factionsList_)
  {
    return false;
  }

  SCROLLINFO scrollInfo {};
  scrollInfo.cbSize = sizeof(scrollInfo);
  scrollInfo.fMask = SIF_ALL;
  GetScrollInfo(verticalScrollBar_, SB_CTL, &scrollInfo);

  int nextScrollPosition = scrollPosition_;
  switch (LOWORD(wp))
  {
    case SB_LINEUP:
      nextScrollPosition -= 24;
      break;
    case SB_LINEDOWN:
      nextScrollPosition += 24;
      break;
    case SB_PAGEUP:
      nextScrollPosition -= static_cast<int>(scrollInfo.nPage);
      break;
    case SB_PAGEDOWN:
      nextScrollPosition += static_cast<int>(scrollInfo.nPage);
      break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      nextScrollPosition = scrollInfo.nTrackPos;
      break;
    case SB_TOP:
      nextScrollPosition = 0;
      break;
    case SB_BOTTOM:
      nextScrollPosition = maxScrollPosition_;
      break;
    default:
      return true;
  }

  nextScrollPosition = (std::clamp)(nextScrollPosition, 0, maxScrollPosition_);
  if (nextScrollPosition != scrollPosition_)
  {
    scrollPosition_ = nextScrollPosition;
    resize(lastDisplayRect_);
  }

  return true;
}

void FactionsTabContent::updateFactionsList()
{
  ListView_DeleteAllItems(factionsList_);

  std::vector<const Faction*> sortedFactions;
  const auto& repository = appData_->factionRepository();
  sortedFactions.reserve(repository.size());

  for (std::size_t index = 0; index < repository.size(); ++index)
  {
    sortedFactions.push_back(&repository.at(index));
  }

  std::stable_sort(sortedFactions.begin(),
                   sortedFactions.end(),
                   [](const Faction* lhs, const Faction* rhs)
                   {
                     if (lhs->isMainFaction() != rhs->isMainFaction())
                     {
                       return lhs->isMainFaction();
                     }

                     return lhs->getFactionNumber() < rhs->getFactionNumber();
                   });

  int selectedRow = -1;
  for (int row = 0; row < static_cast<int>(sortedFactions.size()); ++row)
  {
    const Faction* faction = sortedFactions[static_cast<std::size_t>(row)];
    std::wstring listText;
    if (faction->isMainFaction())
    {
      listText = L"* ";
    }

    if (!faction->getName().empty())
    {
      listText += faction->getName() + L" (" + std::to_wstring(faction->getFactionNumber()) + L")";
    }
    else
    {
      listText += std::to_wstring(faction->getFactionNumber());
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(listText.c_str());
    item.lParam = static_cast<LPARAM>(faction->getFactionNumber());

    const int inserted = ListView_InsertItem(factionsList_, &item);
    if (inserted >= 0 && faction->getFactionNumber() == selectedFactionNumber_)
    {
      selectedRow = inserted;
    }
  }

  if (selectedRow < 0 && ListView_GetItemCount(factionsList_) > 0)
  {
    selectedRow = 0;
  }

  if (selectedRow >= 0)
  {
    ListView_SetItemState(factionsList_, selectedRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(factionsList_, selectedRow, FALSE);
    updateSelectedFactionFromList();
  }
  else
  {
    selectedFactionNumber_ = 0;
    clearFields();
  }
}

void FactionsTabContent::updateSelectedFactionFromList()
{
  const int selectedIndex = ListView_GetNextItem(factionsList_, -1, LVNI_SELECTED);
  if (selectedIndex < 0)
  {
    selectedFactionNumber_ = 0;
    clearFields();
    return;
  }

  LVITEMW item {};
  item.mask = LVIF_PARAM;
  item.iItem = selectedIndex;
  item.iSubItem = 0;
  if (!ListView_GetItem(factionsList_, &item))
  {
    selectedFactionNumber_ = 0;
    clearFields();
    return;
  }

  selectedFactionNumber_ = static_cast<int>(item.lParam);
  pendingAttitudeEdits_.clear();
  const Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
  loadFactionToFields(faction);
}

void FactionsTabContent::loadFactionToFields(const Faction* faction)
{
  if (!faction)
  {
    clearFields();
    return;
  }

  captureOriginalAttitudeSnapshot(faction);

  SetWindowTextW(factionNumberEdit_, std::to_wstring(faction->getFactionNumber()).c_str());
  SetWindowTextW(factionNameEdit_, faction->getName().c_str());
  Button_SetCheck(mainFactionCheck_, faction->isMainFaction() ? BST_CHECKED : BST_UNCHECKED);

  SetWindowTextW(monthEdit_, std::to_wstring(faction->getMonth()).c_str());
  SetWindowTextW(yearEdit_, std::to_wstring(faction->getYear()).c_str());
  SetWindowTextW(passwordEdit_, faction->getPassword().c_str());

  SetWindowTextW(taxedTradedCurrentEdit_, std::to_wstring(faction->getTaxedOrTradedRegionsCurrent()).c_str());
  SetWindowTextW(taxedTradedMaxEdit_, std::to_wstring(faction->getTaxedOrTradedRegionsMax()).c_str());
  SetWindowTextW(quartermastersCurrentEdit_, std::to_wstring(faction->getQuartermastersCurrent()).c_str());
  SetWindowTextW(quartermastersMaxEdit_, std::to_wstring(faction->getQuartermastersMax()).c_str());
  SetWindowTextW(magesCurrentEdit_, std::to_wstring(faction->getMagesCurrent()).c_str());
  SetWindowTextW(magesMaxEdit_, std::to_wstring(faction->getMagesMax()).c_str());
  SetWindowTextW(apprenticesCurrentEdit_, std::to_wstring(faction->getApprenticesCurrent()).c_str());
  SetWindowTextW(apprenticesMaxEdit_, std::to_wstring(faction->getApprenticesMax()).c_str());

  updateAttitudesPanel(faction);
}

void FactionsTabContent::captureOriginalAttitudeSnapshot(const Faction* faction)
{
  originalDeclaredAttitudesText_.clear();
  originalDefaultAttitudeText_ = L"Neutral";

  if (!faction)
  {
    return;
  }

  const int factionNumber = faction->getFactionNumber();
  if (originalDefaultAttitudeByFactionText_.find(factionNumber) == originalDefaultAttitudeByFactionText_.end())
  {
    originalDefaultAttitudeByFactionText_[factionNumber] = FactionAttitudeUtils::attitudeToText(faction->getDefaultAttitude());

    std::map<int, std::wstring> declaredText;
    for (const auto& [targetFactionNumber, attitude] : faction->getDeclaredAttitudes())
    {
      declaredText[targetFactionNumber] = FactionAttitudeUtils::attitudeToText(attitude);
    }
    originalDeclaredAttitudesByFactionText_[factionNumber] = std::move(declaredText);
  }

  originalDefaultAttitudeText_ = originalDefaultAttitudeByFactionText_[factionNumber];
  originalDeclaredAttitudesText_ = originalDeclaredAttitudesByFactionText_[factionNumber];
}

void FactionsTabContent::setAttitudesPanelVisible(bool visible)
{
  const int cmd = visible ? SW_SHOW : SW_HIDE;
  ShowWindow(defaultAttitudeLabel_, cmd);
  ShowWindow(defaultAttitudeCombo_, cmd);
  ShowWindow(attitudesList_, cmd);
  ShowWindow(commandUnitLabel_, cmd);
  ShowWindow(commandUnitEdit_, cmd);
  ShowWindow(commandUnitSaveButton_, cmd);
  ShowWindow(unclaimedSilverLabel_, cmd);
  ShowWindow(unclaimedSilverEdit_, cmd);
}


void FactionsTabContent::handleDefaultAttitudeSelection(Faction& faction, const std::wstring& selectedAttitudeText)
{
  const Faction::Attitude selectedAttitude = FactionAttitudeUtils::textToAttitude(selectedAttitudeText);
  faction.setDefaultAttitude(selectedAttitude);
  updateAttitudesList(&faction);
}

void FactionsTabContent::showAttitudeContextMenu()
{
  if (!appData_ || !attitudesList_)
  {
    return;
  }

  Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
  if (!faction || !faction->isMainFaction())
  {
    return;
  }

  POINT screenPoint {};
  if (!GetCursorPos(&screenPoint))
  {
    return;
  }

  POINT clientPoint = screenPoint;
  ScreenToClient(attitudesList_, &clientPoint);

  LVHITTESTINFO hitInfo {};
  hitInfo.pt = clientPoint;
  const int hitRow = ListView_SubItemHitTest(attitudesList_, &hitInfo);
  if (hitRow < 0)
  {
    return;
  }

  ListView_SetItemState(attitudesList_, hitRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

  LVITEMW rowItem {};
  rowItem.mask = LVIF_PARAM;
  rowItem.iItem = hitRow;
  rowItem.iSubItem = 0;
  if (!ListView_GetItem(attitudesList_, &rowItem))
  {
    return;
  }

  const int targetFactionNumber = static_cast<int>(rowItem.lParam);

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  AppendMenuW(menu, MF_STRING, kAttitudeMenuDefaultCommand, L"Default");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kAttitudeMenuHostileCommand, L"Hostile");
  AppendMenuW(menu, MF_STRING, kAttitudeMenuUnfriendlyCommand, L"Unfriendly");
  AppendMenuW(menu, MF_STRING, kAttitudeMenuNeutralCommand, L"Neutral");
  AppendMenuW(menu, MF_STRING, kAttitudeMenuFriendlyCommand, L"Friendly");
  AppendMenuW(menu, MF_STRING, kAttitudeMenuAllyCommand, L"Ally");

  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    attitudesList_,
    nullptr
  );

  DestroyMenu(menu);

  std::wstring selectedValue;
  switch (selectedCommand)
  {
    case kAttitudeMenuDefaultCommand:
      selectedValue = L"Default";
      break;
    case kAttitudeMenuHostileCommand:
      selectedValue = L"Hostile";
      break;
    case kAttitudeMenuUnfriendlyCommand:
      selectedValue = L"Unfriendly";
      break;
    case kAttitudeMenuNeutralCommand:
      selectedValue = L"Neutral";
      break;
    case kAttitudeMenuFriendlyCommand:
      selectedValue = L"Friendly";
      break;
    case kAttitudeMenuAllyCommand:
      selectedValue = L"Ally";
      break;
    default:
      return;
  }

  applyAttitudeContextSelection(targetFactionNumber, selectedValue);
}

void FactionsTabContent::applyAttitudeContextSelection(int targetFactionNumber, const std::wstring& selectedValue)
{
  if (!appData_ || targetFactionNumber <= 0)
  {
    return;
  }

  Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
  if (!faction || !faction->isMainFaction())
  {
    return;
  }

  const auto declaredIt = faction->getDeclaredAttitudes().find(targetFactionNumber);
  const bool hadDeclaredAttitude = declaredIt != faction->getDeclaredAttitudes().end();

  if (selectedValue == L"Default")
  {
    if (hadDeclaredAttitude)
    {
      pendingAttitudeEdits_[targetFactionNumber] = PendingAttitudeEdit { true, L"" };
    }
    else
    {
      pendingAttitudeEdits_.erase(targetFactionNumber);
    }
  }
  else
  {
    const Faction::Attitude selectedAttitude = FactionAttitudeUtils::textToAttitude(selectedValue);
    const bool unchanged = hadDeclaredAttitude && declaredIt->second == selectedAttitude;
    if (unchanged)
    {
      pendingAttitudeEdits_.erase(targetFactionNumber);
    }
    else
    {
      pendingAttitudeEdits_[targetFactionNumber] = PendingAttitudeEdit { false, FactionAttitudeUtils::attitudeToText(selectedAttitude) };
    }
  }

  updateAttitudesList(faction);
}

void FactionsTabContent::saveAttitudeEdits()
{
  if (!appData_ || pendingAttitudeEdits_.empty())
  {
    return;
  }

  Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
  if (!faction || !faction->isMainFaction())
  {
    return;
  }

  std::vector<OrderBusinessLogic::DeclaredAttitudeChange> changes;
  changes.reserve(pendingAttitudeEdits_.size());
  for (const auto& [targetFactionNumber, edit] : pendingAttitudeEdits_)
  {
    OrderBusinessLogic::DeclaredAttitudeChange change;
    change.targetFactionNumber = targetFactionNumber;
    change.useDefault = edit.useDefault;
    change.attitudeText = edit.attitudeText;
    changes.push_back(std::move(change));
  }

  if (OrderBusinessLogic::applyDeclaredAttitudeChanges(*appData_, faction->getFactionNumber(), changes))
  {
    pendingAttitudeEdits_.clear();
    updateAttitudesList(faction);
  }
}

// Now implemented in UnitRepository

void FactionsTabContent::updateAttitudesList(const Faction* faction)
{
  if (!attitudesList_)
  {
    return;
  }

  ListView_DeleteAllItems(attitudesList_);
  if (!appData_ || !faction || !faction->isMainFaction())
  {
    return;
  }

  const Faction::Attitude defaultAttitude = faction->getDefaultAttitude();
  const std::map<int, Faction::Attitude>& declared = faction->getDeclaredAttitudes();
  const auto& repository = appData_->factionRepository();

  std::vector<const Faction*> sortedFactions;
  sortedFactions.reserve(repository.size());
  for (std::size_t index = 0; index < repository.size(); ++index)
  {
    const Faction& targetFaction = repository.at(index);
    if (targetFaction.getFactionNumber() == faction->getFactionNumber())
    {
      continue;
    }

    sortedFactions.push_back(&targetFaction);
  }

  std::sort(
    sortedFactions.begin(),
    sortedFactions.end(),
    [](const Faction* lhs, const Faction* rhs)
    {
      return lhs->getFactionNumber() < rhs->getFactionNumber();
    }
  );

  int row = 0;
  for (const Faction* targetFactionPtr : sortedFactions)
  {
    if (!targetFactionPtr)
    {
      continue;
    }

    const Faction& targetFaction = *targetFactionPtr;

    std::wstring factionText;
    if (!targetFaction.getName().empty())
    {
      factionText = targetFaction.getName() + L" (" + std::to_wstring(targetFaction.getFactionNumber()) + L")";
    }
    else
    {
      factionText = std::to_wstring(targetFaction.getFactionNumber());
    }

    Faction::Attitude resolvedAttitude = defaultAttitude;
    bool resolvedFromDefault = true;
    const auto declaredIt = declared.find(targetFaction.getFactionNumber());
    if (declaredIt != declared.end())
    {
      resolvedAttitude = declaredIt->second;
      resolvedFromDefault = false;
    }

    const auto pendingEditIt = pendingAttitudeEdits_.find(targetFaction.getFactionNumber());
    if (pendingEditIt != pendingAttitudeEdits_.end())
    {
      if (pendingEditIt->second.useDefault)
      {
        resolvedAttitude = defaultAttitude;
        resolvedFromDefault = true;
      }
      else
      {
        resolvedAttitude = FactionAttitudeUtils::textToAttitude(pendingEditIt->second.attitudeText);
        resolvedFromDefault = false;
      }
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(factionText.c_str());
    item.lParam = static_cast<LPARAM>(targetFaction.getFactionNumber());
    const int inserted = ListView_InsertItem(attitudesList_, &item);
    if (inserted >= 0)
    {
      std::wstring attitudeText = FactionAttitudeUtils::attitudeToText(resolvedAttitude);
      if (resolvedFromDefault)
      {
        attitudeText += L" (default)";
      }
      ListView_SetItemText(attitudesList_, inserted, 1, const_cast<LPWSTR>(attitudeText.c_str()));
    }

    ++row;
  }
}

void FactionsTabContent::updateAttitudesPanel(const Faction* faction)
{
  if (!defaultAttitudeCombo_ || !attitudesList_ || !commandUnitEdit_)
  {
    return;
  }

  // Keep the dropdown complete in a stable order.
  populateAttitudeCombo(defaultAttitudeCombo_);

  if (!faction || !faction->isMainFaction())
  {
    ComboBox_SetCurSel(defaultAttitudeCombo_, -1);
    ListView_DeleteAllItems(attitudesList_);
    ShowWindow(unclaimedSilverLabel_, SW_HIDE);
    ShowWindow(unclaimedSilverEdit_, SW_HIDE);
    SetWindowTextW(unclaimedSilverEdit_, L"");
    setAttitudesPanelVisible(false);
    return;
  }

  setAttitudesPanelVisible(true);
  ShowWindow(unclaimedSilverLabel_, SW_SHOW);
  ShowWindow(unclaimedSilverEdit_, SW_SHOW);

  const wchar_t* selectedAttitude = FactionAttitudeUtils::attitudeToText(faction->getDefaultAttitude());
  const int defaultIndex = ComboBox_FindStringExact(defaultAttitudeCombo_, -1, selectedAttitude);
  ComboBox_SetCurSel(defaultAttitudeCombo_, defaultIndex >= 0 ? defaultIndex : 2);

  int commandUnitNumber = 0;
  if (faction && appData_)
    commandUnitNumber = faction->resolveCommandUnitNumber(appData_->unitRepository());
  SetWindowTextW(commandUnitEdit_, commandUnitNumber > 0 ? std::to_wstring(commandUnitNumber).c_str() : L"");

  if (appData_)
  {
    const int actualUnclaimedSilver = faction->getUnclaimedSilver();
    const int afterCommandsUnclaimedSilver = faction->getUnclaimedSilverAfterOrders();
    const std::wstring displayText =
      std::to_wstring(actualUnclaimedSilver) + L" (" + std::to_wstring(afterCommandsUnclaimedSilver) + L")";
    SetWindowTextW(unclaimedSilverEdit_, displayText.c_str());
  }

  updateAttitudesList(faction);
}

void FactionsTabContent::clearFields()
{
  pendingAttitudeEdits_.clear();
  originalDeclaredAttitudesText_.clear();
  originalDefaultAttitudeText_ = L"Neutral";

  SetWindowTextW(factionNumberEdit_, L"");
  SetWindowTextW(factionNameEdit_, L"");
  Button_SetCheck(mainFactionCheck_, BST_UNCHECKED);

  SetWindowTextW(monthEdit_, L"");
  SetWindowTextW(yearEdit_, L"");
  SetWindowTextW(passwordEdit_, L"");

  SetWindowTextW(taxedTradedCurrentEdit_, L"");
  SetWindowTextW(taxedTradedMaxEdit_, L"");
  SetWindowTextW(quartermastersCurrentEdit_, L"");
  SetWindowTextW(quartermastersMaxEdit_, L"");
  SetWindowTextW(magesCurrentEdit_, L"");
  SetWindowTextW(magesMaxEdit_, L"");
  SetWindowTextW(apprenticesCurrentEdit_, L"");
  SetWindowTextW(apprenticesMaxEdit_, L"");
  SetWindowTextW(unclaimedSilverEdit_, L"");

  if (defaultAttitudeCombo_)
  {
    ComboBox_SetCurSel(defaultAttitudeCombo_, -1);
  }
  if (attitudesList_)
  {
    ListView_DeleteAllItems(attitudesList_);
  }
  if (commandUnitEdit_)
  {
    SetWindowTextW(commandUnitEdit_, L"");
  }
  setAttitudesPanelVisible(false);
}

void FactionsTabContent::saveSelectedFaction()
{
  if (!appData_ || selectedFactionNumber_ <= 0)
  {
    return;
  }

  Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
  if (!faction)
  {
    return;
  }

  faction->setName(StringUtils::trimWhitespace(getWindowText(factionNameEdit_)));
  faction->setMonth(StringUtils::parseIntSafe(getWindowText(monthEdit_)));
  faction->setYear(StringUtils::parseIntSafe(getWindowText(yearEdit_)));
  faction->setPassword(getWindowText(passwordEdit_));

  faction->setTaxedOrTradedRegionsCurrent(StringUtils::parseIntSafe(getWindowText(taxedTradedCurrentEdit_)));
  faction->setTaxedOrTradedRegionsMax(StringUtils::parseIntSafe(getWindowText(taxedTradedMaxEdit_)));
  faction->setQuartermastersCurrent(StringUtils::parseIntSafe(getWindowText(quartermastersCurrentEdit_)));
  faction->setQuartermastersMax(StringUtils::parseIntSafe(getWindowText(quartermastersMaxEdit_)));
  faction->setMagesCurrent(StringUtils::parseIntSafe(getWindowText(magesCurrentEdit_)));
  faction->setMagesMax(StringUtils::parseIntSafe(getWindowText(magesMaxEdit_)));
  faction->setApprenticesCurrent(StringUtils::parseIntSafe(getWindowText(apprenticesCurrentEdit_)));
  faction->setApprenticesMax(StringUtils::parseIntSafe(getWindowText(apprenticesMaxEdit_)));

  if (faction->isMainFaction() && defaultAttitudeCombo_)
  {
    const int selectedIndex = ComboBox_GetCurSel(defaultAttitudeCombo_);
    if (selectedIndex >= 0)
    {
      wchar_t buffer[64] {};
      ComboBox_GetLBText(defaultAttitudeCombo_, selectedIndex, buffer);
      faction->setDefaultAttitude(FactionAttitudeUtils::textToAttitude(buffer));
    }
  }

  faction->setCommandUnitNumber(StringUtils::parseIntSafe(getWindowText(commandUnitEdit_)));

  const bool selectedAsMain = (Button_GetCheck(mainFactionCheck_) == BST_CHECKED);
  if (selectedAsMain)
  {
    auto& repository = appData_->factionRepository();
    for (std::size_t index = 0; index < repository.size(); ++index)
    {
      Faction* candidate = repository.findByNumber(repository.at(index).getFactionNumber());
      if (candidate)
      {
        candidate->setMainFaction(candidate->getFactionNumber() == selectedFactionNumber_);
      }
    }
  }
  else
  {
    faction->setMainFaction(false);
  }

  refresh();
}
