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
 * File: BattleFormattingUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/BattleFormattingUtils.hpp"

#include "Data/Battle.hpp"
#include "Function/CoordinateUtils.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace
{

std::wstring joinDamaged(const std::vector<int>& damagedUnitIds)
{
  if (damagedUnitIds.empty())
  {
    return L"none";
  }

  std::wstringstream stream;
  for (std::size_t index = 0; index < damagedUnitIds.size(); ++index)
  {
    if (index != 0)
    {
      stream << L", ";
    }
    stream << damagedUnitIds[index];
  }

  return stream.str();
}

std::wstring formatSpoils(const Battle& battle)
{
  if (battle.getSpoils().empty())
  {
    return L"none";
  }

  std::wstringstream stream;
  for (std::size_t index = 0; index < battle.getSpoils().size(); ++index)
  {
    if (index != 0)
    {
      stream << L", ";
    }

    const BattleSpoil& spoil = battle.getSpoils()[index];
    stream << spoil.amount << L" [" << spoil.token << L"]";
  }

  return stream.str();
}

} // namespace

namespace BattleFormattingUtils
{

std::wstring formatPeriod(int month, int year)
{
  std::wstringstream stream;
  if (month < 10)
  {
    stream << L"0";
  }
  stream << month << L"/" << year;
  return stream.str();
}

std::wstring formatBattleCoordinates(const Battle& battle)
{
  return CoordinateFormattingUtils::formatCoordinates(
    battle.getRegionXCoordinate(),
    battle.getRegionYCoordinate(),
    battle.getRegionZCoordinate()
  );
}

std::wstring formatBattleListEntry(const Battle& battle)
{
  std::wstringstream stream;
  stream << battle.getAttackerUnitName() << L" (" << battle.getAttackerUnitId() << L") attacks "
         << battle.getDefenderUnitName() << L" (" << battle.getDefenderUnitId() << L") in "
         << formatBattleCoordinates(battle);
  return stream.str();
}

std::wstring formatSummary(const Battle& battle)
{
  std::wstringstream stream;
  stream << L"Attacker " << battle.getAttackerUnitName() << L" (" << battle.getAttackerUnitId() << L") loses "
         << battle.getAttackerLosses() << L". Damaged units: " << joinDamaged(battle.getAttackerDamagedUnitIds()) << L"\r\n"
         << L"Defender " << battle.getDefenderUnitName() << L" (" << battle.getDefenderUnitId() << L") loses "
         << battle.getDefenderLosses() << L". Damaged units: " << joinDamaged(battle.getDefenderDamagedUnitIds()) << L"\r\n"
         << L"Spoils: " << formatSpoils(battle);
  return stream.str();
}

} // namespace BattleFormattingUtils
