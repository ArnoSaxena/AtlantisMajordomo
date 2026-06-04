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
 * File: Item.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Item.hpp"

#include <utility>

Item::Item(std::wstring identifierToken,
          std::wstring itemName,
          int weight,
          bool meeleWeapon,
          bool rangedWeapon,
          bool armour,
          bool resource,
          bool mount,
          int moves,
          int walkCapacity,
          int rideCapacity,
          int swimCapacity,
          int flyCapacity,
          bool man)
  : identifierToken_(std::move(identifierToken))
  , itemName_(std::move(itemName))
  , weight_(weight)
  , meeleWeapon_(meeleWeapon)
  , rangedWeapon_(rangedWeapon)
  , armour_(armour)
  , resource_(resource)
  , mount_(mount)
  , moves_(moves)
  , walkCapacity_(walkCapacity)
  , rideCapacity_(rideCapacity)
  , swimCapacity_(swimCapacity)
  , flyCapacity_(flyCapacity)
  , man_(man)
{
}

const std::wstring& Item::getIdentifierToken() const
{
  return identifierToken_;
}

const std::wstring& Item::getItemName() const
{
  return itemName_;
}

int Item::getWeight() const
{
  return weight_;
}

bool Item::isMeeleWeapon() const
{
  return meeleWeapon_;
}

bool Item::isRangedWeapon() const
{
  return rangedWeapon_;
}

bool Item::isArmour() const
{
  return armour_;
}

bool Item::isResource() const
{
  return resource_;
}

bool Item::isMount() const
{
  return mount_;
}

int Item::getMoves() const
{
  return moves_;
}

int Item::getWalkCapacity() const
{
  return walkCapacity_;
}

int Item::getRideCapacity() const
{
  return rideCapacity_;
}

int Item::getSwimCapacity() const
{
  return swimCapacity_;
}

int Item::getFlyCapacity() const
{
  return flyCapacity_;
}

int Item::getShipSpeedHexesPerMonth() const
{
  return shipSpeedHexesPerMonth_;
}

int Item::getShipSailingSkillRequired() const
{
  return shipSailingSkillRequired_;
}

int Item::getMagesStudy() const
{
  return magesStudy_;
}

bool Item::isMan() const
{
  return man_;
}

int Item::getDefaultSkillMax() const
{
  return defaultSkillMax_;
}

const std::wstring& Item::getFullText() const
{
  return fullText_;
}

const std::map<std::wstring, int>& Item::getResources() const
{
  return resources_;
}

const std::map<std::wstring, int>& Item::getSkillsMax() const
{
  return skillsMax_;
}

const std::map<std::wstring, int>& Item::getProductionSkill() const
{
  return productionSkill_;
}

void Item::setItemName(std::wstring itemName)
{
  itemName_ = std::move(itemName);
}

void Item::setWeight(int weight)
{
  weight_ = weight;
}

void Item::setMeeleWeapon(bool meeleWeapon)
{
  meeleWeapon_ = meeleWeapon;
}

void Item::setRangedWeapon(bool rangedWeapon)
{
  rangedWeapon_ = rangedWeapon;
}

void Item::setArmour(bool armour)
{
  armour_ = armour;
}

void Item::setResource(bool resource)
{
  resource_ = resource;
}

void Item::setMount(bool mount)
{
  mount_ = mount;
}

void Item::setMoves(int moves)
{
  moves_ = moves;
}

void Item::setWalkCapacity(int walkCapacity)
{
  walkCapacity_ = walkCapacity;
}

void Item::setRideCapacity(int rideCapacity)
{
  rideCapacity_ = rideCapacity;
}

void Item::setSwimCapacity(int swimCapacity)
{
  swimCapacity_ = swimCapacity;
}

void Item::setFlyCapacity(int flyCapacity)
{
  flyCapacity_ = flyCapacity;
}

void Item::setShipSpeedHexesPerMonth(int shipSpeedHexesPerMonth)
{
  shipSpeedHexesPerMonth_ = shipSpeedHexesPerMonth;
}

void Item::setShipSailingSkillRequired(int shipSailingSkillRequired)
{
  shipSailingSkillRequired_ = shipSailingSkillRequired;
}

void Item::setMagesStudy(int magesStudy)
{
  magesStudy_ = magesStudy;
}

void Item::setMan(bool man)
{
  man_ = man;
}

void Item::setDefaultSkillMax(int defaultSkillMax)
{
  defaultSkillMax_ = defaultSkillMax < 0 ? 0 : defaultSkillMax;
}

void Item::setFullText(std::wstring fullText)
{
  fullText_ = std::move(fullText);
}

void Item::setResources(std::map<std::wstring, int> resources)
{
  resources_ = std::move(resources);
}

void Item::setSkillsMax(std::map<std::wstring, int> skillsMax)
{
  skillsMax_ = std::move(skillsMax);
}

void Item::setProductionSkill(std::map<std::wstring, int> productionSkill)
{
  productionSkill_ = std::move(productionSkill);
}

void Item::setProductionSkillLevel(std::wstring skillId, int level)
{
  productionSkill_[std::move(skillId)] = level;
}

const std::map<std::wstring, int>& Item::getProductionHelp() const
{
  return productionHelp_;
}

void Item::setProductionHelp(std::map<std::wstring, int> productionHelp)
{
  productionHelp_ = std::move(productionHelp);
}
