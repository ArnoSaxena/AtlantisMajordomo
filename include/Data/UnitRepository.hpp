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
 * File: UnitRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Unit.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/**
* @brief Repository for unit entities.
*
* Uses UnitNumber as the stable identifier and enforces uniqueness.
*/
class UnitRepository
{
public:
  // Returns the smallest unit number for a given faction, or 0 if none found.
  int findSmallestUnitNumberForFaction(int factionNumber) const;
  UnitRepository() = default;
  ~UnitRepository() = default;

  UnitRepository(const UnitRepository&) = default;
  UnitRepository& operator=(const UnitRepository&) = default;
  UnitRepository(UnitRepository&&) = default;
  UnitRepository& operator=(UnitRepository&&) = default;

  bool add(int unitNumber,
          std::wstring unitName,
          int factionNumber,
          int structureId,
          int xCoordinate,
          int yCoordinate,
          int zCoordinate,
          std::vector<std::wstring> flags,
          std::map<std::wstring, int> itemCounts,
          int weight,
          int capacityWalk,
          int capacityRide,
          int capacityFly,
          int capacitySwim,
          std::map<std::wstring, int> skills,
          int month,
          int year,
          bool onGuard = false);

  bool addOrUpdateIfLater(int unitNumber,
                          std::wstring unitName,
                          int factionNumber,
                          int structureId,
                          int xCoordinate,
                          int yCoordinate,
                          int zCoordinate,
                          std::vector<std::wstring> flags,
                          std::map<std::wstring, int> itemCounts,
                          int weight,
                          int capacityWalk,
                          int capacityRide,
                          int capacityFly,
                          int capacitySwim,
                          std::map<std::wstring, int> skills,
                          int month,
                          int year,
                          bool onGuard = false);

  bool removeByNumber(int unitNumber);

  Unit* findByNumber(int unitNumber);
  const Unit* findByNumber(int unitNumber) const;

  void clear();

  std::size_t size() const;
  const Unit& at(std::size_t index) const;

  const std::wstring& getLastError() const;

  std::vector<int> findUnitNumbersWithWarnings() const;
  int countTotalWarnings() const;
  bool hasUnitInStructureAtCoordinates(int structureId,
                                       int xCoordinate,
                                       int yCoordinate,
                                       int zCoordinate) const;

private:
  std::vector<Unit> units_;
  std::wstring      lastError_;
};
