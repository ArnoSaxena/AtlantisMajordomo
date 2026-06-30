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
 * File: UiSizeProfile.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/UiSizeProfile.hpp"

#include <algorithm>

namespace UiSizeProfile
{

DisplayInfo queryDisplayInfoForWindow(HWND hwnd)
{
  DisplayInfo displayInfo;

  const HWND referenceWindow = hwnd ? hwnd : GetDesktopWindow();
  HMONITOR monitor = MonitorFromWindow(referenceWindow, MONITOR_DEFAULTTONEAREST);

  MONITORINFO monitorInfo {};
  monitorInfo.cbSize = sizeof(monitorInfo);
  if (monitor && GetMonitorInfoW(monitor, &monitorInfo) != FALSE)
  {
    displayInfo.availableWidth =
      (std::max)(0, static_cast<int>(monitorInfo.rcWork.right - monitorInfo.rcWork.left));
    displayInfo.availableHeight =
      (std::max)(0, static_cast<int>(monitorInfo.rcWork.bottom - monitorInfo.rcWork.top));
  }

  UINT dpi = 0;
  if (referenceWindow)
  {
    dpi = GetDpiForWindow(referenceWindow);
  }

  if (dpi == 0)
  {
    HDC screenDc = GetDC(nullptr);
    if (screenDc)
    {
      const int logicalDpiX = GetDeviceCaps(screenDc, LOGPIXELSX);
      if (logicalDpiX > 0)
      {
        dpi = static_cast<UINT>(logicalDpiX);
      }
      ReleaseDC(nullptr, screenDc);
    }
  }

  displayInfo.dpi = (dpi == 0) ? 96u : dpi;
  return displayInfo;
}

Profile detectAutoProfile(const DisplayInfo& displayInfo)
{
  const int width = displayInfo.availableWidth;
  const int height = displayInfo.availableHeight;
  const UINT dpi = displayInfo.dpi;

  if (width <= 1280 || height <= 720)
  {
    return Profile::Compact;
  }

  if (width >= 2400 || height >= 1400 || dpi >= 150)
  {
    return Profile::Large;
  }

  return Profile::Standard;
}

Profile resolveProfile(Profile requestedProfile, const DisplayInfo& displayInfo)
{
  if (requestedProfile == Profile::Auto)
  {
    return detectAutoProfile(displayInfo);
  }

  return requestedProfile;
}

Metrics getMetrics(Profile profile)
{
  switch (profile)
  {
    case Profile::Compact:
      return Metrics {
        .baseFontPx = 12,
        .smallFontPx = 10,
        .buttonHeight = 20,
        .buttonMinWidth = 62,
        .rowHeight = 16,
        .headerHeight = 18,
        .spacing = 3,
        .margin = 5,
        .dialogWidthScale = 0.86,
        .dialogHeightScale = 0.86,
        .mapHexWidthScale = 0.85,
      };

    case Profile::Large:
      return Metrics {
        .baseFontPx = 16,
        .smallFontPx = 14,
        .buttonHeight = 24,
        .buttonMinWidth = 76,
        .rowHeight = 20,
        .headerHeight = 22,
        .spacing = 6,
        .margin = 8,
        .dialogWidthScale = 1.0,
        .dialogHeightScale = 1.0,
        .mapHexWidthScale = 1.0,
      };

    case Profile::Auto:
    case Profile::Standard:
    default:
      return Metrics {
        .baseFontPx = 14,
        .smallFontPx = 12,
        .buttonHeight = 22,
        .buttonMinWidth = 68,
        .rowHeight = 18,
        .headerHeight = 20,
        .spacing = 4,
        .margin = 6,
        .dialogWidthScale = 0.92,
        .dialogHeightScale = 0.90,
        .mapHexWidthScale = 0.90,
      };
  }
}

} // namespace UiSizeProfile
