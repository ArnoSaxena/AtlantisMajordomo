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
 * File: RegionRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Region.hpp"

#include <cstddef>
#include <string>
#include <vector>

/**
* @brief Repository for region entities.
*
* Uses coordinate triple (x, y, z) as the identifier and enforces uniqueness.
*/
class RegionRepository
{
public:
  RegionRepository() = default;
  ~RegionRepository() = default;

  RegionRepository(const RegionRepository&) = default;
  RegionRepository& operator=(const RegionRepository&) = default;
  RegionRepository(RegionRepository&&) = default;
  RegionRepository& operator=(RegionRepository&&) = default;

  bool add(int xCoordinate,
          int yCoordinate,
          int zCoordinate,
          std::wstring regionType,
          std::wstring provinceName,
          bool containsSettlement,
          std::wstring settlementName,
          std::wstring settlementType,
          std::wstring peasantType,
          int peasantNumber,
          double wages,
          int wagesMax,
          int taxableIncome,
          bool visited = false);

  bool add(int xCoordinate,
          int yCoordinate,
          std::wstring regionType,
          std::wstring provinceName,
          bool containsSettlement,
          std::wstring settlementName,
          std::wstring settlementType,
          std::wstring peasantType,
          int peasantNumber,
          double wages,
          int wagesMax,
          int taxableIncome,
          bool visited = false);
  
  /**
  * @brief Adds or updates a region with temporal metadata.
  *
  * If region with (x,y) already exists and newMonth > existing month, updates it.
  * Otherwise, creates a new region if it doesn't exist.
  * 
  * @param xCoordinate X coordinate.
  * @param yCoordinate Y coordinate.
  * @param regionType Type of region (plain, ocean, etc).
  * @param provinceName Name of province.
  * @param peasantType Type of peasants.
  * @param peasantNumber Number of peasants.
  * @param newMonth Month from the report (for temporal tracking).
  * @param newYear Year from the report (for temporal tracking).
  * @return true if added/updated, false if region already exists with same or newer month.
  */
  bool upsertRegion(int xCoordinate,
                    int yCoordinate,
                    int zCoordinate,
                    std::wstring regionType,
                    std::wstring provinceName,
                    bool containsSettlement,
                    std::wstring settlementName,
                    std::wstring settlementType,
                    std::wstring peasantType,
                    int peasantNumber,
                    double wages,
                    int wagesMax,
                    int taxableIncome,
                    int newMonth,
                    int newYear,
                    bool visited);

  bool removeByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate);
  bool removeByCoordinates(int xCoordinate, int yCoordinate);

  Region* findByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate);
  const Region* findByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate) const;
  Region* findByCoordinates(int xCoordinate, int yCoordinate);
  const Region* findByCoordinates(int xCoordinate, int yCoordinate) const;

  void clear();

  std::size_t size() const;
  const Region& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<Region> regions_;
  std::wstring        lastError_;
};
