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
 * File: MapTabContent_private.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
//
// Internal constants and templates shared across the MapTabContent_*.cpp
// translation units.  Do NOT include from outside the GUI module.
//
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <array>
#include <windows.h>

// ---------------------------------------------------------------------------
// Layout / panel sizing
// ---------------------------------------------------------------------------
constexpr int kMargin              = 8;
constexpr int kSplitterThickness   = 8;
constexpr int kMinRightPanelWidth  = 250;
constexpr int kMinLeftPanelWidth   = 280;
constexpr int kMinDetailsWidth     = 180;
constexpr int kMinMapWidth         = 220;
constexpr int kMinTopHeight        = 160;
constexpr int kMinBottomHeight     = 140;

// ---------------------------------------------------------------------------
// Map canvas
// ---------------------------------------------------------------------------
constexpr wchar_t    kMapCanvasClassName[]   = L"WindowsAppMapCanvas";
constexpr UINT_PTR   kUnitSearchEditSubclassId = 231001;
constexpr UINT_PTR   kUnitsListHeaderSubclassId = 231002;

// ---------------------------------------------------------------------------
// Region context menu command IDs
// ---------------------------------------------------------------------------
constexpr int  kZContextMenuBaseId                    = 4600;
constexpr UINT kRegionContextShowTextEditorCommandId  = 4750;
constexpr UINT kRegionContextShowBattleReportCommandId = 4751;

// ---------------------------------------------------------------------------
// Read-only text popup window
// ---------------------------------------------------------------------------
constexpr wchar_t kReadOnlyTextPopupClassName[] = L"AtlantisMajordomoReadOnlyTextPopup";
constexpr int     kReadOnlyTextPopupEditId      = 9001;

// ---------------------------------------------------------------------------
// Unit-details inner tabs
// ---------------------------------------------------------------------------
constexpr int kOrdersTabIndex   = 0;
constexpr int kEventsTabIndex   = 1;
constexpr int kErrorsTabIndex   = 2;
constexpr int kWarningsTabIndex = 3;

// ---------------------------------------------------------------------------
// Units list columns
// ---------------------------------------------------------------------------
struct UnitsListColumnDefinition
{
  const wchar_t* title;
  int width;
};

inline constexpr std::array<UnitsListColumnDefinition, 13> kUnitsListColumns {{
  { L"#", 50 },
  { L"Name", 180 },
  { L"Faction", 50 },
  { L"Faction Name", 120 },
  { L"Structure", 150 },
  { L"Men", 90 },
  { L"Silver", 96 },
  { L"Flags", 240 },
  { L"Skills", 260 },
  { L"Month Order", 180 },
  { L"!", 28 },
  { L"B", 28 },
  { L"D", 28 }
}};

inline const wchar_t* getUnitsListColumnTitle(int columnIndex)
{
  if (columnIndex < 0 || columnIndex >= static_cast<int>(kUnitsListColumns.size()))
  {
    return L"";
  }

  return kUnitsListColumns[static_cast<std::size_t>(columnIndex)].title;
}

// ---------------------------------------------------------------------------
// Skill list context menu command IDs
// ---------------------------------------------------------------------------
constexpr UINT kSkillStudyContextCommandId            = 4711;
constexpr UINT kSkillDescriptionListContextCommandId  = 4712;
constexpr UINT kSkillDescriptionPopupContextCommandId = 4713;
constexpr UINT kWarningClearContextCommandId          = 4714;
constexpr UINT kRegionItemDescriptionContextCommandId = 4715;
constexpr UINT kRegionItemTabContextCommandId         = 4716;

// ---------------------------------------------------------------------------
// Utility template
// ---------------------------------------------------------------------------
template <typename T>
T clampValue(T value, T low, T high)
{
  return (std::max)(low, (std::min)(value, high));
}
