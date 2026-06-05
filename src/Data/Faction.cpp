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
 * File: Faction.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Faction.hpp"

#include "Data/UnitRepository.hpp"

#include <utility>

int Faction::resolveCommandUnitNumber(const UnitRepository& unitRepo) const
{
  if (commandUnitNumber_ > 0)
  {
    return commandUnitNumber_;
  }

  return unitRepo.findSmallestUnitNumberForFaction(factionNumber_);
}

Faction::Faction(int factionNumber,
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
                Attitude defaultAttitude,
                std::map<int, Attitude> declaredAttitudes,
                int unclaimedSilver)
  : factionNumber_(factionNumber)
  , name_(std::move(name))
  , mainFaction_(mainFaction)
  , month_(month)
  , year_(year)
  , password_(std::move(password))
  , taxedOrTradedRegionsCurrent_(taxedOrTradedRegionsCurrent)
  , taxedOrTradedRegionsMax_(taxedOrTradedRegionsMax)
  , quartermastersCurrent_(quartermastersCurrent)
  , quartermastersMax_(quartermastersMax)
  , magesCurrent_(magesCurrent)
  , magesMax_(magesMax)
  , apprenticesCurrent_(apprenticesCurrent)
  , apprenticesMax_(apprenticesMax)
  , unclaimedSilver_(unclaimedSilver)
  , commandUnitNumber_(commandUnitNumber)
  , defaultAttitude_(defaultAttitude)
  , declaredAttitudes_(std::move(declaredAttitudes))
{
}

int Faction::getFactionNumber() const
{
  return factionNumber_;
}

const std::wstring& Faction::getName() const
{
  return name_;
}

bool Faction::isMainFaction() const
{
  return mainFaction_;
}

int Faction::getMonth() const
{
  return month_;
}

int Faction::getYear() const
{
  return year_;
}

const std::wstring& Faction::getPassword() const
{
  return password_;
}

int Faction::getTaxedOrTradedRegionsCurrent() const
{
  return taxedOrTradedRegionsCurrent_;
}

int Faction::getTaxedOrTradedRegionsMax() const
{
  return taxedOrTradedRegionsMax_;
}

int Faction::getQuartermastersCurrent() const
{
  return quartermastersCurrent_;
}

int Faction::getQuartermastersMax() const
{
  return quartermastersMax_;
}

int Faction::getMagesCurrent() const
{
  return magesCurrent_;
}

int Faction::getMagesMax() const
{
  return magesMax_;
}

int Faction::getApprenticesCurrent() const
{
  return apprenticesCurrent_;
}

int Faction::getApprenticesMax() const
{
  return apprenticesMax_;
}

int Faction::getUnclaimedSilver() const
{
  return unclaimedSilver_;
}

int Faction::getUnclaimedSilverAfterOrders() const
{
  return unclaimedSilverAfterOrders_;
}

int Faction::getCommandUnitNumber() const
{
  return commandUnitNumber_;
}

Faction::Attitude Faction::getDefaultAttitude() const
{
  return defaultAttitude_;
}

const std::map<int, Faction::Attitude>& Faction::getDeclaredAttitudes() const
{
  return declaredAttitudes_;
}

void Faction::setName(std::wstring name)
{
  name_ = std::move(name);
}

void Faction::setMainFaction(bool mainFaction)
{
  mainFaction_ = mainFaction;
}

void Faction::setMonth(int month)
{
  month_ = month;
}

void Faction::setYear(int year)
{
  year_ = year;
}

void Faction::setPassword(std::wstring password)
{
  password_ = std::move(password);
}

void Faction::setTaxedOrTradedRegionsCurrent(int value)
{
  taxedOrTradedRegionsCurrent_ = value;
}

void Faction::setTaxedOrTradedRegionsMax(int value)
{
  taxedOrTradedRegionsMax_ = value;
}

void Faction::setQuartermastersCurrent(int value)
{
  quartermastersCurrent_ = value;
}

void Faction::setQuartermastersMax(int value)
{
  quartermastersMax_ = value;
}

void Faction::setMagesCurrent(int value)
{
  magesCurrent_ = value;
}

void Faction::setMagesMax(int value)
{
  magesMax_ = value;
}

void Faction::setApprenticesCurrent(int value)
{
  apprenticesCurrent_ = value;
}

void Faction::setApprenticesMax(int value)
{
  apprenticesMax_ = value;
}

void Faction::setUnclaimedSilver(int value)
{
  unclaimedSilver_ = value;
}

void Faction::setUnclaimedSilverAfterOrders(int value)
{
  unclaimedSilverAfterOrders_ = value;
}

void Faction::setCommandUnitNumber(int unitNumber)
{
  commandUnitNumber_ = unitNumber;
}

void Faction::setDefaultAttitude(Attitude attitude)
{
  defaultAttitude_ = attitude;
}

void Faction::setDeclaredAttitude(int factionNumber, Attitude attitude)
{
  declaredAttitudes_[factionNumber] = attitude;
}

void Faction::removeDeclaredAttitude(int factionNumber)
{
  declaredAttitudes_.erase(factionNumber);
}

void Faction::clearDeclaredAttitudes()
{
  declaredAttitudes_.clear();
}
