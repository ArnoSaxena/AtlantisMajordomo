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
 * File: MonthUtils.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/MonthUtils.hpp"
#include "Function/StringUtils.hpp"

#include <array>
#include <cwctype>

namespace
{
const std::array<const wchar_t*, 12>& monthNames()
{
  static const std::array<const wchar_t*, 12> kMonthNames = {
    L"January", L"February", L"March", L"April", L"May", L"June",
    L"July", L"August", L"September", L"October", L"November", L"December"
  };
  return kMonthNames;
}
}

namespace MonthUtils
{
std::wstring monthNumberToName(int monthNumber)
{
  if (monthNumber < 1 || monthNumber > 12)
  {
    return L"";
  }

  return monthNames()[static_cast<std::size_t>(monthNumber - 1)];
}

std::wstring monthNumberToNameOr(int monthNumber, const std::wstring& fallback)
{
  const std::wstring monthName = monthNumberToName(monthNumber);
  return monthName.empty() ? fallback : monthName;
}

bool tryParseMonthName(const std::wstring& monthName, int& monthNumber)
{
  const std::wstring normalized = StringUtils::toLower(StringUtils::trimWhitespace(monthName));
  if (normalized.empty())
  {
    return false;
  }

  const auto& names = monthNames();
  for (std::size_t index = 0; index < names.size(); ++index)
  {
    if (StringUtils::toLower(names[index]) == normalized)
    {
      monthNumber = static_cast<int>(index + 1);
      return true;
    }
  }

  return false;
}
}
