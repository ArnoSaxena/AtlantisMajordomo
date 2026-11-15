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
 * File: MapTabContent_MapCanvas.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GUI/MapTabContent.hpp"
#include "GUI/MapTabContent_private.hpp"
#include "Function/MapUtils.hpp"

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
#include "Data/Unit.hpp"
#include "Data/UnitRepository.hpp"
#include "GUI/ControlIds.hpp"
#include "GUI/OrdersEditorUtils.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/CoordinateUtils.hpp"
#include "Function/MonthUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
#include "Function/ReadOnlyPopupService.hpp"
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
#include <utility>
#include <vector>
#include <windowsx.h>

namespace
{
LRESULT CALLBACK readOnlyTextPopupWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_CREATE:
    {
      const auto* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lp);
      //const auto* initialText = static_cast<const std::wstring*>(createStruct->lpCreateParams);

      // We pass a small struct via lpCreateParams so we can also get the wrap flag.
      // (See showReadOnlyTextPopup below.)
      struct CreateData
      {
        const std::wstring* text;
        bool softWrap;
      };

      auto* data = static_cast<CreateData*>(createStruct->lpCreateParams);

      HWND edit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        data && data->text ? data->text->c_str() : L"",
        WS_CHILD | WS_VISIBLE |
        ES_LEFT | ES_MULTILINE | ES_READONLY |
        (data && data->softWrap ? (ES_WANTRETURN) : 0) |
        WS_VSCROLL |
        (data && data->softWrap ? 0 : (ES_AUTOHSCROLL | WS_HSCROLL)),
        10, 10,
        100, 100,
        hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kReadOnlyTextPopupEditId)),
        GetModuleHandleW(nullptr),
        nullptr
      );

      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(edit));

      if (data)
        delete data;

      return 0;
    }

    case WM_SIZE:
    {
      HWND edit = reinterpret_cast<HWND>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      if (edit)
      {
        const int width = (std::max)(0, LOWORD(lp) - 20);
        const int height = (std::max)(0, HIWORD(lp) - 20);
        MoveWindow(edit, 10, 10, width, height, TRUE);
      }
      return 0;
    }

    case WM_SETFOCUS:
    {
      HWND edit = reinterpret_cast<HWND>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      if (edit)
      {
        SetFocus(edit);
        return 0;
      }
      break;
    }

    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_NCDESTROY:
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      return 0;
  }

  return DefWindowProcW(hwnd, msg, wp, lp);
}


void showReadOnlyTextPopup(HWND ownerWindow, const std::wstring& windowTitle, const std::wstring& text, bool softWrap = false)
{
  HINSTANCE instance = GetModuleHandleW(nullptr);

  static bool popupClassRegistered = false;
  if (!popupClassRegistered)
  {
    WNDCLASSEXW popupClass {};
    popupClass.cbSize = sizeof(popupClass);
    popupClass.lpfnWndProc = readOnlyTextPopupWndProc;
    popupClass.hInstance = instance;
    popupClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    popupClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    popupClass.lpszClassName = kReadOnlyTextPopupClassName;

    if (!RegisterClassExW(&popupClass))
    {
      return;
    }

    popupClassRegistered = true;
  }

  constexpr int clientWidth = 760;
  constexpr int clientHeight = 440;

  RECT windowRect { 0, 0, clientWidth, clientHeight };
  const DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  AdjustWindowRectEx(&windowRect, windowStyle, FALSE, WS_EX_DLGMODALFRAME);

  const int windowWidth = windowRect.right - windowRect.left;
  const int windowHeight = windowRect.bottom - windowRect.top;

  RECT ownerRect { 0, 0, 0, 0 };
  if (ownerWindow)
  {
    GetWindowRect(ownerWindow, &ownerRect);
  }
  else
  {
    ownerRect.right = GetSystemMetrics(SM_CXSCREEN);
    ownerRect.bottom = GetSystemMetrics(SM_CYSCREEN);
  }

  const int x = ownerRect.left + ((ownerRect.right - ownerRect.left) - windowWidth) / 2;
  const int y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - windowHeight) / 2;

  struct CreateData
  {
    const std::wstring* text;
    bool softWrap;
  };
  auto* data = new CreateData{ &text, softWrap };

  HWND popup = CreateWindowExW(
    WS_EX_DLGMODALFRAME,
    kReadOnlyTextPopupClassName,
    windowTitle.c_str(),
    windowStyle,
    x,
    y,
    windowWidth,
    windowHeight,
    ownerWindow,
    nullptr,
    instance,
    data
    //const_cast<std::wstring*>(&text)
  );

  if (!popup)
  {
    return;
  }

  ShowWindow(popup, SW_SHOW);
  UpdateWindow(popup);
}

class Win32ReadOnlyPopupBackend : public ReadOnlyPopupService::IBackend
{
public:
  explicit Win32ReadOnlyPopupBackend(HWND ownerWindow)
    : ownerWindow_(ownerWindow)
  {
  }

  void show(const ReadOnlyPopupService::PopupRequest& request) override
  {
    showReadOnlyTextPopup(ownerWindow_, request.title, request.text, request.softWrap);
  }

private:
  HWND ownerWindow_ { nullptr };
};

struct GuardWedgeStyle
{
  std::array<int, 3> color { 128, 128, 128 };
  bool guardPresent { false };
};

GuardWedgeStyle resolveGuardWedgeStyle(const AppData& appData,
                                       const AppConfig& appConfig,
                                       const Region& region)
{
  const auto& factionRepository = appData.factionRepository();
  const Faction* mainFaction = factionRepository.getMainFaction();
  const int mainFactionNumber = factionRepository.getMainFactionNumber();
  bool hasOtherGuard = false;
  bool hasUnfriendlyGuard = false;
  bool hasFriendlyGuard = false;

  const auto& units = appData.unitRepository();
  for (std::size_t unitIndex = 0; unitIndex < units.size(); ++unitIndex)
  {
    const Unit& unit = units.at(unitIndex);
    if (!unit.isOnGuard() ||
        unit.getXCoordinate() != region.getXCoordinate() ||
        unit.getYCoordinate() != region.getYCoordinate() ||
        unit.getZCoordinate() != region.getZCoordinate())
    {
      continue;
    }

    if (mainFactionNumber > 0 && unit.getFactionNumber() == mainFactionNumber)
    {
      return GuardWedgeStyle { appConfig.getGuardedRegionBorderColor(), true };
    }

    hasOtherGuard = true;
    if (!mainFaction)
    {
      continue;
    }

    const auto& declaredAttitudes = mainFaction->getDeclaredAttitudes();
    const auto attitude = declaredAttitudes.find(unit.getFactionNumber());
    const Faction::Attitude effectiveAttitude = attitude != declaredAttitudes.end()
      ? attitude->second
      : mainFaction->getDefaultAttitude();
    
    if (effectiveAttitude == Faction::Attitude::Hostile ||
        effectiveAttitude == Faction::Attitude::Unfriendly)
    {
      hasUnfriendlyGuard = true;
    }
    if (effectiveAttitude == Faction::Attitude::Friendly ||
        effectiveAttitude == Faction::Attitude::Ally)
    {
      hasFriendlyGuard = true;
    }
  }

  if (hasUnfriendlyGuard)
  {
    return GuardWedgeStyle { { 255, 0, 0 }, true };
  }
  if (hasOtherGuard)
  {
    return GuardWedgeStyle { { 128, 128, 128 }, true };
  }
  if (hasFriendlyGuard)
  {
    return GuardWedgeStyle { { 0, 255, 0 }, true };
  }
  return GuardWedgeStyle { { 128, 128, 128 }, false };
}

} // namespace


void MapTabContent::showZSelectionContextMenu(HWND ownerWindow, POINT screenPoint)
{
  if (!appData_)
  {
    return;
  }

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  if (availableZLevels_.empty())
  {
    AppendMenuW(menu, MF_STRING | MF_GRAYED, kZContextMenuBaseId, L"No Z levels");
  }
  else
  {
    for (std::size_t index = 0; index < availableZLevels_.size(); ++index)
    {
      const UINT flags = MF_STRING | (availableZLevels_[index] == selectedZ_ ? MF_CHECKED : 0);
      const UINT id = static_cast<UINT>(kZContextMenuBaseId + static_cast<int>(index));
      const std::wstring text = L"Z = " + std::to_wstring(availableZLevels_[index]);
      AppendMenuW(menu, flags, id, text.c_str());
    }
  }

  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    ownerWindow,
    nullptr
  );

  DestroyMenu(menu);

  if (selectedCommand < static_cast<UINT>(kZContextMenuBaseId))
  {
    return;
  }

  const int selectedIndex = static_cast<int>(selectedCommand) - kZContextMenuBaseId;
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(availableZLevels_.size()))
  {
    return;
  }

  selectedZ_ = availableZLevels_[static_cast<std::size_t>(selectedIndex)];
  hasSelectedRegion_ = false;
  refresh();
}


LRESULT CALLBACK MapTabContent::mapCanvasWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  if (msg == WM_NCCREATE)
  {
    const auto* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lp);
    auto* self = static_cast<MapTabContent*>(createStruct->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }

  auto* self = reinterpret_cast<MapTabContent*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self)
  {
    return DefWindowProcW(hwnd, msg, wp, lp);
  }

  return self->handleMapCanvasMessage(hwnd, msg, wp, lp);
}

LRESULT MapTabContent::handleMapCanvasMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_SIZE:
      updateMapScrollbars();
      return 0;

    case WM_HSCROLL:
    {
      SCROLLINFO info {};
      info.cbSize = sizeof(info);
      info.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_HORZ, &info);
      int nextPosition = info.nPos;

      switch (LOWORD(wp))
      {
        case SB_LINELEFT:   nextPosition -= 20; break;
        case SB_LINERIGHT:  nextPosition += 20; break;
        case SB_PAGELEFT:   nextPosition -= static_cast<int>(info.nPage); break;
        case SB_PAGERIGHT:  nextPosition += static_cast<int>(info.nPage); break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: nextPosition = HIWORD(wp); break;
        default: break;
      }

      nextPosition = (std::max)(info.nMin, (std::min)(nextPosition, info.nMax - static_cast<int>(info.nPage) + 1));
      if (nextPosition != info.nPos)
      {
        info.fMask = SIF_POS;
        info.nPos = nextPosition;
        SetScrollInfo(hwnd, SB_HORZ, &info, TRUE);
        scrollX_ = nextPosition;
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_VSCROLL:
    {
      SCROLLINFO info {};
      info.cbSize = sizeof(info);
      info.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_VERT, &info);
      int nextPosition = info.nPos;

      switch (LOWORD(wp))
      {
        case SB_LINEUP:     nextPosition -= 20; break;
        case SB_LINEDOWN:   nextPosition += 20; break;
        case SB_PAGEUP:     nextPosition -= static_cast<int>(info.nPage); break;
        case SB_PAGEDOWN:   nextPosition += static_cast<int>(info.nPage); break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: nextPosition = HIWORD(wp); break;
        default: break;
      }

      nextPosition = (std::max)(info.nMin, (std::min)(nextPosition, info.nMax - static_cast<int>(info.nPage) + 1));
      if (nextPosition != info.nPos)
      {
        info.fMask = SIF_POS;
        info.nPos = nextPosition;
        SetScrollInfo(hwnd, SB_VERT, &info, TRUE);
        scrollY_ = nextPosition;
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_LBUTTONDOWN:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };
      onMapLeftClick(cursorPoint);
      return 0;
    }

    case WM_MOUSEMOVE:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };

      if (!trackingMouseLeave_)
      {
        TRACKMOUSEEVENT trackEvent {};
        trackEvent.cbSize = sizeof(trackEvent);
        trackEvent.dwFlags = TME_LEAVE;
        trackEvent.hwndTrack = hwnd;
        if (TrackMouseEvent(&trackEvent) != FALSE)
        {
          trackingMouseLeave_ = true;
        }
      }

      const RegionVisual* region = hitTestRegion(cursorPoint);
      if (region && region->region)
      {
        updateHoverTooltip(cursorPoint, *(region->region));
      }
      else
      {
        int xCoordinate = 0;
        int yCoordinate = 0;
        if (hitTestMapCoordinate(cursorPoint, xCoordinate, yCoordinate))
        {
          hoverRegionText_ = L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
            xCoordinate,
            yCoordinate,
            selectedZ_);
          SetWindowTextW(hoverRegionLabel_, hoverRegionText_.c_str());
        }
        else
        {
          hideHoverTooltip();
        }
      }
      return 0;
    }

    case WM_MOUSELEAVE:
      trackingMouseLeave_ = false;
      hideHoverTooltip();
      return 0;

    case WM_RBUTTONDOWN:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };
      onMapRightClick(cursorPoint);
      return 0;
    }

    case WM_LBUTTONDBLCLK:
    {
      POINT cursorPoint {
        static_cast<int>(static_cast<short>(LOWORD(lp))),
        static_cast<int>(static_cast<short>(HIWORD(lp)))
      };
      onMapDoubleClick(cursorPoint);
      return 0;
    }

    case WM_PAINT:
    {
      PAINTSTRUCT ps {};
      HDC hdc = BeginPaint(hwnd, &ps);
      paintMap(hdc);
      EndPaint(hwnd, &ps);
      return 0;
    }
  }

  return DefWindowProcW(hwnd, msg, wp, lp);
}


void MapTabContent::recalculateVisibleMap()
{
  visibleRegions_.clear();
  availableZLevels_.clear();
  hasMapBounds_ = false;

  if (!appData_ || !appConfig_)
  {
    return;
  }

  const auto& regionRepository = appData_->regionRepository();
  if (regionRepository.size() == 0)
  {
    contentWidth_ = 0;
    contentHeight_ = 0;
    hasSelectedRegion_ = false;
    return;
  }

  std::set<int> zSet;
  for (std::size_t index = 0; index < regionRepository.size(); ++index)
  {
    zSet.insert(regionRepository.at(index).getZCoordinate());
  }

  availableZLevels_.assign(zSet.begin(), zSet.end());
  if (availableZLevels_.empty())
  {
    selectedZ_ = 1;
  }
  else if (std::find(availableZLevels_.begin(), availableZLevels_.end(), selectedZ_) == availableZLevels_.end())
  {
    selectedZ_ = availableZLevels_.front();
  }

  std::vector<const Region*> zRegions;
  zRegions.reserve(regionRepository.size());

  int minX = 0;
  int maxX = 0;
  int minY = 0;
  int maxY = 0;
  bool firstRegion = true;

  for (std::size_t index = 0; index < regionRepository.size(); ++index)
  {
    const Region& region = regionRepository.at(index);
    if (region.getZCoordinate() != selectedZ_)
    {
      continue;
    }

    zRegions.push_back(&region);
    if (firstRegion)
    {
      minX = maxX = region.getXCoordinate();
      minY = maxY = region.getYCoordinate();
      firstRegion = false;
    }
    else
    {
      minX = (std::min)(minX, region.getXCoordinate());
      maxX = (std::max)(maxX, region.getXCoordinate());
      minY = (std::min)(minY, region.getYCoordinate());
      maxY = (std::max)(maxY, region.getYCoordinate());
    }
  }

  if (zRegions.empty())
  {
    contentWidth_ = 0;
    contentHeight_ = 0;
    hasSelectedRegion_ = false;
    return;
  }

  bool leftRolloverDiscovered = false;
  bool rightRolloverDiscovered = false;
  for (const Region* region : zRegions)
  {
    if (!region)
    {
      continue;
    }

    for (const auto& direction : region->getExitDirections())
    {
      if (HexDirectionUtils::isWestDirection(direction) && region->getXCoordinate() <= minX + 1)
      {
        for (const Region* candidate : zRegions)
        {
          if (!candidate)
          {
            continue;
          }
          if (candidate->getXCoordinate() >= maxX - 1 &&
              std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
          {
            leftRolloverDiscovered = true;
            break;
          }
        }
      }

      if (HexDirectionUtils::isEastDirection(direction) && region->getXCoordinate() >= maxX - 1)
      {
        for (const Region* candidate : zRegions)
        {
          if (!candidate)
          {
            continue;
          }
          if (candidate->getXCoordinate() <= minX + 1 &&
              std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
          {
            rightRolloverDiscovered = true;
            break;
          }
        }
      }
    }
  }

  const int leftPaddingColumns = leftRolloverDiscovered ? 0 : 3;
  const int rightPaddingColumns = rightRolloverDiscovered ? 0 : 3;

  mapMinX_ = minX;
  mapMaxX_ = maxX;
  mapMinY_ = minY;
  mapMaxY_ = maxY;
  mapLeftPaddingColumns_ = leftPaddingColumns;
  mapRightPaddingColumns_ = rightPaddingColumns;
  hasMapBounds_ = true;

  const UiSizeProfile::Metrics uiMetrics = resolveUiMetrics();
  const int hexWidth = resolveScaledMapHexWidth();
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
  const int rowStep = hexHeight;

  int maxCenterY = 0;
  for (const Region* region : zRegions)
  {
    if (!region)
    {
      continue;
    }

    const int mapColumn = (region->getXCoordinate() - minX) + leftPaddingColumns;
    const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

    const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
    const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

    RegionVisual visual;
    visual.region = region;
    visual.center = { centerX, centerY };
    visual.polygon = MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth);
    visibleRegions_.push_back(visual);
    maxCenterY = (std::max)(maxCenterY, centerY);
  }

  // Mirror the right-edge regions into the left padding columns so wrapped map
  // rendering shows real region tiles instead of empty placeholders.
  if (mapLeftPaddingColumns_ > 0)
  {
    for (const Region* region : zRegions)
    {
      if (!region)
      {
        continue;
      }

      const int wrappedLeftX = region->getXCoordinate() - (maxX - minX + 1);
      if (wrappedLeftX < (minX - mapLeftPaddingColumns_) || wrappedLeftX >= minX)
      {
        continue;
      }

      const int mapColumn = (wrappedLeftX - minX) + leftPaddingColumns;
      const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

      const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
      const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

      RegionVisual wrappedVisual;
      wrappedVisual.region = region;
      wrappedVisual.center = { centerX, centerY };
      wrappedVisual.polygon = MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth);
      visibleRegions_.push_back(wrappedVisual);
      maxCenterY = (std::max)(maxCenterY, centerY);
    }
  }

  const int totalColumns = (maxX - minX + 1) + leftPaddingColumns + rightPaddingColumns;
  contentWidth_ = kMargin * 2 + (std::max)(1, totalColumns) * columnStep + hexWidth;
  contentHeight_ = kMargin * 2 + maxCenterY + hexHeight;

  if (hasSelectedRegion_)
  {
    const bool selectedStillVisible = std::any_of(
      visibleRegions_.begin(),
      visibleRegions_.end(),
      [this](const RegionVisual& visual)
      {
        return visual.region != nullptr &&
              visual.region->getXCoordinate() == selectedRegionX_ &&
              visual.region->getYCoordinate() == selectedRegionY_;
      }
    );

    if (!selectedStillVisible)
    {
      hasSelectedRegion_ = false;
    }
  }
}


void MapTabContent::updateMapScrollbars()
{
  if (!mapCanvas_)
  {
    return;
  }

  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);
  const int clientWidthRaw = static_cast<int>(clientRect.right - clientRect.left);
  const int clientHeightRaw = static_cast<int>(clientRect.bottom - clientRect.top);
  const int clientWidth = clientWidthRaw > 0 ? clientWidthRaw : 0;
  const int clientHeight = clientHeightRaw > 0 ? clientHeightRaw : 0;

  const int maxScrollX = (std::max)(0, contentWidth_ - clientWidth);
  const int maxScrollY = (std::max)(0, contentHeight_ - clientHeight);

  scrollX_ = (std::min)(scrollX_, maxScrollX);
  scrollY_ = (std::min)(scrollY_, maxScrollY);

  SCROLLINFO horizontal {};
  horizontal.cbSize = sizeof(horizontal);
  horizontal.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  horizontal.nMin = 0;
  horizontal.nMax = contentWidth_ > 0 ? contentWidth_ - 1 : 0;
  horizontal.nPage = static_cast<UINT>(clientWidth);
  horizontal.nPos = scrollX_;
  SetScrollInfo(mapCanvas_, SB_HORZ, &horizontal, TRUE);

  SCROLLINFO vertical {};
  vertical.cbSize = sizeof(vertical);
  vertical.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  vertical.nMin = 0;
  vertical.nMax = contentHeight_ > 0 ? contentHeight_ - 1 : 0;
  vertical.nPage = static_cast<UINT>(clientHeight);
  vertical.nPos = scrollY_;
  SetScrollInfo(mapCanvas_, SB_VERT, &vertical, TRUE);
}


void MapTabContent::paintMap(HDC hdc) const
{
  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);

  const int paintWidthRaw = static_cast<int>(clientRect.right - clientRect.left);
  const int paintHeightRaw = static_cast<int>(clientRect.bottom - clientRect.top);
  const int paintWidth = paintWidthRaw > 0 ? paintWidthRaw : 1;
  const int paintHeight = paintHeightRaw > 0 ? paintHeightRaw : 1;

  HDC memoryDc = CreateCompatibleDC(hdc);
  HBITMAP bitmap = CreateCompatibleBitmap(hdc, paintWidth, paintHeight);
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

  FillRect(memoryDc, &clientRect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

  HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
  HGDIOBJ oldPen = SelectObject(memoryDc, borderPen);

  drawEmptyHexes(memoryDc);

  // Use the latest REPORT period (not latest battle period) so markers match the displayed data.
  int latestBattlePeriodMonth = 0;
  int latestBattlePeriodYear  = 0;
  if (appData_)
  {
    const auto& rr = appData_->reportRepository();
    for (std::size_t i = 0; i < rr.size(); ++i)
    {
      const Report& rep = rr.at(i);
      const int rm = rep.getMonth();
      const int repYear = rep.getYear();
      if (rm >= 1 && rm <= 12 && repYear > 0)
      {
        if (repYear > latestBattlePeriodYear ||
            (repYear == latestBattlePeriodYear && rm > latestBattlePeriodMonth))
        {
          latestBattlePeriodMonth = rm;
          latestBattlePeriodYear  = repYear;
        }
      }
    }
  }
  const bool hasLatestBattlePeriod =
    latestBattlePeriodMonth >= 1 && latestBattlePeriodMonth <= 12 && latestBattlePeriodYear > 0;

  drawRegionHexes(memoryDc, hasLatestBattlePeriod, latestBattlePeriodMonth, latestBattlePeriodYear);

  SelectObject(memoryDc, oldPen);
  DeleteObject(borderPen);

  drawSelectedRegionBorder(memoryDc);

  drawMovePath(memoryDc);

  drawStructureMarkers(memoryDc);

  drawMissingExitEdges(memoryDc);

  BitBlt(
    hdc,
    0,
    0,
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
    memoryDc,
    0,
    0,
    SRCCOPY
  );

  SelectObject(memoryDc, oldBitmap);
  DeleteObject(bitmap);
  DeleteDC(memoryDc);
}

void MapTabContent::drawMovePath(HDC memoryDc) const
{
  if (movePathCoordinates_.size() < 2)
  {
    return;
  }

  const COLORREF defaultColor = movePathIsSail_ ? RGB(173, 216, 230) : RGB(144, 238, 144);
  const COLORREF arrowColor = movePathHasNegativeCapacity_ ? RGB(255, 0, 0) : defaultColor;
  const COLORREF borderColor = RGB(0, 0, 0);
  HPEN arrowPen = CreatePen(PS_SOLID, 4, arrowColor);
  HPEN arrowBorderPen = CreatePen(PS_SOLID, 8, borderColor);
  HPEN arrowTipBorderPen = CreatePen(PS_SOLID, 2, borderColor);
  HBRUSH arrowBrush = CreateSolidBrush(arrowColor);

  for (std::size_t index = 0; index + 1 < movePathCoordinates_.size(); ++index)
  {
    const int startX = movePathCoordinates_[index].first;
    const int startY = movePathCoordinates_[index].second;
    const int endX = movePathCoordinates_[index + 1].first;
    const int endY = movePathCoordinates_[index + 1].second;
    POINT startCenter { 0, 0 };
    POINT endCenter { 0, 0 };
    for (const RegionVisual& visual : visibleRegions_)
    {
      if (visual.region && visual.region->getXCoordinate() == startX && visual.region->getYCoordinate() == startY)
      {
        startCenter = visual.center;
      }
      if (visual.region && visual.region->getXCoordinate() == endX && visual.region->getYCoordinate() == endY)
      {
        endCenter = visual.center;
      }
    }
    if ((startCenter.x == 0 && startCenter.y == 0) || (endCenter.x == 0 && endCenter.y == 0))
    {
      continue;
    }

    startCenter.x -= scrollX_;
    startCenter.y -= scrollY_;
    endCenter.x -= scrollX_;
    endCenter.y -= scrollY_;
    const double dx = endCenter.x - startCenter.x;
    const double dy = endCenter.y - startCenter.y;
    const double length = std::sqrt(dx * dx + dy * dy);
    int startLineX = startCenter.x;
    int startLineY = startCenter.y;
    int endLineX = endCenter.x;
    int endLineY = endCenter.y;
    if (length > 0.0)
    {
      const double ux = dx / length;
      const double uy = dy / length;
      const double shortening = (std::max)(1.0, length * 0.25);
      startLineX = static_cast<int>(std::lround(startCenter.x + ux * shortening));
      startLineY = static_cast<int>(std::lround(startCenter.y + uy * shortening));
      endLineX = static_cast<int>(std::lround(endCenter.x - ux * shortening));
      endLineY = static_cast<int>(std::lround(endCenter.y - uy * shortening));
    }

    int shaftEndX = endLineX;
    int shaftEndY = endLineY;
    if (length > 0.0)
    {
      shaftEndX = static_cast<int>(std::lround(endLineX - dx / length * 10.0));
      shaftEndY = static_cast<int>(std::lround(endLineY - dy / length * 10.0));
    }
    HGDIOBJ oldPen = SelectObject(memoryDc, arrowBorderPen);
    MoveToEx(memoryDc, startLineX, startLineY, nullptr);
    LineTo(memoryDc, shaftEndX, shaftEndY);
    SelectObject(memoryDc, arrowPen);
    MoveToEx(memoryDc, startLineX, startLineY, nullptr);
    LineTo(memoryDc, shaftEndX, shaftEndY);
    SelectObject(memoryDc, oldPen);

    if (length > 0.0)
    {
      const double ux = dx / length;
      const double uy = dy / length;
      constexpr double arrowLength = 10.0;
      constexpr double arrowWidth = 6.0;
      const POINT tip { endLineX, endLineY };
      const POINT baseOne {
        static_cast<int>(endLineX - ux * arrowLength + uy * arrowWidth),
        static_cast<int>(endLineY - uy * arrowLength - ux * arrowWidth)
      };
      const POINT baseTwo {
        static_cast<int>(endLineX - ux * arrowLength - uy * arrowWidth),
        static_cast<int>(endLineY - uy * arrowLength + ux * arrowWidth)
      };
      const POINT points[] = { tip, baseOne, baseTwo };
      HGDIOBJ oldBrush = SelectObject(memoryDc, arrowBrush);
      oldPen = SelectObject(memoryDc, arrowTipBorderPen);
      Polygon(memoryDc, points, 3);
      SelectObject(memoryDc, oldPen);
      SelectObject(memoryDc, oldBrush);
    }
  }

  DeleteObject(arrowPen);
  DeleteObject(arrowBorderPen);
  DeleteObject(arrowTipBorderPen);
  DeleteObject(arrowBrush);
}

void MapTabContent::drawSelectedRegionBorder(HDC memoryDc) const
{
  if (!hasSelectedRegion_ || !appConfig_)
  {
    return;
  }

  const std::array<int, 3> color = appConfig_->getSelectedRegionBorderColor();
  HPEN selectedPen = CreatePen(PS_SOLID, 3, RGB(color[0], color[1], color[2]));
  HGDIOBJ oldPen = SelectObject(memoryDc, selectedPen);
  HGDIOBJ oldBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
  for (const RegionVisual& visual : visibleRegions_)
  {
    if (!visual.region || visual.region->getXCoordinate() != selectedRegionX_ ||
        visual.region->getYCoordinate() != selectedRegionY_)
    {
      continue;
    }
    std::array<POINT, 6> translated = visual.polygon;
    for (POINT& point : translated)
    {
      point.x -= scrollX_;
      point.y -= scrollY_;
    }
    Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
    break;
  }
  SelectObject(memoryDc, oldBrush);
  SelectObject(memoryDc, oldPen);
  DeleteObject(selectedPen);
}

void MapTabContent::drawStructureMarkers(HDC memoryDc) const
{
  if (!appData_ || !appConfig_)
  {
    return;
  }

  const std::array<int, 3> markerRgb = appConfig_->getStructureMarkerColor();
  const COLORREF defaultMarkerColor = RGB(std::clamp(markerRgb[0], 0, 255),
                                          std::clamp(markerRgb[1], 0, 255),
                                          std::clamp(markerRgb[2], 0, 255));
  const COLORREF caravanseraiColor = RGB(255, 165, 0);
  const COLORREF shipColor = RGB(173, 216, 230);

  for (const RegionVisual& visual : visibleRegions_)
  {
    if (!visual.region)
    {
      continue;
    }

    const auto structures = appData_->structureRepository().findByCoordinates(
      visual.region->getXCoordinate(), visual.region->getYCoordinate(), visual.region->getZCoordinate());
    bool hasRegionMarker = false;
    bool hasShip = false;
    bool hasFlyingShip = false;
    for (const Structure* structure : structures)
    {
      if (!structure)
      {
        continue;
      }
      const bool isRoad = !HexDirectionUtils::extractRoadDirectionFromStructure(*structure).empty();
      const StructInfo* info = appData_->structInfoRepository().findByType(structure->getStructureType());
      const bool isShip = info && info->isShip();
      hasShip = hasShip || isShip;
      hasFlyingShip = hasFlyingShip || (isShip && structure->isFlying());
      hasRegionMarker = hasRegionMarker || (!isRoad && !isShip);
    }
    if (!hasRegionMarker && !hasShip)
    {
      continue;
    }

    const bool hasCaravanserai = !appData_->structureRepository().findByCoordinatesAndType(
      visual.region->getXCoordinate(), visual.region->getYCoordinate(), visual.region->getZCoordinate(),
      L"caravanserai").empty();
    const bool hasShaft = !appData_->structureRepository().findByCoordinatesAndType(
      visual.region->getXCoordinate(), visual.region->getYCoordinate(), visual.region->getZCoordinate(),
      L"shaft").empty();

    const POINT north = MapUtils::getRoadEndpointForDirection(visual.polygon, L"N");
    const POINT northWest = MapUtils::getRoadEndpointForDirection(visual.polygon, L"NW");
    const POINT south = MapUtils::getRoadEndpointForDirection(visual.polygon, L"S");
    const POINT southWest = MapUtils::getRoadEndpointForDirection(visual.polygon, L"SW");
    const POINT topMidpoint { (north.x + northWest.x) / 2, (north.y + northWest.y) / 2 };
    const POINT bottomMidpoint { (south.x + southWest.x) / 2, (south.y + southWest.y) / 2 };
    const int markerX = topMidpoint.x + (visual.center.x - topMidpoint.x) / 4 - scrollX_;
    const int markerY = topMidpoint.y + (visual.center.y - topMidpoint.y) / 4 - scrollY_;
    const int shipX = bottomMidpoint.x + (visual.center.x - bottomMidpoint.x) / 4 - scrollX_;
    const int shipY = bottomMidpoint.y + (visual.center.y - bottomMidpoint.y) / 4 - scrollY_;
    constexpr int markerRadius = 2;

    if (hasRegionMarker)
    {
      HBRUSH brush = CreateSolidBrush(hasCaravanserai ? caravanseraiColor : defaultMarkerColor);
      HGDIOBJ oldPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));
      HGDIOBJ oldBrush = SelectObject(memoryDc, brush);
      Ellipse(memoryDc, markerX - markerRadius, markerY - markerRadius,
              markerX + markerRadius + 1, markerY + markerRadius + 1);
      SelectObject(memoryDc, oldBrush);
      SelectObject(memoryDc, oldPen);
      DeleteObject(brush);

      if (hasShaft)
      {
        HPEN shaftPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
        oldPen = SelectObject(memoryDc, shaftPen);
        oldBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        Ellipse(memoryDc, markerX - markerRadius, markerY - markerRadius,
                markerX + markerRadius + 1, markerY + markerRadius + 1);
        SelectObject(memoryDc, oldBrush);
        SelectObject(memoryDc, oldPen);
        DeleteObject(shaftPen);
      }
    }

    if (hasShip)
    {
      HBRUSH brush = CreateSolidBrush(shipColor);
      HPEN shipPen = hasFlyingShip ? CreatePen(PS_SOLID, 1, RGB(0, 0, 0)) : nullptr;
      HGDIOBJ penObject = shipPen ? static_cast<HGDIOBJ>(shipPen) : GetStockObject(NULL_PEN);
      HGDIOBJ oldPen = SelectObject(memoryDc, penObject);
      HGDIOBJ oldBrush = SelectObject(memoryDc, brush);
      Ellipse(memoryDc, shipX - markerRadius, shipY - markerRadius,
              shipX + markerRadius + 1, shipY + markerRadius + 1);
      SelectObject(memoryDc, oldBrush);
      SelectObject(memoryDc, oldPen);
      DeleteObject(brush);
      if (shipPen)
      {
        DeleteObject(shipPen);
      }
    }
  }
}

void MapTabContent::drawMissingExitEdges(HDC memoryDc) const
{
  if (!appData_)
  {
    return;
  }

  constexpr std::array<const wchar_t*, 6> directions = { L"NW", L"N", L"NE", L"SE", L"S", L"SW" };
  constexpr std::array<std::pair<std::size_t, std::size_t>, 6> edgeVertices = {
    std::pair<std::size_t, std::size_t>{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 0 }
  };
  HPEN missingExitPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
  HGDIOBJ oldPen = SelectObject(memoryDc, missingExitPen);
  for (const RegionVisual& visual : visibleRegions_)
  {
    if (!visual.region || !visual.region->getVisited())
    {
      continue;
    }
    std::set<std::wstring> availableDirections;
    for (const std::wstring& direction : visual.region->getExitDirections())
    {
      const std::wstring normalized = HexDirectionUtils::normalizeHexDirection(direction);
      if (!normalized.empty())
      {
        availableDirections.insert(normalized);
      }
    }
    for (std::size_t index = 0; index < directions.size(); ++index)
    {
      if (availableDirections.find(directions[index]) != availableDirections.end())
      {
        continue;
      }
      const auto [firstVertex, secondVertex] = edgeVertices[index];
      MoveToEx(memoryDc, visual.polygon[firstVertex].x - scrollX_,
               visual.polygon[firstVertex].y - scrollY_, nullptr);
      LineTo(memoryDc, visual.polygon[secondVertex].x - scrollX_,
             visual.polygon[secondVertex].y - scrollY_);
    }
  }
  SelectObject(memoryDc, oldPen);
  DeleteObject(missingExitPen);
}

void MapTabContent::drawEmptyHexes(HDC memoryDc) const
{
  if (!appConfig_ || !hasMapBounds_)
  {
    return;
  }

  std::set<std::pair<int, int>> occupiedCoordinates;
  int coordinateParity = 0;
  bool hasCoordinateParity = false;
  for (const RegionVisual& visual : visibleRegions_)
  {
    if (!visual.region)
    {
      continue;
    }
    const int xCoordinate = visual.region->getXCoordinate();
    const int yCoordinate = visual.region->getYCoordinate();
    occupiedCoordinates.emplace(xCoordinate, yCoordinate);
    if (!hasCoordinateParity)
    {
      coordinateParity = (xCoordinate + yCoordinate) & 1;
      hasCoordinateParity = true;
    }
  }

  const int hexWidth = resolveScaledMapHexWidth();
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
  HPEN emptyHexPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
  HGDIOBJ oldPen = SelectObject(memoryDc, emptyHexPen);
  HGDIOBJ oldBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));

  for (int xCoordinate = mapMinX_ - mapLeftPaddingColumns_;
       xCoordinate <= mapMaxX_ + mapRightPaddingColumns_;
       ++xCoordinate)
  {
    for (int yCoordinate = mapMinY_; yCoordinate <= mapMaxY_; ++yCoordinate)
    {
      if ((hasCoordinateParity && (((xCoordinate + yCoordinate) & 1) != coordinateParity)) ||
          occupiedCoordinates.find({ xCoordinate, yCoordinate }) != occupiedCoordinates.end())
      {
        continue;
      }

      const int mapColumn = (xCoordinate - mapMinX_) + mapLeftPaddingColumns_;
      const double mapRow = static_cast<double>(yCoordinate - mapMinY_) / 2.0;
      const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2) - scrollX_;
      const int centerY = kMargin + static_cast<int>(std::lround(mapRow * hexHeight)) + (hexHeight / 2) - scrollY_;
      const std::array<POINT, 6> polygon = MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth);
      Polygon(memoryDc, polygon.data(), static_cast<int>(polygon.size()));
    }
  }

  SelectObject(memoryDc, oldBrush);
  SelectObject(memoryDc, oldPen);
  DeleteObject(emptyHexPen);
}

void MapTabContent::drawRegionHexes(HDC memoryDc,
                                    bool hasLatestBattlePeriod,
                                    int battleMonth,
                                    int battleYear) const
{
  for (const RegionVisual& visual : visibleRegions_)
  {
    if (visual.region)
    {
      drawRegionHex(memoryDc, visual, hasLatestBattlePeriod, battleMonth, battleYear);
    }
  }
}

void MapTabContent::drawRegionHex(HDC memoryDc,
                                  const RegionVisual& visual,
                                  bool hasLatestBattlePeriod,
                                  int battleMonth,
                                  int battleYear) const
{
  std::array<POINT, 6> translated = visual.polygon;
  for (POINT& point : translated)
  {
    point.x -= scrollX_;
    point.y -= scrollY_;
  }

  drawRegionFill(memoryDc, visual, translated);
  drawPeasantWedge(memoryDc, visual, translated);
  drawGuardWedge(memoryDc, visual, translated);
  drawRoads(memoryDc, visual);
  drawSettlement(memoryDc, visual);
  drawBattleMarker(memoryDc, visual, hasLatestBattlePeriod, battleMonth, battleYear);
}

void MapTabContent::drawRegionFill(HDC memoryDc,
                                  const RegionVisual& visual,
                                  const std::array<POINT, 6>& translated) const
{
  HBRUSH fillBrush = CreateSolidBrush(getRegionFillColor(visual.region->getRegionType()));
  HGDIOBJ oldBrush = SelectObject(memoryDc, fillBrush);
  Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
  SelectObject(memoryDc, oldBrush);
  DeleteObject(fillBrush);
}

void MapTabContent::drawPeasantWedge(HDC memoryDc,
                                     const RegionVisual& visual,
                                     const std::array<POINT, 6>& translated) const
{
  // Peasant wedge: color-coded trapezoid along the northeast edge, framed in black.
  if (!appConfig_ || visual.region->getPeasantType().empty() || visual.region->getPeasantNumber() <= 0)
  {
    return;
  }

  const std::array<int, 3> peasantRgb = appConfig_->getPeasantColour(visual.region->getPeasantType());
  const COLORREF peasantColor = RGB(std::clamp(peasantRgb[0], 0, 255),
                                    std::clamp(peasantRgb[1], 0, 255),
                                    std::clamp(peasantRgb[2], 0, 255));
  const LONG centerX = static_cast<LONG>(visual.center.x - scrollX_);
  const LONG centerY = static_cast<LONG>(visual.center.y - scrollY_);
  const POINT neCorner = translated[2];
  const POINT eCorner = translated[3];
  const double dx = static_cast<double>(neCorner.x - centerX);
  const double dy = static_cast<double>(neCorner.y - centerY);
  const double radialDistance = std::sqrt(dx * dx + dy * dy);
  const double fraction = radialDistance > 0.0
    ? std::min(1.0, std::max(0.20, 10.0 / radialDistance))
    : 0.20;
  const auto lerp = [](LONG from, LONG to, double amount)
  {
    return static_cast<LONG>(from + amount * static_cast<double>(to - from));
  };

  const POINT wedge[4] = {
    neCorner,
    eCorner,
    { lerp(centerX, eCorner.x, 1.0 - fraction), lerp(centerY, eCorner.y, 1.0 - fraction) },
    { lerp(centerX, neCorner.x, 1.0 - fraction), lerp(centerY, neCorner.y, 1.0 - fraction) }
  };
  HBRUSH brush = CreateSolidBrush(peasantColor);
  HPEN framePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HGDIOBJ oldBrush = SelectObject(memoryDc, brush);
  HGDIOBJ oldPen = SelectObject(memoryDc, framePen);
  Polygon(memoryDc, wedge, 4);
  SelectObject(memoryDc, oldBrush);
  SelectObject(memoryDc, oldPen);
  DeleteObject(brush);
  DeleteObject(framePen);
}

void MapTabContent::drawGuardWedge(HDC memoryDc,
                                   const RegionVisual& visual,
                                   const std::array<POINT, 6>& translated) const
{
  // Guard wedge: guarding-faction trapezoid along the north edge, framed in black.
  if (!appData_ || !appConfig_)
  {
    return;
  }

  const GuardWedgeStyle style = resolveGuardWedgeStyle(*appData_, *appConfig_, *visual.region);
  // no guard, no wedge
  if (!style.guardPresent)
  {
    return;
  }

  const LONG centerX = static_cast<LONG>(visual.center.x - scrollX_);
  const LONG centerY = static_cast<LONG>(visual.center.y - scrollY_);
  const POINT nwCorner = translated[1];
  const POINT neCorner = translated[2];
  const double dx = static_cast<double>(nwCorner.x - centerX);
  const double dy = static_cast<double>(nwCorner.y - centerY);
  const double radialDistance = std::sqrt(dx * dx + dy * dy);
  const double fraction = radialDistance > 0.0
    ? std::min(1.0, std::max(0.20, 10.0 / radialDistance))
    : 0.20;
  const auto lerp = [](LONG from, LONG to, double amount)
  {
    return static_cast<LONG>(from + amount * static_cast<double>(to - from));
  };

  const POINT wedge[4] = {
    nwCorner,
    neCorner,
    { lerp(centerX, neCorner.x, 1.0 - fraction), lerp(centerY, neCorner.y, 1.0 - fraction) },
    { lerp(centerX, nwCorner.x, 1.0 - fraction), lerp(centerY, nwCorner.y, 1.0 - fraction) }
  };
  COLORREF fillColor = RGB(std::clamp(style.color[0], 0, 255),
                           std::clamp(style.color[1], 0, 255),
                           std::clamp(style.color[2], 0, 255));
  if (!style.guardPresent)
  {
    fillColor = getRegionFillColor(visual.region->getRegionType());
  }

  HBRUSH brush = CreateSolidBrush(fillColor);
  HPEN framePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HGDIOBJ oldBrush = SelectObject(memoryDc, brush);
  HGDIOBJ oldPen = SelectObject(memoryDc, framePen);
  Polygon(memoryDc, wedge, 4);
  SelectObject(memoryDc, oldBrush);
  SelectObject(memoryDc, oldPen);
  DeleteObject(brush);
  DeleteObject(framePen);
}

void MapTabContent::drawRoads(HDC memoryDc, const RegionVisual& visual) const
{
  if (!appData_ || !appConfig_)
  {
    return;
  }

  std::set<std::wstring> availableExits;
  for (const std::wstring& exitDirection : visual.region->getExitDirections())
  {
    const std::wstring normalized = HexDirectionUtils::normalizeHexDirection(exitDirection);
    if (!normalized.empty())
    {
      availableExits.insert(normalized);
    }
  }
  if (availableExits.empty())
  {
    return;
  }

  std::set<std::wstring> roadDirections;
  const auto& structures = appData_->structureRepository();
  for (std::size_t index = 0; index < structures.size(); ++index)
  {
    const Structure& structure = structures.at(index);
    if (structure.getXCoordinate() != visual.region->getXCoordinate() ||
        structure.getYCoordinate() != visual.region->getYCoordinate() ||
        structure.getZCoordinate() != selectedZ_)
    {
      continue;
    }
    const std::wstring direction = HexDirectionUtils::extractRoadDirectionFromStructure(structure);
    if (!direction.empty() && availableExits.find(direction) != availableExits.end())
    {
      roadDirections.insert(direction);
    }
  }
  if (roadDirections.empty())
  {
    return;
  }

  const std::array<int, 3> roadRgb = appConfig_->getRoadColor();
  HPEN roadPen = CreatePen(PS_SOLID, 4,
                           RGB(std::clamp(roadRgb[0], 0, 255),
                               std::clamp(roadRgb[1], 0, 255),
                               std::clamp(roadRgb[2], 0, 255)));
  HGDIOBJ oldPen = SelectObject(memoryDc, roadPen);
  const int centerX = visual.center.x - scrollX_;
  const int centerY = visual.center.y - scrollY_;
  for (const std::wstring& direction : roadDirections)
  {
    const POINT endpoint = MapUtils::getRoadEndpointForDirection(visual.polygon, direction);
    MoveToEx(memoryDc, centerX, centerY, nullptr);
    LineTo(memoryDc, endpoint.x - scrollX_, endpoint.y - scrollY_);
  }
  SelectObject(memoryDc, oldPen);
  DeleteObject(roadPen);
}

void MapTabContent::drawSettlement(HDC memoryDc, const RegionVisual& visual) const
{
  if (!visual.region->getContainsSettlement())
  {
    return;
  }

  const std::wstring settlementType = StringUtils::toLower(visual.region->getSettlementType());
  const int markerDiameter = (std::max)(4, resolveScaledMapHexWidth() / 4);
  const int coreDiameter = (std::max)(2, markerDiameter / 4);
  const int centerX = visual.center.x - scrollX_;
  const int centerY = visual.center.y - scrollY_;
  HPEN markerPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
  HBRUSH markerBrush = CreateSolidBrush(RGB(0, 0, 0));
  HGDIOBJ oldPen = SelectObject(memoryDc, markerPen);
  HGDIOBJ oldBrush = SelectObject(memoryDc, markerBrush);

  if (settlementType == L"town" || settlementType == L"city")
  {
    Ellipse(memoryDc, centerX - coreDiameter / 2, centerY - coreDiameter / 2,
            centerX + coreDiameter / 2, centerY + coreDiameter / 2);
    SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
    const int firstRing = settlementType == L"town"
      ? markerDiameter
      : (std::max)(coreDiameter + 3, markerDiameter * 2 / 3);
    Ellipse(memoryDc, centerX - firstRing / 2, centerY - firstRing / 2,
            centerX + firstRing / 2, centerY + firstRing / 2);
    if (settlementType == L"city")
    {
      Ellipse(memoryDc, centerX - markerDiameter / 2, centerY - markerDiameter / 2,
              centerX + markerDiameter / 2, centerY + markerDiameter / 2);
    }
  }
  else
  {
    SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
    Ellipse(memoryDc, centerX - markerDiameter / 2, centerY - markerDiameter / 2,
            centerX + markerDiameter / 2, centerY + markerDiameter / 2);
  }

  SelectObject(memoryDc, oldBrush);
  SelectObject(memoryDc, oldPen);
  DeleteObject(markerBrush);
  DeleteObject(markerPen);
}

void MapTabContent::drawBattleMarker(HDC memoryDc,
                                     const RegionVisual& visual,
                                     bool hasLatestBattlePeriod,
                                     int battleMonth,
                                     int battleYear) const
{
  if (!hasLatestBattlePeriod || !appData_ ||
      !appData_->battleRepository().hasBattleInRegionForPeriod(
        visual.region->getXCoordinate(), visual.region->getYCoordinate(), visual.region->getZCoordinate(),
        battleMonth, battleYear))
  {
    return;
  }

  const int centerX = visual.center.x - scrollX_;
  const int centerY = visual.center.y - scrollY_;
  const int hexWidth = resolveScaledMapHexWidth();
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int crossSize = (std::max)(4, (((hexHeight * 2) / 5) * 2) / 3);
  const int halfCross = crossSize / 2;
  const int bottomMargin = (std::max)(4, hexHeight / 8);
  const int crossCenterY = centerY + (hexHeight / 2) - bottomMargin - halfCross;
  HPEN battlePen = CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
  HGDIOBJ oldPen = SelectObject(memoryDc, battlePen);
  MoveToEx(memoryDc, centerX - halfCross, crossCenterY - halfCross, nullptr);
  LineTo(memoryDc, centerX + halfCross, crossCenterY + halfCross);
  MoveToEx(memoryDc, centerX - halfCross, crossCenterY + halfCross, nullptr);
  LineTo(memoryDc, centerX + halfCross, crossCenterY - halfCross);
  SelectObject(memoryDc, oldPen);
  DeleteObject(battlePen);
}

const MapTabContent::RegionVisual* MapTabContent::hitTestRegion(POINT pointInMapClient) const
{
  const POINT mapPoint {
    pointInMapClient.x + scrollX_,
    pointInMapClient.y + scrollY_
  };

  for (const auto& visual : visibleRegions_)
  {
    HRGN region = CreatePolygonRgn(visual.polygon.data(), static_cast<int>(visual.polygon.size()), WINDING);
    const bool inside = region != nullptr && PtInRegion(region, mapPoint.x, mapPoint.y) != FALSE;
    if (region)
    {
      DeleteObject(region);
    }

    if (inside)
    {
      return &visual;
    }
  }

  return nullptr;
}


bool MapTabContent::hitTestMapCoordinate(POINT pointInMapClient, int& xCoordinate, int& yCoordinate) const
{
  if (!hasMapBounds_ || !appConfig_)
  {
    return false;
  }

  const POINT mapPoint {
    pointInMapClient.x + scrollX_,
    pointInMapClient.y + scrollY_
  };

  int coordinateParity = 0;
  bool hasCoordinateParity = false;
  for (const auto& visual : visibleRegions_)
  {
    if (!visual.region)
    {
      continue;
    }

    coordinateParity = (visual.region->getXCoordinate() + visual.region->getYCoordinate()) & 1;
    hasCoordinateParity = true;
    break;
  }

  const UiSizeProfile::Metrics uiMetrics = resolveUiMetrics();
  const int hexWidth = resolveScaledMapHexWidth();
  const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
  const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
  const int rowStep = hexHeight;

  for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
  {
    for (int y = mapMinY_; y <= mapMaxY_; ++y)
    {
      if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
      {
        continue;
      }

      const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
      const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
      const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
      const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

      const std::array<POINT, 6> polygon = MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth);
      HRGN region = CreatePolygonRgn(polygon.data(), static_cast<int>(polygon.size()), WINDING);
      const bool inside = region != nullptr && PtInRegion(region, mapPoint.x, mapPoint.y) != FALSE;
      if (region)
      {
        DeleteObject(region);
      }

      if (inside)
      {
        xCoordinate = HexDirectionUtils::wrapMapXCoordinate(x, mapMinX_, mapMaxX_);
        yCoordinate = y;
        return true;
      }
    }
  }

  return false;
}


void MapTabContent::showSkillDescription(const std::wstring& skillToken)
{
  if (!appData_ || skillToken.empty())
  {
    return;
  }

  const SkillFormattingUtils::SkillDescriptionContent content =
    SkillFormattingUtils::buildSkillDescriptionContent(*appData_, skillToken);
  if (!content.found)
  {
    MessageBoxW(GetParent(mapCanvas_),
                StringUtils::toCRLF(content.notFoundMessage).c_str(),
                L"Skill Description",
                MB_OK | MB_ICONINFORMATION);
    return;
  }

  Win32ReadOnlyPopupBackend popupBackend(GetParent(mapCanvas_));
  ReadOnlyPopupService::show(popupBackend,
                             ReadOnlyPopupService::PopupRequest{
                               content.title,
                               StringUtils::toCRLF(content.description),
                               true
                             });
}

void MapTabContent::showItemDescription(const std::wstring& itemToken)
{
  if (!appData_ || itemToken.empty())
  {
    return;
  }

  const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken);
  if (!item)
  {
    MessageBoxW(GetParent(mapCanvas_),
                (L"No description found for item '" + itemToken + L"'.").c_str(),
                L"Item Description",
                MB_OK | MB_ICONINFORMATION);
    return;
  }

  const std::wstring itemName = item->getItemName();
  std::wstring description = item->getFullText();
  if (description.empty())
  {
    description = L"No description available.";
  }

  Win32ReadOnlyPopupBackend popupBackend(GetParent(mapCanvas_));
  ReadOnlyPopupService::show(popupBackend,
                             ReadOnlyPopupService::PopupRequest{
                               L"Item Description " + itemName + L" [" + item->getIdentifierToken() + L"]",
                               StringUtils::toCRLF(description),
                               true
                             });
}


void MapTabContent::onMapLeftClick(POINT pointInMapClient)
{
  const RegionVisual* region = hitTestRegion(pointInMapClient);
  if (!region || !region->region)
  {
    hasSelectedRegion_ = false;
    updateRegionDetailsView(nullptr);
    clearUnitsList();
    return;
  }

  hasSelectedRegion_ = true;
  selectedRegionX_ = region->region->getXCoordinate();
  selectedRegionY_ = region->region->getYCoordinate();
  updateRegionDetailsView(region->region);
  populateUnitsForSelectedRegion();
  InvalidateRect(mapCanvas_, nullptr, FALSE);
}


void MapTabContent::onMapDoubleClick(POINT pointInMapClient)
{
  const RegionVisual* region = hitTestRegion(pointInMapClient);
  if (!region || !region->region)
  {
    return;
  }

  // center map on double clicked region
  RECT clientRect {};
  GetClientRect(mapCanvas_, &clientRect);
  const int clientWidth = clientRect.right - clientRect.left;
  const int clientHeight = clientRect.bottom - clientRect.top;

  const int targetX = region->center.x - clientWidth / 2;
  const int targetY = region->center.y - clientHeight / 2;
  const int maxScrollX = (std::max)(0, contentWidth_ - clientWidth);
  const int maxScrollY = (std::max)(0, contentHeight_ - clientHeight);

  scrollX_ = (std::max)(0, (std::min)(targetX, maxScrollX));
  scrollY_ = (std::max)(0, (std::min)(targetY, maxScrollY));
  updateMapScrollbars();
  InvalidateRect(mapCanvas_, nullptr, TRUE);
}


void MapTabContent::onMapRightClick(POINT pointInMapClient)
{
  const RegionVisual* region = hitTestRegion(pointInMapClient);
  if (!region || !region->region || !mapCanvas_)
  {
    return;
  }

  // open context menu for region
  POINT screenPoint = pointInMapClient;
  ClientToScreen(mapCanvas_, &screenPoint);

  const int rx = region->region->getXCoordinate();
  const int ry = region->region->getYCoordinate();
  const int rz = region->region->getZCoordinate();

  // Compute latest report period so the battle check and navigation payload
  // are consistent with what is currently displayed on the map.
  int latestReportMonth = 0;
  int latestReportYear  = 0;
  if (appData_)
  {
    const auto& rr = appData_->reportRepository();
    for (std::size_t i = 0; i < rr.size(); ++i)
    {
      const Report& rep = rr.at(i);
      const int rm = rep.getMonth();
      const int repYear = rep.getYear();
      if (rm >= 1 && rm <= 12 && repYear > 0)
      {
        if (repYear > latestReportYear ||
            (repYear == latestReportYear && rm > latestReportMonth))
        {
          latestReportMonth = rm;
          latestReportYear  = repYear;
        }
      }
    }
  }

  bool regionHasBattle = false;
  if (appData_ && latestReportMonth > 0 && latestReportYear > 0)
  {
    regionHasBattle = appData_->battleRepository().hasBattleInRegionForPeriod(
      rx, ry, rz,
      latestReportMonth,
      latestReportYear);
  }

  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  AppendMenuW(menu,
              MF_STRING,
              kRegionContextShowTextEditorCommandId,
              L"Show Region Report");

  if (regionHasBattle)
  {
              AppendMenuW(menu,
              MF_STRING | (regionHasBattle ? 0u : MF_GRAYED),
              kRegionContextShowBattleReportCommandId,
              L"Show Battle Report");
  }

  const UINT selectedCommand = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    mapCanvas_,
    nullptr
  );

  DestroyMenu(menu);

  if (selectedCommand == kRegionContextShowTextEditorCommandId)
  {
    Win32ReadOnlyPopupBackend popupBackend(GetParent(mapCanvas_));
    ReadOnlyPopupService::show(popupBackend,
                               ReadOnlyPopupService::PopupRequest{
                                 L"Region Report",
                                 StringUtils::toCRLF(region->region->getRegionReport()),
                                 false
                               });
  }
  else if (selectedCommand == kRegionContextShowBattleReportCommandId && navigationCallback_)
  {
    navigationCallback_(NavigationRequest{
      NavigationTarget::Battles,
      BattleNavigationPayload{ rx, ry, rz, latestReportMonth, latestReportYear }
    });
  }
}


void MapTabContent::updateHoverTooltip(POINT pointInMapClient, const Region& region)
{
  (void)pointInMapClient;
  if (!hoverRegionLabel_)
  {
    return;
  }

  hoverRegionText_ = L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate()
  );
  SetWindowTextW(hoverRegionLabel_, hoverRegionText_.c_str());
}


void MapTabContent::hideHoverTooltip()
{
  if (!hoverRegionLabel_)
  {
    return;
  }

  SetWindowTextW(hoverRegionLabel_, L"Hover: -");
}

COLORREF MapTabContent::getRegionFillColor(const std::wstring& regionType) const
{
  if (!appConfig_)
  {
    return RGB(192, 192, 192);
  }

  const std::array<int, 3> rgb = appConfig_->getRegionColor(regionType);
  const int red = std::clamp(rgb[0], 0, 255);
  const int green = std::clamp(rgb[1], 0, 255);
  const int blue = std::clamp(rgb[2], 0, 255);
  return RGB(red, green, blue);
}
