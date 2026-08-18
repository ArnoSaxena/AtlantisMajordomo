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
 * File: AppConfig.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "AppConfig.hpp"
#include "DebugLog.hpp"
#include "Function/JsonUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
  constexpr wchar_t kConfigFileName[] = L"atlantis_majordomo.config.json";
  constexpr int kConfigVersion = 1;

  std::wstring normalizeUiSizeMode(const std::wstring& input)
  {
    std::wstring normalized = StringUtils::trimWhitespace(input);
    for (wchar_t& ch : normalized)
    {
      ch = static_cast<wchar_t>(std::towlower(ch));
    }

    if (normalized == L"compact")
    {
      return L"Compact";
    }
    if (normalized == L"standard")
    {
      return L"Standard";
    }
    if (normalized == L"large")
    {
      return L"Large";
    }

    return L"Auto";
  }

  std::wstring normalizeMapHexSizeMode(const std::wstring& input)
  {
    std::wstring normalized = StringUtils::trimWhitespace(input);
    for (wchar_t& ch : normalized)
    {
      ch = static_cast<wchar_t>(std::towlower(ch));
    }

    if (normalized == L"small")
    {
      return L"Small";
    }
    if (normalized == L"large")
    {
      return L"Large";
    }

    return L"Medium";
  }
}

AppConfig::AppConfig()
{
  configFilePath_ = (std::filesystem::path(getExecutableDirectory()) / kConfigFileName).wstring();
  applyDefaults();
}

bool AppConfig::load()
{
  DebugLog(L"AppConfig::load() - begin, path: " + configFilePath_);
  applyDefaults();

  std::wifstream file{std::filesystem::path(configFilePath_)};
  if (!file.is_open())
  {
    DebugLog(L"AppConfig::load() - config file not found, creating defaults");
    // First run: create a deterministic config file with defaults.
    return save();
  }

  DebugLog(L"AppConfig::load() - config file opened, reading content");
  const std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                            std::istreambuf_iterator<wchar_t>());
  DebugLog(L"AppConfig::load() - content read, length: " + std::to_wstring(content.size()));

  std::wstring configuredSaveFilePath;
  if (JsonUtils::extractJsonStringField(content, L"saveFilePath", configuredSaveFilePath)
      && !configuredSaveFilePath.empty())
  {
    saveFilePath_ = configuredSaveFilePath;
    DebugLog(L"AppConfig::load() - saveFilePath: " + saveFilePath_);
  }

  std::wstring configuredReportImportFolder;
  if (JsonUtils::extractJsonStringField(content, L"reportImportFolder", configuredReportImportFolder)
      && !configuredReportImportFolder.empty())
  {
    reportImportFolder_ = configuredReportImportFolder;
    DebugLog(L"AppConfig::load() - reportImportFolder: " + reportImportFolder_);
  }

  bool hasExportOrdersFolder = false;
  std::wstring configuredExportOrdersFolder;
  if (JsonUtils::extractJsonStringField(content, L"exportOrdersFolder", configuredExportOrdersFolder)
      && !configuredExportOrdersFolder.empty())
  {
    hasExportOrdersFolder = true;
    exportOrdersFolder_ = configuredExportOrdersFolder;
    DebugLog(L"AppConfig::load() - exportOrdersFolder: " + exportOrdersFolder_);
  }

  bool hasDataFilePath = false;
  std::wstring configuredDataFilePath;
  if (JsonUtils::extractJsonStringField(content, L"dataFilePath", configuredDataFilePath)
      && !configuredDataFilePath.empty())
  {
    hasDataFilePath = true;
    dataFilePath_ = configuredDataFilePath;
    DebugLog(L"AppConfig::load() - dataFilePath: " + dataFilePath_);
  }

  bool hasMainWindowWidth = false;
  std::wstring configuredMainWindowWidth;
  if (JsonUtils::extractJsonFieldValue(content, L"mainWindowWidth", configuredMainWindowWidth))
  {
    int parsedWidth = 0;
    if (JsonUtils::parseJsonInteger(configuredMainWindowWidth, parsedWidth) && parsedWidth > 0)
    {
      hasMainWindowWidth = true;
      mainWindowWidth_ = parsedWidth;
      DebugLog(L"AppConfig::load() - mainWindowWidth: " + std::to_wstring(mainWindowWidth_));
    }
  }

  bool hasMainWindowHeight = false;
  std::wstring configuredMainWindowHeight;
  if (JsonUtils::extractJsonFieldValue(content, L"mainWindowHeight", configuredMainWindowHeight))
  {
    int parsedHeight = 0;
    if (JsonUtils::parseJsonInteger(configuredMainWindowHeight, parsedHeight) && parsedHeight > 0)
    {
      hasMainWindowHeight = true;
      mainWindowHeight_ = parsedHeight;
      DebugLog(L"AppConfig::load() - mainWindowHeight: " + std::to_wstring(mainWindowHeight_));
    }
  }

  bool hasMapHexWidth = false;
  std::wstring configuredMapHexWidth;
  if (JsonUtils::extractJsonFieldValue(content, L"mapHexWidth", configuredMapHexWidth))
  {
    int parsedMapHexWidth = 0;
    if (JsonUtils::parseJsonInteger(configuredMapHexWidth, parsedMapHexWidth) && parsedMapHexWidth > 0)
    {
      hasMapHexWidth = true;
      mapHexWidth_ = parsedMapHexWidth;
      DebugLog(L"AppConfig::load() - mapHexWidth: " + std::to_wstring(mapHexWidth_));
    }
  }

  bool hasUiSizeMode = false;
  std::wstring configuredUiSizeMode;
  if (JsonUtils::extractJsonStringField(content, L"uiSizeMode", configuredUiSizeMode))
  {
    uiSizeMode_ = normalizeUiSizeMode(configuredUiSizeMode);
    hasUiSizeMode = true;
    DebugLog(L"AppConfig::load() - uiSizeMode: " + uiSizeMode_);
  }

  bool hasMapHexSizeMode = false;
  std::wstring configuredMapHexSizeMode;
  if (JsonUtils::extractJsonStringField(content, L"mapHexSizeMode", configuredMapHexSizeMode))
  {
    mapHexSizeMode_ = normalizeMapHexSizeMode(configuredMapHexSizeMode);
    hasMapHexSizeMode = true;
    DebugLog(L"AppConfig::load() - mapHexSizeMode: " + mapHexSizeMode_);
  }

  bool hasOnlyLeaderCanTeach = false;
  std::wstring configuredOnlyLeaderCanTeach;
  if (JsonUtils::extractJsonFieldValue(content, L"onlyLeaderCanTeach", configuredOnlyLeaderCanTeach))
  {
    const std::wstring normalized = StringUtils::trimWhitespace(configuredOnlyLeaderCanTeach);
    if (normalized == L"true")
    {
      onlyLeaderCanTeach_ = true;
      hasOnlyLeaderCanTeach = true;
    }
    else if (normalized == L"false")
    {
      onlyLeaderCanTeach_ = false;
      hasOnlyLeaderCanTeach = true;
    }
    DebugLog(L"AppConfig::load() - onlyLeaderCanTeach: " + normalized);
  }

  bool hasLeaderMages = false;
  std::wstring configuredLeaderMages;
  if (JsonUtils::extractJsonFieldValue(content, L"leaderMages", configuredLeaderMages))
  {
    const std::wstring normalized = StringUtils::trimWhitespace(configuredLeaderMages);
    if (normalized == L"true")
    {
      leaderMages_ = true;
      hasLeaderMages = true;
    }
    else if (normalized == L"false")
    {
      leaderMages_ = false;
      hasLeaderMages = true;
    }
    DebugLog(L"AppConfig::load() - leaderMages: " + normalized);
  }

  bool hasFlyingShipsCsv = false;
  std::wstring configuredFlyingShipsCsv;
  if (JsonUtils::extractJsonStringField(content, L"flyingShipsCsv", configuredFlyingShipsCsv))
  {
    flyingShipsCsv_ = StringUtils::trimWhitespace(configuredFlyingShipsCsv);
    hasFlyingShipsCsv = true;
    DebugLog(L"AppConfig::load() - flyingShipsCsv: " + flyingShipsCsv_);
  }

  bool hasFullMonthOrdersCsv = false;
  std::wstring configuredFullMonthOrdersCsv;
  if (JsonUtils::extractJsonStringField(content, L"fullMonthOrdersCsv", configuredFullMonthOrdersCsv))
  {
    fullMonthOrdersCsv_ = configuredFullMonthOrdersCsv;
    hasFullMonthOrdersCsv = true;
    DebugLog(L"AppConfig::load() - fullMonthOrdersCsv: " + fullMonthOrdersCsv_);
  }

  bool hasMagicSkillTriggersCsv = false;
  std::wstring configuredMagicSkillTriggersCsv;
  if (JsonUtils::extractJsonStringField(content, L"magicSkillTriggersCsv", configuredMagicSkillTriggersCsv))
  {
    magicSkillTriggersCsv_ = configuredMagicSkillTriggersCsv;
    hasMagicSkillTriggersCsv = true;
    DebugLog(L"AppConfig::load() - magicSkillTriggersCsv: " + magicSkillTriggersCsv_);
  }

  bool hasColoursBlock = false;
  bool hasRegionsBlock = false;
  bool hasPeasantsBlock = false;
  bool hasRoadEntry = false;
  bool hasStructureMarkerEntry = false;
  bool hasMainFactionUnitTextEntry = false;
  bool hasOtherFactionUnitTextEntry = false;
  std::wstring coloursObject;
  if (JsonUtils::extractJsonObjectField(content, L"colours", coloursObject))
  {
    hasColoursBlock = true;
    DebugLog(L"AppConfig::load() - colours block found");

    std::wstring regionsObject;
    if (JsonUtils::extractJsonObjectField(coloursObject, L"regions", regionsObject))
    {
      hasRegionsBlock = true;
      DebugLog(L"AppConfig::load() - colours.regions block found");
      for (auto& regionColor : regionColors_)
      {
        std::wstring jsonRgb;
        if (JsonUtils::extractJsonFieldValue(regionsObject, regionColor.first, jsonRgb))
        {
          std::array<int, 3> parsedRgb { 0, 0, 0 };
          if (JsonUtils::parseRgbColorArray(jsonRgb, parsedRgb))
          {
            regionColor.second = parsedRgb;
          }
        }
      }
    }

    std::wstring peasantsObject;
    if (JsonUtils::extractJsonObjectField(coloursObject, L"peasants", peasantsObject))
    {
      hasPeasantsBlock = true;
      DebugLog(L"AppConfig::load() - colours.peasants block found");
      for (auto& peasantColor : peasantColors_)
      {
        std::wstring jsonRgb;
        if (JsonUtils::extractJsonFieldValue(peasantsObject, peasantColor.first, jsonRgb))
        {
          std::array<int, 3> parsedRgb { 0, 0, 0 };
          if (JsonUtils::parseRgbColorArray(jsonRgb, parsedRgb))
          {
            peasantColor.second = parsedRgb;
          }
        }
      }
    }

    std::wstring jsonRoadRgb;
    if (JsonUtils::extractJsonFieldValue(coloursObject, L"roads", jsonRoadRgb) ||
      JsonUtils::extractJsonFieldValue(coloursObject, L"road", jsonRoadRgb))
    {
      hasRoadEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (JsonUtils::parseRgbColorArray(jsonRoadRgb, parsedRgb))
      {
        roadColor_ = parsedRgb;
      }
    }

    std::wstring jsonStructureMarkerRgb;
    if (JsonUtils::extractJsonFieldValue(coloursObject, L"structureMarker", jsonStructureMarkerRgb))
    {
      hasStructureMarkerEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (JsonUtils::parseRgbColorArray(jsonStructureMarkerRgb, parsedRgb))
      {
        structureMarkerColor_ = parsedRgb;
      }
    }

    std::wstring jsonSelectedBorderRgb;
    if (JsonUtils::extractJsonFieldValue(coloursObject, L"selectedRegionBorder", jsonSelectedBorderRgb))
    {
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (JsonUtils::parseRgbColorArray(jsonSelectedBorderRgb, parsedRgb))
      {
        selectedRegionBorderColor_ = parsedRgb;
      }
    }

    std::wstring jsonMainFactionUnitTextRgb;
    if (JsonUtils::extractJsonFieldValue(coloursObject, L"mainFactionUnitText", jsonMainFactionUnitTextRgb))
    {
      hasMainFactionUnitTextEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (JsonUtils::parseRgbColorArray(jsonMainFactionUnitTextRgb, parsedRgb))
      {
        mainFactionUnitTextColor_ = parsedRgb;
      }
    }

    std::wstring jsonOtherFactionUnitTextRgb;
    if (JsonUtils::extractJsonFieldValue(coloursObject, L"otherFactionUnitText", jsonOtherFactionUnitTextRgb))
    {
      hasOtherFactionUnitTextEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (JsonUtils::parseRgbColorArray(jsonOtherFactionUnitTextRgb, parsedRgb))
      {
        otherFactionUnitTextColor_ = parsedRgb;
      }
    }
  }
  else
  {
    DebugLog(L"AppConfig::load() - colours block NOT found in config");
  }

  if (!hasColoursBlock || !hasRegionsBlock || !hasPeasantsBlock || !hasRoadEntry || !hasStructureMarkerEntry || !hasMainFactionUnitTextEntry || !hasOtherFactionUnitTextEntry || !hasMainWindowWidth || !hasMainWindowHeight || !hasMapHexWidth || !hasUiSizeMode || !hasExportOrdersFolder || !hasDataFilePath || !hasOnlyLeaderCanTeach || !hasLeaderMages || !hasFlyingShipsCsv || !hasFullMonthOrdersCsv || !hasMagicSkillTriggersCsv)
  {
    DebugLog(L"AppConfig::load() - missing fields detected, re-saving config with defaults");
    save();
  }

  DebugLog(L"AppConfig::load() - completed successfully");
  return true;
}

bool AppConfig::save() const
{
  std::wofstream file{std::filesystem::path(configFilePath_)};
  if (!file.is_open())
  {
    return false;
  }

  file << L"{\n";
  file << L"  \"version\": " << kConfigVersion << L",\n";
  file << L"  \"saveFilePath\": \"" << JsonUtils::escapeJsonString(saveFilePath_) << L"\",\n";
  file << L"  \"reportImportFolder\": \"" << JsonUtils::escapeJsonString(reportImportFolder_) << L"\",\n";
  file << L"  \"dataFilePath\": \"" << JsonUtils::escapeJsonString(dataFilePath_) << L"\",\n";
  file << L"  \"exportOrdersFolder\": \"" << JsonUtils::escapeJsonString(exportOrdersFolder_) << L"\",\n";
  file << L"  \"mainWindowWidth\": " << mainWindowWidth_ << L",\n";
  file << L"  \"mainWindowHeight\": " << mainWindowHeight_ << L",\n";
  file << L"  \"mapHexWidth\": " << mapHexWidth_ << L",\n";
  file << L"  \"uiSizeMode\": \"" << JsonUtils::escapeJsonString(uiSizeMode_) << L"\",\n";
  file << L"  \"mapHexSizeMode\": \"" << JsonUtils::escapeJsonString(mapHexSizeMode_) << L"\",\n";
  file << L"  \"onlyLeaderCanTeach\": " << (onlyLeaderCanTeach_ ? L"true" : L"false") << L",\n";
  file << L"  \"leaderMages\": " << (leaderMages_ ? L"true" : L"false") << L",\n";
  file << L"  \"flyingShipsCsv\": \"" << JsonUtils::escapeJsonString(flyingShipsCsv_) << L"\",\n";
  file << L"  \"fullMonthOrdersCsv\": \"" << JsonUtils::escapeJsonString(fullMonthOrdersCsv_) << L"\",\n";
  file << L"  \"magicSkillTriggersCsv\": \"" << JsonUtils::escapeJsonString(magicSkillTriggersCsv_) << L"\",\n";
  file << L"  \"colours\": {\n";
  file << L"    \"regions\": {\n";
  for (size_t i = 0; i < regionColors_.size(); ++i)
  {
    const auto& regionColor = regionColors_[i];
    file << L"      \"" << JsonUtils::escapeJsonString(regionColor.first) << L"\": ["
        << regionColor.second[0] << L", "
        << regionColor.second[1] << L", "
        << regionColor.second[2] << L"]";
    if (i + 1 < regionColors_.size())
    {
      file << L",";
    }
    file << L"\n";
  }
      file << L"    },\n";
      file << L"    \"peasants\": {\n";
      for (size_t i = 0; i < peasantColors_.size(); ++i)
      {
        const auto& peasantColor = peasantColors_[i];
        file << L"      \"" << JsonUtils::escapeJsonString(peasantColor.first) << L"\": ["
            << peasantColor.second[0] << L", "
            << peasantColor.second[1] << L", "
            << peasantColor.second[2] << L"]";
        if (i + 1 < peasantColors_.size())
        {
          file << L",";
        }
        file << L"\n";
      }
      file << L"    },\n";
      file << L"    \"roads\": ["
        << roadColor_[0] << L", " << roadColor_[1] << L", " << roadColor_[2] << L"],\n";
      file << L"    \"structureMarker\": ["
        << structureMarkerColor_[0] << L", "
        << structureMarkerColor_[1] << L", "
        << structureMarkerColor_[2] << L"],\n";
      file << L"    \"selectedRegionBorder\": ["
        << selectedRegionBorderColor_[0] << L", "
        << selectedRegionBorderColor_[1] << L", "
        << selectedRegionBorderColor_[2] << L"],\n";
      file << L"    \"mainFactionUnitText\": ["
        << mainFactionUnitTextColor_[0] << L", "
        << mainFactionUnitTextColor_[1] << L", "
        << mainFactionUnitTextColor_[2] << L"],\n";
      file << L"    \"otherFactionUnitText\": ["
        << otherFactionUnitTextColor_[0] << L", "
        << otherFactionUnitTextColor_[1] << L", "
        << otherFactionUnitTextColor_[2] << L"]\n";
      file << L"  }\n";
  file << L"}\n";

  return true;
}

const std::wstring& AppConfig::getSaveFilePath() const
{
  return saveFilePath_;
}

void AppConfig::setSaveFilePath(const std::wstring& saveFilePath)
{
  if (saveFilePath.empty())
  {
    saveFilePath_ = getDefaultSaveFilePath();
    return;
  }

  saveFilePath_ = saveFilePath;
}

const std::wstring& AppConfig::getReportImportFolder() const
{
  return reportImportFolder_;
}

void AppConfig::setReportImportFolder(const std::wstring& reportImportFolder)
{
  if (reportImportFolder.empty())
  {
    reportImportFolder_ = getDefaultReportImportFolder();
    return;
  }

  reportImportFolder_ = reportImportFolder;
}

const std::wstring& AppConfig::getExportOrdersFolder() const
{
  return exportOrdersFolder_;
}

void AppConfig::setExportOrdersFolder(const std::wstring& exportOrdersFolder)
{
  if (exportOrdersFolder.empty())
  {
    exportOrdersFolder_ = getDefaultExportOrdersFolder();
    return;
  }

  exportOrdersFolder_ = exportOrdersFolder;
}

const std::wstring& AppConfig::getDataFilePath() const
{
  return dataFilePath_;
}

void AppConfig::setDataFilePath(const std::wstring& dataFilePath)
{
  if (dataFilePath.empty())
  {
    dataFilePath_ = getDefaultDataFilePath();
    return;
  }

  dataFilePath_ = dataFilePath;
}

int AppConfig::getMainWindowWidth() const
{
  return mainWindowWidth_;
}

int AppConfig::getMainWindowHeight() const
{
  return mainWindowHeight_;
}

void AppConfig::setMainWindowWidth(int width)
{
  if (width > 0)
  {
    mainWindowWidth_ = width;
  }
}

void AppConfig::setMainWindowHeight(int height)
{
  if (height > 0)
  {
    mainWindowHeight_ = height;
  }
}

int AppConfig::getMapHexWidth() const
{
  return mapHexWidth_;
}

void AppConfig::setMapHexWidth(int mapHexWidth)
{
  if (mapHexWidth > 0)
  {
    mapHexWidth_ = mapHexWidth;
  }
}

const std::wstring& AppConfig::getUiSizeMode() const
{
  return uiSizeMode_;
}

void AppConfig::setUiSizeMode(const std::wstring& uiSizeMode)
{
  uiSizeMode_ = normalizeUiSizeMode(uiSizeMode);
}

const std::wstring& AppConfig::getMapHexSizeMode() const
{
  return mapHexSizeMode_;
}

void AppConfig::setMapHexSizeMode(const std::wstring& mapHexSizeMode)
{
  mapHexSizeMode_ = normalizeMapHexSizeMode(mapHexSizeMode);
}

bool AppConfig::getOnlyLeaderCanTeach() const
{
  return onlyLeaderCanTeach_;
}

bool AppConfig::getLeaderMages() const
{
  return leaderMages_;
}

void AppConfig::setOnlyLeaderCanTeach(bool onlyLeaderCanTeach)
{
  onlyLeaderCanTeach_ = onlyLeaderCanTeach;
}

void AppConfig::setLeaderMages(bool leaderMages)
{
  leaderMages_ = leaderMages;
}

const std::wstring& AppConfig::getFlyingShipsCsv() const
{
  return flyingShipsCsv_;
}

void AppConfig::setFlyingShipsCsv(const std::wstring& flyingShipsCsv)
{
  flyingShipsCsv_ = flyingShipsCsv;
}

const std::wstring& AppConfig::getFullMonthOrdersCsv() const
{
  return fullMonthOrdersCsv_;
}

void AppConfig::setFullMonthOrdersCsv(const std::wstring& fullMonthOrdersCsv)
{
  fullMonthOrdersCsv_ = fullMonthOrdersCsv;
}

const std::wstring& AppConfig::getMagicSkillTriggersCsv() const
{
  return magicSkillTriggersCsv_;
}

void AppConfig::setMagicSkillTriggersCsv(const std::wstring& magicSkillTriggersCsv)
{
  magicSkillTriggersCsv_ = magicSkillTriggersCsv;
}

std::array<int, 3> AppConfig::getRegionColor(const std::wstring& regionType) const
{
  auto normalize = [](std::wstring value)
  {
    for (auto& ch : value)
    {
      ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return value;
  };

  const std::wstring normalizedType = normalize(regionType);
  const std::wstring unknownType = L"unknown";

  for (const auto& entry : regionColors_)
  {
    if (normalize(entry.first) == normalizedType)
    {
      return entry.second;
    }
  }

  for (const auto& entry : regionColors_)
  {
    if (normalize(entry.first) == unknownType)
    {
      return entry.second;
    }
  }

  return { 192, 192, 192 };
}

std::array<int, 3> AppConfig::getPeasantColour(const std::wstring& itemToken) const
{
  auto normalize = [](std::wstring value)
  {
    for (auto& ch : value)
    {
      ch = static_cast<wchar_t>(std::towupper(ch));
    }
    return value;
  };

  const std::wstring normalizedToken = normalize(itemToken);

  for (const auto& entry : peasantColors_)
  {
    if (normalize(entry.first) == normalizedToken)
    {
      return entry.second;
    }
  }

  return { 211, 211, 211 };
}

std::array<int, 3> AppConfig::getSelectedRegionBorderColor() const{
  return selectedRegionBorderColor_;
}

std::array<int, 3> AppConfig::getRoadColor() const
{
  return roadColor_;
}

std::array<int, 3> AppConfig::getStructureMarkerColor() const
{
  return structureMarkerColor_;
}

std::array<int, 3> AppConfig::getMainFactionUnitTextColor() const
{
  return mainFactionUnitTextColor_;
}

std::array<int, 3> AppConfig::getOtherFactionUnitTextColor() const
{
  return otherFactionUnitTextColor_;
}

void AppConfig::setMainFactionUnitTextColor(const std::array<int, 3>& rgbColor)
{
  mainFactionUnitTextColor_ = {
    std::clamp(rgbColor[0], 0, 255),
    std::clamp(rgbColor[1], 0, 255),
    std::clamp(rgbColor[2], 0, 255)
  };
}

void AppConfig::setOtherFactionUnitTextColor(const std::array<int, 3>& rgbColor)
{
  otherFactionUnitTextColor_ = {
    std::clamp(rgbColor[0], 0, 255),
    std::clamp(rgbColor[1], 0, 255),
    std::clamp(rgbColor[2], 0, 255)
  };
}

std::wstring AppConfig::getExecutableDirectory()
{
#ifdef _WIN32
  wchar_t modulePath[MAX_PATH] = {};
  const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
  if (length > 0 && length < MAX_PATH)
  {
    std::filesystem::path fullPath(modulePath);
    if (fullPath.has_parent_path())
    {
      return fullPath.parent_path().wstring();
    }
  }

  // Fallback: try to get the executable path from the process handle
  wchar_t alternativePath[MAX_PATH] = {};
  if (GetModuleFileNameW(GetModuleHandleW(nullptr), alternativePath, MAX_PATH) > 0 &&
      alternativePath[0] != L'\0')
  {
    std::filesystem::path altPath(alternativePath);
    if (altPath.has_parent_path())
    {
      return altPath.parent_path().wstring();
    }
  }

  // Last resort: return current working directory as absolute path
  wchar_t cwd[MAX_PATH] = {};
  if (GetCurrentDirectoryW(MAX_PATH, cwd) > 0)
  {
    return std::wstring(cwd);
  }

  return L".";
#else
  // Linux: resolve /proc/self/exe to get the real executable path
  std::error_code ec;
  const std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe", ec);
  if (!ec && exePath.has_parent_path())
  {
    return exePath.parent_path().wstring();
  }

  // Fallback: current working directory
  const std::filesystem::path cwd = std::filesystem::current_path(ec);
  if (!ec)
  {
    return cwd.wstring();
  }

  return L".";
#endif
}

std::wstring AppConfig::getDefaultSaveFilePath()
{
  return (std::filesystem::path(getExecutableDirectory()) / L"dataset.dat").wstring();
}

std::wstring AppConfig::getDefaultDataFilePath()
{
  return (std::filesystem::path(getExecutableDirectory()) / L"data.txt").wstring();
}

std::wstring AppConfig::getDefaultReportImportFolder()
{
  return (std::filesystem::path(getExecutableDirectory()) / L"Reports").wstring();
}

std::wstring AppConfig::getDefaultExportOrdersFolder()
{
  return getExecutableDirectory();
}

void AppConfig::applyDefaults()
{
  const std::filesystem::path exeDir(getExecutableDirectory());
  saveFilePath_       = (exeDir / L"dataset.dat").wstring();
  reportImportFolder_ = (exeDir / L"Reports").wstring();
  dataFilePath_       = (exeDir / L"data.txt").wstring();
  exportOrdersFolder_ = (exeDir / L"Reports").wstring();
  mainWindowWidth_ = 900;
  mainWindowHeight_ = 600;
  mapHexWidth_ = 40;
  uiSizeMode_ = L"Auto";
  mapHexSizeMode_ = L"Medium";
  onlyLeaderCanTeach_ = false;
  leaderMages_ = true;
  flyingShipsCsv_.clear();
  fullMonthOrdersCsv_ = L"ADVANCE, BUILD, ENTERTAIN, MOVE, PILLAGE, PRODUCE, SAIL, STUDY, TAX, TEACH, WORK";
  magicSkillTriggersCsv_ = L"a mage with this skill, a mage with, forms of magic, allows a mage";
  regionColors_ = {
    { L"cavern", { 128, 128, 192 } },
    { L"chasm", { 188, 117, 111 } },
    { L"deepforest", { 62, 126, 47 } },
    { L"desert", { 224, 164, 56 } },
    { L"forest", { 72, 200, 72 } },
    { L"grotto", { 119, 203, 107 } },
    { L"jungle", { 0, 128, 0 } },
    { L"lake", { 79, 158, 255 } },
    { L"mountain", { 188, 96, 0 } },
    { L"nexus", { 0, 147, 217 } },
    { L"ocean", { 0, 0, 255 } },
    { L"plain", { 255, 232, 168 } },
    { L"swamp", { 168, 168, 84 } },
    { L"tundra", { 184, 200, 224 } },
    { L"tunnels", { 65, 146, 137 } },
    { L"underforest", { 116, 158, 44 } },
    { L"unknown", { 192, 192, 192 } }
  };
  peasantColors_ = {
    { L"CTAU", { 191, 139,  17 } },
    { L"DRLF", { 148,   0, 211 } },
    { L"GBLN", {  47, 176,  13 } },
    { L"GNOL", { 107,  76,   5 } },
    { L"GNOM", { 201, 169,  95 } },
    { L"HDWA", { 100, 100, 100 } },
    { L"HELF", { 211, 211, 211 } },
    { L"HUMN", { 255, 242, 212 } },
    { L"IDWA", {  54, 156, 232 } },
    { L"LIZA", { 183, 214, 131 } },
    { L"ORC",  { 168, 167,  19 } },
    { L"UDWA", {  77,  60,  36 } },
    { L"WELF", {  30,  92,  14 } }
  };
  roadColor_ = { 112, 128, 144 };
  structureMarkerColor_ = { 112, 128, 144 };
  selectedRegionBorderColor_ = { 173, 216, 230 };
  mainFactionUnitTextColor_ = { 0, 0, 0 };
  otherFactionUnitTextColor_ = { 128, 128, 128 };
}
