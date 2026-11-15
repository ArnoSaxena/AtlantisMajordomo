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
 * File: Unit.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>
#include <vector>

/**
* @brief Data model for a unit entity.
*
* UnitNumber is the stable identifier and cannot be modified after
* construction.
*/
class Unit
{
public:
  Unit(int unitNumber,
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
      bool onGuard = false);
  virtual ~Unit() = default;

  Unit(const Unit&) = default;
  Unit& operator=(const Unit&) = default;
  Unit(Unit&&) = default;
  Unit& operator=(Unit&&) = default;

  virtual int getUnitNumber() const;
  virtual const std::wstring& getUnitName() const;
  const std::wstring& getUnitNameAfterOrders() const;
  virtual int getFactionNumber() const;
  virtual int getStructureId() const;
  int getFutureStructureId() const;
  virtual int getXCoordinate() const;
  virtual int getYCoordinate() const;
  virtual int getZCoordinate() const;
  virtual const std::vector<std::wstring>& getFlags() const;
  virtual const std::map<std::wstring, int>& getItems() const;
  const std::map<std::wstring, int>& getItemsAfterOrders() const;
  virtual int getWeight() const;
  virtual int getCapacityWalk() const;
  virtual int getCapacityRide() const;
  virtual int getCapacityFly() const;
  virtual int getCapacitySwim() const;
  virtual const std::map<std::wstring, int>& getSkills() const;
  const std::map<std::wstring, int>& getSkillsAfterOrders() const;
  virtual const std::vector<std::wstring>& getCanStudySkillTokens() const;
  virtual const std::vector<std::wstring>& getOrders() const;
  const std::vector<std::wstring>& getWarnings() const;
  virtual int getMonth() const;
  virtual int getYear() const;
  virtual bool isOnGuard() const;
  virtual int getSkillDays(const std::wstring& skillToken) const;
  int getSkillsAfterOrdersDays(const std::wstring& skillToken) const;

  void setUnitName(std::wstring unitName);
  void setUnitNameAfterOrders(std::wstring unitNameAfterOrders);
  void setFactionNumber(int factionNumber);
  void setStructureId(int structureId);
  void setFutureStructureId(int futureStructureId);
  void setCoordinates(int xCoordinate, int yCoordinate, int zCoordinate);
  void setFlags(std::vector<std::wstring> flags);
  void setItems(std::map<std::wstring, int> items);
  void setItemsAfterOrders(std::map<std::wstring, int> items);
  void addFlag(std::wstring flag);
  void setItem(std::wstring itemToken, int amount);
  void setItemAfterOrders(std::wstring itemToken, int amount);
  void clearFlags();
  void clearItems();
  void setWeight(int weight);
  void setCapacityWalk(int capacityWalk);
  void setCapacityRide(int capacityRide);
  void setCapacityFly(int capacityFly);
  void setCapacitySwim(int capacitySwim);
  void setSkills(std::map<std::wstring, int> skills);
  void setSkillsAfterOrders(std::map<std::wstring, int> skillsAfterOrders);
  virtual void setCanStudySkillTokens(std::vector<std::wstring> canStudySkillTokens);
  virtual void setOrders(std::vector<std::wstring> orders);
  void setMonth(int month);
  void setYear(int year);
  virtual void setOnGuard(bool onGuard);
  void addSkill(std::wstring token, int days);
  void addSkillAfterOrders(std::wstring token, int days);
  void addOrder(std::wstring order);
  void addWarning(std::wstring warning);
  void clearWarnings();
  void clearSkills();
  void clearSkillsAfterOrders();
  void clearCanStudySkillTokens();
  void clearOrders();

private:
  int                       unitNumber_ { 0 };
  std::wstring              unitName_;
  std::wstring              unitNameAfterOrders_;
  int                       factionNumber_ { 0 };
  int                       structureId_ { 0 };
  int                       futureStructureId_ { structureId_ };
  int                       xCoordinate_ { 0 };
  int                       yCoordinate_ { 0 };
  int                       zCoordinate_ { 1 };
  std::vector<std::wstring> flags_;
  std::map<std::wstring, int> items_;
  std::map<std::wstring, int> itemsAfterOrders_;
  int                       weight_ { 0 };
  int                       capacityWalk_ { 0 };
  int                       capacityRide_ { 0 };
  int                       capacityFly_ { 0 };
  int                       capacitySwim_ { 0 };
  std::map<std::wstring, int> skills_;
  std::map<std::wstring, int> skillsAfterOrders_;
  std::vector<std::wstring> canStudySkillTokens_;
  std::vector<std::wstring> orders_;
  std::vector<std::wstring> warnings_;
  int                       month_ { 0 };
  int                       year_ { 0 };
  bool                      onGuard_ { false };
};
