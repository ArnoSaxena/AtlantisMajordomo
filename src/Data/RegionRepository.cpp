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
 * File: RegionRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/RegionRepository.hpp"

#include <algorithm>
#include <utility>

bool RegionRepository::add(int xCoordinate,
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
                          bool visited)
{
  if (findByCoordinates(xCoordinate, yCoordinate, zCoordinate) != nullptr)
  {
    lastError_ = L"Region coordinates already exist";
    return false;
  }

  regions_.emplace_back(
    xCoordinate,
    yCoordinate,
    zCoordinate,
    std::move(regionType),
    std::move(provinceName),
    containsSettlement,
    std::move(settlementName),
    std::move(settlementType),
    std::move(peasantType),
    peasantNumber,
    wages,
    wagesMax,
    taxableIncome, 
    0,
    0,
    visited
  );
  Region& createdRegion = regions_.back();
  createdRegion.setWagesAfterOrders(0);
  createdRegion.setTaxableIncomeAfterOrders(0);
  createdRegion.setEntertainmentAfterOrders(0);
  createdRegion.setResourcesAfterOrders({});
  createdRegion.setWantedAfterOrders({});
  createdRegion.setForSaleAfterOrders({});

  lastError_.clear();
  return true;
}

bool RegionRepository::add(int xCoordinate,
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
                          bool visited)
{
  return add(
    xCoordinate,
    yCoordinate,
    1,
    std::move(regionType),
    std::move(provinceName),
    containsSettlement,
    std::move(settlementName),
    std::move(settlementType),
    std::move(peasantType),
    peasantNumber,
    wages,
    wagesMax,
    taxableIncome,
    visited
  );
}

bool RegionRepository::upsertRegion(int xCoordinate,
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
                                    bool visited)
{
  Region* existing = findByCoordinates(xCoordinate, yCoordinate, zCoordinate);
  if (existing)
  {
    const bool isLater = (newYear > existing->getYear()) ||
                        (newYear == existing->getYear() && newMonth > existing->getMonth());
    const bool isSameReportPeriod = (newYear == existing->getYear() && newMonth == existing->getMonth());
    const bool shouldPromoteVisited = visited && !existing->getVisited();

    if (isLater || isSameReportPeriod || shouldPromoteVisited)
    {
      existing->setMonth(newMonth);
      existing->setYear(newYear);
      existing->setZCoordinate(zCoordinate);
      existing->setRegionType(std::move(regionType));
      existing->setProvinceName(std::move(provinceName));
      existing->setContainsSettlement(containsSettlement);
      existing->setSettlementName(std::move(settlementName));
      existing->setSettlementType(std::move(settlementType));
      existing->setPeasantType(std::move(peasantType));
      existing->setPeasantNumber(peasantNumber);
      existing->setWages(wages);
      existing->setWagesMax(wagesMax);
      existing->setTaxableIncome(taxableIncome);
      existing->setWagesAfterOrders(0);
      existing->setTaxableIncomeAfterOrders(0);
      existing->setEntertainmentAfterOrders(0);
      existing->setResourcesAfterOrders({});
      existing->setWantedAfterOrders({});
      existing->setForSaleAfterOrders({});
      existing->setVisited(visited);
      lastError_.clear();
      return true;
    }

    return false;
  }

  regions_.emplace_back(
    xCoordinate,
    yCoordinate,
    zCoordinate,
    std::move(regionType),
    std::move(provinceName),
    containsSettlement,
    std::move(settlementName),
    std::move(settlementType),
    std::move(peasantType),
    peasantNumber,
    wages,
    wagesMax,
    taxableIncome,
    newMonth,
    newYear,
    visited
  );
  Region& createdRegion = regions_.back();
  createdRegion.setWagesAfterOrders(0);
  createdRegion.setTaxableIncomeAfterOrders(0);
  createdRegion.setEntertainmentAfterOrders(0);
  createdRegion.setResourcesAfterOrders({});
  createdRegion.setWantedAfterOrders({});
  createdRegion.setForSaleAfterOrders({});

  lastError_.clear();
  return true;
}

bool RegionRepository::removeByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate)
{
  const auto it = std::find_if(
    regions_.begin(),
    regions_.end(),
    [xCoordinate, yCoordinate, zCoordinate](const Region& region)
    {
      return region.getXCoordinate() == xCoordinate &&
            region.getYCoordinate() == yCoordinate &&
            region.getZCoordinate() == zCoordinate;
    }
  );

  if (it == regions_.end())
  {
    lastError_ = L"Region not found";
    return false;
  }

  regions_.erase(it);
  lastError_.clear();
  return true;
}

bool RegionRepository::removeByCoordinates(int xCoordinate, int yCoordinate)
{
  return removeByCoordinates(xCoordinate, yCoordinate, 1);
}

Region* RegionRepository::findByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate)
{
  const auto it = std::find_if(
    regions_.begin(),
    regions_.end(),
    [xCoordinate, yCoordinate, zCoordinate](const Region& region)
    {
      return region.getXCoordinate() == xCoordinate &&
            region.getYCoordinate() == yCoordinate &&
            region.getZCoordinate() == zCoordinate;
    }
  );

  return it == regions_.end() ? nullptr : &(*it);
}

const Region* RegionRepository::findByCoordinates(int xCoordinate, int yCoordinate, int zCoordinate) const
{
  const auto it = std::find_if(
    regions_.cbegin(),
    regions_.cend(),
    [xCoordinate, yCoordinate, zCoordinate](const Region& region)
    {
      return region.getXCoordinate() == xCoordinate &&
            region.getYCoordinate() == yCoordinate &&
            region.getZCoordinate() == zCoordinate;
    }
  );

  return it == regions_.cend() ? nullptr : &(*it);
}

Region* RegionRepository::findByCoordinates(int xCoordinate, int yCoordinate)
{
  return findByCoordinates(xCoordinate, yCoordinate, 1);
}

const Region* RegionRepository::findByCoordinates(int xCoordinate, int yCoordinate) const
{
  return findByCoordinates(xCoordinate, yCoordinate, 1);
}

void RegionRepository::clear()
{
  regions_.clear();
  lastError_.clear();
}

std::size_t RegionRepository::size() const
{
  return regions_.size();
}

const Region& RegionRepository::at(std::size_t index) const
{
  return regions_.at(index);
}

const std::wstring& RegionRepository::getLastError() const
{
  return lastError_;
}
