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
 * File: OrderBusinessLogic.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <vector>
#include <map>

class AppData;

namespace OrderBusinessLogic
{
struct DeclaredAttitudeChange
{
	int targetFactionNumber { 0 };
	bool useDefault { false };
	std::wstring attitudeText;
};

struct CommandUnitResolution
{
	int commandUnitNumber { 0 };
	bool usedFallback { false };
};

// Updates DECLARE DEFAULT orders: removes all such orders from all units, and adds the correct one to the command unit if needed.
// Returns true if any changes were made.
bool updateDeclareDefaultOrder(AppData& appData, int commandUnitNumber, const std::wstring& attitudeText, bool addOrder);

// Resolves command unit selection for a faction, falling back to the faction default command unit when invalid.
CommandUnitResolution resolveFactionCommandUnit(AppData& appData,
												int sourceFactionNumber,
												int requestedCommandUnitNumber);

// Applies changed per-faction attitudes by updating faction data only.
bool applyDeclaredAttitudeChanges(AppData& appData,
								  int sourceFactionNumber,
								  const std::vector<DeclaredAttitudeChange>& changes);

// Rewrites all DECLARE-based faction commands for the given faction onto the selected command unit.
bool rewriteFactionDeclareOrders(AppData& appData,
								 int sourceFactionNumber,
								 int commandUnitNumber,
								 const std::wstring& originalDefaultAttitudeText,
								 const std::map<int, std::wstring>& originalDeclaredAttitudesText);

// Rebuilds OrderRepository entries for a saved origin unit.
// - Orders outside FORM/END blocks are stored under origin unit with fromNewUnit=false.
// - Orders inside each FORM/END block are stored under the FORM unit id with fromNewUnit=true.
void syncOrderRepositoryForSavedUnit(AppData& appData,
									 int originUnitNumber,
									 bool recalculateAfterOrdersValues = true);
}
