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
 * File: Unit.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Unit.hpp"
#include "Data/Skill.hpp"

#include <utility>

Unit::Unit(int unitNumber,
          std::wstring unitName,
          int factionNumber,
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
          bool onGuard)
  : unitNumber_(unitNumber)
  , unitName_(std::move(unitName))
  , unitNameAfterOrders_()
  , factionNumber_(factionNumber)
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
  , onGuard_(onGuard)
{
}

int Unit::getUnitNumber() const
{
  return unitNumber_;
}

const std::wstring& Unit::getUnitName() const
{
  return unitName_;
}

const std::wstring& Unit::getUnitNameAfterOrders() const
{
  if (unitNameAfterOrders_.empty())
  {
    return getUnitName();
  }

  return unitNameAfterOrders_;
}

int Unit::getFactionNumber() const
{
  return factionNumber_;
}

int Unit::getStructureId() const
{
  return structureId_;
}

int Unit::getFutureStructureId() const
{
  return futureStructureId_;
}

int Unit::getXCoordinate() const
{
  return xCoordinate_;
}

int Unit::getYCoordinate() const
{
  return yCoordinate_;
}

int Unit::getZCoordinate() const
{
  return zCoordinate_;
}

const std::vector<std::wstring>& Unit::getFlags() const
{
  return flags_;
}

const std::map<std::wstring, int>& Unit::getItems() const
{
  return items_;
}

const std::map<std::wstring, int>& Unit::getItemsAfterOrders() const
{
  return itemsAfterOrders_;
}

int Unit::getWeight() const
{
  return weight_;
}

int Unit::getCapacityWalk() const
{
  return capacityWalk_;
}

int Unit::getCapacityRide() const
{
  return capacityRide_;
}

int Unit::getCapacityFly() const
{
  return capacityFly_;
}

int Unit::getCapacitySwim() const
{
  return capacitySwim_;
}

const std::map<std::wstring, int>& Unit::getSkills() const
{
  return skills_;
}

const std::map<std::wstring, int>& Unit::getSkillsAfterOrders() const
{
  return skillsAfterOrders_;
}

const std::vector<std::wstring>& Unit::getCanStudySkillTokens() const
{
  return canStudySkillTokens_;
}

const std::vector<std::wstring>& Unit::getOrders() const
{
  return orders_;
}

const std::vector<std::wstring>& Unit::getWarnings() const
{
  return warnings_;
}

int Unit::getMonth() const
{
  return month_;
}

int Unit::getYear() const
{
  return year_;
}

bool Unit::isOnGuard() const
{
  return onGuard_;
}

void Unit::setUnitName(std::wstring unitName)
{
  unitName_ = std::move(unitName);
}

void Unit::setUnitNameAfterOrders(std::wstring unitNameAfterOrders)
{
  unitNameAfterOrders_ = std::move(unitNameAfterOrders);
}

void Unit::setFactionNumber(int factionNumber)
{
  factionNumber_ = factionNumber;
}

void Unit::setStructureId(int structureId)
{
  structureId_ = structureId;
}

void Unit::setFutureStructureId(int futureStructureId)
{
  futureStructureId_ = futureStructureId;
}

void Unit::setCoordinates(int xCoordinate, int yCoordinate, int zCoordinate)
{
  xCoordinate_ = xCoordinate;
  yCoordinate_ = yCoordinate;
  zCoordinate_ = zCoordinate;
}

void Unit::setFlags(std::vector<std::wstring> flags)
{
  flags_ = std::move(flags);
}

void Unit::setItems(std::map<std::wstring, int> items)
{
  items_ = std::move(items);
}

void Unit::setItemsAfterOrders(std::map<std::wstring, int> items)
{
  itemsAfterOrders_ = std::move(items);
}

void Unit::addFlag(std::wstring flag)
{
  flags_.push_back(std::move(flag));
}

void Unit::setItem(std::wstring itemToken, int amount)
{
  if (itemToken.empty() || amount <= 0)
  {
    return;
  }

  items_[std::move(itemToken)] = amount;
}

void Unit::setItemAfterOrders(std::wstring itemToken, int amount)
{
  if (itemToken.empty() || amount <= 0)
  {
    return;
  }

  itemsAfterOrders_[std::move(itemToken)] = amount;
}

void Unit::clearFlags()
{
  flags_.clear();
}

void Unit::clearItems()
{
  items_.clear();
}

void Unit::setWeight(int weight)
{
  weight_ = weight;
}

void Unit::setCapacityWalk(int capacityWalk)
{
  capacityWalk_ = capacityWalk;
}

void Unit::setCapacityRide(int capacityRide)
{
  capacityRide_ = capacityRide;
}

void Unit::setCapacityFly(int capacityFly)
{
  capacityFly_ = capacityFly;
}

void Unit::setCapacitySwim(int capacitySwim)
{
  capacitySwim_ = capacitySwim;
}

void Unit::setSkills(std::map<std::wstring, int> skills)
{
  skills_ = std::move(skills);
}

void Unit::setSkillsAfterOrders(std::map<std::wstring, int> skillsAfterOrders)
{
  skillsAfterOrders_ = std::move(skillsAfterOrders);
}

void Unit::setCanStudySkillTokens(std::vector<std::wstring> canStudySkillTokens)
{
  canStudySkillTokens_ = std::move(canStudySkillTokens);
}

void Unit::setOrders(std::vector<std::wstring> orders)
{
  orders_ = std::move(orders);
}

void Unit::setMonth(int month)
{
  month_ = month;
}

void Unit::setYear(int year)
{
  year_ = year;
}

void Unit::setOnGuard(bool onGuard)
{
  onGuard_ = onGuard;
}

void Unit::addSkill(std::wstring token, int days)
{
  if (days > 0)
  {
    skills_[std::move(token)] = days;
  }
}

void Unit::addSkillAfterOrders(std::wstring token, int days)
{
  if (days > 0)
  {
    skillsAfterOrders_[std::move(token)] = days;
  }
}

void Unit::addOrder(std::wstring order)
{
  orders_.push_back(std::move(order));
}

void Unit::addWarning(std::wstring warning)
{
  if (warning.empty())
  {
    return;
  }

  warnings_.push_back(std::move(warning));
}

void Unit::clearWarnings()
{
  warnings_.clear();
}

void Unit::clearSkills()
{
  skills_.clear();
}

void Unit::clearSkillsAfterOrders()
{
  skillsAfterOrders_.clear();
}

void Unit::clearCanStudySkillTokens()
{
  canStudySkillTokens_.clear();
}

void Unit::clearOrders()
{
  orders_.clear();
}

/**
* @brief Get days value of the given skill. If skill is not found return 0.
*/
int Unit::getSkillDays(const std::wstring& skillToken) const
{
  for (const auto& [unitSkillToken, days] : getSkills())
  {
    if (unitSkillToken != skillToken)
    {
      continue;
    }

    return days;
  }
  return 0;
}

int Unit::getSkillsAfterOrdersDays(const std::wstring& skillToken) const
{
  for (const auto& [unitSkillToken, days] : getSkillsAfterOrders())
  {
    if (unitSkillToken != skillToken)
    {
      continue;
    }

    return days;
  }
  return 0;
}