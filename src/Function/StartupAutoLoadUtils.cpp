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
 * File: StartupAutoLoadUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/StartupAutoLoadUtils.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"

#include <algorithm>
#include <array>
#include <cwctype>
#include <filesystem>

namespace StartupAutoLoadUtils
{

AutoLoadResult runAutoLoad(AppData& appData, const AppConfig& appConfig)
{
  AutoLoadResult result {};

  const std::wstring configuredDataFile = appConfig.getDataFilePath();
  if (!configuredDataFile.empty())
  {
    std::filesystem::path dataPath(configuredDataFile);
    if (std::filesystem::exists(dataPath) && std::filesystem::is_regular_file(dataPath))
    {
      auto& reportRepo = appData.reportRepository();
      if (!reportRepo.addFromFile(configuredDataFile,
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
                                  false))
      {
        result.dataFileError = L"Failed to auto-load the configured data file:\n\n" + reportRepo.getLastError();
      }
    }
  }

  const std::wstring configuredReportFolder = appConfig.getReportImportFolder();
  if (configuredReportFolder.empty())
  {
    return result;
  }

  std::filesystem::path reportFolderPath(configuredReportFolder);
  if (!std::filesystem::exists(reportFolderPath) || !std::filesystem::is_directory(reportFolderPath))
  {
    return result;
  }

  auto& reportRepo = appData.reportRepository();
  const std::array<std::wstring, 4> allowedExtensions = { L".rep", L".txt", L".html", L".htm" };
  for (const auto& entry : std::filesystem::directory_iterator(reportFolderPath))
  {
    if (!entry.is_regular_file())
    {
      continue;
    }

    std::wstring extension = entry.path().extension().wstring();
    std::transform(extension.begin(), extension.end(), extension.begin(), std::towlower);
    if (std::find(allowedExtensions.begin(), allowedExtensions.end(), extension) == allowedExtensions.end())
    {
      continue;
    }

    if (!reportRepo.addFromFile(entry.path().wstring(),
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
                                true))
    {
      result.reportLoadErrors.push_back(entry.path().filename().wstring() + L": " + reportRepo.getLastError());
    }
  }

  return result;
}

std::wstring buildReportFolderErrorMessage(const std::vector<std::wstring>& reportLoadErrors,
                                           std::size_t maxEntries)
{
  if (reportLoadErrors.empty())
  {
    return L"";
  }

  std::wstring message = L"Some reports failed to auto-load from the configured report folder:\n\n";
  for (std::size_t i = 0; i < reportLoadErrors.size() && i < maxEntries; ++i)
  {
    message += reportLoadErrors[i] + L"\n";
  }
  if (reportLoadErrors.size() > maxEntries)
  {
    message += L"...and more\n";
  }

  return message;
}

} // namespace StartupAutoLoadUtils
