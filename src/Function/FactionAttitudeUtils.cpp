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
 * File: FactionAttitudeUtils.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/FactionAttitudeUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>

namespace FactionAttitudeUtils
{
const wchar_t* attitudeToText(Faction::Attitude attitude)
{
  switch (attitude)
  {
    case Faction::Attitude::Hostile:
      return L"Hostile";
    case Faction::Attitude::Unfriendly:
      return L"Unfriendly";
    case Faction::Attitude::Neutral:
      return L"Neutral";
    case Faction::Attitude::Friendly:
      return L"Friendly";
    case Faction::Attitude::Ally:
      return L"Ally";
  }

  return L"Neutral";
}

Faction::Attitude textToAttitude(const std::wstring& text)
{
  const std::wstring upper = StringUtils::toUpper(StringUtils::trimWhitespace(text));
  if (upper == L"HOSTILE")
  {
    return Faction::Attitude::Hostile;
  }

  if (upper == L"UNFRIENDLY")
  {
    return Faction::Attitude::Unfriendly;
  }

  if (upper == L"FRIENDLY")
  {
    return Faction::Attitude::Friendly;
  }

  if (upper == L"ALLY")
  {
    return Faction::Attitude::Ally;
  }

  return Faction::Attitude::Neutral;
}

std::wstring normalizeAttitudeText(const std::wstring& text)
{
  return attitudeToText(textToAttitude(text));
}
}
