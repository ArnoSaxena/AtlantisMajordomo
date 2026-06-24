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
 * File: CoordinateUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/CoordinateUtils.hpp"

#include "Data/StructInfo.hpp"
#include "Data/Structure.hpp"
#include "Function/StringUtils.hpp"

namespace CoordinateUtils
{

// --- Coordinate formatting ---

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

// --- Hex direction helpers ---

bool isWestDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(direction);
  return normalized == L"w" || normalized == L"west" || normalized == L"nw" ||
        normalized == L"northwest" || normalized == L"sw" || normalized == L"southwest";
}

bool isEastDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(direction);
  return normalized == L"e" || normalized == L"east" || normalized == L"ne" ||
        normalized == L"northeast" || normalized == L"se" || normalized == L"southeast";
}

std::wstring normalizeHexDirection(const std::wstring& direction)
{
  const std::wstring normalized = StringUtils::toLower(StringUtils::trimWhitespace(direction));
  if (normalized == L"n" || normalized == L"north")
  {
    return L"N";
  }

  if (normalized == L"ne" || normalized == L"northeast")
  {
    return L"NE";
  }

  if (normalized == L"se" || normalized == L"southeast")
  {
    return L"SE";
  }

  if (normalized == L"s" || normalized == L"south")
  {
    return L"S";
  }

  if (normalized == L"sw" || normalized == L"southwest")
  {
    return L"SW";
  }

  if (normalized == L"nw" || normalized == L"northwest")
  {
    return L"NW";
  }

  return L"";
}

std::vector<std::pair<int, int>> calculateMovePathCoordinates(int startX, int startY, const std::vector<std::wstring>& directions)
{
  std::vector<std::pair<int, int>> path;
  path.push_back({startX, startY});

  int x = startX;
  int y = startY;

  for (const auto& direction : directions)
  {
    if (direction == L"N")
    {
      y -= 2;
    }
    else if (direction == L"S")
    {
      y += 2;
    }
    else if (direction == L"NE")
    {
      x += 1;
      y -= 1;
    }
    else if (direction == L"NW")
    {
      x -= 1;
      y -= 1;
    }
    else if (direction == L"SE")
    {
      x += 1;
      y += 1;
    }
    else if (direction == L"SW")
    {
      x -= 1;
      y += 1;
    }

    path.push_back({x, y});
  }

  return path;
}

int wrapMapXCoordinate(int xCoordinate, int minX, int maxX)
{
  const int width = maxX - minX + 1;
  if (width <= 0)
  {
    return xCoordinate;
  }

  int wrappedOffset = (xCoordinate - minX) % width;
  if (wrappedOffset < 0)
  {
    wrappedOffset += width;
  }

  return minX + wrappedOffset;
}

std::wstring extractRoadDirectionFromStructure(const Structure& structure)
{
  return StructInfo::extractRoadDirectionFromStructureType(structure.getStructureType());
}

} // namespace CoordinateUtils
