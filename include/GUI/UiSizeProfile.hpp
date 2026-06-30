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
 * File: UiSizeProfile.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

namespace UiSizeProfile
{

enum class Profile
{
  Auto,
  Compact,
  Standard,
  Large,
};

struct Metrics
{
  int baseFontPx { 16 };
  int smallFontPx { 14 };
  int buttonHeight { 24 };
  int buttonMinWidth { 76 };
  int rowHeight { 20 };
  int headerHeight { 22 };
  int spacing { 6 };
  int margin { 8 };
  double dialogWidthScale { 1.0 };
  double dialogHeightScale { 1.0 };
  double mapHexWidthScale { 1.0 };
};

struct DisplayInfo
{
  int availableWidth { 1920 };
  int availableHeight { 1080 };
  UINT dpi { 96 };
};

DisplayInfo queryDisplayInfoForWindow(HWND hwnd);
Profile detectAutoProfile(const DisplayInfo& displayInfo);
Profile resolveProfile(Profile requestedProfile, const DisplayInfo& displayInfo);
Metrics getMetrics(Profile profile);

} // namespace UiSizeProfile
