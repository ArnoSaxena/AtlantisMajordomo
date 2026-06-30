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
 * File: WinSizingUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/WinSizingUtils.hpp"

#include <commctrl.h>

#include <algorithm>

namespace
{

constexpr wchar_t kDensityImageListProperty[] = L"AtlantisMajordomo_ListDensityImageList";

bool isSingleLineEditControl(HWND control)
{
  const LONG_PTR style = GetWindowLongPtrW(control, GWL_STYLE);
  return (style & ES_MULTILINE) == 0;
}

} // namespace

namespace WinSizingUtils
{

void applyControlFont(HWND control, HFONT fontHandle)
{
  if (control == nullptr || fontHandle == nullptr)
  {
    return;
  }

  SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(fontHandle), FALSE);
}

int scalePx(int basePx, const UiSizeProfile::Metrics& metrics)
{
  const int safeBase = (std::max)(0, basePx);
  if (safeBase == 0)
  {
    return 0;
  }

  const int scaled = MulDiv(safeBase, metrics.baseFontPx, 16);
  return (std::max)(1, scaled);
}

void listViewApplyDensity(HWND listViewHandle,
                          const UiSizeProfile::Metrics& metrics,
                          HFONT listFont,
                          HFONT headerFont)
{
  if (listViewHandle == nullptr)
  {
    return;
  }

  applyControlFont(listViewHandle, listFont);

  HWND headerHandle = ListView_GetHeader(listViewHandle);
  if (headerHandle != nullptr)
  {
    applyControlFont(headerHandle, headerFont != nullptr ? headerFont : listFont);
  }

  HIMAGELIST oldDensityImageList = reinterpret_cast<HIMAGELIST>(
    GetPropW(listViewHandle, kDensityImageListProperty));
  if (oldDensityImageList != nullptr)
  {
    ListView_SetImageList(listViewHandle, nullptr, LVSIL_SMALL);
    RemovePropW(listViewHandle, kDensityImageListProperty);
    ImageList_Destroy(oldDensityImageList);
  }

  const int densityHeight = (std::max)(1, metrics.rowHeight);
  HIMAGELIST densityImageList = ImageList_Create(1, densityHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
  if (densityImageList != nullptr)
  {
    HBITMAP bitmap = CreateBitmap(1, densityHeight, 1, 32, nullptr);
    if (bitmap != nullptr)
    {
      ImageList_Add(densityImageList, bitmap, nullptr);
      DeleteObject(bitmap);
      ListView_SetImageList(listViewHandle, densityImageList, LVSIL_SMALL);
      SetPropW(listViewHandle,
               kDensityImageListProperty,
               reinterpret_cast<HANDLE>(densityImageList));
    }
    else
    {
      ImageList_Destroy(densityImageList);
    }
  }

  InvalidateRect(listViewHandle, nullptr, TRUE);
  if (headerHandle != nullptr)
  {
    InvalidateRect(headerHandle, nullptr, TRUE);
  }
}

void comboApplyHeight(HWND comboHandle, const UiSizeProfile::Metrics& metrics)
{
  if (comboHandle == nullptr)
  {
    return;
  }

  RECT windowRect {};
  if (!GetWindowRect(comboHandle, &windowRect))
  {
    return;
  }

  HWND parent = GetParent(comboHandle);
  if (parent == nullptr)
  {
    return;
  }

  POINT topLeft { windowRect.left, windowRect.top };
  MapWindowPoints(HWND_DESKTOP, parent, &topLeft, 1);

  const int width = windowRect.right - windowRect.left;
  const int targetHeight = (std::max)(metrics.buttonHeight, scalePx(24, metrics));

  SetWindowPos(comboHandle,
               nullptr,
               topLeft.x,
               topLeft.y,
               width,
               targetHeight,
               SWP_NOZORDER | SWP_NOACTIVATE);
}

void editApplyHeight(HWND editHandle, const UiSizeProfile::Metrics& metrics)
{
  if (editHandle == nullptr || !isSingleLineEditControl(editHandle))
  {
    return;
  }

  RECT windowRect {};
  if (!GetWindowRect(editHandle, &windowRect))
  {
    return;
  }

  HWND parent = GetParent(editHandle);
  if (parent == nullptr)
  {
    return;
  }

  POINT topLeft { windowRect.left, windowRect.top };
  MapWindowPoints(HWND_DESKTOP, parent, &topLeft, 1);

  const int width = windowRect.right - windowRect.left;
  const int targetHeight = (std::max)(metrics.buttonHeight, scalePx(22, metrics));

  SetWindowPos(editHandle,
               nullptr,
               topLeft.x,
               topLeft.y,
               width,
               targetHeight,
               SWP_NOZORDER | SWP_NOACTIVATE);
}

} // namespace WinSizingUtils
