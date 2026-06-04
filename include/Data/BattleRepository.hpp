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
 * File: BattleRepository.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Battle.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

/**
* @brief Repository for parsed battle reports.
*
* Maintains an in-memory collection of battles loaded from reports.
* Battle identity is the artificial identifier stored in Battle::identifier.
*/
class BattleRepository
{
public:
  BattleRepository() = default;
  ~BattleRepository() = default;

  BattleRepository(const BattleRepository&) = default;
  BattleRepository& operator=(const BattleRepository&) = default;
  BattleRepository(BattleRepository&&) = default;
  BattleRepository& operator=(BattleRepository&&) = default;

  /**
  * @brief Adds a battle if its identifier is not already present.
  * @param[in] battleValue Battle object to add.
  * @return true if inserted; false if duplicate identifier exists.
  */
  bool add(Battle battleValue);

  /**
  * @brief Finds a battle by artificial identifier.
  * @param[in] identifier Battle identifier.
  * @return Pointer to matching battle or nullptr when not found.
  */
  const Battle* findByIdentifier(const std::wstring& identifier) const;

  /**
  * @brief Gets all available battle periods sorted from latest to oldest.
  * @return Vector of {month, year} pairs.
  */
  std::vector<std::pair<int, int>> getAvailablePeriodsDescending() const;

  /**
  * @brief Gets the latest battle period available in repository.
  * @param[out] month Latest month when true is returned.
  * @param[out] year Latest year when true is returned.
  * @return true if at least one valid battle period exists.
  */
  bool getLatestPeriod(int& month, int& year) const;

  /**
  * @brief Gets battles for a specific period.
  * @param[in] month Report month.
  * @param[in] year Report year.
  * @return List of pointers to matching battles.
  */
  std::vector<const Battle*> findByPeriod(int month, int year) const;

  /**
  * @brief Checks whether any battle exists for a region in a given report period.
  * @param[in] xCoordinate Region X coordinate.
  * @param[in] yCoordinate Region Y coordinate.
  * @param[in] zCoordinate Region Z coordinate.
  * @param[in] month Report month.
  * @param[in] year Report year.
  * @return true when at least one battle matches all parameters.
  */
  bool hasBattleInRegionForPeriod(int xCoordinate,
                                  int yCoordinate,
                                  int zCoordinate,
                                  int month,
                                  int year) const;

  /**
  * @brief Checks whether unit id appears in any damaged list for a period.
  * @param[in] unitId Unit identifier to check.
  * @param[in] month Report month.
  * @param[in] year Report year.
  * @return true when unit is damaged in at least one battle for that period.
  */
  bool isUnitDamagedInAnyBattleForPeriod(int unitId, int month, int year) const;

  /** @brief Removes all stored battles. */
  void clear();

  /** @brief Gets number of stored battles. */
  std::size_t size() const;

  /**
  * @brief Gets battle by index.
  * @param[in] index Zero-based battle index.
  * @return Battle at index.
  * @throws std::out_of_range if index is invalid.
  */
  const Battle& at(std::size_t index) const;

private:
  std::vector<Battle> battles_;
};
