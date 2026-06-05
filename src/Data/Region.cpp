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
 * File: Region.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Region.hpp"

#include <map>
#include <utility>

Region::Region(int xCoordinate,
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
              int month,
              int year,
              bool visited)
  : xCoordinate_(xCoordinate)
  , yCoordinate_(yCoordinate)
  , zCoordinate_(zCoordinate)
  , month_(month)
  , year_(year)
  , regionType_(std::move(regionType))
  , provinceName_(std::move(provinceName))
  , containsSettlement_(containsSettlement)
  , settlementName_(std::move(settlementName))
  , settlementType_(std::move(settlementType))
  , peasantType_(std::move(peasantType))
  , peasantNumber_(peasantNumber)
  , wages_(wages)
  , wagesMax_(wagesMax)
  , taxableIncome_(taxableIncome)
  , visited_(visited)
{
}

Region::Region(int xCoordinate,
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
              int month,
              int year,
              bool visited)
  : Region(
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
      month,
      year,
      visited
    )
{
}

// getter

int Region::getXCoordinate() const
{
  return xCoordinate_;
}

int Region::getYCoordinate() const
{
  return yCoordinate_;
}

int Region::getZCoordinate() const
{
  return zCoordinate_;
}

int Region::getMonth() const
{
  return month_;
}

int Region::getYear() const
{
  return year_;
}

const std::wstring& Region::getRegionType() const
{
  return regionType_;
}

const std::wstring& Region::getProvinceName() const
{
  return provinceName_;
}

bool Region::getContainsSettlement() const
{
  return containsSettlement_;
}

const std::wstring& Region::getSettlementName() const
{
  return settlementName_;
}

const std::wstring& Region::getSettlementType() const
{
  return settlementType_;
}

const std::wstring& Region::getPeasantType() const
{
  return peasantType_;
}

int Region::getPeasantNumber() const
{
  return peasantNumber_;
}

double Region::getWages() const
{
  return wages_;
}

int Region::getWagesMax() const
{
  return wagesMax_;
}

int Region::getWagesAfterOrders() const
{
  return wagesAfterOrders_;
}

int Region::getTaxableIncome() const
{
    return taxableIncome_;
}

int Region::getTaxableIncomeAfterOrders() const
{
  return taxableIncomeAfterOrders_;
}

int Region::getEntertainment() const
{
  return entertainment_;
}

int Region::getEntertainmentAfterOrders() const
{
  return entertainmentAfterOrders_;
}

bool Region::getVisited() const
{
  return visited_;
}

const std::vector<std::wstring>& Region::getExitDirections() const
{
  return exitDirections_;
}

bool Region::isOcean() const
{
  return regionType_ == L"ocean";
}

// setter

void Region::setMonth(int month)
{
  month_ = month;
}

void Region::setYear(int year)
{
  year_ = year;
}

void Region::setZCoordinate(int zCoordinate)
{
  zCoordinate_ = zCoordinate;
}

void Region::setRegionType(std::wstring regionType)
{
  regionType_ = std::move(regionType);
}

void Region::setProvinceName(std::wstring provinceName)
{
  provinceName_ = std::move(provinceName);
}

void Region::setContainsSettlement(bool containsSettlement)
{
  containsSettlement_ = containsSettlement;
}

void Region::setSettlementName(std::wstring settlementName)
{
  settlementName_ = std::move(settlementName);
}

void Region::setSettlementType(std::wstring settlementType)
{
  settlementType_ = std::move(settlementType);
}

void Region::setPeasantType(std::wstring peasantType)
{
  peasantType_ = std::move(peasantType);
}

void Region::setPeasantNumber(int peasantNumber)
{
  peasantNumber_ = peasantNumber;
}

void Region::setWages(double wages)
{
  wages_ = wages;
}

void Region::setWagesMax(int wagesMax)
{
  wagesMax_ = wagesMax;
}

void Region::setWagesAfterOrders(int wagesAfterOrders)
{
  wagesAfterOrders_ = wagesAfterOrders;
}

void Region::setTaxableIncome(int taxableIncome)
{
    taxableIncome_ = taxableIncome;
}

void Region::setTaxableIncomeAfterOrders(int taxableIncomeAfterOrders)
{
  taxableIncomeAfterOrders_ = taxableIncomeAfterOrders;
}

void Region::setEntertainment(int entertainment)
{
  entertainment_ = entertainment;
}

void Region::setEntertainmentAfterOrders(int entertainmentAfterOrders)
{
  entertainmentAfterOrders_ = entertainmentAfterOrders;
}

void Region::setVisited(bool visited)
{
  visited_ = visited;
}

void Region::clearExitDirections()
{
  exitDirections_.clear();
}

void Region::addExitDirection(std::wstring direction)
{
  if (exitDirections_.size() >= 6)
  {
    return;
  }

  exitDirections_.push_back(std::move(direction));
}

const std::map<std::wstring, int>& Region::getResources() const
{
  return resources_;
}

const std::map<std::wstring, std::pair<int, int>>& Region::getWanted() const
{
  return wanted_;
}

const std::map<std::wstring, int>& Region::getResourcesAfterOrders() const
{
  return resourcesAfterOrders_;
}

const std::map<std::wstring, std::pair<int, int>>& Region::getWantedAfterOrders() const
{
  return wantedAfterOrders_;
}

const std::map<std::wstring, std::pair<int, int>>& Region::getForSale() const
{
  return forSale_;
}

const std::map<std::wstring, std::pair<int, int>>& Region::getForSaleAfterOrders() const
{
  return forSaleAfterOrders_;
}

void Region::setResources(std::map<std::wstring, int> resources)
{
  resources_ = std::move(resources);
}

void Region::setResource(std::wstring token, int amount)
{
  resources_[std::move(token)] = amount;
}

void Region::setResourcesAfterOrders(std::map<std::wstring, int> resourcesAfterOrders)
{
  resourcesAfterOrders_ = std::move(resourcesAfterOrders);
}

void Region::setResourceAfterOrders(std::wstring token, int amount)
{
  resourcesAfterOrders_[std::move(token)] = amount;
}

void Region::setWanted(std::map<std::wstring, std::pair<int, int>> wanted)
{
  wanted_ = std::move(wanted);
}

void Region::setWantedAfterOrders(std::map<std::wstring, std::pair<int, int>> wantedAfterOrders)
{
  wantedAfterOrders_ = std::move(wantedAfterOrders);
}

void Region::setForSale(std::map<std::wstring, std::pair<int, int>> forSale)
{
  forSale_ = std::move(forSale);
}

void Region::setForSaleAfterOrders(std::map<std::wstring, std::pair<int, int>> forSaleAfterOrders)
{
  forSaleAfterOrders_ = std::move(forSaleAfterOrders);
}

void Region::setWantedItem(std::wstring token, int amount, int price)
{
  wanted_[std::move(token)] = std::make_pair(amount, price);
}

void Region::setWantedItemAfterOrders(std::wstring token, int amount, int price)
{
  wantedAfterOrders_[std::move(token)] = std::make_pair(amount, price);
}

void Region::setForSaleItem(std::wstring token, int amount, int price)
{
  forSale_[std::move(token)] = std::make_pair(amount, price);
}

void Region::setForSaleItemAfterOrders(std::wstring token, int amount, int price)
{
  forSaleAfterOrders_[std::move(token)] = std::make_pair(amount, price);
}
