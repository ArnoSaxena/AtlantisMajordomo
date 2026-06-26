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
 * File: MapUtils.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <array>
#include <windows.h>

/**
 * @brief Pure geometric utilities for hex-map calculations.
 *
 * These functions have no dependency on Win32 GUI state and are designed
 * to be reusable by both the Win32 and Qt builds (the Qt build will provide
 * an equivalent using QPoint in place of POINT).
 */
namespace MapUtils
{

/**
 * @brief Computes the six vertices of a flat-top hexagon.
 *
 * @param centerX  Pixel x-coordinate of the hex centre.
 * @param centerY  Pixel y-coordinate of the hex centre.
 * @param hexWidth Pixel width of the hexagon (tip-to-tip).
 * @return Array of six POINT values in clockwise order starting from the
 *         left vertex.
 */
std::array<POINT, 6> buildHexagonPolygon(int centerX, int centerY, int hexWidth);

} // namespace MapUtils
