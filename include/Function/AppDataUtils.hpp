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
 * File: AppDataUtils.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <vector>

class AppConfig;
class AppData;

namespace AppDataUtils
{
std::wstring buildDateLabelText(const AppData* appData);

// Imports a report from filePath into appData. Optionally updates appConfig with the
// import folder and/or data file path and saves config. Returns true on success.
bool importReportFromFile(AppData& appData,
                          AppConfig* appConfig,
                          const std::wstring& filePath,
                          bool syncFactionFromHeader,
                          bool rememberReportImportFolder,
                          bool rememberDataFilePath);

// Returns the unit numbers of all units that have at least one warning AND
// belong to the latest report period.  The list is unsorted.
// This is the period filter used by the units-list display; every unit
// returned here is guaranteed to appear in that list when its region is
// selected, making it safe to use for warning navigation.
std::vector<int> getWarningUnitNumbersForLatestPeriod(const AppData& appData);
}
