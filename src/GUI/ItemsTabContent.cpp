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
 * File: ItemsTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/ItemsTabContent.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Function/StringUtils.hpp"

#include <commctrl.h>
#include <windowsx.h>

#include <algorithm>
#include <cwctype>
#include <map>
#include <sstream>
#include <string>

namespace
{
constexpr int kMargin = 8;
constexpr int kListWidth = 170;

void drawDividerLine(HDC hdc, const RECT& bounds)
{
  if (!hdc)
  {
    return;
  }

  HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
  if (!pen)
  {
    return;
  }

  HGDIOBJ oldPen = SelectObject(hdc, pen);
  const int y = bounds.bottom - 1;
  MoveToEx(hdc, bounds.left + 4, y, nullptr);
  LineTo(hdc, bounds.right - 4, y);
  SelectObject(hdc, oldPen);
  DeleteObject(pen);
}

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

HWND createEdit(HWND parent, HINSTANCE instance)
{
  return CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
    0, 0, 100, 22,
    parent,
    nullptr,
    instance,
    nullptr);
}

HWND createMultilineEdit(HWND parent, HINSTANCE instance)
{
  return CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
    0, 0, 100, 100,
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

std::wstring formatStringIntMap(const std::map<std::wstring, int>& data)
{
  std::wstring result;
  bool first = true;
  for (const auto& [token, amount] : data)
  {
    if (!first)
    {
      result += L"\r\n";
    }
    result += token + L":" + std::to_wstring(amount);
    first = false;
  }
  return result;
}

std::map<std::wstring, int> parseStringIntMap(const std::wstring& text)
{
  std::map<std::wstring, int> result;

  std::wstringstream stream(text);
  std::wstring line;
  while (std::getline(stream, line))
  {
    if (!line.empty() && line.back() == L'\r')
    {
      line.pop_back();
    }

    line = StringUtils::trimWhitespace(line);
    if (line.empty())
    {
      continue;
    }

    const std::size_t separatorPos = line.find(L':');
    if (separatorPos == std::wstring::npos)
    {
      continue;
    }

    const std::wstring token = StringUtils::trimWhitespace(line.substr(0, separatorPos));
    const std::wstring amountText = StringUtils::trimWhitespace(line.substr(separatorPos + 1));
    if (token.empty())
    {
      continue;
    }

    try
    {
      const int amount = std::stoi(amountText);
      result[token] = amount;
    }
    catch (...)
    {
      continue;
    }
  }

  return result;
}
}

bool ItemsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  itemsList_ = CreateWindowExW(
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

  if (!itemsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(itemsList_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

  LVCOLUMNW column {};
  column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  column.pszText = const_cast<LPWSTR>(L"Item ID");
  column.cx = 140;
  column.iSubItem = 0;
  ListView_InsertColumn(itemsList_, 0, &column);

  tokenLabel_ = createLabel(parentWindow, instance, L"Item ID");
  tokenEdit_ = createEdit(parentWindow, instance);
  nameLabel_ = createLabel(parentWindow, instance, L"Name");
  nameEdit_ = createEdit(parentWindow, instance);
  weightLabel_ = createLabel(parentWindow, instance, L"Weight");
  weightEdit_ = createEdit(parentWindow, instance);

  meeleWeaponCheck_ = CreateWindowExW(0, L"BUTTON", L"Meele Weapon", WS_CHILD | BS_AUTOCHECKBOX,
                                      0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);
  rangedWeaponCheck_ = CreateWindowExW(0, L"BUTTON", L"Ranged Weapon", WS_CHILD | BS_AUTOCHECKBOX,
                                       0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);
  armourCheck_ = CreateWindowExW(0, L"BUTTON", L"Armour", WS_CHILD | BS_AUTOCHECKBOX,
                                 0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);
  resourceCheck_ = CreateWindowExW(0, L"BUTTON", L"Resource", WS_CHILD | BS_AUTOCHECKBOX,
                                   0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);
  mountCheck_ = CreateWindowExW(0, L"BUTTON", L"Mount", WS_CHILD | BS_AUTOCHECKBOX,
                                0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);
  manCheck_ = CreateWindowExW(0, L"BUTTON", L"Is Man", WS_CHILD | BS_AUTOCHECKBOX,
                              0, 0, 120, 20, parentWindow, nullptr, instance, nullptr);

  movesLabel_ = createLabel(parentWindow, instance, L"Moves");
  movesEdit_ = createEdit(parentWindow, instance);
  walkCapacityLabel_ = createLabel(parentWindow, instance, L"Walk Cap.");
  walkCapacityEdit_ = createEdit(parentWindow, instance);
  rideCapacityLabel_ = createLabel(parentWindow, instance, L"Ride Cap.");
  rideCapacityEdit_ = createEdit(parentWindow, instance);
  swimCapacityLabel_ = createLabel(parentWindow, instance, L"Swim Cap.");
  swimCapacityEdit_ = createEdit(parentWindow, instance);
  flyCapacityLabel_ = createLabel(parentWindow, instance, L"Fly Cap.");
  flyCapacityEdit_ = createEdit(parentWindow, instance);
  shipSpeedLabel_ = createLabel(parentWindow, instance, L"Ship Speed (hexes/month)");
  shipSpeedEdit_ = createEdit(parentWindow, instance);
  shipSailingSkillLabel_ = createLabel(parentWindow, instance, L"Ship Sailing Skill Required");
  shipSailingSkillEdit_ = createEdit(parentWindow, instance);
  magesStudyLabel_ = createLabel(parentWindow, instance, L"Mages Study Above L2");
  magesStudyEdit_ = createEdit(parentWindow, instance);
  defaultSkillMaxLabel_ = createLabel(parentWindow, instance, L"Default Skill Max");
  defaultSkillMaxEdit_ = createEdit(parentWindow, instance);

  skillsMaxLabel_ = createLabel(parentWindow, instance, L"Skills Max (SKILL:LEVEL per line)");
  skillsMaxEdit_ = createMultilineEdit(parentWindow, instance);

  resourcesLabel_ = createLabel(parentWindow, instance, L"Resources (TOKEN:AMOUNT per line)");
  resourcesEdit_ = createMultilineEdit(parentWindow, instance);

  productionSkillLabel_ = createLabel(parentWindow, instance, L"Production Skills (SKILL:LEVEL per line)");
  productionSkillEdit_ = createMultilineEdit(parentWindow, instance);

  productionHelpLabel_ = createLabel(parentWindow, instance, L"Production Help (ITEM:AMOUNT per line)");
  productionHelpEdit_ = createMultilineEdit(parentWindow, instance);

  fullTextLabel_ = createLabel(parentWindow, instance, L"Full Text (display only)");
  fullTextEdit_ = createMultilineEdit(parentWindow, instance);

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

    if (!tokenLabel_ || !nameLabel_ || !weightLabel_ || !movesLabel_ || !walkCapacityLabel_ || !rideCapacityLabel_ ||
      !swimCapacityLabel_ || !flyCapacityLabel_ || !shipSpeedLabel_ || !shipSailingSkillLabel_ || !magesStudyLabel_ ||
      !defaultSkillMaxLabel_ || !skillsMaxLabel_ || !resourcesLabel_ || !productionSkillLabel_ || !productionHelpLabel_ || !fullTextLabel_ ||
      !tokenEdit_ || !nameEdit_ || !weightEdit_ || !meeleWeaponCheck_ || !rangedWeaponCheck_ ||
      !armourCheck_ || !resourceCheck_ || !mountCheck_ || !movesEdit_ || !walkCapacityEdit_ ||
      !rideCapacityEdit_ || !swimCapacityEdit_ || !flyCapacityEdit_ || !shipSpeedEdit_ || !shipSailingSkillEdit_ ||
      !magesStudyEdit_ || !defaultSkillMaxEdit_ || !manCheck_ || !skillsMaxEdit_ || !resourcesEdit_ || !productionSkillEdit_ ||
      !productionHelpEdit_ || !fullTextEdit_ || !saveButton_ || !verticalScrollBar_)
  {
    return false;
  }

  SendMessageW(fullTextEdit_, EM_SETREADONLY, TRUE, 0);

  refresh();
  return true;
}

void ItemsTabContent::resize(const RECT& displayRect)
{
  if (!itemsList_)
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
  int &scrollPosition = scrollPosition_;

  auto layoutControls = [&, this]()
  {
    SetWindowPos(itemsList_, HWND_TOP, leftX, contentTop, kListWidth, contentHeight, SWP_NOACTIVATE);

    int logicalY = contentTop;
    const int labelHeight = 18;
    const int editHeight = 22;
    const int lineGap = 4;
    const int blockGap = 8;
    const int viewBottom = contentTop + contentHeight;

    auto setCtrlPos = [&](HWND ctrl, int x, int y, int w, int h)
    {
      const bool inView = (y >= contentTop) && (y + h <= viewBottom);
      ShowWindow(ctrl, inView ? SW_SHOW : SW_HIDE);
      SetWindowPos(ctrl, HWND_TOP, x, y, w, h, SWP_NOACTIVATE);
    };

    auto placePair = [&](HWND label, HWND edit)
    {
      setCtrlPos(label, rightX, logicalY - scrollPosition, rightWidth, labelHeight);
      logicalY += labelHeight + 2;
      setCtrlPos(edit, rightX, logicalY - scrollPosition, rightWidth, editHeight);
      logicalY += editHeight + lineGap;
    };

    placePair(tokenLabel_, tokenEdit_);
    placePair(nameLabel_, nameEdit_);
    placePair(weightLabel_, weightEdit_);

    setCtrlPos(meeleWeaponCheck_, rightX, logicalY - scrollPosition_, 140, 20);
    setCtrlPos(rangedWeaponCheck_, rightX + 145, logicalY - scrollPosition_, 140, 20);
    setCtrlPos(armourCheck_, rightX + 290, logicalY - scrollPosition_, 110, 20);
    logicalY += 22;
    setCtrlPos(resourceCheck_, rightX, logicalY - scrollPosition_, 140, 20);
    setCtrlPos(mountCheck_, rightX + 145, logicalY - scrollPosition_, 140, 20);
    setCtrlPos(manCheck_, rightX + 290, logicalY - scrollPosition_, 110, 20);
    logicalY += 24 + blockGap;

    placePair(movesLabel_, movesEdit_);
    placePair(walkCapacityLabel_, walkCapacityEdit_);
    placePair(rideCapacityLabel_, rideCapacityEdit_);
    placePair(swimCapacityLabel_, swimCapacityEdit_);
    placePair(flyCapacityLabel_, flyCapacityEdit_);
    placePair(shipSpeedLabel_, shipSpeedEdit_);
    placePair(shipSailingSkillLabel_, shipSailingSkillEdit_);
    placePair(magesStudyLabel_, magesStudyEdit_);
    placePair(defaultSkillMaxLabel_, defaultSkillMaxEdit_);

    const int sectionHeight = 90;

    setCtrlPos(skillsMaxLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(skillsMaxEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    setCtrlPos(resourcesLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(resourcesEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    setCtrlPos(productionSkillLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(productionSkillEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    setCtrlPos(productionHelpLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(productionHelpEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    const int fullTextHeight = 120;
    setCtrlPos(fullTextLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(fullTextEdit_, rightX, logicalY - scrollPosition_, rightWidth, fullTextHeight);
    logicalY += fullTextHeight + blockGap;

    const int saveButtonHeight = 30;
    setCtrlPos(saveButton_, rightX + rightWidth - 120, logicalY - scrollPosition_, 120, saveButtonHeight);
    logicalY += saveButtonHeight;

    return logicalY;
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

void ItemsTabContent::setVisible(bool visible)
{
  if (!itemsList_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  const HWND controls[] = {
    itemsList_,
    tokenLabel_, tokenEdit_,
    nameLabel_, nameEdit_,
    weightLabel_, weightEdit_,
    meeleWeaponCheck_, rangedWeaponCheck_, armourCheck_, resourceCheck_, mountCheck_, manCheck_,
    movesLabel_, movesEdit_,
    walkCapacityLabel_, walkCapacityEdit_,
    rideCapacityLabel_, rideCapacityEdit_,
    swimCapacityLabel_, swimCapacityEdit_,
    flyCapacityLabel_, flyCapacityEdit_,
    shipSpeedLabel_, shipSpeedEdit_,
    shipSailingSkillLabel_, shipSailingSkillEdit_,
    magesStudyLabel_, magesStudyEdit_,
    defaultSkillMaxLabel_, defaultSkillMaxEdit_,
    skillsMaxLabel_, skillsMaxEdit_,
    resourcesLabel_, resourcesEdit_,
    productionSkillLabel_, productionSkillEdit_,
    productionHelpLabel_, productionHelpEdit_,
    fullTextLabel_, fullTextEdit_,
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

  if (visible && lastDisplayRect_.right > lastDisplayRect_.left)
  {
    resize(lastDisplayRect_);
  }
}

void ItemsTabContent::refresh()
{
  if (!appData_ || !itemsList_)
  {
    return;
  }

  updateItemsList();
}

bool ItemsTabContent::handleNotify(const NMHDR* hdr)
{
  notifyResult_ = 0;

  if (!hdr || hdr->idFrom != static_cast<UINT>(kListControlId))
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
      notifyResult_ = CDRF_NOTIFYPOSTPAINT;
      return true;
    }

    if (customDraw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
    {
      if (dividerAfterRow_ >= 0 && static_cast<int>(customDraw->nmcd.dwItemSpec) == dividerAfterRow_)
      {
        drawDividerLine(customDraw->nmcd.hdc, customDraw->nmcd.rc);
      }

      notifyResult_ = CDRF_DODEFAULT;
      return true;
    }
  }

  if (hdr->code == LVN_ITEMCHANGED)
  {
    const auto* listView = reinterpret_cast<const NMLISTVIEW*>(hdr);
    if ((listView->uChanged & LVIF_STATE) != 0 && (listView->uNewState & LVIS_SELECTED) != 0)
    {
      updateSelectedItemFromList();
    }
    return true;
  }

  return false;
}

bool ItemsTabContent::handleCommand(int commandId, int /*notificationCode*/)
{
  if (commandId == kSaveButtonId)
  {
    saveSelectedItem();
    return true;
  }

  return false;
}

bool ItemsTabContent::handleVScroll(WPARAM wp, LPARAM lp)
{
  if (reinterpret_cast<HWND>(lp) != verticalScrollBar_ || !itemsList_)
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

void ItemsTabContent::updateItemsList()
{
  ListView_DeleteAllItems(itemsList_);
  dividerAfterRow_ = -1;

  const auto& repository = appData_->itemRepository();
  
  // Create a sorted list with LEAD at top, then other isMan items, separator, then others
  std::vector<const Item*> leadItems;
  std::vector<const Item*> otherManItems;
  std::vector<const Item*> otherItems;
  
  for (std::size_t index = 0; index < repository.size(); ++index)
  {
    const Item& itemModel = repository.at(index);
    const std::wstring token = itemModel.getIdentifierToken();
    
    // Check if this is LEAD
    if (token == L"LEAD")
    {
      leadItems.push_back(&itemModel);
    }
    else if (itemModel.isMan())
    {
      otherManItems.push_back(&itemModel);
    }
    else
    {
      otherItems.push_back(&itemModel);
    }
  }
  
  // Sort non-LEAD lists alphabetically by token
  auto sortByToken = [](const Item* a, const Item* b)
  {
    return a->getIdentifierToken() < b->getIdentifierToken();
  };
  std::sort(otherManItems.begin(), otherManItems.end(), sortByToken);
  std::sort(otherItems.begin(), otherItems.end(), sortByToken);
  
  // Build final sorted list: LEAD, other man items, then other items.
  // A custom-drawn divider line separates the two categories.
  std::vector<const Item*> sortedItems;
  sortedItems.insert(sortedItems.end(), leadItems.begin(), leadItems.end());
  sortedItems.insert(sortedItems.end(), otherManItems.begin(), otherManItems.end());

  if (!sortedItems.empty() && !otherItems.empty())
  {
    dividerAfterRow_ = static_cast<int>(sortedItems.size()) - 1;
  }
  
  int selectedRow = -1;
  
  // Add LEAD and other man items
  for (int index = 0; index < static_cast<int>(sortedItems.size()); ++index)
  {
    const Item* itemModel = sortedItems[index];
    const std::wstring token = itemModel->getIdentifierToken();

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = index;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(token.c_str());
    const int row = ListView_InsertItem(itemsList_, &item);

    if (selectedItemToken_ == token)
    {
      selectedRow = row;
    }
  }
  
  // Add other items
  int startOtherIndex = static_cast<int>(sortedItems.size());
  for (int index = 0; index < static_cast<int>(otherItems.size()); ++index)
  {
    const Item* itemModel = otherItems[index];
    const std::wstring token = itemModel->getIdentifierToken();

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = startOtherIndex + index;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(token.c_str());
    const int row = ListView_InsertItem(itemsList_, &item);

    if (selectedItemToken_ == token)
    {
      selectedRow = row;
    }
  }

  if (selectedRow >= 0)
  {
    ListView_SetItemState(itemsList_, selectedRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(itemsList_, selectedRow, FALSE);
    updateSelectedItemFromList();
  }
  else
  {
    selectedItemToken_.clear();
    clearFields();
  }
}

void ItemsTabContent::updateSelectedItemFromList()
{
  const int selectedRow = ListView_GetNextItem(itemsList_, -1, LVNI_SELECTED);
  if (selectedRow < 0)
  {
    selectedItemToken_.clear();
    clearFields();
    return;
  }

  wchar_t tokenBuffer[256] = {};
  ListView_GetItemText(itemsList_, selectedRow, 0, tokenBuffer, static_cast<int>(std::size(tokenBuffer)));
  selectedItemToken_ = tokenBuffer;

  const Item* item = appData_->itemRepository().findByIdentifierToken(selectedItemToken_);
  loadItemToFields(item);
}

void ItemsTabContent::loadItemToFields(const Item* item)
{
  if (!item)
  {
    clearFields();
    return;
  }

  SetWindowTextW(tokenEdit_, item->getIdentifierToken().c_str());
  SetWindowTextW(nameEdit_, item->getItemName().c_str());
  SetWindowTextW(weightEdit_, std::to_wstring(item->getWeight()).c_str());
  Button_SetCheck(meeleWeaponCheck_, item->isMeeleWeapon() ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(rangedWeaponCheck_, item->isRangedWeapon() ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(armourCheck_, item->isArmour() ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(resourceCheck_, item->isResource() ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(mountCheck_, item->isMount() ? BST_CHECKED : BST_UNCHECKED);
  SetWindowTextW(movesEdit_, std::to_wstring(item->getMoves()).c_str());
  SetWindowTextW(walkCapacityEdit_, std::to_wstring(item->getWalkCapacity()).c_str());
  SetWindowTextW(rideCapacityEdit_, std::to_wstring(item->getRideCapacity()).c_str());
  SetWindowTextW(swimCapacityEdit_, std::to_wstring(item->getSwimCapacity()).c_str());
  SetWindowTextW(flyCapacityEdit_, std::to_wstring(item->getFlyCapacity()).c_str());
  SetWindowTextW(shipSpeedEdit_, std::to_wstring(item->getShipSpeedHexesPerMonth()).c_str());
  SetWindowTextW(shipSailingSkillEdit_, std::to_wstring(item->getShipSailingSkillRequired()).c_str());
  SetWindowTextW(magesStudyEdit_, std::to_wstring(item->getMagesStudy()).c_str());
  SetWindowTextW(defaultSkillMaxEdit_, std::to_wstring(item->getDefaultSkillMax()).c_str());
  Button_SetCheck(manCheck_, item->isMan() ? BST_CHECKED : BST_UNCHECKED);

  const std::wstring skillsMaxText = formatStringIntMap(item->getSkillsMax());
  SetWindowTextW(skillsMaxEdit_, skillsMaxText.c_str());

  const std::wstring resourcesText = formatStringIntMap(item->getResources());
  SetWindowTextW(resourcesEdit_, resourcesText.c_str());

  const std::wstring productionSkillText = formatStringIntMap(item->getProductionSkill());
  SetWindowTextW(productionSkillEdit_, productionSkillText.c_str());

  const std::wstring productionHelpText = formatStringIntMap(item->getProductionHelp());
  SetWindowTextW(productionHelpEdit_, productionHelpText.c_str());

  SetWindowTextW(fullTextEdit_, item->getFullText().c_str());
}

void ItemsTabContent::clearFields()
{
  const HWND controls[] = {
    tokenEdit_, nameEdit_, weightEdit_, movesEdit_, walkCapacityEdit_, rideCapacityEdit_,
    swimCapacityEdit_, flyCapacityEdit_, shipSpeedEdit_, shipSailingSkillEdit_, magesStudyEdit_,
    defaultSkillMaxEdit_, skillsMaxEdit_, resourcesEdit_, productionSkillEdit_, productionHelpEdit_, fullTextEdit_
  };

  for (HWND control : controls)
  {
    if (control)
    {
      SetWindowTextW(control, L"");
    }
  }

  Button_SetCheck(meeleWeaponCheck_, BST_UNCHECKED);
  Button_SetCheck(rangedWeaponCheck_, BST_UNCHECKED);
  Button_SetCheck(armourCheck_, BST_UNCHECKED);
  Button_SetCheck(resourceCheck_, BST_UNCHECKED);
  Button_SetCheck(mountCheck_, BST_UNCHECKED);
  Button_SetCheck(manCheck_, BST_UNCHECKED);
}

void ItemsTabContent::saveSelectedItem()
{
  if (!appData_ || selectedItemToken_.empty())
  {
    return;
  }

  Item* item = appData_->itemRepository().findByIdentifierToken(selectedItemToken_);
  if (!item)
  {
    return;
  }

  const std::wstring editedToken = StringUtils::trimWhitespace(getWindowText(tokenEdit_));
  if (!editedToken.empty() && editedToken != selectedItemToken_)
  {
    MessageBoxW(GetParent(itemsList_),
                L"Item ID is immutable in this editor. Other fields were saved.",
                L"Items",
                MB_ICONWARNING | MB_OK);
  }

  item->setItemName(getWindowText(nameEdit_));
  item->setWeight(StringUtils::parseIntSafe(getWindowText(weightEdit_)));
  item->setMeeleWeapon(Button_GetCheck(meeleWeaponCheck_) == BST_CHECKED);
  item->setRangedWeapon(Button_GetCheck(rangedWeaponCheck_) == BST_CHECKED);
  item->setArmour(Button_GetCheck(armourCheck_) == BST_CHECKED);
  item->setResource(Button_GetCheck(resourceCheck_) == BST_CHECKED);
  item->setMount(Button_GetCheck(mountCheck_) == BST_CHECKED);
  item->setMoves(StringUtils::parseIntSafe(getWindowText(movesEdit_)));
  item->setWalkCapacity(StringUtils::parseIntSafe(getWindowText(walkCapacityEdit_)));
  item->setRideCapacity(StringUtils::parseIntSafe(getWindowText(rideCapacityEdit_)));
  item->setSwimCapacity(StringUtils::parseIntSafe(getWindowText(swimCapacityEdit_)));
  item->setFlyCapacity(StringUtils::parseIntSafe(getWindowText(flyCapacityEdit_)));
  item->setShipSpeedHexesPerMonth(StringUtils::parseIntSafe(getWindowText(shipSpeedEdit_)));
  item->setShipSailingSkillRequired(StringUtils::parseIntSafe(getWindowText(shipSailingSkillEdit_)));
  item->setMagesStudy(StringUtils::parseIntSafe(getWindowText(magesStudyEdit_)));
  item->setDefaultSkillMax(StringUtils::parseIntSafe(getWindowText(defaultSkillMaxEdit_)));
  item->setMan(Button_GetCheck(manCheck_) == BST_CHECKED);
  item->setSkillsMax(parseStringIntMap(getWindowText(skillsMaxEdit_)));
  item->setResources(parseStringIntMap(getWindowText(resourcesEdit_)));
  item->setProductionSkill(parseStringIntMap(getWindowText(productionSkillEdit_)));
  item->setProductionHelp(parseStringIntMap(getWindowText(productionHelpEdit_)));

  updateItemsList();
}
