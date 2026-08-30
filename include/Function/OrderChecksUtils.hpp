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
 * File: OrderChecksUtils.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <functional>

class AppData;

namespace OrderChecksUtils
{
void runOrderChecksForMainFaction(AppData& appData,
                                  int selectedUnitNumber,
                                  const std::function<void()>& saveOrders,
                                  const std::function<void()>& populateUnits,
                                  const std::function<void()>& updateWarningsSummary,
                                  const std::function<void(int)>& updateSelectedUnitDetails);

int selectPreviousWarningUnitNumber(const AppData& appData, int selectedUnitNumber, bool selectedUnitIsNew);
int selectNextWarningUnitNumber(const AppData& appData, int selectedUnitNumber, bool selectedUnitIsNew);
}
