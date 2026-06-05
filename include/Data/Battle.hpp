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
 * File: Battle.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <vector>

/**
* @brief A single spoil entry captured from a battle report.
*/
struct BattleSpoil
{
  /** @brief Quantity of the spoil item. Defaults to 1 when omitted in report text. */
  int amount { 0 };
  /** @brief Spoil item token (for example, "SILV"). */
  std::wstring token;
};

/**
* @brief Data model for one parsed battle report.
*
* Stores attacker/defender identities, region and report period, casualties,
* damaged-unit lists, spoils, and the full original battle text.
*/
class Battle
{
public:
  /** @brief Constructs a battle with an auto-generated unique identifier. */
  Battle();
  ~Battle() = default;

  Battle(const Battle&) = default;
  Battle& operator=(const Battle&) = default;
  Battle(Battle&&) = default;
  Battle& operator=(Battle&&) = default;

  /** @brief Gets the artificial unique battle identifier. */
  const std::wstring& getIdentifier() const;
  /** @brief Gets the full raw battle text captured from report lines. */
  const std::wstring& getFullText() const;
  /** @brief Gets attacker unit id from battle header. */
  int getAttackerUnitId() const;
  /** @brief Gets attacker unit name from battle header. */
  const std::wstring& getAttackerUnitName() const;
  /** @brief Gets defender unit id from battle header. */
  int getDefenderUnitId() const;
  /** @brief Gets defender unit name from battle header. */
  const std::wstring& getDefenderUnitName() const;
  /** @brief Gets battle region x coordinate. */
  int getRegionXCoordinate() const;
  /** @brief Gets battle region y coordinate. */
  int getRegionYCoordinate() const;
  /** @brief Gets battle region z coordinate. */
  int getRegionZCoordinate() const;
  /** @brief Gets report month this battle belongs to. */
  int getMonth() const;
  /** @brief Gets report year this battle belongs to. */
  int getYear() const;
  /** @brief Gets region type from battle header. */
  const std::wstring& getRegionType() const;
  /** @brief Gets province name from battle header. */
  const std::wstring& getProvinceName() const;
  /** @brief True when region exists in RegionRepository for parsed coordinates. */
  bool isRegionFoundInRepository() const;
  /** @brief Gets attacker side casualty count. */
  int getAttackerLosses() const;
  /** @brief Gets defender side casualty count. */
  int getDefenderLosses() const;
  /** @brief Gets attacker side damaged unit ids. */
  const std::vector<int>& getAttackerDamagedUnitIds() const;
  /** @brief Gets defender side damaged unit ids. */
  const std::vector<int>& getDefenderDamagedUnitIds() const;
  /** @brief Gets parsed spoils list (amount + token). */
  const std::vector<BattleSpoil>& getSpoils() const;

  /**
  * @brief Overrides the artificial battle identifier.
  *
  * By default, the identifier is auto-generated in the constructor using
  * generateIdentifier() in Battle.cpp (random UUID-like value). This setter
  * exists for explicit import/restore scenarios where an already persisted id
  * must be preserved.
  */
  void setIdentifier(std::wstring identifier);
  /** @brief Sets the full raw battle text block. */
  void setFullText(std::wstring fullText);
  /** @brief Sets attacker unit id. */
  void setAttackerUnitId(int attackerUnitId);
  /** @brief Sets attacker unit name. */
  void setAttackerUnitName(std::wstring attackerUnitName);
  /** @brief Sets defender unit id. */
  void setDefenderUnitId(int defenderUnitId);
  /** @brief Sets defender unit name. */
  void setDefenderUnitName(std::wstring defenderUnitName);
  /** @brief Sets region coordinates. */
  void setRegionCoordinates(int xCoordinate, int yCoordinate, int zCoordinate);
  /** @brief Sets report month. */
  void setMonth(int month);
  /** @brief Sets report year. */
  void setYear(int year);
  /** @brief Sets region type. */
  void setRegionType(std::wstring regionType);
  /** @brief Sets province name. */
  void setProvinceName(std::wstring provinceName);
  /** @brief Marks whether region lookup succeeded in RegionRepository. */
  void setRegionFoundInRepository(bool regionFoundInRepository);
  /** @brief Sets attacker side casualty count. */
  void setAttackerLosses(int attackerLosses);
  /** @brief Sets defender side casualty count. */
  void setDefenderLosses(int defenderLosses);
  /** @brief Adds a damaged unit id to attacker side. */
  void addAttackerDamagedUnitId(int unitId);
  /** @brief Adds a damaged unit id to defender side. */
  void addDefenderDamagedUnitId(int unitId);
  /** @brief Clears attacker damaged-unit id list. */
  void clearAttackerDamagedUnitIds();
  /** @brief Clears defender damaged-unit id list. */
  void clearDefenderDamagedUnitIds();
  /** @brief Appends one spoil entry (amount + token). */
  void addSpoil(BattleSpoil spoil);
  /** @brief Clears all spoil entries. */
  void clearSpoils();

private:
  std::wstring identifier_;
  std::wstring fullText_;
  int attackerUnitId_ { 0 };
  std::wstring attackerUnitName_;
  int defenderUnitId_ { 0 };
  std::wstring defenderUnitName_;
  int regionXCoordinate_ { 0 };
  int regionYCoordinate_ { 0 };
  int regionZCoordinate_ { 1 };
  int month_ { 0 };
  int year_ { 0 };
  std::wstring regionType_;
  std::wstring provinceName_;
  bool regionFoundInRepository_ { false };
  int attackerLosses_ { 0 };
  int defenderLosses_ { 0 };
  std::vector<int> attackerDamagedUnitIds_;
  std::vector<int> defenderDamagedUnitIds_;
  std::vector<BattleSpoil> spoils_;
};
