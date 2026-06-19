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
 * File: AppDataUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/AppDataUtils.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Report.hpp"
#include "Data/ReportRepository.hpp"
#include "Function/MonthUtils.hpp"

#include <filesystem>
#include <string>

namespace AppDataUtils
{

std::wstring buildDateLabelText(const AppData* appData)
{
  if (!appData)
  {
    return L"Date: -";
  }

  int month = 0;
  int year = 0;

  auto adoptIfLater = [&](int candidateMonth, int candidateYear)
  {
    if (candidateMonth < 1 || candidateMonth > 12 || candidateYear <= 0)
    {
      return;
    }

    if (candidateYear > year || (candidateYear == year && candidateMonth > month))
    {
      month = candidateMonth;
      year = candidateYear;
    }
  };

  const ReportRepository& reportRepository = appData->reportRepository();
  for (std::size_t index = 0; index < reportRepository.size(); ++index)
  {
    const Report& report = reportRepository.at(index);
    adoptIfLater(report.getMonth(), report.getYear());
  }

  int latestBattleMonth = 0;
  int latestBattleYear = 0;
  if (appData->battleRepository().getLatestPeriod(latestBattleMonth, latestBattleYear))
  {
    adoptIfLater(latestBattleMonth, latestBattleYear);
  }

  if (month < 1 || month > 12 || year <= 0)
  {
    return L"Date: -";
  }

  return L"Date: " + MonthUtils::monthNumberToNameOr(month, L"Unknown") + L" " + std::to_wstring(year);
}

bool importReportFromFile(AppData& appData,
                          AppConfig* appConfig,
                          const std::wstring& filePath,
                          bool syncFactionFromHeader,
                          bool rememberReportImportFolder,
                          bool rememberDataFilePath)
{
  if (appConfig)
  {
    bool shouldSaveConfig = false;

    if (rememberReportImportFolder)
    {
      const std::filesystem::path selectedPath(filePath);
      if (selectedPath.has_parent_path())
      {
        appConfig->setReportImportFolder(selectedPath.parent_path().wstring());
        shouldSaveConfig = true;
      }
    }

    if (rememberDataFilePath)
    {
      appConfig->setDataFilePath(filePath);
      shouldSaveConfig = true;
    }

    if (shouldSaveConfig)
    {
      appConfig->save();
    }
  }

  return appData.reportRepository().addFromFile(
    filePath,
    appData.factionRepository(),
    appData.regionRepository(),
    appData.unitRepository(),
    appData.battleRepository(),
    appData.eventRepository(),
    appData.itemRepository(),
    appData.skillRepository(),
    appData.structureRepository(),
    appData.structInfoRepository(),
    appData.orderRepository(),
    appData.getShipStructureIdThreshold(),
    appData.getFlyingShipTypeTokens(),
    appData.getMagicSkillTriggerPhrases(),
    syncFactionFromHeader
  );
}

} // namespace AppDataUtils
