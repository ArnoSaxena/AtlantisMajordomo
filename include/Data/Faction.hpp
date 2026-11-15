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
 * File: Faction.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>

/**
* @brief Data model for a faction entity.
*
* FactionNumber is the stable identifier and cannot be modified after
* construction. Name and main-faction state are mutable.
*/
class Faction
{
public:
  // Returns the command unit number, or if not set, finds the smallest unit for this faction from the given UnitRepository.
  int resolveCommandUnitNumber(const class UnitRepository& unitRepo) const;
  enum class Attitude
  {
    Hostile,
    Unfriendly,
    Neutral,
    Friendly,
    Ally,
  };

  Faction(int factionNumber,
          std::wstring name,
          bool mainFaction,
          int month = 0,
          int year = 0,
          std::wstring password = L"",
          int taxedOrTradedRegionsCurrent = 0,
          int taxedOrTradedRegionsMax = 0,
          int quartermastersCurrent = 0,
          int quartermastersMax = 0,
          int magesCurrent = 0,
          int magesMax = 0,
          int apprenticesCurrent = 0,
          int apprenticesMax = 0,
          int commandUnitNumber = 0,
          Attitude defaultAttitude = Attitude::Neutral,
          std::map<int, Attitude> declaredAttitudes = {},
          int unclaimedSilver = 0);
  virtual ~Faction() = default;

  Faction(const Faction&) = default;
  Faction& operator=(const Faction&) = default;
  Faction(Faction&&) = default;
  Faction& operator=(Faction&&) = default;

  virtual int getFactionNumber() const;
  virtual const std::wstring& getName() const;
  virtual bool isMainFaction() const;
  virtual int getMonth() const;
  virtual int getYear() const;
  virtual const std::wstring& getPassword() const;
  virtual int getTaxedOrTradedRegionsCurrent() const;
  virtual int getTaxedOrTradedRegionsMax() const;
  virtual int getQuartermastersCurrent() const;
  virtual int getQuartermastersMax() const;
  virtual int getMagesCurrent() const;
  virtual int getMagesMax() const;
  virtual int getApprenticesCurrent() const;
  virtual int getApprenticesMax() const;
  virtual int getUnclaimedSilver() const;
  virtual int getUnclaimedSilverAfterOrders() const;
  virtual int getCommandUnitNumber() const;
  virtual Attitude getDefaultAttitude() const;
  virtual const std::map<int, Attitude>& getDeclaredAttitudes() const;

  void setName(std::wstring name);
  void setMainFaction(bool mainFaction);
  void setMonth(int month);
  void setYear(int year);
  void setPassword(std::wstring password);
  void setTaxedOrTradedRegionsCurrent(int value);
  void setTaxedOrTradedRegionsMax(int value);
  void setQuartermastersCurrent(int value);
  void setQuartermastersMax(int value);
  void setMagesCurrent(int value);
  void setMagesMax(int value);
  void setApprenticesCurrent(int value);
  void setApprenticesMax(int value);
  virtual void setUnclaimedSilver(int value);
  virtual void setUnclaimedSilverAfterOrders(int value);
  virtual void setCommandUnitNumber(int unitNumber);
  virtual void setDefaultAttitude(Attitude attitude);
  virtual void setDeclaredAttitude(int factionNumber, Attitude attitude);
  void removeDeclaredAttitude(int factionNumber);
  void clearDeclaredAttitudes();

private:
  int          factionNumber_ { 0 };
  std::wstring name_;
  bool         mainFaction_ { false };
  int          month_ { 0 };
  int          year_ { 0 };
  std::wstring password_;
  int          taxedOrTradedRegionsCurrent_ { 0 };
  int          taxedOrTradedRegionsMax_ { 0 };
  int          quartermastersCurrent_ { 0 };
  int          quartermastersMax_ { 0 };
  int          magesCurrent_ { 0 };
  int          magesMax_ { 0 };
  int          apprenticesCurrent_ { 0 };
  int          apprenticesMax_ { 0 };
  int          unclaimedSilver_ { 0 };
  int          unclaimedSilverAfterOrders_ {0};
  int          commandUnitNumber_ { 0 };
  Attitude     defaultAttitude_ { Attitude::Neutral };
  std::map<int, Attitude> declaredAttitudes_;
};
