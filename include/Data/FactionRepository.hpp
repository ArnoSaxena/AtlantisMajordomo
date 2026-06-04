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
 * File: FactionRepository.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Faction.hpp"

#include <cstddef>
#include <deque>
#include <map>
#include <string>

/**
* @brief Repository for faction entities.
*
* Uses FactionNumber as the stable identifier and enforces uniqueness.
*/
class FactionRepository
{
public:
  FactionRepository() = default;
  ~FactionRepository() = default;

  FactionRepository(const FactionRepository&) = default;
  FactionRepository& operator=(const FactionRepository&) = default;
  FactionRepository(FactionRepository&&) = default;
  FactionRepository& operator=(FactionRepository&&) = default;

  bool add(int factionNumber,
          std::wstring name,
          bool mainFaction,
          int month = 0,
          int year = 0,
          std::wstring password = L"",
          int taxedOrTradedRegionsCurrent = 0,
          int taxedOrTradedRegionsMax = 0,
          int quartermastersCurrent = 0,
          int quartermastersMax = 0,
          int magesCurrent = 0,
          int magesMax = 0,
          int apprenticesCurrent = 0,
          int apprenticesMax = 0,
          int commandUnitNumber = 0,
          Faction::Attitude defaultAttitude = Faction::Attitude::Neutral,
          std::map<int, Faction::Attitude> declaredAttitudes = {},
          int unclaimedSilver = 0);
  bool removeByNumber(int factionNumber);

  bool updateName(int factionNumber, std::wstring newName);
  bool setMainFaction(int factionNumber, bool mainFaction);

  Faction* findByNumber(int factionNumber);
  const Faction* findByNumber(int factionNumber) const;

  int getMainFactionNumber() const;
  Faction* getMainFaction();
  const Faction* getMainFaction() const;

  void clear();

  std::size_t size() const;
  const Faction& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  // std::deque is used intentionally: push_back does not invalidate existing
  // pointers or references, so Report objects can safely hold Faction* into this
  // container across the lifetime of the repository.
  std::deque<Faction> factions_;
  std::wstring        lastError_;
};
