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
 * File: Commands.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>
#include <vector>

class AppData;
class Region;
class Unit;
class UnitNew;

/**
* @brief Applies hardcoded command effects to item counts.
*
* The command language is intentionally narrow and explicitly encoded in code
* (not a generic parser) so behavior can be extended command-by-command.
*/
class Commands
{
public:
  /**
  * @brief Aggregated regional economy values after command simulation.
  */
  struct RegionEconomyAfterCommands
  {
    int remainingTaxableIncome { 0 }; /**< Tax income still available after TAX resolution. */
    int remainingEntertainment { 0 }; /**< Entertainment income still available after ENTERTAIN resolution. */
    int remainingWorkWages { 0 }; /**< Wage pool still available after WORK resolution. */
  };

  /**
  * @brief Calculates item amounts for a unit after applying supported commands.
  *
  * Only commands issued by units in the same region (x, y, z) are considered.
  */
  static std::map<std::wstring, int> calculateAfterCommandItemCountsForUnit(const AppData& appData,
                                                                            const Unit& unit);

  /**
  * @brief Calculates item amounts for a UnitNew after applying supported commands.
  *
  * Only commands issued by units in the same region (x, y, z) are considered.
  */
  static std::map<std::wstring, int> calculateAfterCommandItemCountsForUnitNew(const AppData& appData,
                                                                               const class UnitNew& unitNew);

  /**
  * @brief Calculates skill amounts (in days) for a unit after applying supported commands.
  *
  * Only commands issued by units in the same region (x, y, z) are considered.
  */
  static std::map<std::wstring, int> calculateAfterCommandSkillDaysForUnit(const AppData& appData,
                                                                           const Unit& unit);

  /**
  * @brief Calculates skill amounts (in days) for a UnitNew after applying supported commands.
  *
  * Only commands issued by units in the same region (x, y, z) are considered.
  */
  static std::map<std::wstring, int> calculateAfterCommandSkillDaysForUnitNew(const AppData& appData,
                                                                              const class UnitNew& unitNew);

  /**
  * @brief Calculates unit display name after applying supported commands.
  */
  static std::wstring calculateAfterCommandUnitNameForUnit(const AppData& appData,
                                                           const Unit& unit);

  /**
  * @brief Calculates UnitNew display name after applying supported commands.
  */
  static std::wstring calculateAfterCommandUnitNameForUnitNew(const AppData& appData,
                                                              const class UnitNew& unitNew);

  /**
  * @brief Calculates region resource amounts after applying supported commands.
  *
  * Currently used by map-region UI to show post-order availability.
  */
  static std::map<std::wstring, int> calculateAfterCommandRegionResources(const AppData& appData,
                                                                          const Region& region);

  /**
  * @brief Calculates for-sale market entries after BUY command effects are applied.
  *
  * Map values store {remainingAmount, unitPrice} by item token.
  */
  static std::map<std::wstring, std::pair<int, int>> calculateAfterCommandRegionForSale(const AppData& appData,
                                                                                         const Region& region);

  /**
  * @brief Calculates wanted-market entries after SELL command effects are applied.
  *
  * Map values store {remainingAmount, unitPrice} by item token.
  */
  static std::map<std::wstring, std::pair<int, int>> calculateAfterCommandRegionWanted(const AppData& appData,
                                                                                        const Region& region);

  /**
  * @brief Calculates region economy values after applying supported commands.
  *
  * Includes values consumed by TAX and ENTERTAIN command effects.
  */
  static RegionEconomyAfterCommands calculateAfterCommandRegionEconomy(const AppData& appData,
                                                                        const Region& region);

  /**
  * @brief Gets full-month command keywords as comma-separated text.
  */
  static std::wstring getFullMonthOrderKeywordsCsv();

  /**
  * @brief Sets full-month command keywords from comma-separated text.
  */
  static void setFullMonthOrderKeywordsCsv(const std::wstring& csvKeywords);

  /**
  * @brief Gets normalized (upper-case) full-month command keywords.
  */
  static std::vector<std::wstring> getFullMonthOrderKeywords();
};
