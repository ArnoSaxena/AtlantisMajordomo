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
 * File: MainFactionExportUtils.hpp
 */

#pragma once

#include <string>

class AppData;

namespace MainFactionExportUtils
{

struct ExportContext
{
  int factionNumber { 0 };
  std::wstring password;
  int currentMonth { 0 };
  int currentYear { 0 };
  int nextMonth { 0 };
  int nextYear { 0 };
};

bool tryBuildContext(const AppData& appData,
                    ExportContext& context,
                    std::wstring& errorMessage);

std::wstring buildSuggestedOrdersFilename(const ExportContext& context);

} // namespace MainFactionExportUtils
