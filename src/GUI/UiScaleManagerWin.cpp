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
 * File: UiScaleManagerWin.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/UiScaleManagerWin.hpp"

#include <algorithm>

namespace
{

int fontPixelToLogicalHeight(int pixelSize, UINT dpi)
{
  const int clampedPx = (std::max)(1, pixelSize);
  const UINT effectiveDpi = (dpi == 0) ? 96u : dpi;
  return -MulDiv(clampedPx, static_cast<int>(effectiveDpi), 96);
}

HFONT createUiFont(int pixelSize, UINT dpi)
{
  return CreateFontW(
    fontPixelToLogicalHeight(pixelSize, dpi),
    0,
    0,
    0,
    FW_NORMAL,
    FALSE,
    FALSE,
    FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Segoe UI");
}

} // namespace

UiScaleManagerWin::UiScaleManagerWin()
{
  metrics_ = UiSizeProfile::getMetrics(effectiveProfile_);
}

UiScaleManagerWin::~UiScaleManagerWin()
{
  destroyFonts();
}

UiSizeProfile::Profile UiScaleManagerWin::currentProfile() const
{
  return effectiveProfile_;
}

const UiSizeProfile::Metrics& UiScaleManagerWin::currentMetrics() const
{
  return metrics_;
}

void UiScaleManagerWin::setProfileOverride(UiSizeProfile::Profile profile)
{
  requestedProfile_ = profile;
}

void UiScaleManagerWin::refreshFromWindow(HWND hwnd)
{
  const UiSizeProfile::DisplayInfo displayInfo = UiSizeProfile::queryDisplayInfoForWindow(hwnd);
  activeDpi_ = displayInfo.dpi;

  effectiveProfile_ = UiSizeProfile::resolveProfile(requestedProfile_, displayInfo);
  metrics_ = UiSizeProfile::getMetrics(effectiveProfile_);
}

void UiScaleManagerWin::createUiFontHandles()
{
  destroyFonts();

  normalFont_ = createUiFont(metrics_.baseFontPx, activeDpi_);
  smallFont_ = createUiFont(metrics_.smallFontPx, activeDpi_);

  if (normalFont_ == nullptr)
  {
    normalFont_ = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
  }
  if (smallFont_ == nullptr)
  {
    smallFont_ = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
  }
}

HFONT UiScaleManagerWin::normalFont() const
{
  return normalFont_;
}

HFONT UiScaleManagerWin::smallFont() const
{
  return smallFont_;
}

void UiScaleManagerWin::destroyFonts()
{
  if (normalFont_ != nullptr && normalFont_ != reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)))
  {
    DeleteObject(normalFont_);
  }
  if (smallFont_ != nullptr && smallFont_ != reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)))
  {
    DeleteObject(smallFont_);
  }

  normalFont_ = nullptr;
  smallFont_ = nullptr;
}
