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
// Skill list context menu command IDs
// ---------------------------------------------------------------------------
constexpr UINT kSkillStudyContextCommandId            = 4711;
constexpr UINT kSkillDescriptionListContextCommandId  = 4712;
constexpr UINT kSkillDescriptionPopupContextCommandId = 4713;

// ---------------------------------------------------------------------------
// Utility template
// ---------------------------------------------------------------------------
template <typename T>
T clampValue(T value, T low, T high)
{
  return (std::max)(low, (std::min)(value, high));
}
