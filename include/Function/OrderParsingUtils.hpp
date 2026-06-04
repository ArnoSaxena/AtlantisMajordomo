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
 * File: OrderParsingUtils.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <vector>

class AppData;

namespace OrderParsingUtils
{
enum class WarningGiveMode
{
  Quantity,
  All,
  AllExcept
};

struct WarningGiveTakeOrder
{
  bool isTake { false };
  int otherUnitNumber { 0 };
  WarningGiveMode mode { WarningGiveMode::Quantity };
  int quantity { 0 };
  int exceptQuantity { 0 };
  std::wstring itemOperand;
  bool itemOperandWasQuoted { false };
};

bool tryParseIntStrict(const std::wstring& value, int& parsed);

bool tokenizeOrderLine(const std::wstring& line,
                       std::vector<std::wstring>& tokens,
                       std::vector<bool>& tokenWasQuoted);

std::wstring normalizeItemTokenForWarning(std::wstring token);

bool tryExtractOrderKeywordUpper(const std::wstring& orderLine, std::wstring& keyword);
bool isMonthLongOrderLine(const std::wstring& orderLine);

bool tryParseGiveTakeOrder(const std::wstring& orderLine, WarningGiveTakeOrder& parsedOrder);

bool tryResolveItemTokenForWarning(const AppData& appData,
                                   const std::wstring& operand,
                                   bool operandWasQuoted,
                                   std::wstring& resolvedToken);

bool tryParseFormNewUnitLine(const std::wstring& orderLine, int& newUnitNumber);
std::vector<int> extractFormNewUnitNumbers(const std::vector<std::wstring>& orders);
std::vector<std::wstring> filterOrdersIgnoringFormBlocks(const std::vector<std::wstring>& orders);
std::vector<std::wstring> extractFormNewUnitBlock(const std::vector<std::wstring>& orders,
                                                  int formUnitNumber);
}
