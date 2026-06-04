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
 * File: OrdersEditorUtils.cpp
 */
 
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/OrdersEditorUtils.hpp"

#include "Data/AppData.hpp"

#include <commctrl.h>
#include <windowsx.h>

namespace OrdersEditorUtils
{
UINT showOrdersEditorMenu(HWND ownerHwnd, LPARAM lp)
{
  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return 0;
  }

  AppendMenuW(menu, MF_STRING, kOrdersUndoCmd, L"Undo");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kOrdersCutCmd, L"Cut");
  AppendMenuW(menu, MF_STRING, kOrdersCopyCmd, L"Copy");
  AppendMenuW(menu, MF_STRING, kOrdersPasteCmd, L"Paste");
  AppendMenuW(menu, MF_STRING, kOrdersDeleteCmd, L"Delete");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kOrdersSelectAllCmd, L"Select All");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kOrdersFormNewUnitCmd, L"Form new unit");

  POINT pt;
  if (lp == -1)
  {
    GetCursorPos(&pt);
  }
  else
  {
    pt.x = GET_X_LPARAM(lp);
    pt.y = GET_Y_LPARAM(lp);
  }

  const UINT selected = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, ownerHwnd, nullptr);
  DestroyMenu(menu);
  return selected;
}

int computeNextNewUnitNumber(AppData* appData, int x, int y, int z)
{
  if (!appData)
  {
    return 1;
  }
  int newNumber = 1;
  while (appData->unitNewRepository().hasUnitWithNumberAtCoordinates(newNumber, x, y, z))
  {
    ++newNumber;
  }
  return newNumber;
}

void insertFormBlockAtEnd(HWND editHwnd, int newNumber)
{
  std::wstring formLine = L"FORM " + std::to_wstring(newNumber) + L"\r\n";
  std::wstring emptyLine = L"\r\n";
  std::wstring endLine = L"END\r\n";
  std::wstring insertion = formLine + emptyLine + endLine;

  const int len = GetWindowTextLengthW(editHwnd);
  // Move caret to end
  SendMessageW(editHwnd, EM_SETSEL, len, len);
  SendMessageW(editHwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(insertion.c_str()));
  const DWORD caretPos = static_cast<DWORD>(len) + static_cast<DWORD>(formLine.size());
  SendMessageW(editHwnd, EM_SETSEL, static_cast<WPARAM>(caretPos), static_cast<LPARAM>(caretPos));
  SetFocus(editHwnd);
}

}
