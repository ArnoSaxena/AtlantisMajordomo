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
 * File: FactionRepository.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/FactionRepository.hpp"

#include <algorithm>
#include <deque>
#include <utility>

bool FactionRepository::add(int factionNumber,
                            std::wstring name,
                            bool mainFaction,
                            int month,
                            int year,
                            std::wstring password,
                            int taxedOrTradedRegionsCurrent,
                            int taxedOrTradedRegionsMax,
                            int quartermastersCurrent,
                            int quartermastersMax,
                            int magesCurrent,
                            int magesMax,
                            int apprenticesCurrent,
                            int apprenticesMax,
                            int commandUnitNumber,
                            Faction::Attitude defaultAttitude,
                            std::map<int, Faction::Attitude> declaredAttitudes,
                            int unclaimedSilver)
{
  if (findByNumber(factionNumber) != nullptr)
  {
    lastError_ = L"Faction number already exists";
    return false;
  }

  factions_.emplace_back(factionNumber,
                        std::move(name),
                        mainFaction,
                        month,
                        year,
                        std::move(password),
                        taxedOrTradedRegionsCurrent,
                        taxedOrTradedRegionsMax,
                        quartermastersCurrent,
                        quartermastersMax,
                        magesCurrent,
                        magesMax,
                        apprenticesCurrent,
                        apprenticesMax,
                        commandUnitNumber,
                        defaultAttitude,
                        std::move(declaredAttitudes),
                        unclaimedSilver);
  lastError_.clear();
  return true;
}

bool FactionRepository::removeByNumber(int factionNumber)
{
  const auto it = std::find_if(
    factions_.begin(),
    factions_.end(),
    [factionNumber](const Faction& faction)
    {
      return faction.getFactionNumber() == factionNumber;
    }
  );

  if (it == factions_.end())
  {
    lastError_ = L"Faction not found";
    return false;
  }

  factions_.erase(it);
  lastError_.clear();
  return true;
}

bool FactionRepository::updateName(int factionNumber, std::wstring newName)
{
  Faction* faction = findByNumber(factionNumber);
  if (!faction)
  {
    lastError_ = L"Faction not found";
    return false;
  }

  faction->setName(std::move(newName));
  lastError_.clear();
  return true;
}

bool FactionRepository::setMainFaction(int factionNumber, bool mainFaction)
{
  // there can be only one
  Faction* currentMainFaction = getMainFaction();
  if (mainFaction && currentMainFaction && factionNumber != currentMainFaction->getFactionNumber())
  {
    currentMainFaction->setMainFaction(false);
  }

  Faction* faction = findByNumber(factionNumber);
  if (!faction)
  {
    lastError_ = L"Faction not found";
    return false;
  }

  faction->setMainFaction(mainFaction);
  lastError_.clear();
  return true;
}

Faction* FactionRepository::findByNumber(int factionNumber)
{
  const auto it = std::find_if(
    factions_.begin(),
    factions_.end(),
    [factionNumber](const Faction& faction)
    {
      return faction.getFactionNumber() == factionNumber;
    }
  );

  return it == factions_.end() ? nullptr : &(*it);
}

const Faction* FactionRepository::findByNumber(int factionNumber) const
{
  const auto it = std::find_if(
    factions_.cbegin(),
    factions_.cend(),
    [factionNumber](const Faction& faction)
    {
      return faction.getFactionNumber() == factionNumber;
    }
  );

  return it == factions_.cend() ? nullptr : &(*it);
}

int FactionRepository::getMainFactionNumber() const
{
  const Faction* mainFaction = getMainFaction();
  return mainFaction != nullptr ? mainFaction->getFactionNumber() : 0;
}

Faction* FactionRepository::getMainFaction()
{
  const auto it = std::find_if(
    factions_.begin(),
    factions_.end(),
    [](const Faction& faction)
    {
      return faction.isMainFaction();
    }
  );

  return it == factions_.end() ? nullptr : &(*it);
}

const Faction* FactionRepository::getMainFaction() const
{
  const auto it = std::find_if(
    factions_.cbegin(),
    factions_.cend(),
    [](const Faction& faction)
    {
      return faction.isMainFaction();
    }
  );

  return it == factions_.cend() ? nullptr : &(*it);
}

void FactionRepository::clear()
{
  factions_.clear();
  lastError_.clear();
}

std::size_t FactionRepository::size() const
{
  return factions_.size();
}

const Faction& FactionRepository::at(std::size_t index) const
{
  return factions_.at(index);
}

const std::wstring& FactionRepository::getLastError() const
{
  return lastError_;
}
