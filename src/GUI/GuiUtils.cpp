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
 * File: GuiUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/GuiUtils.hpp"

namespace GuiUtils
{

std::wstring getWindowText(HWND control)
{
  if (!control)
  {
    return L"";
  }

  const int length = GetWindowTextLengthW(control);
  if (length <= 0)
  {
    return L"";
  }

  std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(control, text.data(), length + 1);
  return text.c_str();
}

} // namespace GuiUtils
