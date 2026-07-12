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



POINT getRoadEndpointForDirection(const std::array<POINT, 6>& polygon, const std::wstring& direction)
{
  auto midpoint = [&polygon](int firstIndex, int secondIndex)
  {
    POINT point {};
    point.x = (polygon[firstIndex].x + polygon[secondIndex].x) / 2;
    point.y = (polygon[firstIndex].y + polygon[secondIndex].y) / 2;
    return point;
  };

  if (direction == L"N")
  {
    return midpoint(1, 2);
  }
  if (direction == L"NE")
  {
    return midpoint(2, 3);
  }
  if (direction == L"SE")
  {
    return midpoint(3, 4);
  }
  if (direction == L"S")
  {
    return midpoint(4, 5);
  }
  if (direction == L"SW")
  {
    return midpoint(5, 0);
  }

  return midpoint(0, 1);
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

  if (appConfig_ && hasMapBounds_)
  {
    std::set<std::pair<int, int>> occupiedCoordinates;
    int coordinateParity = 0;
    bool hasCoordinateParity = false;
    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region)
      {
        continue;
      }

      const int regionX = visual.region->getXCoordinate();
      const int regionY = visual.region->getYCoordinate();
      occupiedCoordinates.emplace(regionX, regionY);
      if (!hasCoordinateParity)
      {
        coordinateParity = (regionX + regionY) & 1;
        hasCoordinateParity = true;
      }
    }

    const UiSizeProfile::Metrics uiMetrics = resolveUiMetrics();
    const int hexWidth = resolveScaledMapHexWidth();
    const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
    HPEN emptyHexPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192)); // colour of empty hex borders
    HGDIOBJ oldEmptyPen = SelectObject(memoryDc, emptyHexPen);
    HGDIOBJ oldEmptyBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));

    for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
    {
      for (int y = mapMinY_; y <= mapMaxY_; ++y)
      {
        if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
        {
          continue;
        }

        if (occupiedCoordinates.find({ x, y }) != occupiedCoordinates.end())
        {
          continue;
        }

        const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
        const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
        const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2) - scrollX_;
        const int centerY = kMargin + static_cast<int>(std::lround(mapRow * (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)))) ) +
          ((std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0))) / 2) - scrollY_;

        const std::array<POINT, 6> emptyPolygon = MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth);
        Polygon(memoryDc, emptyPolygon.data(), static_cast<int>(emptyPolygon.size()));
      }
    }

    SelectObject(memoryDc, oldEmptyBrush);
    SelectObject(memoryDc, oldEmptyPen);
    DeleteObject(emptyHexPen);
  }

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

  for (const auto& visual : visibleRegions_)
  {
    std::array<POINT, 6> translated = visual.polygon;
    for (auto& point : translated)
    {
      point.x -= scrollX_;
      point.y -= scrollY_;
    }

    HBRUSH fillBrush = CreateSolidBrush(getRegionFillColor(visual.region->getRegionType()));
    HGDIOBJ oldBrush = SelectObject(memoryDc, fillBrush);
    Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
    SelectObject(memoryDc, oldBrush);
    DeleteObject(fillBrush);

    // Peasant colour: outer strip of the NE wedge (center → NE corner → E corner).
    // Covers the outermost 20% of the wedge depth (min 10 px), drawn as a trapezoid
    // whose outer edge is the NE–E hex side and whose inner edge is 20% inward toward center.
    if (appConfig_ && visual.region && !visual.region->getPeasantType().empty()
        && visual.region->getPeasantNumber() > 0)
    {
      const std::array<int, 3> peasantRgb = appConfig_->getPeasantColour(visual.region->getPeasantType());
      const COLORREF peasantColor = RGB(
        std::clamp(peasantRgb[0], 0, 255),
        std::clamp(peasantRgb[1], 0, 255),
        std::clamp(peasantRgb[2], 0, 255));

      const LONG cx = static_cast<LONG>(visual.center.x - scrollX_);
      const LONG cy = static_cast<LONG>(visual.center.y - scrollY_);
      const POINT neCorner = translated[2];
      const POINT eCorner  = translated[3];

      // Determine strip depth: 20% of the radial distance from center to NE corner,
      // but at least 10 pixels so the marker is always visible at small hex sizes.
      const double dxNE = static_cast<double>(neCorner.x - cx);
      const double dyNE = static_cast<double>(neCorner.y - cy);
      const double radialDist = std::sqrt(dxNE * dxNE + dyNE * dyNE);
      const double fraction = (radialDist > 0.0)
        ? std::min(1.0, std::max(0.20, 10.0 / radialDist))
        : 0.20;

      // lerp(a, b, t): from a toward b by factor t.
      auto lerp = [](LONG a, LONG b, double t) -> LONG {
        return static_cast<LONG>(a + t * static_cast<double>(b - a));
      };

      // Trapezoid: outer edge = neCorner→eCorner,
      //            inner edge = points at (1-fraction) of the way from center to each corner.
      POINT wedge[4];
      wedge[0] = neCorner;
      wedge[1] = eCorner;
      wedge[2] = { lerp(cx, eCorner.x,  1.0 - fraction), lerp(cy, eCorner.y,  1.0 - fraction) };
      wedge[3] = { lerp(cx, neCorner.x, 1.0 - fraction), lerp(cy, neCorner.y, 1.0 - fraction) };

      HBRUSH peasantBrush    = CreateSolidBrush(peasantColor);
      HPEN peasantBorderPen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
      HGDIOBJ oldPBrush      = SelectObject(memoryDc, peasantBrush);
      HGDIOBJ oldPPen        = SelectObject(memoryDc, peasantBorderPen);
      Polygon(memoryDc, wedge, 4);
      SelectObject(memoryDc, oldPBrush);
      SelectObject(memoryDc, oldPPen);
      DeleteObject(peasantBrush);
      DeleteObject(peasantBorderPen);
    }

    if (appData_ && appConfig_ && visual.region)
    {
      std::set<std::wstring> availableExitDirections;
      for (const auto& exitDirection : visual.region->getExitDirections())
      {
        const std::wstring normalizedDirection = HexDirectionUtils::normalizeHexDirection(exitDirection);
        if (!normalizedDirection.empty())
        {
          availableExitDirections.insert(normalizedDirection);
        }
      }

      if (!availableExitDirections.empty())
      {
        std::set<std::wstring> roadDirectionsToDraw;
        const auto& structureRepository = appData_->structureRepository();
        for (std::size_t structureIndex = 0; structureIndex < structureRepository.size(); ++structureIndex)
        {
          const Structure& structure = structureRepository.at(structureIndex);
          if (structure.getXCoordinate() != visual.region->getXCoordinate() ||
              structure.getYCoordinate() != visual.region->getYCoordinate() ||
              structure.getZCoordinate() != selectedZ_)
          {
            continue;
          }

          const std::wstring roadDirection = HexDirectionUtils::extractRoadDirectionFromStructure(structure);
          if (roadDirection.empty() || availableExitDirections.find(roadDirection) == availableExitDirections.end())
          {
            continue;
          }

          roadDirectionsToDraw.insert(roadDirection);
        }

        if (!roadDirectionsToDraw.empty())
        {
          const std::array<int, 3> roadColor = appConfig_->getRoadColor();
          HPEN roadPen = CreatePen(
            PS_SOLID,
            4,
            RGB(std::clamp(roadColor[0], 0, 255),
                std::clamp(roadColor[1], 0, 255),
                std::clamp(roadColor[2], 0, 255)));
          HGDIOBJ oldRoadPen = SelectObject(memoryDc, roadPen);

          const int centerX = visual.center.x - scrollX_;
          const int centerY = visual.center.y - scrollY_;
          for (const auto& direction : roadDirectionsToDraw)
          {
            const POINT mapEndpoint = getRoadEndpointForDirection(visual.polygon, direction);
            MoveToEx(memoryDc, centerX, centerY, nullptr);
            LineTo(memoryDc, mapEndpoint.x - scrollX_, mapEndpoint.y - scrollY_);
          }

          SelectObject(memoryDc, oldRoadPen);
          DeleteObject(roadPen);
        }
      }
    }

    if (visual.region->getContainsSettlement())
    {
      const std::wstring settlementType = StringUtils::toLower(visual.region->getSettlementType());
      const UiSizeProfile::Metrics uiMetrics = resolveUiMetrics();
      const int markerDiameter = (std::max)(4, resolveScaledMapHexWidth() / 4);
      const int coreDiameter = (std::max)(2, markerDiameter / 4);
      const int townRingDiameter = markerDiameter;
      const int cityInnerRingDiameter = (std::max)(coreDiameter + 3, markerDiameter * 2 / 3);
      const int cityOuterRingDiameter = markerDiameter;
      const int centerX = visual.center.x - scrollX_;
      const int centerY = visual.center.y - scrollY_;

      const COLORREF markerColor = RGB(0, 0, 0);
      HPEN markerPen = CreatePen(PS_SOLID, 2, markerColor);
      HBRUSH markerBrush = CreateSolidBrush(markerColor);
      HGDIOBJ oldMarkerPen = SelectObject(memoryDc, markerPen);
      HGDIOBJ oldMarkerBrush = SelectObject(memoryDc, markerBrush);

      if (settlementType == L"village")
      {
        // Draw village as an unfilled ring (outline only, 2 pixels wide).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        Ellipse(memoryDc,
                centerX - markerDiameter / 2,
                centerY - markerDiameter / 2,
                centerX + markerDiameter / 2,
                centerY + markerDiameter / 2);
        // Restore filled brush for subsequent settlement types.
        SelectObject(memoryDc, markerBrush);
      }
      else if (settlementType == L"town")
      {
        // Draw town with two rings: filled core + unfilled outer ring.
        // The filled core circle represents the town center (filled with current brush/pen).
        Ellipse(memoryDc,
          centerX - coreDiameter / 2,
          centerY - coreDiameter / 2,
          centerX + coreDiameter / 2,
          centerY + coreDiameter / 2);

        // Switch to NULL_BRUSH to draw subsequent circles unfilled (outline only).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        
        // Draw outer ring circle to show town's influence/extent area.
        Ellipse(memoryDc,
          centerX - townRingDiameter / 2,
          centerY - townRingDiameter / 2,
          centerX + townRingDiameter / 2,
          centerY + townRingDiameter / 2);
      }
      else if (settlementType == L"city")
      {
        // Draw city with three rings: filled core + two unfilled rings showing influence zones.
        // The filled core circle represents the city center.
        Ellipse(memoryDc,
          centerX - coreDiameter / 2,
          centerY - coreDiameter / 2,
          centerX + coreDiameter / 2,
          centerY + coreDiameter / 2);

        // Switch to NULL_BRUSH for unfilled circles.
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        
        // Draw inner ring (first influence zone).
        Ellipse(memoryDc,
          centerX - cityInnerRingDiameter / 2,
          centerY - cityInnerRingDiameter / 2,
          centerX + cityInnerRingDiameter / 2,
          centerY + cityInnerRingDiameter / 2);
        
        // Draw outer ring (second influence zone).
        Ellipse(memoryDc,
          centerX - cityOuterRingDiameter / 2,
          centerY - cityOuterRingDiameter / 2,
          centerX + cityOuterRingDiameter / 2,
          centerY + cityOuterRingDiameter / 2);
      }
      else
      {
        // Fallback for unknown settlement types: draw as an unfilled ring (like village).
        SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
        Ellipse(memoryDc,
                centerX - markerDiameter / 2,
                centerY - markerDiameter / 2,
                centerX + markerDiameter / 2,
                centerY + markerDiameter / 2);
        // Restore filled brush.
        SelectObject(memoryDc, markerBrush);
      }

      SelectObject(memoryDc, oldMarkerBrush);
      SelectObject(memoryDc, oldMarkerPen);
      DeleteObject(markerBrush);
      DeleteObject(markerPen);
    }

    if (hasLatestBattlePeriod && appData_ && visual.region)
    {
      const bool hasBattle = appData_->battleRepository().hasBattleInRegionForPeriod(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        latestBattlePeriodMonth,
        latestBattlePeriodYear
      );

      if (hasBattle)
      {
        const int centerX = visual.center.x - scrollX_;
        const int centerY = visual.center.y - scrollY_;

        const UiSizeProfile::Metrics uiMetrics = resolveUiMetrics();
        const int hexWidth = resolveScaledMapHexWidth();
        const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
        const int crossSize = (std::max)(4, (((hexHeight * 2) / 5) * 2) / 3);
        const int halfCross = crossSize / 2;
        const int bottomMargin = (std::max)(4, hexHeight / 8);
        const int crossCenterY = centerY + (hexHeight / 2) - bottomMargin - halfCross;

        HPEN battlePen = CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
        HGDIOBJ oldBattlePen = SelectObject(memoryDc, battlePen);

        MoveToEx(memoryDc, centerX - halfCross, crossCenterY - halfCross, nullptr);
        LineTo(memoryDc, centerX + halfCross, crossCenterY + halfCross);
        MoveToEx(memoryDc, centerX - halfCross, crossCenterY + halfCross, nullptr);
        LineTo(memoryDc, centerX + halfCross, crossCenterY - halfCross);

        SelectObject(memoryDc, oldBattlePen);
        DeleteObject(battlePen);
      }
    }
  }

  SelectObject(memoryDc, oldPen);
  DeleteObject(borderPen);

  // Draw selected region border on top in the configured highlight colour.
  if (hasSelectedRegion_ && appConfig_)
  {
    const std::array<int, 3> selColor = appConfig_->getSelectedRegionBorderColor();
    HPEN selPen = CreatePen(PS_SOLID, 3, RGB(selColor[0], selColor[1], selColor[2]));
    HGDIOBJ oldSelPen = SelectObject(memoryDc, selPen);
    HGDIOBJ oldSelBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));

    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region ||
          visual.region->getXCoordinate() != selectedRegionX_ ||
          visual.region->getYCoordinate() != selectedRegionY_)
      {
        continue;
      }

      std::array<POINT, 6> translated = visual.polygon;
      for (auto& point : translated)
      {
        point.x -= scrollX_;
        point.y -= scrollY_;
      }
      Polygon(memoryDc, translated.data(), static_cast<int>(translated.size()));
      break;
    }

    SelectObject(memoryDc, oldSelBrush);
    SelectObject(memoryDc, oldSelPen);
    DeleteObject(selPen);
  }

  // Draw move path arrows
  if (!movePathCoordinates_.empty() && movePathCoordinates_.size() > 1)
  {
    const COLORREF defaultArrowColor = movePathIsSail_ ? RGB(173, 216, 230) : RGB(144, 238, 144); // Light Blue for SAIL, Light Green for MOVE/ADVANCE
    const COLORREF arrowColor = movePathHasNegativeCapacity_ ? RGB(255, 0, 0) : defaultArrowColor;
    const COLORREF arrowBorderColor = RGB(0, 0, 0); // Black border
    
    HPEN arrowPen = CreatePen(PS_SOLID, 4, arrowColor);
    HPEN arrowBorderPen = CreatePen(PS_SOLID, 8, arrowBorderColor);
    HPEN arrowTipBorderPen = CreatePen(PS_SOLID, 2, arrowBorderColor);
    HBRUSH arrowBrush = CreateSolidBrush(arrowColor);
    
    for (size_t i = 0; i < movePathCoordinates_.size() - 1; ++i)
    {
      const int x1 = movePathCoordinates_[i].first;
      const int y1 = movePathCoordinates_[i].second;
      const int x2 = movePathCoordinates_[i + 1].first;
      const int y2 = movePathCoordinates_[i + 1].second;
      
      // Find visual centers for these coordinates
      POINT startCenter = {0, 0};
      POINT endCenter = {0, 0};
      
      for (const auto& visual : visibleRegions_)
      {
        if (visual.region->getXCoordinate() == x1 && visual.region->getYCoordinate() == y1)
        {
          startCenter = visual.center;
        }
        if (visual.region->getXCoordinate() == x2 && visual.region->getYCoordinate() == y2)
        {
          endCenter = visual.center;
        }
      }
      
      if (startCenter.x == 0 && startCenter.y == 0)
        continue;
      if (endCenter.x == 0 && endCenter.y == 0)
        continue;
      
      // Apply scroll offset
      startCenter.x -= scrollX_;
      startCenter.y -= scrollY_;
      endCenter.x -= scrollX_;
      endCenter.y -= scrollY_;

      // Keep each arrow segment at 50% of center-to-center distance.
      const double dx = endCenter.x - startCenter.x;
      const double dy = endCenter.y - startCenter.y;
      const double length = std::sqrt(dx * dx + dy * dy);

      int adjustedStartX = startCenter.x;
      int adjustedStartY = startCenter.y;
      int adjustedEndX = endCenter.x;
      int adjustedEndY = endCenter.y;

      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double shorteningAmount = (std::max)(1.0, length * 0.25);
        adjustedStartX = static_cast<int>(std::lround(startCenter.x + ux * shorteningAmount));
        adjustedStartY = static_cast<int>(std::lround(startCenter.y + uy * shorteningAmount));
        adjustedEndX = static_cast<int>(std::lround(endCenter.x - ux * shorteningAmount));
        adjustedEndY = static_cast<int>(std::lround(endCenter.y - uy * shorteningAmount));
      }
      
      // Draw border first, then the 4px arrow on top.
      int shaftEndX = adjustedEndX;
      int shaftEndY = adjustedEndY;
      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double arrowLen = 10.0;
        shaftEndX = static_cast<int>(std::lround(adjustedEndX - ux * arrowLen));
        shaftEndY = static_cast<int>(std::lround(adjustedEndY - uy * arrowLen));
      }

      HGDIOBJ oldBorderPen = SelectObject(memoryDc, arrowBorderPen);
      MoveToEx(memoryDc, adjustedStartX, adjustedStartY, nullptr);
      LineTo(memoryDc, shaftEndX, shaftEndY);
      SelectObject(memoryDc, oldBorderPen);

      HGDIOBJ oldArrowPen = SelectObject(memoryDc, arrowPen);
      MoveToEx(memoryDc, adjustedStartX, adjustedStartY, nullptr);
      LineTo(memoryDc, shaftEndX, shaftEndY);
      SelectObject(memoryDc, oldArrowPen);
      
      // Draw arrow head at end
      if (length > 0)
      {
        const double ux = dx / length;
        const double uy = dy / length;
        const double arrowLen = 10;
        const double arrowWidth = 6;
        
        // Arrow tip
        const POINT arrowTip = {adjustedEndX, adjustedEndY};
        
        // Arrow base points
        const POINT arrowBase1 = {
          static_cast<int>(adjustedEndX - ux * arrowLen + uy * arrowWidth),
          static_cast<int>(adjustedEndY - uy * arrowLen - ux * arrowWidth)
        };
        const POINT arrowBase2 = {
          static_cast<int>(adjustedEndX - ux * arrowLen - uy * arrowWidth),
          static_cast<int>(adjustedEndY - uy * arrowLen + ux * arrowWidth)
        };
        
        HGDIOBJ oldBrushArrow = SelectObject(memoryDc, arrowBrush);
        HGDIOBJ oldTipPen = SelectObject(memoryDc, arrowTipBorderPen);
        const POINT arrowPoints[] = {arrowTip, arrowBase1, arrowBase2};
        Polygon(memoryDc, arrowPoints, 3);
        SelectObject(memoryDc, oldTipPen);
        SelectObject(memoryDc, oldBrushArrow);
      }
    }
    
    DeleteObject(arrowPen);
    DeleteObject(arrowBorderPen);
    DeleteObject(arrowTipBorderPen);
    DeleteObject(arrowBrush);
  }

  // Draw structure markers as a final overlay so they remain visible above map symbols.
  if (appData_ && appConfig_)
  {
    const std::array<int, 3> markerColor = appConfig_->getStructureMarkerColor();
    const COLORREF defaultMarkerColor = RGB(
      std::clamp(markerColor[0], 0, 255),
      std::clamp(markerColor[1], 0, 255),
      std::clamp(markerColor[2], 0, 255));
    const COLORREF caravanseraiMarkerColor = RGB(255, 165, 0);
    const COLORREF shipMarkerColor = RGB(173, 216, 230);

    for (const auto& visual : visibleRegions_)
    {
      if (!visual.region)
      {
        continue;
      }

      const auto structuresInRegion = appData_->structureRepository().findByCoordinates(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate());
      if (structuresInRegion.empty())
      {
        continue;
      }

      bool hasNonRoadNonShipStructure = false;
      bool hasShipStructure = false;
      bool hasFlyingShipStructure = false;
      for (const Structure* structure : structuresInRegion)
      {
        if (!structure)
        {
          continue;
        }

        const bool isRoadStructure = !HexDirectionUtils::extractRoadDirectionFromStructure(*structure).empty();
        const StructInfo* structInfo = appData_->structInfoRepository().findByType(structure->getStructureType());
        const bool isShipStructure = structInfo && structInfo->isShip();

        if (isShipStructure)
        {
          hasShipStructure = true;
          if (structure->isFlying())
          {
            hasFlyingShipStructure = true;
          }
        }

        if (!isRoadStructure && !isShipStructure)
        {
          hasNonRoadNonShipStructure = true;
        }
      }

      const bool hasCaravanserai = appData_->structureRepository().findByCoordinatesAndType(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        L"caravanserai").size() > 0;

      const bool hasShaft = appData_->structureRepository().findByCoordinatesAndType(
        visual.region->getXCoordinate(),
        visual.region->getYCoordinate(),
        visual.region->getZCoordinate(),
        L"shaft").size() > 0;

      if (!hasNonRoadNonShipStructure && !hasShipStructure)
      {
        continue;
      }

      const POINT northEndpoint = getRoadEndpointForDirection(visual.polygon, L"N");
      const POINT northWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"NW");
      const POINT borderMidpoint {
        (northEndpoint.x + northWestEndpoint.x) / 2,
        (northEndpoint.y + northWestEndpoint.y) / 2
      };
      const POINT southEndpoint = getRoadEndpointForDirection(visual.polygon, L"S");
      const POINT southWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"SW");
      const POINT borderMidpointBottomLeft {
        (southEndpoint.x + southWestEndpoint.x) / 2,
        (southEndpoint.y + southWestEndpoint.y) / 2
      };

      const POINT centerPoint { visual.center.x, visual.center.y };
      const int markerCenterX = borderMidpoint.x + (centerPoint.x - borderMidpoint.x) / 4 - scrollX_;
      const int markerCenterY = borderMidpoint.y + (centerPoint.y - borderMidpoint.y) / 4 - scrollY_;
      const int shipMarkerCenterX = borderMidpointBottomLeft.x + (centerPoint.x - borderMidpointBottomLeft.x) / 4 - scrollX_;
      const int shipMarkerCenterY = borderMidpointBottomLeft.y + (centerPoint.y - borderMidpointBottomLeft.y) / 4 - scrollY_;
      const int markerRadius = 2;

      if (hasNonRoadNonShipStructure)
      {
        const COLORREF topMarkerColor = hasCaravanserai ? caravanseraiMarkerColor : defaultMarkerColor;
        HBRUSH markerBrush = CreateSolidBrush(topMarkerColor);
        HGDIOBJ oldMarkerPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));
        HGDIOBJ oldMarkerBrush = SelectObject(memoryDc, markerBrush);
        Ellipse(memoryDc,
            markerCenterX - markerRadius,
            markerCenterY - markerRadius,
            markerCenterX + markerRadius + 1,
            markerCenterY + markerRadius + 1);
        SelectObject(memoryDc, oldMarkerBrush);
        SelectObject(memoryDc, oldMarkerPen);
        DeleteObject(markerBrush);

        if (hasShaft)
        {
          HPEN shaftBorderPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
          HGDIOBJ oldShaftPen = SelectObject(memoryDc, shaftBorderPen);
          HGDIOBJ oldShaftBrush = SelectObject(memoryDc, GetStockObject(NULL_BRUSH));
          Ellipse(memoryDc,
              markerCenterX - markerRadius,
              markerCenterY - markerRadius,
              markerCenterX + markerRadius + 1,
              markerCenterY + markerRadius + 1);
          SelectObject(memoryDc, oldShaftBrush);
          SelectObject(memoryDc, oldShaftPen);
          DeleteObject(shaftBorderPen);
        }
      }

      if (hasShipStructure)
      {
        HBRUSH shipBrush = CreateSolidBrush(shipMarkerColor);
        HPEN shipPen = nullptr;
        HGDIOBJ oldShipPen;
        if (hasFlyingShipStructure)
        {
          shipPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
          oldShipPen = SelectObject(memoryDc, shipPen);
        }
        else
        {
          oldShipPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));
        }
        HGDIOBJ oldShipBrush = SelectObject(memoryDc, shipBrush);
        Ellipse(memoryDc,
            shipMarkerCenterX - markerRadius,
            shipMarkerCenterY - markerRadius,
            shipMarkerCenterX + markerRadius + 1,
            shipMarkerCenterY + markerRadius + 1);
        SelectObject(memoryDc, oldShipBrush);
        SelectObject(memoryDc, oldShipPen);
        DeleteObject(shipBrush);
        if (shipPen)
        {
          DeleteObject(shipPen);
        }
      }
    }
  }

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
  std::wstring skillName = appData_->skillRepository().findByIdentifier(skillToken)->getName();
  std::wstring skillDescription = appData_->skillRepository().findByIdentifier(skillToken)->getAllLevelDescriptions();
  showReadOnlyTextPopup(GetParent(mapCanvas_),
                          L"Skill Description " + skillName,
                          StringUtils::toCRLF(skillDescription),
                          true);
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
    showReadOnlyTextPopup(GetParent(mapCanvas_),
                          L"Region Report",
                          StringUtils::toCRLF(region->region->getRegionReport()),
                          false);
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
