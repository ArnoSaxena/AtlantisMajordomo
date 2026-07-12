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

/// @struct Metrics
/// @brief Defines all UI sizing metrics for a particular profile
///
/// The Metrics struct contains all size and scale parameters that control the visual layout
/// and appearance of the UI. Each metric is carefully tuned to work together for a specific
/// profile (Compact, Standard, Large), ensuring consistent proportions across all UI elements.
///
/// Font Metrics:
///   - baseFontPx:    Primary font size used for regular text, labels, and body content.
///                    Controls the readability and prominence of main UI text.
///   - smallFontPx:   Secondary font size for less prominent text, helper text, status messages,
///                    and annotations. Typically 2-4px smaller than baseFontPx.
///
/// Control Sizing:
///   - buttonHeight:  Standard height for all pushbuttons, checkboxes, radio buttons, and
///                    input fields. Ensures touch-friendly and visually consistent controls.
///   - buttonMinWidth: Minimum width for buttons to ensure sufficient clickable area and to
///                     prevent buttons from appearing too narrow. Used by button layout logic
///                     to maintain usability across different text lengths.
///   - rowHeight:     Height of rows in list views, tree views, data grids, and table rows.
///                    Controls vertical spacing for list-based content display.
///   - headerHeight:  Height of column/section headers in list views, data grids, and
///                    groupbox titles. Usually slightly taller than rowHeight for emphasis.
///
/// Spacing & Margins:
///   - spacing:       Horizontal and vertical gap between adjacent UI elements within a
///                    container or control group. Controls internal layout density (e.g., space
///                    between buttons in a button group, gap between list items).
///   - margin:        Outer spacing around groups of elements and dialog content areas.
///                    Applied around the edges of dialogs, panels, and major sections to
///                    provide visual breathing room and frame content appropriately.
///
/// Scale Factors:
///   - dialogWidthScale:   Multiplier for dialog window width. Used to scale dialogs
///                         proportionally based on profile. 1.0 = standard, <1.0 = narrower,
///                         >1.0 = wider. Affects all modeless and modal dialog dimensions.
///   - dialogHeightScale:  Multiplier for dialog window height. Controls vertical expansion
///                         of dialogs independently from width to accommodate profile-specific
///                         layout requirements.
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
};

/// @enum MapHexProfile
/// @brief Defines independent scale profiles for hexagon tiles on the map canvas
///
/// This enum allows independent control of map hexagon sizes from the main UI profile.
/// This enables flexible combinations, such as Compact UI with Large hex tiles, or vice versa.
enum class MapHexProfile
{
  Small,
  Medium,
  Large,
};

/// @struct MapHexMetrics
/// @brief Controls the scale of hexagon tiles on the map canvas
///
/// MapHexMetrics defines a single scale factor that is applied to hexagon cell dimensions.
/// This metric affects both the hexagon tile size and the scale of content (units, flags,
/// text overlays) rendered within the hexes. This setting is completely independent from
/// the main UI profile, allowing fine-grained control over map visibility and usability.
///
/// Metrics:
///   - mapHexWidthScale:  Multiplier for hexagon tile width on the map canvas. Controls the
///                        size of hexagon cells in the tactical/strategic map. Affects both
///                        hex sizing and the scale of content (units, flags) within hexes.
struct MapHexMetrics
{
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
MapHexMetrics getMapHexMetrics(MapHexProfile profile);

} // namespace UiSizeProfile
