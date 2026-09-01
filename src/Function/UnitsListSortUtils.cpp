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
 * File: UnitsListSortUtils.cpp
 */

#include "Function/UnitsListSortUtils.hpp"

#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>

namespace UnitsListSortUtils
{
namespace
{
int compareCaseInsensitive(const std::wstring& left, const std::wstring& right)
{
    const std::size_t commonLength = std::min(left.size(), right.size());
    for (std::size_t index = 0; index < commonLength; ++index)
    {
        const wchar_t leftChar = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(left[index])));
        const wchar_t rightChar = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(right[index])));
        if (leftChar != rightChar)
        {
            return leftChar < rightChar ? -1 : 1;
        }
    }

    if (left.size() == right.size())
    {
        return 0;
    }
    return left.size() < right.size() ? -1 : 1;
}
} // namespace

int compareCellValues(const std::wstring& leftValue,
                      const std::wstring& rightValue,
                      bool ascending)
{
    const std::wstring trimmedLeft = StringUtils::trimWhitespace(leftValue);
    const std::wstring trimmedRight = StringUtils::trimWhitespace(rightValue);

    const bool leftEmpty = trimmedLeft.empty();
    const bool rightEmpty = trimmedRight.empty();
    if (leftEmpty != rightEmpty)
    {
        // Keep empty entries at the bottom in both sort directions.
        return leftEmpty ? 1 : -1;
    }
    if (leftEmpty)
    {
        return 0;
    }

    long long leftNumber = 0;
    long long rightNumber = 0;
    const bool leftHasNumber = StringUtils::tryParseLeadingInteger(trimmedLeft, leftNumber);
    const bool rightHasNumber = StringUtils::tryParseLeadingInteger(trimmedRight, rightNumber);

    if (leftHasNumber && rightHasNumber && leftNumber != rightNumber)
    {
        return ascending ? (leftNumber < rightNumber ? -1 : 1) : (leftNumber > rightNumber ? -1 : 1);
    }

    const int textComparison = compareCaseInsensitive(trimmedLeft, trimmedRight);
    if (textComparison == 0)
    {
        return 0;
    }

    return ascending ? textComparison : -textComparison;
}
} // namespace UnitsListSortUtils
