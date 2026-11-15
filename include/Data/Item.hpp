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
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
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
  virtual ~Item() = default;

  Item(const Item&) = default;
  Item& operator=(const Item&) = default;
  Item(Item&&) = default;
  Item& operator=(Item&&) = default;

  virtual const std::wstring& getIdentifierToken() const;
  virtual const std::wstring& getItemName() const;
  virtual const std::wstring& getItemNamePlural() const;

  /**
  * @brief Returns the English plural of @p name.
  *
  * Rules applied in order:
  *   - ends in "man"  → replace with "men"  (e.g. "lizardman" → "lizardmen")
  *   - ends in "f"    → replace with "ves"  (e.g. "elf" → "elves", "dwarf" → "dwarves")
  *   - otherwise      → append "s"
  */
  static std::wstring pluralizeName(const std::wstring& name);
  virtual int getWeight() const;
  virtual bool isMeeleWeapon() const;
  virtual bool isRangedWeapon() const;
  virtual bool isArmour() const;
  virtual bool isResource() const;
  virtual bool isMount() const;
  /** True when the item was auto-created from a skill description and is
   *  awaiting its real definition from an "Item reports:" entry. */
  virtual bool isProvisional() const;
  virtual int getMoves() const;
  virtual int getWalkCapacity() const;
  virtual int getRideCapacity() const;
  virtual int getSwimCapacity() const;
  virtual int getFlyCapacity() const;
  virtual int getShipSpeedHexesPerMonth() const;
  virtual int getShipSailingSkillRequired() const;
  virtual int getMagesStudy() const;
  virtual bool isMan() const;
  virtual int getDefaultSkillMax() const;
  virtual const std::wstring& getFullText() const;
  virtual const std::map<std::wstring, int>& getResources() const;
  virtual const std::map<std::wstring, int>& getSkillsMax() const;
  virtual const std::map<std::wstring, int>& getProductionSkill() const;
  virtual const std::map<std::wstring, int>& getProductionHelp() const;

  virtual void setItemName(std::wstring itemName);
  virtual void setItemNamePlural(std::wstring itemNamePlural);
  virtual void setWeight(int weight);
  virtual void setMeeleWeapon(bool meeleWeapon);
  virtual void setRangedWeapon(bool rangedWeapon);
  virtual void setArmour(bool armour);
  virtual void setResource(bool resource);
  virtual void setMount(bool mount);
  virtual void setProvisional(bool provisional);
  virtual void setMoves(int moves);
  virtual void setWalkCapacity(int walkCapacity);
  virtual void setRideCapacity(int rideCapacity);
  virtual void setSwimCapacity(int swimCapacity);
  virtual void setFlyCapacity(int flyCapacity);
  virtual void setShipSpeedHexesPerMonth(int shipSpeedHexesPerMonth);
  virtual void setShipSailingSkillRequired(int shipSailingSkillRequired);
  virtual void setMagesStudy(int magesStudy);
  virtual void setMan(bool man);
  virtual void setDefaultSkillMax(int defaultSkillMax);
  virtual void setFullText(std::wstring fullText);
  virtual void setResources(std::map<std::wstring, int> resources);
  virtual void setSkillsMax(std::map<std::wstring, int> skillsMax);
  virtual void setProductionSkill(std::map<std::wstring, int> productionSkill);
  virtual void setProductionSkillLevel(std::wstring skillId, int level);
  virtual void setProductionHelp(std::map<std::wstring, int> productionHelp);

private:
  std::wstring identifierToken_;
  std::wstring itemName_;
  std::wstring itemNamePlural_;
  int          weight_ { 0 };
  bool         meeleWeapon_ { false };
  bool         rangedWeapon_ { false };
  bool         armour_ { false };
  bool         resource_ { false };
  bool         mount_ { false };
  bool         provisional_ { false };
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
