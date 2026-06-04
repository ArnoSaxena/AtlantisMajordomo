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
 * File: OrdersEditorUtils.hpp
 */
 
// Shared utilities for orders editor context menu and insertion behavior
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>

class AppData;

namespace OrdersEditorUtils
{
  // Command IDs
  constexpr UINT kOrdersUndoCmd = 7001;
  constexpr UINT kOrdersCutCmd = 7002;
  constexpr UINT kOrdersCopyCmd = 7003;
  constexpr UINT kOrdersPasteCmd = 7004;
  constexpr UINT kOrdersDeleteCmd = 7005;
  constexpr UINT kOrdersSelectAllCmd = 7006;
  constexpr UINT kOrdersFormNewUnitCmd = 7007;

  // Show the standard orders editor context menu. Returns selected command id or 0.
  UINT showOrdersEditorMenu(HWND ownerHwnd, LPARAM lp);

  // Compute the next available new-unit number at coordinates
  int computeNextNewUnitNumber(AppData* appData, int x, int y, int z);

  // Insert a FORM/empty/END block at the end of the edit control and focus empty line
  void insertFormBlockAtEnd(HWND editHwnd, int newNumber);
}
