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
 * File: UnitNewRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/UnitNewRepository.hpp"

#include <algorithm>

bool UnitNewRepository::add(int unitNumber,
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
                            int originalUnit)
{
  if (findByNumberAndCoordinates(unitNumber, xCoordinate, yCoordinate, zCoordinate) != nullptr)
  {
    lastError_ = L"Unit number already exists";
    return false;
  }

  if (hasUnitWithNumberAtCoordinates(unitNumber, xCoordinate, yCoordinate, zCoordinate))
  {
    lastError_ = L"Unit number and coordinates combination already exists";
    return false;
  }

  units_.emplace_back(unitNumber,
                      std::move(unitName),
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
                      originalUnit);
  lastError_.clear();
  return true;
}

bool UnitNewRepository::removeByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate)
{
  const auto it = std::find_if(
    units_.begin(),
    units_.end(),
    [unitNumber, xCoordinate, yCoordinate, zCoordinate](const UnitNew& unit)
    {
      return unit.getUnitNumber() == unitNumber &&
             unit.getXCoordinate() == xCoordinate &&
             unit.getYCoordinate() == yCoordinate &&
             unit.getZCoordinate() == zCoordinate;
    });

  if (it == units_.end())
  {
    lastError_ = L"Unit not found";
    return false;
  }

  units_.erase(it);
  lastError_.clear();
  return true;
}

void UnitNewRepository::removeByOriginUnit(int originUnitNumber)
{
  units_.erase(
    std::remove_if(
      units_.begin(),
      units_.end(),
      [originUnitNumber](const UnitNew& unit)
      {
        return unit.getOriginUnit() == originUnitNumber;
      }),
    units_.end());
  lastError_.clear();
}

std::vector<UnitNew*> UnitNewRepository::findByOriginUnit(int originUnitNumber)
{
  std::vector<UnitNew*> found;
  for (UnitNew& unit : units_)
  {
    if (unit.getOriginUnit() == originUnitNumber)
    {
      found.push_back(&unit);
    }
  }
  return found;
}

std::vector<const UnitNew*> UnitNewRepository::findByOriginUnit(int originUnitNumber) const
{
  std::vector<const UnitNew*> found;
  for (const UnitNew& unit : units_)
  {
    if (unit.getOriginUnit() == originUnitNumber)
    {
      found.push_back(&unit);
    }
  }
  return found;
}

UnitNew* UnitNewRepository::findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate)
{
  const auto it = std::find_if(
    units_.begin(),
    units_.end(),
    [unitNumber, xCoordinate, yCoordinate, zCoordinate](const UnitNew& unit)
    {
      return unit.getUnitNumber() == unitNumber &&
             unit.getXCoordinate() == xCoordinate &&
             unit.getYCoordinate() == yCoordinate &&
             unit.getZCoordinate() == zCoordinate;
    });

  return it == units_.end() ? nullptr : &(*it);
}

const UnitNew* UnitNewRepository::findByNumberAndCoordinates(int unitNumber, int xCoordinate, int yCoordinate, int zCoordinate) const
{
  const auto it = std::find_if(
    units_.cbegin(),
    units_.cend(),
    [unitNumber, xCoordinate, yCoordinate, zCoordinate](const UnitNew& unit)
    {
      return unit.getUnitNumber() == unitNumber &&
             unit.getXCoordinate() == xCoordinate &&
             unit.getYCoordinate() == yCoordinate &&
             unit.getZCoordinate() == zCoordinate;
    });

  return it == units_.cend() ? nullptr : &(*it);
}

bool UnitNewRepository::hasUnitWithNumberAtCoordinates(int unitNumber,
                                                        int xCoordinate,
                                                        int yCoordinate,
                                                        int zCoordinate) const
{
  for (const UnitNew& unit : units_)
  {
    if (unit.getUnitNumber() == unitNumber &&
        unit.getXCoordinate() == xCoordinate &&
        unit.getYCoordinate() == yCoordinate &&
        unit.getZCoordinate() == zCoordinate)
    {
      return true;
    }
  }

  return false;
}

void UnitNewRepository::clear()
{
  units_.clear();
  lastError_.clear();
}

std::size_t UnitNewRepository::size() const
{
  return units_.size();
}

const UnitNew& UnitNewRepository::at(std::size_t index) const
{
  return units_.at(index);
}

const std::wstring& UnitNewRepository::getLastError() const
{
  return lastError_;
}
