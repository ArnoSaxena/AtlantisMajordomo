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
* unitNumber must be unique as well. While there can be multiple units with
* the same unitNumber within the repository.
*
* **Lifecycle and Clearing Constraints**:
* UnitNew entities are generated only by order simulations (FORM orders).
* This repository must be cleared:
* - When a new report is loaded that covers a later period than the current one
* - When order simulation is rebuilt for a new game month
* - When order simulation is triggered for any other reason
*
* The clear() method is called by OrderBusinessLogic::rebuildUnitNewRepositoryFromOrders()
* and by ReportRepository::addFromFile() when a later report is loaded.
* UnitNew entities are ephemeral and should never persist across these operations.
*/
class UnitNewRepository
{
public:
  UnitNewRepository() = default;
  virtual ~UnitNewRepository() = default;

  UnitNewRepository(const UnitNewRepository&) = default;
  UnitNewRepository& operator=(const UnitNewRepository&) = default;
  UnitNewRepository(UnitNewRepository&&) = default;
  UnitNewRepository& operator=(UnitNewRepository&&) = default;

  virtual bool add(int unitNumber,
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

  virtual UnitNew* findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate);
  virtual const UnitNew* findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate) const;
  std::vector<UnitNew*> findByOriginUnit(int originUnitNumber);
  std::vector<const UnitNew*> findByOriginUnit(int originUnitNumber) const;

  bool hasUnitWithNumberAtCoordinates(int unitNumber,
                                      int xCoordinate,
                                      int yCoordinate,
                                      int zCoordinate) const;

  virtual void clear();

  virtual std::size_t size() const;
  virtual const UnitNew& at(std::size_t index) const;

  const std::wstring& getLastError() const;
  int countTotalWarnings() const;

private:
  std::vector<UnitNew> units_;
  std::wstring lastError_;
};
