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
 * File: CommandSimulationService.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

class AppData;

class CommandSimulationService
{
public:
  static int calculateMainFactionUnclaimedSilverAfterCommands(const AppData& appData);

  // Collects CLAIM commands from OrderRepository, updates main-faction
  // unclaimed silver after orders, and emits CLAIM insufficiency warnings.
  static void processMainFactionClaimEffects(AppData& appData);

  // Recomputes and writes all *AfterOrders values for units, regions, and main faction.
  static void recalculateAfterOrdersValues(AppData& appData);
};
