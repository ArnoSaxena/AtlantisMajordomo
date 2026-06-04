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
 * File: BattleRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/BattleRepository.hpp"

#include <algorithm>
#include <set>
#include <utility>

bool BattleRepository::add(Battle battleValue)
{
  if (findByIdentifier(battleValue.getIdentifier()) != nullptr)
  {
    return false;
  }

  battles_.push_back(std::move(battleValue));
  return true;
}

const Battle* BattleRepository::findByIdentifier(const std::wstring& identifier) const
{
  const auto it = std::find_if(
    battles_.cbegin(),
    battles_.cend(),
    [&identifier](const Battle& battle)
    {
      return battle.getIdentifier() == identifier;
    }
  );

  return it == battles_.cend() ? nullptr : &(*it);
}

std::vector<std::pair<int, int>> BattleRepository::getAvailablePeriodsDescending() const
{
  std::set<std::pair<int, int>, std::greater<>> uniquePeriods;
  for (const Battle& battle : battles_)
  {
    if (battle.getMonth() < 1 || battle.getMonth() > 12 || battle.getYear() <= 0)
    {
      continue;
    }

    uniquePeriods.insert({ battle.getYear(), battle.getMonth() });
  }

  std::vector<std::pair<int, int>> periods;
  periods.reserve(uniquePeriods.size());
  for (const auto& [year, month] : uniquePeriods)
  {
    periods.push_back({ month, year });
  }

  return periods;
}

bool BattleRepository::getLatestPeriod(int& month, int& year) const
{
  const std::vector<std::pair<int, int>> periods = getAvailablePeriodsDescending();
  if (periods.empty())
  {
    return false;
  }

  month = periods.front().first;
  year = periods.front().second;
  return true;
}

std::vector<const Battle*> BattleRepository::findByPeriod(int month, int year) const
{
  std::vector<const Battle*> results;
  for (const Battle& battle : battles_)
  {
    if (battle.getMonth() == month && battle.getYear() == year)
    {
      results.push_back(&battle);
    }
  }

  return results;
}

bool BattleRepository::hasBattleInRegionForPeriod(int xCoordinate,
                                                  int yCoordinate,
                                                  int zCoordinate,
                                                  int month,
                                                  int year) const
{
  const auto it = std::find_if(
    battles_.cbegin(),
    battles_.cend(),
    [xCoordinate, yCoordinate, zCoordinate, month, year](const Battle& battle)
    {
      return battle.getRegionXCoordinate() == xCoordinate &&
             battle.getRegionYCoordinate() == yCoordinate &&
             battle.getRegionZCoordinate() == zCoordinate &&
             battle.getMonth() == month &&
             battle.getYear() == year;
    }
  );

  return it != battles_.cend();
}

bool BattleRepository::isUnitDamagedInAnyBattleForPeriod(int unitId, int month, int year) const
{
  if (unitId <= 0)
  {
    return false;
  }

  for (const Battle& battle : battles_)
  {
    if (battle.getMonth() != month || battle.getYear() != year)
    {
      continue;
    }

    const auto attackerIt = std::find(battle.getAttackerDamagedUnitIds().cbegin(),
                                      battle.getAttackerDamagedUnitIds().cend(),
                                      unitId);
    if (attackerIt != battle.getAttackerDamagedUnitIds().cend())
    {
      return true;
    }

    const auto defenderIt = std::find(battle.getDefenderDamagedUnitIds().cbegin(),
                                      battle.getDefenderDamagedUnitIds().cend(),
                                      unitId);
    if (defenderIt != battle.getDefenderDamagedUnitIds().cend())
    {
      return true;
    }
  }

  return false;
}

void BattleRepository::clear()
{
  battles_.clear();
}

std::size_t BattleRepository::size() const
{
  return battles_.size();
}

const Battle& BattleRepository::at(std::size_t index) const
{
  return battles_.at(index);
}
