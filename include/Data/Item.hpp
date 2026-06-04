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
 * File: Item.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>

/**
* @brief Data model for an item entity.
*
* Identifier token is the stable identifier and cannot be modified after
* construction.
*/
class Item
{
public:
  Item(std::wstring identifierToken,
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
      bool man);
  ~Item() = default;

  Item(const Item&) = default;
  Item& operator=(const Item&) = default;
  Item(Item&&) = default;
  Item& operator=(Item&&) = default;

  const std::wstring& getIdentifierToken() const;
  const std::wstring& getItemName() const;
  int getWeight() const;
  bool isMeeleWeapon() const;
  bool isRangedWeapon() const;
  bool isArmour() const;
  bool isResource() const;
  bool isMount() const;
  int getMoves() const;
  int getWalkCapacity() const;
  int getRideCapacity() const;
  int getSwimCapacity() const;
  int getFlyCapacity() const;
  int getShipSpeedHexesPerMonth() const;
  int getShipSailingSkillRequired() const;
  int getMagesStudy() const;
  bool isMan() const;
  int getDefaultSkillMax() const;
  const std::wstring& getFullText() const;
  const std::map<std::wstring, int>& getResources() const;
  const std::map<std::wstring, int>& getSkillsMax() const;
  const std::map<std::wstring, int>& getProductionSkill() const;
  const std::map<std::wstring, int>& getProductionHelp() const;

  void setItemName(std::wstring itemName);
  void setWeight(int weight);
  void setMeeleWeapon(bool meeleWeapon);
  void setRangedWeapon(bool rangedWeapon);
  void setArmour(bool armour);
  void setResource(bool resource);
  void setMount(bool mount);
  void setMoves(int moves);
  void setWalkCapacity(int walkCapacity);
  void setRideCapacity(int rideCapacity);
  void setSwimCapacity(int swimCapacity);
  void setFlyCapacity(int flyCapacity);
  void setShipSpeedHexesPerMonth(int shipSpeedHexesPerMonth);
  void setShipSailingSkillRequired(int shipSailingSkillRequired);
  void setMagesStudy(int magesStudy);
  void setMan(bool man);
  void setDefaultSkillMax(int defaultSkillMax);
  void setFullText(std::wstring fullText);
  void setResources(std::map<std::wstring, int> resources);
  void setSkillsMax(std::map<std::wstring, int> skillsMax);
  void setProductionSkill(std::map<std::wstring, int> productionSkill);
  void setProductionSkillLevel(std::wstring skillId, int level);
  void setProductionHelp(std::map<std::wstring, int> productionHelp);

private:
  std::wstring identifierToken_;
  std::wstring itemName_;
  int          weight_ { 0 };
  bool         meeleWeapon_ { false };
  bool         rangedWeapon_ { false };
  bool         armour_ { false };
  bool         resource_ { false };
  bool         mount_ { false };
  int          moves_ { 0 };
  int          walkCapacity_ { 0 };
  int          rideCapacity_ { 0 };
  int          swimCapacity_ { 0 };
  int          flyCapacity_ { 0 };
  int          shipSpeedHexesPerMonth_ { 0 };
  int          shipSailingSkillRequired_ { 0 };
  int          magesStudy_ { 0 };
  bool         man_ { false };
  int          defaultSkillMax_ { 0 };
  std::wstring fullText_;
  std::map<std::wstring, int> resources_;
  std::map<std::wstring, int> skillsMax_;
  std::map<std::wstring, int> productionSkill_;
  std::map<std::wstring, int> productionHelp_;
};
