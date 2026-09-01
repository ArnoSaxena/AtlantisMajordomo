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
 * File: MapUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Function/MapUtils.hpp"

#include <algorithm>
#include <cmath>

namespace MapUtils
{

std::array<POINT, 6> buildHexagonPolygon(int centerX, int centerY, int hexWidth)
{
  const int halfWidth    = hexWidth / 2;
  const int quarterWidth = (std::max)(1, hexWidth / 4);
  const int halfHeight   = (std::max)(1, static_cast<int>(
      std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 4.0)));

  std::array<POINT, 6> points {};
  points[0] = { centerX - halfWidth,    centerY              };
  points[1] = { centerX - quarterWidth, centerY - halfHeight };
  points[2] = { centerX + quarterWidth, centerY - halfHeight };
  points[3] = { centerX + halfWidth,    centerY              };
  points[4] = { centerX + quarterWidth, centerY + halfHeight };
  points[5] = { centerX - quarterWidth, centerY + halfHeight };
  return points;
}

POINT getRoadEndpointForDirection(const std::array<POINT, 6>& polygon, const std::wstring& direction)
{
  const auto midpoint = [&polygon](int firstIndex, int secondIndex)
  {
    POINT point {};
    point.x = (polygon[firstIndex].x + polygon[secondIndex].x) / 2;
    point.y = (polygon[firstIndex].y + polygon[secondIndex].y) / 2;
    return point;
  };

  if (direction == L"N")
  {
    return midpoint(1, 2);
  }
  if (direction == L"NE")
  {
    return midpoint(2, 3);
  }
  if (direction == L"SE")
  {
    return midpoint(3, 4);
  }
  if (direction == L"S")
  {
    return midpoint(4, 5);
  }
  if (direction == L"SW")
  {
    return midpoint(5, 0);
  }

  return midpoint(0, 1);
}

} // namespace MapUtils
