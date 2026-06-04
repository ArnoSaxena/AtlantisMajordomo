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
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "AppConfig.hpp"
#include "Function/StringUtils.hpp"

#include <cwctype>
#include <fstream>
#include <filesystem>

#include <windows.h>

namespace
{
  constexpr wchar_t kConfigFileName[] = L"atlantis_majordomo.config.json";
  constexpr int kConfigVersion = 1;
}

AppConfig::AppConfig()
{
  configFilePath_ = getExecutableDirectory() + L"\\" + kConfigFileName;
  applyDefaults();
}

bool AppConfig::load()
{
  applyDefaults();

  std::wifstream file(configFilePath_);
  if (!file.is_open())
  {
    // First run: create a deterministic config file with defaults.
    return save();
  }

  const std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                            std::istreambuf_iterator<wchar_t>());

  std::wstring configuredSaveFilePath;
  if (extractJsonStringField(content, L"saveFilePath", configuredSaveFilePath)
      && !configuredSaveFilePath.empty())
  {
    saveFilePath_ = configuredSaveFilePath;
  }

  std::wstring configuredReportImportFolder;
  if (extractJsonStringField(content, L"reportImportFolder", configuredReportImportFolder)
      && !configuredReportImportFolder.empty())
  {
    reportImportFolder_ = configuredReportImportFolder;
  }

  bool hasExportOrdersFolder = false;
  std::wstring configuredExportOrdersFolder;
  if (extractJsonStringField(content, L"exportOrdersFolder", configuredExportOrdersFolder)
      && !configuredExportOrdersFolder.empty())
  {
    hasExportOrdersFolder = true;
    exportOrdersFolder_ = configuredExportOrdersFolder;
  }

  bool hasDataFilePath = false;
  std::wstring configuredDataFilePath;
  if (extractJsonStringField(content, L"dataFilePath", configuredDataFilePath)
      && !configuredDataFilePath.empty())
  {
    hasDataFilePath = true;
    dataFilePath_ = configuredDataFilePath;
  }

  bool hasMainWindowWidth = false;
  std::wstring configuredMainWindowWidth;
  if (extractJsonFieldValue(content, L"mainWindowWidth", configuredMainWindowWidth))
  {
    int parsedWidth = 0;
    if (parseJsonInteger(configuredMainWindowWidth, parsedWidth) && parsedWidth > 0)
    {
      hasMainWindowWidth = true;
      mainWindowWidth_ = parsedWidth;
    }
  }

  bool hasMainWindowHeight = false;
  std::wstring configuredMainWindowHeight;
  if (extractJsonFieldValue(content, L"mainWindowHeight", configuredMainWindowHeight))
  {
    int parsedHeight = 0;
    if (parseJsonInteger(configuredMainWindowHeight, parsedHeight) && parsedHeight > 0)
    {
      hasMainWindowHeight = true;
      mainWindowHeight_ = parsedHeight;
    }
  }

  bool hasMapHexWidth = false;
  std::wstring configuredMapHexWidth;
  if (extractJsonFieldValue(content, L"mapHexWidth", configuredMapHexWidth))
  {
    int parsedMapHexWidth = 0;
    if (parseJsonInteger(configuredMapHexWidth, parsedMapHexWidth) && parsedMapHexWidth > 0)
    {
      hasMapHexWidth = true;
      mapHexWidth_ = parsedMapHexWidth;
    }
  }

  bool hasOnlyLeaderCanTeach = false;
  std::wstring configuredOnlyLeaderCanTeach;
  if (extractJsonFieldValue(content, L"onlyLeaderCanTeach", configuredOnlyLeaderCanTeach))
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
  }

  bool hasLeaderMages = false;
  std::wstring configuredLeaderMages;
  if (extractJsonFieldValue(content, L"leaderMages", configuredLeaderMages))
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
  }

  bool hasFlyingShipsCsv = false;
  std::wstring configuredFlyingShipsCsv;
  if (extractJsonStringField(content, L"flyingShipsCsv", configuredFlyingShipsCsv))
  {
    flyingShipsCsv_ = StringUtils::trimWhitespace(configuredFlyingShipsCsv);
    hasFlyingShipsCsv = true;
  }

  bool hasFullMonthOrdersCsv = false;
  std::wstring configuredFullMonthOrdersCsv;
  if (extractJsonStringField(content, L"fullMonthOrdersCsv", configuredFullMonthOrdersCsv))
  {
    fullMonthOrdersCsv_ = configuredFullMonthOrdersCsv;
    hasFullMonthOrdersCsv = true;
  }

  bool hasMagicSkillTriggersCsv = false;
  std::wstring configuredMagicSkillTriggersCsv;
  if (extractJsonStringField(content, L"magicSkillTriggersCsv", configuredMagicSkillTriggersCsv))
  {
    magicSkillTriggersCsv_ = configuredMagicSkillTriggersCsv;
    hasMagicSkillTriggersCsv = true;
  }

  bool hasColoursBlock = false;
  bool hasRegionsBlock = false;
  bool hasRoadEntry = false;
  bool hasStructureMarkerEntry = false;
  std::wstring coloursObject;
  if (extractJsonObjectField(content, L"colours", coloursObject))
  {
    hasColoursBlock = true;

    std::wstring regionsObject;
    if (extractJsonObjectField(coloursObject, L"regions", regionsObject))
    {
      hasRegionsBlock = true;
      for (auto& regionColor : regionColors_)
      {
        std::wstring jsonRgb;
        if (extractJsonFieldValue(regionsObject, regionColor.first, jsonRgb))
        {
          std::array<int, 3> parsedRgb { 0, 0, 0 };
          if (parseRgbColorArray(jsonRgb, parsedRgb))
          {
            regionColor.second = parsedRgb;
          }
        }
      }
    }

    std::wstring jsonRoadRgb;
    if (extractJsonFieldValue(coloursObject, L"roads", jsonRoadRgb) ||
      extractJsonFieldValue(coloursObject, L"road", jsonRoadRgb))
    {
      hasRoadEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (parseRgbColorArray(jsonRoadRgb, parsedRgb))
      {
        roadColor_ = parsedRgb;
      }
    }

    std::wstring jsonStructureMarkerRgb;
    if (extractJsonFieldValue(coloursObject, L"structureMarker", jsonStructureMarkerRgb))
    {
      hasStructureMarkerEntry = true;
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (parseRgbColorArray(jsonStructureMarkerRgb, parsedRgb))
      {
        structureMarkerColor_ = parsedRgb;
      }
    }

    std::wstring jsonSelectedBorderRgb;
    if (extractJsonFieldValue(coloursObject, L"selectedRegionBorder", jsonSelectedBorderRgb))
    {
      std::array<int, 3> parsedRgb { 0, 0, 0 };
      if (parseRgbColorArray(jsonSelectedBorderRgb, parsedRgb))
      {
        selectedRegionBorderColor_ = parsedRgb;
      }
    }
  }

  if (!hasColoursBlock || !hasRegionsBlock || !hasRoadEntry || !hasStructureMarkerEntry || !hasMainWindowWidth || !hasMainWindowHeight || !hasMapHexWidth || !hasExportOrdersFolder || !hasDataFilePath || !hasOnlyLeaderCanTeach || !hasLeaderMages || !hasFlyingShipsCsv || !hasFullMonthOrdersCsv || !hasMagicSkillTriggersCsv)
  {
    save();
  }

  return true;
}

bool AppConfig::save() const
{
  std::wofstream file(configFilePath_);
  if (!file.is_open())
  {
    return false;
  }

  file << L"{\n";
  file << L"  \"version\": " << kConfigVersion << L",\n";
  file << L"  \"saveFilePath\": \"" << escapeJsonString(saveFilePath_) << L"\",\n";
  file << L"  \"reportImportFolder\": \"" << escapeJsonString(reportImportFolder_) << L"\",\n";
  file << L"  \"dataFilePath\": \"" << escapeJsonString(dataFilePath_) << L"\",\n";
  file << L"  \"exportOrdersFolder\": \"" << escapeJsonString(exportOrdersFolder_) << L"\",\n";
  file << L"  \"mainWindowWidth\": " << mainWindowWidth_ << L",\n";
  file << L"  \"mainWindowHeight\": " << mainWindowHeight_ << L",\n";
  file << L"  \"mapHexWidth\": " << mapHexWidth_ << L",\n";
  file << L"  \"onlyLeaderCanTeach\": " << (onlyLeaderCanTeach_ ? L"true" : L"false") << L",\n";
  file << L"  \"leaderMages\": " << (leaderMages_ ? L"true" : L"false") << L",\n";
  file << L"  \"flyingShipsCsv\": \"" << escapeJsonString(flyingShipsCsv_) << L"\",\n";
  file << L"  \"fullMonthOrdersCsv\": \"" << escapeJsonString(fullMonthOrdersCsv_) << L"\",\n";
  file << L"  \"magicSkillTriggersCsv\": \"" << escapeJsonString(magicSkillTriggersCsv_) << L"\",\n";
  file << L"  \"colours\": {\n";
  file << L"    \"regions\": {\n";
  for (size_t i = 0; i < regionColors_.size(); ++i)
  {
    const auto& regionColor = regionColors_[i];
    file << L"      \"" << escapeJsonString(regionColor.first) << L"\": ["
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
      file << L"    \"roads\": ["
        << roadColor_[0] << L", " << roadColor_[1] << L", " << roadColor_[2] << L"],\n";
      file << L"    \"structureMarker\": ["
        << structureMarkerColor_[0] << L", "
        << structureMarkerColor_[1] << L", "
        << structureMarkerColor_[2] << L"],\n";
      file << L"    \"selectedRegionBorder\": ["
        << selectedRegionBorderColor_[0] << L", "
        << selectedRegionBorderColor_[1] << L", "
        << selectedRegionBorderColor_[2] << L"]\n";
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

std::array<int, 3> AppConfig::getSelectedRegionBorderColor() const
{
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

std::wstring AppConfig::getExecutableDirectory()
{
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
}

std::wstring AppConfig::getDefaultSaveFilePath()
{
  return getExecutableDirectory() + L"\\dataset.dat";
}

std::wstring AppConfig::getDefaultDataFilePath()
{
  return getExecutableDirectory() + L"\\data.txt";
}

std::wstring AppConfig::getDefaultReportImportFolder()
{
  return getExecutableDirectory() + L"\\Reports";
}

std::wstring AppConfig::getDefaultExportOrdersFolder()
{
  return getExecutableDirectory();
}

//TODO: create JsonUtils and move these functions there
//      use here and at least in Data/DataSerializer.cpp
//      same for unescapeJsonString.
std::wstring AppConfig::escapeJsonString(const std::wstring& value)
{
  std::wstring escaped;
  escaped.reserve(value.size());

  for (const wchar_t ch : value)
  {
    switch (ch)
    {
      case L'"':  escaped += L"\\\""; break;
      case L'\\': escaped += L"\\\\"; break;
      case L'\n': escaped += L"\\n"; break;
      case L'\r': escaped += L"\\r"; break;
      case L'\t': escaped += L"\\t"; break;
      default:     escaped += ch; break;
    }
  }

  return escaped;
}

std::wstring AppConfig::unescapeJsonString(const std::wstring& value)
{
  std::wstring unescaped;
  unescaped.reserve(value.size());

  for (size_t i = 0; i < value.size(); ++i)
  {
    if (value[i] == L'\\' && (i + 1) < value.size())
    {
      ++i;
      switch (value[i])
      {
        case L'"': unescaped += L'"'; break;
        case L'\\': unescaped += L'\\'; break;
        case L'n': unescaped += L'\n'; break;
        case L'r': unescaped += L'\r'; break;
        case L't': unescaped += L'\t'; break;
        default: unescaped += value[i]; break;
      }
    }
    else
    {
      unescaped += value[i];
    }
  }

  return unescaped;
}

bool AppConfig::extractJsonStringField(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& value)
{
  std::wstring jsonValue;
  if (!extractJsonFieldValue(json, fieldName, jsonValue))
  {
    return false;
  }

  if (jsonValue.size() < 2 || jsonValue.front() != L'"' || jsonValue.back() != L'"')
  {
    return false;
  }

  value = unescapeJsonString(jsonValue.substr(1, jsonValue.size() - 2));
  return true;
}

bool AppConfig::extractJsonFieldValue(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& value)
{
  const std::wstring token = L"\"" + fieldName + L"\"";
  const size_t keyPos = json.find(token);
  if (keyPos == std::wstring::npos)
  {
    return false;
  }

  const size_t colonPos = json.find(L':', keyPos + token.size());
  if (colonPos == std::wstring::npos)
  {
    return false;
  }

  size_t valueStart = colonPos + 1;
  while (valueStart < json.size()
        && (json[valueStart] == L' ' || json[valueStart] == L'\t'
            || json[valueStart] == L'\n' || json[valueStart] == L'\r'))
  {
    ++valueStart;
  }

  if (valueStart >= json.size())
  {
    return false;
  }

  size_t valueEnd = valueStart;
  if (json[valueStart] == L'{')
  {
    int braceDepth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = valueStart; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        inString = !inString;
        continue;
      }
      if (inString)
      {
        continue;
      }

      if (ch == L'{')
      {
        ++braceDepth;
      }
      else if (ch == L'}')
      {
        --braceDepth;
        if (braceDepth == 0)
        {
          valueEnd = i + 1;
          break;
        }
      }
    }
  }
  else if (json[valueStart] == L'[')
  {
    int bracketDepth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = valueStart; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        inString = !inString;
        continue;
      }
      if (inString)
      {
        continue;
      }

      if (ch == L'[')
      {
        ++bracketDepth;
      }
      else if (ch == L']')
      {
        --bracketDepth;
        if (bracketDepth == 0)
        {
          valueEnd = i + 1;
          break;
        }
      }
    }
  }
  else if (json[valueStart] == L'"')
  {
    bool escaped = false;
    for (size_t i = valueStart + 1; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        valueEnd = i + 1;
        break;
      }
    }
  }
  else
  {
    valueEnd = valueStart;
    while (valueEnd < json.size() && json[valueEnd] != L',' && json[valueEnd] != L'}')
    {
      ++valueEnd;
    }
  }

  if (valueEnd <= valueStart || valueEnd > json.size())
  {
    return false;
  }

  value = StringUtils::trimWhitespace(json.substr(valueStart, valueEnd - valueStart));
  return !value.empty();
}

bool AppConfig::extractJsonObjectField(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& objectContent)
{
  std::wstring jsonValue;
  if (!extractJsonFieldValue(json, fieldName, jsonValue))
  {
    return false;
  }

  if (jsonValue.size() < 2 || jsonValue.front() != L'{' || jsonValue.back() != L'}')
  {
    return false;
  }

  objectContent = jsonValue.substr(1, jsonValue.size() - 2);
  return true;
}

bool AppConfig::parseJsonInteger(const std::wstring& jsonValue, int& parsedValue)
{
  try
  {
    parsedValue = std::stoi(StringUtils::trimWhitespace(jsonValue));
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool AppConfig::parseRgbColorArray(const std::wstring& jsonArray,
                                  std::array<int, 3>& rgbColor)
{
  const std::wstring trimmedArray = StringUtils::trimWhitespace(jsonArray);
  if (trimmedArray.size() < 2 || trimmedArray.front() != L'[' || trimmedArray.back() != L']')
  {
    return false;
  }

  const std::wstring values = trimmedArray.substr(1, trimmedArray.size() - 2);
  size_t start = 0;
  for (int channel = 0; channel < 3; ++channel)
  {
    const size_t commaPos = values.find(L',', start);
    size_t end = std::wstring::npos;

    if (channel < 2)
    {
      if (commaPos == std::wstring::npos)
      {
        return false;
      }
      end = commaPos;
    }
    else
    {
      end = values.size();
    }

    const std::wstring token = StringUtils::trimWhitespace(values.substr(start, end - start));
    if (token.empty())
    {
      return false;
    }

    try
    {
      rgbColor[channel] = std::stoi(token);
    }
    catch (...)
    {
      return false;
    }

    if (channel < 2)
    {
      start = commaPos + 1;
    }
  }

  return true;
}

void AppConfig::applyDefaults()
{
  saveFilePath_ = getDefaultSaveFilePath();
  reportImportFolder_ = getDefaultReportImportFolder();
  dataFilePath_ = getDefaultDataFilePath();
  exportOrdersFolder_ = getDefaultReportImportFolder();
  mainWindowWidth_ = 900;
  mainWindowHeight_ = 600;
  mapHexWidth_ = 40;
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
  roadColor_ = { 112, 128, 144 };
  structureMarkerColor_ = { 112, 128, 144 };
  selectedRegionBorderColor_ = { 173, 216, 230 };
}
