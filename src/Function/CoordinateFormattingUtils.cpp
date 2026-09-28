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
 * File: CoordinateFormattingUtils.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/CoordinateFormattingUtils.hpp"

#include <sstream>

namespace CoordinateFormattingUtils
{
std::wstring formatCoordinates(int xCoordinate,
                               int yCoordinate,
                               int zCoordinate,
                               bool omitDefaultZ,
                               bool includeSpaces)
{
  const wchar_t* delimiter = includeSpaces ? L", " : L",";
  std::wstring result = L"(" + std::to_wstring(xCoordinate) + delimiter + std::to_wstring(yCoordinate);

  if (!omitDefaultZ || zCoordinate != 1)
  {
    result += delimiter + std::to_wstring(zCoordinate);
  }

  result += L")";
  return result;
}

bool tryParseCoordinates(const std::wstring& coordinates, int& xCoordinate, int& yCoordinate, int& zCoordinate)
{
  std::wistringstream input(coordinates);
  wchar_t delimiter = 0;
  int parsedX = 0;
  int parsedY = 0;
  int parsedZ = 1;

  if (!(input >> delimiter) || delimiter != L'(' || !(input >> parsedX >> delimiter) || delimiter != L',' ||
      !(input >> parsedY))
  {
    return false;
  }

  input >> std::ws;
  if (!(input >> delimiter))
  {
    return false;
  }

  if (delimiter == L',')
  {
    if (!(input >> parsedZ >> delimiter) || delimiter != L')')
    {
      return false;
    }
  }
  else if (delimiter != L')')
  {
    return false;
  }

  input >> std::ws;
  if (input.peek() != std::char_traits<wchar_t>::eof())
  {
    return false;
  }

  xCoordinate = parsedX;
  yCoordinate = parsedY;
  zCoordinate = parsedZ;
  return true;
}
}
