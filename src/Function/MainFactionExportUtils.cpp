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
 * File: MainFactionExportUtils.cpp
 */

#include "Function/MainFactionExportUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"

namespace
{

std::wstring formatTwoDigits(int value)
{
  return (value < 10 ? L"0" : L"") + std::to_wstring(value);
}

} // namespace

namespace MainFactionExportUtils
{

bool tryBuildContext(const AppData& appData,
                     ExportContext& context,
                     std::wstring& errorMessage)
{
  const auto& factionRepository = appData.factionRepository();
  const Faction* mainFaction = nullptr;
  int mainFactionCount = 0;

  for (std::size_t index = 0; index < factionRepository.size(); ++index)
  {
    const Faction& faction = factionRepository.at(index);
    if (faction.isMainFaction())
    {
      ++mainFactionCount;
      mainFaction = &faction;
    }
  }

  if (mainFactionCount == 0 || mainFaction == nullptr)
  {
    errorMessage = L"Cannot export orders: no main faction is selected.";
    return false;
  }

  if (mainFactionCount > 1)
  {
    errorMessage = L"Cannot export orders: more than one main faction is selected.";
    return false;
  }

  int latestMonth = 0;
  int latestYear = 0;
  if (!appData.reportRepository().getLatestPeriod(latestMonth, latestYear))
  {
    errorMessage = L"Cannot export orders: no valid month/year was found in loaded reports.";
    return false;
  }

  context.factionNumber = mainFaction->getFactionNumber();
  context.password = mainFaction->getPassword();
  context.currentMonth = latestMonth;
  context.currentYear = latestYear;

  ++latestMonth;
  if (latestMonth > 12)
  {
    latestMonth = 1;
    ++latestYear;
  }

  context.nextMonth = latestMonth;
  context.nextYear = latestYear;
  errorMessage.clear();
  return true;
}

std::wstring buildSuggestedOrdersFilename(const ExportContext& context)
{
  return L"orders_"
    + std::to_wstring(context.factionNumber)
    + L"_"
    + std::to_wstring(context.nextYear)
    + L"_"
    + formatTwoDigits(context.nextMonth)
    + L".ord";
}

} // namespace MainFactionExportUtils
