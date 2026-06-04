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
 * File: UnitNewRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/UnitNew.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/**
* @brief Repository for new unit entities.
*
* UnitNumber is used as the stable identifier and cannot be modified after
* construction. For new units, the combination of unitNumber and coordinates
* must remain unique within the repository. Within one coordinates set, the
* unitNumber must be unique as well. While there can be multiple units whit
* the same unitNumber within the repository.
*/
class UnitNewRepository
{
public:
  UnitNewRepository() = default;
  ~UnitNewRepository() = default;

  UnitNewRepository(const UnitNewRepository&) = default;
  UnitNewRepository& operator=(const UnitNewRepository&) = default;
  UnitNewRepository(UnitNewRepository&&) = default;
  UnitNewRepository& operator=(UnitNewRepository&&) = default;

  bool add(int unitNumber,
           std::wstring unitName,
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
           int originalUnit);

  bool removeByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate);
  void removeByOriginUnit(int originUnitNumber);

  UnitNew* findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate);
  const UnitNew* findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate) const;
  std::vector<UnitNew*> findByOriginUnit(int originUnitNumber);
  std::vector<const UnitNew*> findByOriginUnit(int originUnitNumber) const;

  bool hasUnitWithNumberAtCoordinates(int unitNumber,
                                      int xCoordinate,
                                      int yCoordinate,
                                      int zCoordinate) const;

  void clear();

  std::size_t size() const;
  const UnitNew& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<UnitNew> units_;
  std::wstring lastError_;
};
