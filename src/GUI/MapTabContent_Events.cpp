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
 * File: MapTabContent_Events.cpp
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

using OrderParsingUtils::tryExtractOrderKeywordUpper;

namespace
{
const wchar_t* getUnitsListBaseColumnTitle(int columnIndex)
{
  static constexpr const wchar_t* kTitles[] = {
    L"#",
    L"Name",
    L"Faction",
    L"Faction Name",
    L"Structure",
    L"Men",
    L"Silver",
    L"Flags",
    L"Skills",
    L"!",
    L"D"
  };

  if (columnIndex < 0 || columnIndex >= static_cast<int>(std::size(kTitles)))
  {
    return L"";
  }

  return kTitles[columnIndex];
}

struct UnitsListRowSnapshot
{
  std::vector<std::wstring> columns;
  LPARAM itemParam { 0 };
  int originalIndex { 0 };
  bool selected { false };
  bool focused { false };
};

bool tryParseLeadingInteger(const std::wstring& text, long long& value)
{
  std::wstring trimmed = StringUtils::trimWhitespace(text);
  if (trimmed.empty())
  {
    return false;
  }

  std::size_t startIndex = 0;
  while (startIndex < trimmed.size() && !iswdigit(trimmed[startIndex]) && trimmed[startIndex] != L'+' && trimmed[startIndex] != L'-')
  {
    ++startIndex;
  }
  if (startIndex >= trimmed.size())
  {
    return false;
  }

  std::size_t parsedLength = 0;
  try
  {
    value = std::stoll(trimmed.substr(startIndex), &parsedLength);
  }
  catch (...)
  {
    return false;
  }

  return parsedLength > 0;
}

std::wstring getUnitsListSubItemText(HWND listView, int row, int column)
{
  std::wstring buffer(1024, L'\0');
  LVITEMW item {};
  item.iSubItem = column;
  item.pszText = buffer.data();
  item.cchTextMax = static_cast<int>(buffer.size());
  const int copiedLength = static_cast<int>(SendMessageW(
    listView,
    LVM_GETITEMTEXTW,
    static_cast<WPARAM>(row),
    reinterpret_cast<LPARAM>(&item)));
  if (copiedLength <= 0)
  {
    return L"";
  }

  buffer.resize(static_cast<std::size_t>(copiedLength));
  return buffer;
}

int compareUnitsListCellValues(const std::wstring& leftValue,
                               const std::wstring& rightValue,
                               bool ascending)
{
  const std::wstring trimmedLeft = StringUtils::trimWhitespace(leftValue);
  const std::wstring trimmedRight = StringUtils::trimWhitespace(rightValue);

  const bool leftEmpty = trimmedLeft.empty();
  const bool rightEmpty = trimmedRight.empty();
  if (leftEmpty != rightEmpty)
  {
    // Keep empty entries at the bottom in both sort directions.
    return leftEmpty ? 1 : -1;
  }
  if (leftEmpty)
  {
    return 0;
  }

  long long leftNumber = 0;
  long long rightNumber = 0;
  const bool leftHasNumber = tryParseLeadingInteger(trimmedLeft, leftNumber);
  const bool rightHasNumber = tryParseLeadingInteger(trimmedRight, rightNumber);

  if (leftHasNumber && rightHasNumber && leftNumber != rightNumber)
  {
    return ascending ? (leftNumber < rightNumber ? -1 : 1) : (leftNumber > rightNumber ? -1 : 1);
  }

  const int textComparison = _wcsicmp(trimmedLeft.c_str(), trimmedRight.c_str());
  if (textComparison == 0)
  {
    return 0;
  }

  return ascending ? textComparison : -textComparison;
}

void reinsertUnitsListRows(HWND listView,
                           const std::vector<UnitsListRowSnapshot>& rows,
                           int columnCount)
{
  ListView_DeleteAllItems(listView);

  for (int rowIndex = 0; rowIndex < static_cast<int>(rows.size()); ++rowIndex)
  {
    const UnitsListRowSnapshot& row = rows[static_cast<std::size_t>(rowIndex)];

    LVITEMW listItem {};
    listItem.mask = LVIF_TEXT | LVIF_PARAM;
    listItem.iItem = rowIndex;
    listItem.iSubItem = 0;
    listItem.pszText = const_cast<LPWSTR>(row.columns.empty() ? L"" : row.columns[0].c_str());
    listItem.lParam = row.itemParam;
    ListView_InsertItem(listView, &listItem);

    for (int columnIndex = 1; columnIndex < columnCount; ++columnIndex)
    {
      const wchar_t* text = columnIndex < static_cast<int>(row.columns.size())
        ? row.columns[static_cast<std::size_t>(columnIndex)].c_str()
        : L"";
      ListView_SetItemText(listView, rowIndex, columnIndex, const_cast<LPWSTR>(text));
    }

    UINT stateMask = 0;
    UINT stateValue = 0;
    if (row.selected)
    {
      stateMask |= LVIS_SELECTED;
      stateValue |= LVIS_SELECTED;
    }
    if (row.focused)
    {
      stateMask |= LVIS_FOCUSED;
      stateValue |= LVIS_FOCUSED;
    }
    if (stateMask != 0)
    {
      ListView_SetItemState(listView, rowIndex, stateValue, stateMask);
      if (row.selected)
      {
        ListView_EnsureVisible(listView, rowIndex, FALSE);
      }
    }
  }
}
} // namespace

LRESULT CALLBACK MapTabContent::unitsListHeaderSubclassProc(HWND hwnd,
                                                            UINT msg,
                                                            WPARAM wp,
                                                            LPARAM lp,
                                                            UINT_PTR subclassId,
                                                            DWORD_PTR refData)
{
  (void)subclassId;
  (void)wp;

  auto* self = reinterpret_cast<MapTabContent*>(refData);
  if (!self)
  {
    return DefSubclassProc(hwnd, msg, wp, lp);
  }

  if (msg == WM_LBUTTONDBLCLK)
  {
    POINT clientPoint {};
    clientPoint.x = GET_X_LPARAM(lp);
    clientPoint.y = GET_Y_LPARAM(lp);

    HDHITTESTINFO hitInfo {};
    hitInfo.pt = clientPoint;
    const int columnIndex = static_cast<int>(SendMessageW(
      hwnd,
      HDM_HITTEST,
      0,
      reinterpret_cast<LPARAM>(&hitInfo)));
    if (columnIndex >= 0 && (hitInfo.flags & HHT_ONHEADER) != 0)
    {
      self->handleUnitsListHeaderDoubleClickNotify(columnIndex);
      return 0;
    }
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
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
    handleUnitSkillsContextMenuNotify();
    return true;
  }

  if (hdr->hwndFrom == unitWarningsList_ && hdr->code == NM_RCLICK)
  {
    handleUnitWarningsContextMenuNotify();
    return true;
  }

  if ((hdr->hwndFrom == regionResourcesList_
      || hdr->hwndFrom == regionForSaleList_
      || hdr->hwndFrom == regionWantedList_)
      && hdr->code == NM_RCLICK)
  {
    handleRegionItemsContextMenuNotify(hdr->hwndFrom);
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

bool MapTabContent::handleUnitSkillsContextMenuNotify()
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

  AppendMenuW(menu, MF_STRING, kSkillStudyContextCommandId, L"Add Study Order");
  AppendMenuW(menu, MF_STRING, kSkillDescriptionPopupContextCommandId, L"Skill Description");
  AppendMenuW(menu, MF_STRING, kSkillDescriptionListContextCommandId, L"Skill Tab");
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
  else if (selectedCommand == kSkillDescriptionListContextCommandId)
  {
    navigateToSkillList(skillToken);
  }
  else if (selectedCommand == kSkillDescriptionPopupContextCommandId)
  {
    showSkillDescription(skillToken);
  }

  return true;
}

bool MapTabContent::handleUnitWarningsContextMenuNotify()
{
  if (!appData_ || !unitWarningsList_ || selectedUnitNumber_ == 0)
  {
    return true;
  }

  Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
  if (!unit || unit->getWarnings().empty())
  {
    return true;
  }

  POINT screenPoint {};
  const DWORD messagePos = GetMessagePos();
  screenPoint.x = GET_X_LPARAM(messagePos);
  screenPoint.y = GET_Y_LPARAM(messagePos);

  POINT clientPoint = screenPoint;
  ScreenToClient(unitWarningsList_, &clientPoint);
  LVHITTESTINFO hitInfo {};
  hitInfo.pt = clientPoint;
  const int hitRow = ListView_SubItemHitTest(unitWarningsList_, &hitInfo);
  if (hitRow < 0)
  {
    return true;
  }

  ListView_SetItemState(unitWarningsList_, hitRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return true;
  }

  AppendMenuW(menu, MF_STRING, kWarningClearContextCommandId, L"Clear");
  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    unitWarningsList_,
    nullptr);
  DestroyMenu(menu);

  if (selectedCommand == kWarningClearContextCommandId)
  {
    clearWarningsForSelectedUnit();
  }

  return true;
}

bool MapTabContent::handleRegionItemsContextMenuNotify(HWND sourceList)
{
  if (!appData_ || sourceList == nullptr)
  {
    return true;
  }

  POINT screenPoint {};
  const DWORD messagePos = GetMessagePos();
  screenPoint.x = GET_X_LPARAM(messagePos);
  screenPoint.y = GET_Y_LPARAM(messagePos);

  POINT clientPoint = screenPoint;
  ScreenToClient(sourceList, &clientPoint);
  LVHITTESTINFO hitInfo {};
  hitInfo.pt = clientPoint;
  const int hitRow = ListView_SubItemHitTest(sourceList, &hitInfo);
  if (hitRow < 0)
  {
    return true;
  }

  wchar_t tokenBuffer[256] = {};
  LVITEMW tokenItem {};
  tokenItem.iSubItem = 0;
  tokenItem.pszText = tokenBuffer;
  tokenItem.cchTextMax = static_cast<int>(std::size(tokenBuffer));
  const int copiedLength = static_cast<int>(SendMessageW(
    sourceList,
    LVM_GETITEMTEXTW,
    static_cast<WPARAM>(hitRow),
    reinterpret_cast<LPARAM>(&tokenItem)));
  if (copiedLength <= 0)
  {
    return true;
  }

  std::wstring itemToken = StringUtils::trimWhitespace(
    std::wstring(tokenBuffer, static_cast<std::size_t>(copiedLength)));
  if (itemToken.empty())
  {
    return true;
  }

  const std::wstring itemTokenUpper = StringUtils::toUpper(itemToken);
  const Item* item = appData_->itemRepository().findByIdentifierToken(itemTokenUpper);
  if (!item)
  {
    return true;
  }

  ListView_SetItemState(sourceList, hitRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return true;
  }

  AppendMenuW(menu, MF_STRING, kRegionItemDescriptionContextCommandId, L"Item Description");
  AppendMenuW(menu, MF_STRING, kRegionItemTabContextCommandId, L"Items Tab");
  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    sourceList,
    nullptr);
  DestroyMenu(menu);

  if (selectedCommand == kRegionItemDescriptionContextCommandId)
  {
    showItemDescription(item->getIdentifierToken());
  }
  else if (selectedCommand == kRegionItemTabContextCommandId)
  {
    navigateToItemList(item->getIdentifierToken());
  }

  return true;
}

void MapTabContent::sortUnitsListByColumn(int columnIndex, bool ascending)
{
  if (!unitsList_ || columnIndex < 0)
  {
    return;
  }

  HWND header = ListView_GetHeader(unitsList_);
  if (!header)
  {
    return;
  }

  const int columnCount = Header_GetItemCount(header);
  if (columnIndex >= columnCount)
  {
    return;
  }

  const int rowCount = ListView_GetItemCount(unitsList_);
  if (rowCount <= 1)
  {
    return;
  }

  std::vector<UnitsListRowSnapshot> rows;
  rows.reserve(static_cast<std::size_t>(rowCount));
  for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
  {
    UnitsListRowSnapshot row {};
    row.columns.resize(static_cast<std::size_t>(columnCount));
    row.originalIndex = rowIndex;

    for (int currentColumn = 0; currentColumn < columnCount; ++currentColumn)
    {
      row.columns[static_cast<std::size_t>(currentColumn)] =
        getUnitsListSubItemText(unitsList_, rowIndex, currentColumn);
    }

    LVITEMW item {};
    item.mask = LVIF_PARAM;
    item.iItem = rowIndex;
    item.iSubItem = 0;
    if (ListView_GetItem(unitsList_, &item))
    {
      row.itemParam = item.lParam;
    }

    const UINT state = ListView_GetItemState(unitsList_, rowIndex, LVIS_SELECTED | LVIS_FOCUSED);
    row.selected = (state & LVIS_SELECTED) != 0;
    row.focused = (state & LVIS_FOCUSED) != 0;
    rows.push_back(std::move(row));
  }

  std::stable_sort(rows.begin(), rows.end(), [columnIndex, ascending](const UnitsListRowSnapshot& left,
                                                                       const UnitsListRowSnapshot& right)
  {
    const int comparison = compareUnitsListCellValues(
      left.columns[static_cast<std::size_t>(columnIndex)],
      right.columns[static_cast<std::size_t>(columnIndex)],
      ascending);
    if (comparison != 0)
    {
      return comparison < 0;
    }
    return left.originalIndex < right.originalIndex;
  });

  reinsertUnitsListRows(unitsList_, rows, columnCount);
}

void MapTabContent::updateUnitsListSortHeaderMarkers()
{
  if (!unitsList_)
  {
    return;
  }

  HWND header = ListView_GetHeader(unitsList_);
  if (!header)
  {
    return;
  }

  const int columnCount = Header_GetItemCount(header);
  for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
  {
    const bool isActive = (columnIndex == unitsListSortColumn_);
    const wchar_t* baseTitle = getUnitsListBaseColumnTitle(columnIndex);
    std::wstring displayTitle = baseTitle;
    if (isActive)
    {
      displayTitle += unitsListSortAscending_ ? L" ^" : L" v";
    }

    HDITEMW textItem {};
    textItem.mask = HDI_TEXT;
    textItem.pszText = const_cast<LPWSTR>(displayTitle.c_str());
    SendMessageW(header,
                 HDM_SETITEMW,
                 static_cast<WPARAM>(columnIndex),
                 reinterpret_cast<LPARAM>(&textItem));

#if defined(HDF_SORTUP) && defined(HDF_SORTDOWN)
    HDITEMW headerItem {};
    headerItem.mask = HDI_FORMAT;
    if (SendMessageW(header,
                     HDM_GETITEMW,
                     static_cast<WPARAM>(columnIndex),
                     reinterpret_cast<LPARAM>(&headerItem)) == FALSE)
    {
      continue;
    }

    headerItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    if (columnIndex == unitsListSortColumn_)
    {
      headerItem.fmt |= unitsListSortAscending_ ? HDF_SORTUP : HDF_SORTDOWN;
    }

    SendMessageW(header,
                 HDM_SETITEMW,
                 static_cast<WPARAM>(columnIndex),
                 reinterpret_cast<LPARAM>(&headerItem));
#endif
  }
}

bool MapTabContent::handleUnitsListHeaderDoubleClickNotify(int forcedColumnIndex)
{
  if (!unitsList_)
  {
    return true;
  }

  HWND header = ListView_GetHeader(unitsList_);
  if (!header)
  {
    return true;
  }

  int columnIndex = forcedColumnIndex;
  if (columnIndex < 0)
  {
    POINT screenPoint {};
    const DWORD messagePos = GetMessagePos();
    screenPoint.x = GET_X_LPARAM(messagePos);
    screenPoint.y = GET_Y_LPARAM(messagePos);

    POINT clientPoint = screenPoint;
    ScreenToClient(header, &clientPoint);

    HDHITTESTINFO hitInfo {};
    hitInfo.pt = clientPoint;
    columnIndex = static_cast<int>(SendMessageW(
      header,
      HDM_HITTEST,
      0,
      reinterpret_cast<LPARAM>(&hitInfo)));
    if (columnIndex < 0 || (hitInfo.flags & HHT_ONHEADER) == 0)
    {
      return true;
    }
  }

  if (unitsListSortColumn_ != columnIndex)
  {
    unitsListSortColumn_ = columnIndex;
    unitsListSortAscending_ = true;
  }
  else
  {
    unitsListSortAscending_ = !unitsListSortAscending_;
  }

  updateUnitsListSortHeaderMarkers();
  sortUnitsListByColumn(unitsListSortColumn_, unitsListSortAscending_);
  updateSelectedUnitFromList();
  return true;
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
  if (commandId == static_cast<int>(kWarningClearContextCommandId))
  {
    clearWarningsForSelectedUnit();
    return true;
  }

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

