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
 * File: MapRegionDetailsUtils.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <vector>

class AppData;
class Region;

namespace MapRegionDetailsUtils
{
struct ResourceRow
{
  std::wstring token;
  int amount { 0 };
  int amountAfterOrders { 0 };
};

struct ForSaleRow
{
  std::wstring token;
  int amount { 0 };
  int price { 0 };
  int amountAfterOrders { 0 };
};

struct WantedRow
{
  std::wstring token;
  int amount { 0 };
  int price { 0 };
  int amountAfterOrders { 0 };
};

std::wstring buildRegionSummaryText(const Region& region,
                                    const AppData* appData,
                                    int selectedUnitNumber,
                                    int selectedZCoordinate,
                                    const wchar_t* lineBreak = L"\r\n");

std::vector<ResourceRow> buildResourcesRows(const Region& region);
std::vector<ForSaleRow> buildForSaleRows(const Region& region, const AppData* appData);
std::vector<WantedRow> buildWantedRows(const Region& region);
}
