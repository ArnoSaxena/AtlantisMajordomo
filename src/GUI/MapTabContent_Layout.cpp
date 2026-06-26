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
 * File: MapTabContent_Layout.cpp
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

