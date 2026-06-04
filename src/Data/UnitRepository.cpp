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
 * File: UnitRepository.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/UnitRepository.hpp"

#include <algorithm>
#include <limits>
#include <utility>

int UnitRepository::findSmallestUnitNumberForFaction(int factionNumber) const
{
  if (factionNumber <= 0)
  {
    return 0;
  }

  int smallest = std::numeric_limits<int>::max();
  for (const auto& unit : units_)
  {
    if (unit.getFactionNumber() == factionNumber && unit.getUnitNumber() > 0)
    {
      smallest = std::min(smallest, unit.getUnitNumber());
    }
  }

  return smallest == std::numeric_limits<int>::max() ? 0 : smallest;
}

bool UnitRepository::add(int unitNumber,
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
                        bool onGuard)
{
  if (findByNumber(unitNumber) != nullptr)
  {
    lastError_ = L"Unit number already exists";
    return false;
  }

  units_.emplace_back(unitNumber,
                      std::move(unitName),
                      factionNumber,
                      structureId,
                      xCoordinate,
                      yCoordinate,
                      zCoordinate,
                      std::move(flags),
                      std::move(itemCounts),
                      weight,
                      capacityWalk,
                      capacityRide,
                      capacityFly,
                      capacitySwim,
                      std::move(skills),
                      month,
                      year,
                      onGuard);
  lastError_.clear();
  return true;
}

bool UnitRepository::addOrUpdateIfLater(int unitNumber,
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
                                        bool onGuard)
{
  Unit* existing = findByNumber(unitNumber);
  if (existing == nullptr)
  {
    return add(unitNumber,
              std::move(unitName),
              factionNumber,
              structureId,
              xCoordinate,
              yCoordinate,
              zCoordinate,
              std::move(flags),
              std::move(itemCounts),
              weight,
              capacityWalk,
              capacityRide,
              capacityFly,
              capacitySwim,
              std::move(skills),
              month,
              year,
              onGuard);
  }

  const bool isLater = (year > existing->getYear()) ||
                      (year == existing->getYear() && month > existing->getMonth());
  if (!isLater)
  {
    lastError_.clear();
    return true;
  }

  existing->setUnitName(std::move(unitName));
  existing->setFactionNumber(factionNumber);
  existing->setStructureId(structureId);
  existing->setCoordinates(xCoordinate, yCoordinate, zCoordinate);
  existing->setFlags(std::move(flags));
  existing->setItems(std::move(itemCounts));
  existing->setWeight(weight);
  existing->setCapacityWalk(capacityWalk);
  existing->setCapacityRide(capacityRide);
  existing->setCapacityFly(capacityFly);
  existing->setCapacitySwim(capacitySwim);
  existing->setSkills(std::move(skills));
  existing->setMonth(month);
  existing->setYear(year);
  existing->setOnGuard(onGuard);
  lastError_.clear();
  return true;
}

bool UnitRepository::removeByNumber(int unitNumber)
{
  const auto it = std::find_if(
    units_.begin(),
    units_.end(),
    [unitNumber](const Unit& unit)
    {
      return unit.getUnitNumber() == unitNumber;
    }
  );

  if (it == units_.end())
  {
    lastError_ = L"Unit not found";
    return false;
  }

  units_.erase(it);
  lastError_.clear();
  return true;
}

Unit* UnitRepository::findByNumber(int unitNumber)
{
  const auto it = std::find_if(
    units_.begin(),
    units_.end(),
    [unitNumber](const Unit& unit)
    {
      return unit.getUnitNumber() == unitNumber;
    }
  );

  return it == units_.end() ? nullptr : &(*it);
}

const Unit* UnitRepository::findByNumber(int unitNumber) const
{
  const auto it = std::find_if(
    units_.cbegin(),
    units_.cend(),
    [unitNumber](const Unit& unit)
    {
      return unit.getUnitNumber() == unitNumber;
    }
  );

  return it == units_.cend() ? nullptr : &(*it);
}

void UnitRepository::clear()
{
  units_.clear();
  lastError_.clear();
}

std::size_t UnitRepository::size() const
{
  return units_.size();
}

const Unit& UnitRepository::at(std::size_t index) const
{
  return units_.at(index);
}

const std::wstring& UnitRepository::getLastError() const
{
  return lastError_;
}

std::vector<int> UnitRepository::findUnitNumbersWithWarnings() const
{
  std::vector<int> result;
  for (const Unit& unit : units_)
  {
    if (!unit.getWarnings().empty())
    {
      result.push_back(unit.getUnitNumber());
    }
  }
  return result;
}

int UnitRepository::countTotalWarnings() const
{
  int count = 0;
  for (const Unit& unit : units_)
  {
    count += static_cast<int>(unit.getWarnings().size());
  }
  return count;
}

bool UnitRepository::hasUnitInStructureAtCoordinates(int structureId,
                                                     int xCoordinate,
                                                     int yCoordinate,
                                                     int zCoordinate) const
{
  if (structureId == 0)
  {
    return false;
  }

  for (const Unit& unit : units_)
  {
    if (unit.getStructureId() == structureId &&
        unit.getXCoordinate() == xCoordinate &&
        unit.getYCoordinate() == yCoordinate &&
        unit.getZCoordinate() == zCoordinate)
    {
      return true;
    }
  }

  return false;
}
