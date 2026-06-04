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
 * File: SkillsTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/SkillsTabContent.hpp"

#include "Data/AppData.hpp"
#include "Data/Skill.hpp"
#include "Function/StringUtils.hpp"

#include <commctrl.h>
#include <windowsx.h>

#include <algorithm>
#include <cwctype>
#include <map>
#include <regex>
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

std::wstring formatPrerequisites(const std::vector<SkillPrerequisite>& prerequisites)
{
  std::wstring result;
  bool first = true;
  for (const auto& prerequisite : prerequisites)
  {
    if (!first)
    {
      result += L"\r\n";
    }

    result += prerequisite.token + L":" + std::to_wstring(prerequisite.requiredLevel);
    first = false;
  }

  return result;
}

std::vector<SkillPrerequisite> parsePrerequisites(const std::wstring& text)
{
  std::vector<SkillPrerequisite> prerequisites;

  std::wstringstream stream(text);
  std::wstring line;
  static const std::wregex linePattern(L"^([A-Za-z0-9]{3,})\\s*:\\s*(\\d+)\\s*$");
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

    std::wsmatch match;
    if (!std::regex_match(line, match, linePattern))
    {
      continue;
    }

    SkillPrerequisite prerequisite;
    prerequisite.token = StringUtils::trimWhitespace(match[1].str());
    prerequisite.requiredLevel = StringUtils::parseIntSafe(match[2].str());
    if (!prerequisite.token.empty() && prerequisite.requiredLevel > 0)
    {
      prerequisites.push_back(std::move(prerequisite));
    }
  }

  return prerequisites;
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

int resolveSkillStudyCost(const Skill* skill)
{
  if (!skill)
  {
    return 0;
  }

  return skill->getStudyCost();
}
}

bool SkillsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  skillsList_ = CreateWindowExW(
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

  if (!skillsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(skillsList_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

  LVCOLUMNW column {};
  column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  column.pszText = const_cast<LPWSTR>(L"Skill");
  column.cx = 140;
  column.iSubItem = 0;
  ListView_InsertColumn(skillsList_, 0, &column);

  tokenLabel_ = createLabel(parentWindow, instance, L"Skill ID");
  tokenEdit_ = createEdit(parentWindow, instance);
  levelLabel_ = createLabel(parentWindow, instance, L"Level");
  levelDropdown_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"COMBOBOX",
    nullptr,
    WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
    0, 0, 100, 200,
    parentWindow,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLevelDropdownId)),
    instance,
    nullptr
  );
  nameLabel_ = createLabel(parentWindow, instance, L"Name");
  nameEdit_ = createEdit(parentWindow, instance);
  studyCostLabel_ = createLabel(parentWindow, instance, L"Study Silver Cost (per man-month)");
  studyCostEdit_ = createEdit(parentWindow, instance);

  productionCheck_ = CreateWindowExW(0, L"BUTTON", L"Production", WS_CHILD | BS_AUTOCHECKBOX,
                                     0, 0, 130, 20, parentWindow, nullptr, instance, nullptr);
  magicCheck_ = CreateWindowExW(0, L"BUTTON", L"Magic", WS_CHILD | BS_AUTOCHECKBOX,
                                0, 0, 130, 20, parentWindow, nullptr, instance, nullptr);
  magicFoundationCheck_ = CreateWindowExW(0, L"BUTTON", L"Magic Foundation", WS_CHILD | BS_AUTOCHECKBOX,
                                          0, 0, 150, 20, parentWindow, nullptr, instance, nullptr);

  prerequisitesLabel_ = createLabel(parentWindow, instance, L"Prerequisites (TOKEN:LEVEL per line)");
  prerequisitesEdit_ = createMultilineEdit(parentWindow, instance);

  productionItemsLabel_ = createLabel(parentWindow, instance, L"Production Items (ITEM:AMOUNT per line)");
  productionItemsEdit_ = createMultilineEdit(parentWindow, instance);

  descriptionLabel_ = createLabel(parentWindow, instance, L"Description (display only)");
  descriptionEdit_ = createMultilineEdit(parentWindow, instance);

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

    if (!tokenLabel_ || !tokenEdit_ || !levelLabel_ || !levelDropdown_ || !nameLabel_ || !nameEdit_
      || !studyCostLabel_ || !studyCostEdit_
      || !productionCheck_ || !magicCheck_ || !magicFoundationCheck_
      || !prerequisitesLabel_ || !prerequisitesEdit_
      || !productionItemsLabel_ || !productionItemsEdit_
      || !descriptionLabel_ || !descriptionEdit_ || !saveButton_ || !verticalScrollBar_)
  {
    return false;
  }

  SendMessageW(descriptionEdit_, EM_SETREADONLY, TRUE, 0);

  refresh();
  return true;
}

void SkillsTabContent::resize(const RECT& displayRect)
{
  if (!skillsList_)
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
    SetWindowPos(skillsList_, HWND_TOP, leftX, contentTop, kListWidth, contentHeight, SWP_NOACTIVATE);

    int logicalY = contentTop;
    const int labelHeight = 18;
    const int editHeight = 22;
    const int lineGap = 4;
    const int blockGap = 10;
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

    setCtrlPos(levelLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(levelDropdown_, rightX, logicalY - scrollPosition_, rightWidth, 200);
    logicalY += editHeight + lineGap;

    placePair(nameLabel_, nameEdit_);
    placePair(studyCostLabel_, studyCostEdit_);

    setCtrlPos(productionCheck_, rightX, logicalY - scrollPosition_, 130, 20);
    setCtrlPos(magicCheck_, rightX + 140, logicalY - scrollPosition_, 130, 20);
    logicalY += 22;
    setCtrlPos(magicFoundationCheck_, rightX, logicalY - scrollPosition_, 170, 20);
    logicalY += 24 + blockGap;

    const int sectionHeight = 120;

    setCtrlPos(prerequisitesLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(prerequisitesEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    setCtrlPos(productionItemsLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(productionItemsEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

    setCtrlPos(descriptionLabel_, rightX, logicalY - scrollPosition_, rightWidth, labelHeight);
    logicalY += labelHeight + 2;
    setCtrlPos(descriptionEdit_, rightX, logicalY - scrollPosition_, rightWidth, sectionHeight);
    logicalY += sectionHeight + blockGap;

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

void SkillsTabContent::setVisible(bool visible)
{
  if (!skillsList_)
  {
    return;
  }

  if (visible)
  {
    refresh();
  }

  const HWND controls[] = {
    skillsList_,
    tokenLabel_, tokenEdit_,
    levelLabel_, levelDropdown_,
    nameLabel_, nameEdit_,
    studyCostLabel_, studyCostEdit_,
    productionCheck_, magicCheck_, magicFoundationCheck_,
    prerequisitesLabel_, prerequisitesEdit_,
    productionItemsLabel_, productionItemsEdit_,
    descriptionLabel_, descriptionEdit_,
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

void SkillsTabContent::refresh()
{
  if (!appData_ || !skillsList_)
  {
    return;
  }

  updateSkillsList();
}

bool SkillsTabContent::handleNotify(const NMHDR* hdr)
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
      updateSelectedSkillFromList();
    }
    return true;
  }

  return false;
}

bool SkillsTabContent::handleCommand(int commandId, int notificationCode)
{
  if (commandId == kSaveButtonId)
  {
    saveSelectedSkill();
    return true;
  }

  if (commandId == kLevelDropdownId && notificationCode == CBN_SELCHANGE)
  {
    const int sel = ComboBox_GetCurSel(levelDropdown_);
    if (sel >= 0 && !selectedSkillToken_.empty())
    {
      // The item data stores the level number
      displayedLevel_ = static_cast<int>(ComboBox_GetItemData(levelDropdown_, sel));
      const Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
      loadSkillLevelToFields(skill, displayedLevel_);
    }
    return true;
  }

  return false;
}

bool SkillsTabContent::handleVScroll(WPARAM wp, LPARAM lp)
{
  if (reinterpret_cast<HWND>(lp) != verticalScrollBar_ || !skillsList_)
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

void SkillsTabContent::updateSkillsList()
{
  ListView_DeleteAllItems(skillsList_);
  dividerAfterRow_ = -1;

  const auto& repository = appData_->skillRepository();
  
  // Create a sorted list: non-magic skills first, then magic skills.
  // A custom-drawn divider line separates the two categories.
  std::vector<const Skill*> nonMagicSkills;
  std::vector<const Skill*> foundationSkills;
  std::vector<const Skill*> otherMagicSkills;
  
  for (std::size_t index = 0; index < repository.size(); ++index)
  {
    const Skill& skillModel = repository.at(index);
    if (skillModel.isMagicFoundation())
    {
      foundationSkills.push_back(&skillModel);
    }
    else if (skillModel.isMagic())
    {
      otherMagicSkills.push_back(&skillModel);
    }
    else
    {
      nonMagicSkills.push_back(&skillModel);
    }
  }
  
  // Sort both lists alphabetically by token
  auto sortByToken = [](const Skill* a, const Skill* b)
  {
    return a->getIdentifierToken() < b->getIdentifierToken();
  };
  std::sort(nonMagicSkills.begin(), nonMagicSkills.end(), sortByToken);
  std::sort(foundationSkills.begin(), foundationSkills.end(), sortByToken);
  std::sort(otherMagicSkills.begin(), otherMagicSkills.end(), sortByToken);
  
  int selectedRow = -1;
  int itemIndex = 0;

  if (!nonMagicSkills.empty() && (!foundationSkills.empty() || !otherMagicSkills.empty()))
  {
    dividerAfterRow_ = static_cast<int>(nonMagicSkills.size()) - 1;
  }
  
  // Add non-magic skills
  for (const Skill* skillModel : nonMagicSkills)
  {
    const std::wstring& listText = skillModel->getIdentifierToken();

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(listText.c_str());
    const int row = ListView_InsertItem(skillsList_, &item);

    if (selectedSkillToken_ == skillModel->getIdentifierToken())
    {
      selectedRow = row;
    }
    
    ++itemIndex;
  }
  
  // Add magic foundation skills first, then other magic skills.
  for (const Skill* skillModel : foundationSkills)
  {
    const std::wstring& listText = skillModel->getIdentifierToken();

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(listText.c_str());
    const int row = ListView_InsertItem(skillsList_, &item);

    if (selectedSkillToken_ == skillModel->getIdentifierToken())
    {
      selectedRow = row;
    }
    
    ++itemIndex;
  }

  for (const Skill* skillModel : otherMagicSkills)
  {
    const std::wstring& listText = skillModel->getIdentifierToken();

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(listText.c_str());
    const int row = ListView_InsertItem(skillsList_, &item);

    if (selectedSkillToken_ == skillModel->getIdentifierToken())
    {
      selectedRow = row;
    }
    
    ++itemIndex;
  }

  if (selectedRow >= 0)
  {
    ListView_SetItemState(skillsList_, selectedRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(skillsList_, selectedRow, FALSE);
    updateSelectedSkillFromList();
  }
  else
  {
    selectedSkillToken_.clear();
    displayedLevel_ = 0;
    ComboBox_ResetContent(levelDropdown_);
    clearFields();
  }
}

void SkillsTabContent::updateSelectedSkillFromList()
{
  const int selectedRow = ListView_GetNextItem(skillsList_, -1, LVNI_SELECTED);
  if (selectedRow < 0)
  {
    selectedSkillToken_.clear();
    displayedLevel_ = 0;
    ComboBox_ResetContent(levelDropdown_);
    clearFields();
    return;
  }

  wchar_t listValueBuffer[256] = {};
  ListView_GetItemText(skillsList_, selectedRow, 0, listValueBuffer, static_cast<int>(std::size(listValueBuffer)));

  selectedSkillToken_ = listValueBuffer;

  const Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
  populateLevelDropdown(skill);

  // Select the first available level, or re-select the previously displayed level
  if (skill)
  {
    const auto levels = skill->getLevels();
    if (!levels.empty())
    {
      // Try to keep the previously displayed level if it is still valid
      bool kept = false;
      if (displayedLevel_ > 0)
      {
        for (int i = 0; i < ComboBox_GetCount(levelDropdown_); ++i)
        {
          if (static_cast<int>(ComboBox_GetItemData(levelDropdown_, i)) == displayedLevel_)
          {
            ComboBox_SetCurSel(levelDropdown_, i);
            kept = true;
            break;
          }
        }
      }
      if (!kept)
      {
        ComboBox_SetCurSel(levelDropdown_, 0);
        displayedLevel_ = static_cast<int>(ComboBox_GetItemData(levelDropdown_, 0));
      }
      loadSkillLevelToFields(skill, displayedLevel_);
      return;
    }
  }

  displayedLevel_ = 0;
  clearFields();
}

void SkillsTabContent::populateLevelDropdown(const Skill* skill)
{
  ComboBox_ResetContent(levelDropdown_);
  if (!skill)
    return;

  for (int lv = Skill::kMinLevel; lv <= Skill::kMaxLevel; ++lv)
  {
    if (!skill->hasLevel(lv))
      continue;
    std::wstring label = L"Level " + std::to_wstring(lv);
    const int idx = ComboBox_AddString(levelDropdown_, label.c_str());
    if (idx >= 0)
      ComboBox_SetItemData(levelDropdown_, idx, static_cast<LPARAM>(lv));
  }
}

void SkillsTabContent::loadSkillLevelToFields(const Skill* skill, int level)
{
  if (!skill || level <= 0)
  {
    clearFields();
    return;
  }

  SetWindowTextW(tokenEdit_, skill->getIdentifierToken().c_str());
  SetWindowTextW(nameEdit_, skill->getName().c_str());
  Button_SetCheck(productionCheck_, skill->isProduction(level) ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(magicCheck_, skill->isMagic() ? BST_CHECKED : BST_UNCHECKED);
  Button_SetCheck(magicFoundationCheck_, skill->isMagicFoundation() ? BST_CHECKED : BST_UNCHECKED);

  const std::wstring productionItemsText = formatStringIntMap(skill->getProductionItems(level));
  SetWindowTextW(productionItemsEdit_, productionItemsText.c_str());

  const std::wstring prerequisitesText = formatPrerequisites(skill->getPrerequisites());
  SetWindowTextW(prerequisitesEdit_, prerequisitesText.c_str());

  SetWindowTextW(descriptionEdit_, skill->getDescription(level).c_str());
  const int studyCost = resolveSkillStudyCost(skill);
  SetWindowTextW(studyCostEdit_, std::to_wstring(studyCost).c_str());
}

void SkillsTabContent::clearFields()
{
  const HWND controls[] = {
    tokenEdit_, nameEdit_, studyCostEdit_, prerequisitesEdit_, productionItemsEdit_, descriptionEdit_
  };

  for (HWND control : controls)
  {
    if (control)
    {
      SetWindowTextW(control, L"");
    }
  }

  Button_SetCheck(productionCheck_, BST_UNCHECKED);
  Button_SetCheck(magicCheck_, BST_UNCHECKED);
  Button_SetCheck(magicFoundationCheck_, BST_UNCHECKED);
}

void SkillsTabContent::saveSelectedSkill()
{
  if (!appData_ || selectedSkillToken_.empty() || displayedLevel_ <= 0)
  {
    return;
  }

  Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
  if (!skill || !skill->hasLevel(displayedLevel_))
  {
    return;
  }

  const std::wstring editedToken = StringUtils::trimWhitespace(getWindowText(tokenEdit_));
  if (!editedToken.empty() && editedToken != selectedSkillToken_)
  {
    MessageBoxW(GetParent(skillsList_),
                L"Skill ID is immutable in this editor. Other fields were saved.",
                L"Skills",
                MB_ICONWARNING | MB_OK);
  }

  skill->setName(getWindowText(nameEdit_));
  skill->setMagicFoundation(Button_GetCheck(magicFoundationCheck_) == BST_CHECKED);
  skill->setMagic(Button_GetCheck(magicCheck_) == BST_CHECKED);
  std::map<std::wstring, int> productionItems = parseStringIntMap(getWindowText(productionItemsEdit_));
  if (Button_GetCheck(productionCheck_) != BST_CHECKED)
  {
    productionItems.clear();
  }
  skill->setProductionItems(displayedLevel_, std::move(productionItems));

  const int studyCostValue = (std::max)(0, StringUtils::parseIntSafe(getWindowText(studyCostEdit_)));
  skill->setStudyCost(studyCostValue);

  skill->setPrerequisites(parsePrerequisites(getWindowText(prerequisitesEdit_)));

  updateSkillsList();
}
