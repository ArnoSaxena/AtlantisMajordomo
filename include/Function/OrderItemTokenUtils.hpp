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
 * File: OrderItemTokenUtils.hpp
 */

#pragma once

#include <cstddef>
#include <set>
#include <string>
#include <vector>

class AppData;

namespace OrderItemTokenUtils
{
    // Resolves an order operand (identifier token, item name, or plural name) to its canonical item identifier token.
    bool tryResolveOrderItemToken(const AppData& appData,
                                  const std::wstring& operand,
                                  std::wstring& resolvedToken);

    // Advances tokenIndex past an optional "NEW" keyword and a following unit number; returns false if no valid unit reference is found.
    bool tryConsumeUnitReference(const std::vector<std::wstring>& tokens, std::size_t& tokenIndex);

    // Collects the resolved item tokens referenced by a unit's PRODUCE/BUY/SELL/TRANSPORT/DISTRIBUTE orders.
    std::set<std::wstring> collectTouchedItemTokensForUnit(const AppData& appData,
                                                           int unitNumber,
                                                           bool isNewUnit);
}
