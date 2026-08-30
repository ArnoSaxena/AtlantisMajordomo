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

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>

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

std::vector<WarningRow> getWarningsForLatestPeriod(const AppData& appData)
{
  int latestMonth = 0;
  int latestYear  = 0;
  const auto& reportRepo = appData.reportRepository();
  for (std::size_t i = 0; i < reportRepo.size(); ++i)
  {
    const Report& r = reportRepo.at(i);
    const int rm = r.getMonth();
    const int ry = r.getYear();
    if (rm >= 1 && rm <= 12 && ry > 0)
    {
      if (ry > latestYear || (ry == latestYear && rm > latestMonth))
      {
        latestMonth = rm;
        latestYear  = ry;
      }
    }
  }
  const bool hasLatestPeriod = (latestMonth >= 1 && latestMonth <= 12 && latestYear > 0);

  std::vector<WarningRow> result;

  const auto appendWarnings = [&result, hasLatestPeriod, latestMonth, latestYear](int unitNumber,
                                                                                    bool isNewUnit,
                                                                                    int xCoordinate,
                                                                                    int yCoordinate,
                                                                                    int zCoordinate,
                                                                                    int month,
                                                                                    int year,
                                                                                    const std::vector<std::wstring>& warnings)
  {
    if (unitNumber <= 0 || warnings.empty())
    {
      return;
    }
    if (hasLatestPeriod && (month != latestMonth || year != latestYear))
    {
      return;
    }

    for (const std::wstring& warning : warnings)
    {
      result.push_back({ unitNumber, isNewUnit, xCoordinate, yCoordinate, zCoordinate, warning });
    }
  };

  const auto& unitRepo = appData.unitRepository();
  for (std::size_t i = 0; i < unitRepo.size(); ++i)
  {
    const Unit& unit = unitRepo.at(i);
    appendWarnings(unit.getUnitNumber(), false, unit.getXCoordinate(), unit.getYCoordinate(), unit.getZCoordinate(),
             unit.getMonth(), unit.getYear(), unit.getWarnings());
  }

  const auto& unitNewRepo = appData.unitNewRepository();
  for (std::size_t i = 0; i < unitNewRepo.size(); ++i)
  {
    const UnitNew& unitNew = unitNewRepo.at(i);
    appendWarnings(unitNew.getUnitNumber(), true, unitNew.getXCoordinate(), unitNew.getYCoordinate(), unitNew.getZCoordinate(),
             unitNew.getMonth(), unitNew.getYear(), unitNew.getWarnings());
  }

  return result;
}

std::vector<int> getWarningUnitNumbersForLatestPeriod(const AppData& appData)
{
  std::vector<int> result;
  for (const auto& warning : getWarningsForLatestPeriod(appData))
  {
    if (std::find(result.begin(), result.end(), warning.unitNumber) == result.end())
    {
      result.push_back(warning.unitNumber);
    }
  }
  return result;
}

} // namespace AppDataUtils
