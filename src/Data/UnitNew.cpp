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
 * File: UnitNew.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/AppData.hpp"
#include "Data/UnitNew.hpp"

#include <utility>

UnitNew::UnitNew(int unitNumber,
                 std::wstring unitName,
                 int structureId,
                 int xCoordinate,
                 int yCoordinate,
                 int zCoordinate,
                 std::vector<std::wstring> flags,
                 std::map<std::wstring, int> items,
                 int weight,
                 int capacityWalk,
                 int capacityRide,
                 int capacityFly,
                 int capacitySwim,
                 std::map<std::wstring, int> skills,
                 int month,
                 int year,
                 int originUnit)
  : unitNumber_(unitNumber)
  , unitName_(std::move(unitName))
  , unitNameAfterOrders_()
  , structureId_(structureId)
  , futureStructureId_(structureId)
  , xCoordinate_(xCoordinate)
  , yCoordinate_(yCoordinate)
  , zCoordinate_(zCoordinate)
  , flags_(std::move(flags))
  , items_(std::move(items))
  , weight_(weight)
  , capacityWalk_(capacityWalk)
  , capacityRide_(capacityRide)
  , capacityFly_(capacityFly)
  , capacitySwim_(capacitySwim)
  , skills_(std::move(skills))
  , month_(month)
  , year_(year)
  , originUnit_(originUnit)
{
}

int UnitNew::getUnitNumber() const
{
  return unitNumber_;
}

const std::wstring& UnitNew::getUnitName() const
{
  return unitName_;
}

const std::wstring& UnitNew::getUnitNameAfterOrders() const
{
  if (unitNameAfterOrders_.empty())
  {
    return getUnitName();
  }

  return unitNameAfterOrders_;
}

int UnitNew::getFactionNumber() const
{
  // UnitNew objects do not store a faction number. The main faction is
  // resolved by application logic using the current FactionRepository state.
  return AppData::getInstance().factionRepository().getMainFactionNumber();
}

int UnitNew::getStructureId() const
{
  return structureId_;
}

int UnitNew::getFutureStructureId() const
{
  return futureStructureId_;
}

int UnitNew::getXCoordinate() const
{
  return xCoordinate_;
}

int UnitNew::getYCoordinate() const
{
  return yCoordinate_;
}

int UnitNew::getZCoordinate() const
{
  return zCoordinate_;
}

const std::vector<std::wstring>& UnitNew::getFlags() const
{
  return flags_;
}

const std::map<std::wstring, int>& UnitNew::getItems() const
{
  return items_;
}

const std::map<std::wstring, int>& UnitNew::getItemsAfterOrders() const
{
  return itemsAfterOrders_;
}

int UnitNew::getWeight() const
{
  return weight_;
}

int UnitNew::getCapacityWalk() const
{
  return capacityWalk_;
}

int UnitNew::getCapacityRide() const
{
  return capacityRide_;
}

int UnitNew::getCapacityFly() const
{
  return capacityFly_;
}

int UnitNew::getCapacitySwim() const
{
  return capacitySwim_;
}

const std::map<std::wstring, int>& UnitNew::getSkills() const
{
  return skills_;
}

const std::map<std::wstring, int>& UnitNew::getSkillsAfterOrders() const
{
  return skillsAfterOrders_;
}

const std::vector<std::wstring>& UnitNew::getCanStudySkillTokens() const
{
  return canStudySkillTokens_;
}

const std::vector<std::wstring>& UnitNew::getWarnings() const
{
  return warnings_;
}

int UnitNew::getMonth() const
{
  return month_;
}

int UnitNew::getYear() const
{
  return year_;
}

bool UnitNew::isOnGuard() const
{
  // New units are not on guard.
  return false;
}

int UnitNew::getOriginUnit() const
{
  return originUnit_;
}

void UnitNew::setUnitName(std::wstring unitName)
{
  unitName_ = std::move(unitName);
}

void UnitNew::setUnitNameAfterOrders(std::wstring unitNameAfterOrders)
{
  unitNameAfterOrders_ = std::move(unitNameAfterOrders);
}

void UnitNew::setStructureId(int structureId)
{
  structureId_ = structureId;
}

void UnitNew::setFutureStructureId(int futureStructureId)
{
  futureStructureId_ = futureStructureId;
}

void UnitNew::setCoordinates(int xCoordinate, int yCoordinate, int zCoordinate)
{
  xCoordinate_ = xCoordinate;
  yCoordinate_ = yCoordinate;
  zCoordinate_ = zCoordinate;
}

void UnitNew::setFlags(std::vector<std::wstring> flags)
{
  flags_ = std::move(flags);
}

void UnitNew::setItems(std::map<std::wstring, int> itemCounts)
{
  items_ = std::move(itemCounts);
}

void UnitNew::setItemsAfterOrders(std::map<std::wstring, int> itemsAfterOrders)
{
  itemsAfterOrders_ = std::move(itemsAfterOrders);
}

void UnitNew::addFlag(std::wstring flag)
{
  flags_.push_back(std::move(flag));
}

void UnitNew::setItem(std::wstring itemToken, int amount)
{
  if (itemToken.empty() || amount <= 0)
  {
    return;
  }

  items_[std::move(itemToken)] = amount;
}

void UnitNew::setItemAfterOrders(std::wstring itemToken, int amount)
{
  if (itemToken.empty() || amount <= 0)
  {
    return;
  }

  itemsAfterOrders_[std::move(itemToken)] = amount;
}

void UnitNew::clearFlags()
{
  flags_.clear();
}

void UnitNew::clearItems()
{
  items_.clear();
}

void UnitNew::clearItemsAfterOrders()
{
  itemsAfterOrders_.clear();
}

void UnitNew::setWeight(int weight)
{
  weight_ = weight;
}

void UnitNew::setCapacityWalk(int capacityWalk)
{
  capacityWalk_ = capacityWalk;
}

void UnitNew::setCapacityRide(int capacityRide)
{
  capacityRide_ = capacityRide;
}

void UnitNew::setCapacityFly(int capacityFly)
{
  capacityFly_ = capacityFly;
}

void UnitNew::setCapacitySwim(int capacitySwim)
{
  capacitySwim_ = capacitySwim;
}

void UnitNew::setSkills(std::map<std::wstring, int> skills)
{
  skills_ = std::move(skills);
}

void UnitNew::setSkillsAfterOrders(std::map<std::wstring, int> skillsAfterOrders)
{
  skillsAfterOrders_ = std::move(skillsAfterOrders);
}

void UnitNew::setCanStudySkillTokens(std::vector<std::wstring> canStudySkillTokens)
{
  canStudySkillTokens_ = std::move(canStudySkillTokens);
}

void UnitNew::setMonth(int month)
{
  month_ = month;
}

void UnitNew::setYear(int year)
{
  year_ = year;
}

void UnitNew::setOriginUnit(int originUnit)
{
  originUnit_ = originUnit;
}

void UnitNew::addSkill(std::wstring token, int days)
{
  if (days > 0)
  {
    skills_[std::move(token)] = days;
  }
}

void UnitNew::addSkillAfterOrders(std::wstring token, int days)
{
  if (days > 0)
  {
    skillsAfterOrders_[std::move(token)] = days;
  }
}

void UnitNew::addWarning(std::wstring warning)
{
  if (warning.empty())
  {
    return;
  }

  warnings_.push_back(std::move(warning));
}

void UnitNew::clearWarnings()
{
  warnings_.clear();
}

void UnitNew::clearSkills()
{
  skills_.clear();
}

void UnitNew::clearSkillsAfterOrders()
{
  skillsAfterOrders_.clear();
}

void UnitNew::clearCanStudySkillTokens()
{
  canStudySkillTokens_.clear();
}
