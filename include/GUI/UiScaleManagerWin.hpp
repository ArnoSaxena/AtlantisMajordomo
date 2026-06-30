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
 * File: UiScaleManagerWin.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/UiSizeProfile.hpp"

#include <windows.h>

class UiScaleManagerWin
{
public:
  UiScaleManagerWin();
  ~UiScaleManagerWin();

  UiScaleManagerWin(const UiScaleManagerWin&) = delete;
  UiScaleManagerWin& operator=(const UiScaleManagerWin&) = delete;
  UiScaleManagerWin(UiScaleManagerWin&&) = delete;
  UiScaleManagerWin& operator=(UiScaleManagerWin&&) = delete;

  UiSizeProfile::Profile currentProfile() const;
  const UiSizeProfile::Metrics& currentMetrics() const;

  void setProfileOverride(UiSizeProfile::Profile profile);
  void refreshFromWindow(HWND hwnd);
  void createUiFontHandles();

  HFONT normalFont() const;
  HFONT smallFont() const;

private:
  void destroyFonts();

  UiSizeProfile::Profile requestedProfile_ { UiSizeProfile::Profile::Auto };
  UiSizeProfile::Profile effectiveProfile_ { UiSizeProfile::Profile::Standard };
  UiSizeProfile::Metrics metrics_ {};
  UINT activeDpi_ { 96 };

  HFONT normalFont_ { nullptr };
  HFONT smallFont_ { nullptr };
};
