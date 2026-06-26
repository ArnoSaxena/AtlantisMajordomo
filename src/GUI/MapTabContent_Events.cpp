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

