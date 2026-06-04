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
 * File: DataSerializer.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/DataSerializer.hpp"
#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/Structure.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfo.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitRepository.hpp"
#include "Data/UnitNew.hpp"
#include "Data/UnitNewRepository.hpp"
#include "Data/Item.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/StringUtils.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cerrno>
#include <cwctype>
#include <cstdint>
#include <map>
#include <regex>

std::wstring DataSerializer::lastError_;

namespace
{
  // Simple JSON helper functions
  void appendUnicodeEscape(std::wstring& result, unsigned int codeUnit)
  {
    std::wstringstream stream;
    stream << L"\\u" << std::uppercase << std::hex << std::setw(4) << std::setfill(L'0') << codeUnit;
    result += stream.str();
  }

  bool tryParseHex4(const std::wstring& str, size_t startIndex, std::uint32_t& codeUnit)
  {
    if (startIndex + 4 > str.size())
    {
      return false;
    }

    std::uint32_t value = 0;
    for (size_t index = startIndex; index < startIndex + 4; ++index)
    {
      const wchar_t ch = str[index];
      value <<= 4;
      if (ch >= L'0' && ch <= L'9')
      {
        value |= static_cast<std::uint32_t>(ch - L'0');
      }
      else if (ch >= L'a' && ch <= L'f')
      {
        value |= static_cast<std::uint32_t>(10 + (ch - L'a'));
      }
      else if (ch >= L'A' && ch <= L'F')
      {
        value |= static_cast<std::uint32_t>(10 + (ch - L'A'));
      }
      else
      {
        return false;
      }
    }

    codeUnit = value;
    return true;
  }

  void appendCodePoint(std::wstring& result, std::uint32_t codePoint)
  {
    if constexpr (sizeof(wchar_t) >= 4)
    {
      result.push_back(static_cast<wchar_t>(codePoint));
      return;
    }

    if (codePoint <= 0xFFFF)
    {
      result.push_back(static_cast<wchar_t>(codePoint));
      return;
    }

    codePoint -= 0x10000;
    const std::uint32_t highSurrogate = 0xD800 + (codePoint >> 10);
    const std::uint32_t lowSurrogate = 0xDC00 + (codePoint & 0x3FF);
    result.push_back(static_cast<wchar_t>(highSurrogate));
    result.push_back(static_cast<wchar_t>(lowSurrogate));
  }

  std::wstring escapeJsonString(const std::wstring& str)
  {
    std::wstring result;
    for (wchar_t ch : str)
    {
      switch (ch)
      {
        case L'"':  result += L"\\\""; break;
        case L'\\': result += L"\\\\"; break;
        case L'\b': result += L"\\b";  break;
        case L'\f': result += L"\\f";  break;
        case L'\n': result += L"\\n";  break;
        case L'\r': result += L"\\r";  break;
        case L'\t': result += L"\\t";  break;
        default:
        {
          const unsigned int codeUnit = static_cast<unsigned int>(ch);
          if (codeUnit < 0x20 || (codeUnit >= 0xD800 && codeUnit <= 0xDFFF))
          {
            appendUnicodeEscape(result, codeUnit);
          }
          else
          {
            result += ch;
          }
          break;
        }
      }
    }
    return result;
  }

  bool unescapeJsonString(const std::wstring& str, std::wstring& result)
  {
    result.clear();
    for (size_t i = 0; i < str.length(); ++i)
    {
      const wchar_t current = str[i];
      if (current < 0x20)
      {
        return false;
      }

      if (current != L'\\')
      {
        result += current;
        continue;
      }

      if (i + 1 >= str.length())
      {
        return false;
      }

      ++i;
      const wchar_t escapeType = str[i];
      switch (escapeType)
      {
        case L'"': result += L'"'; break;
        case L'\\': result += L'\\'; break;
        case L'/': result += L'/'; break;
        case L'b': result += L'\b'; break;
        case L'f': result += L'\f'; break;
        case L'n': result += L'\n'; break;
        case L'r': result += L'\r'; break;
        case L't': result += L'\t'; break;
        case L'u':
        {
          std::uint32_t codeUnit = 0;
          if (!tryParseHex4(str, i + 1, codeUnit))
          {
            return false;
          }
          i += 4;

          if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF)
          {
            if (i + 6 >= str.length() || str[i + 1] != L'\\' || str[i + 2] != L'u')
            {
              return false;
            }

            std::uint32_t lowSurrogate = 0;
            if (!tryParseHex4(str, i + 3, lowSurrogate))
            {
              return false;
            }
            if (lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF)
            {
              return false;
            }

            const std::uint32_t codePoint = 0x10000 + (((codeUnit - 0xD800) << 10) | (lowSurrogate - 0xDC00));
            appendCodePoint(result, codePoint);
            i += 6;
          }
          else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF)
          {
            return false;
          }
          else
          {
            appendCodePoint(result, codeUnit);
          }
          break;
        }
        default:
          return false;
      }
    }

    return true;
  }
}

bool DataSerializer::saveToFile(const AppData& appData, const std::wstring& filePath)
{
  try
  {
    std::wofstream file(filePath);
    if (!file.is_open())
    {
      lastError_ = L"Failed to open file for writing: " + filePath;
      return false;
    }

    file << L"{\n";
    file << L"  \"version\": 1,\n";
    file << L"  \"factions\": [\n";

    const FactionRepository& factionRepo = appData.factionRepository();
    for (size_t i = 0; i < factionRepo.size(); ++i)
    {
      const Faction& faction = factionRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"number\": " << faction.getFactionNumber() << L",\n";
      file << L"      \"name\": \"" << escapeJsonString(faction.getName()) << L"\",\n";
      file << L"      \"mainFaction\": " << (faction.isMainFaction() ? L"true" : L"false") << L",\n";
      file << L"      \"password\": \"" << escapeJsonString(faction.getPassword()) << L"\",\n";
      file << L"      \"taxedOrTradedRegionsCurrent\": " << faction.getTaxedOrTradedRegionsCurrent() << L",\n";
      file << L"      \"taxedOrTradedRegionsMax\": " << faction.getTaxedOrTradedRegionsMax() << L",\n";
      file << L"      \"quartermastersCurrent\": " << faction.getQuartermastersCurrent() << L",\n";
      file << L"      \"quartermastersMax\": " << faction.getQuartermastersMax() << L",\n";
      file << L"      \"magesCurrent\": " << faction.getMagesCurrent() << L",\n";
      file << L"      \"magesMax\": " << faction.getMagesMax() << L",\n";
      file << L"      \"apprenticesCurrent\": " << faction.getApprenticesCurrent() << L",\n";
      file << L"      \"apprenticesMax\": " << faction.getApprenticesMax() << L",\n";
      file << L"      \"month\": " << faction.getMonth() << L",\n";
      file << L"      \"year\": " << faction.getYear() << L"\n";
      file << L"    }";
    }

    file << L"\n  ],\n";
    file << L"  \"regions\": [\n";

    const RegionRepository& regionRepo = appData.regionRepository();
    for (size_t i = 0; i < regionRepo.size(); ++i)
    {
      const Region& region = regionRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"x\": " << region.getXCoordinate() << L",\n";
      file << L"      \"y\": " << region.getYCoordinate() << L",\n";
      file << L"      \"z\": " << region.getZCoordinate() << L",\n";
      file << L"      \"type\": \"" << escapeJsonString(region.getRegionType()) << L"\",\n";
      file << L"      \"province\": \"" << escapeJsonString(region.getProvinceName()) << L"\",\n";
      file << L"      \"hasSettlement\": " << (region.getContainsSettlement() ? L"true" : L"false") << L",\n";
      file << L"      \"settlementName\": \"" << escapeJsonString(region.getSettlementName()) << L"\",\n";
      file << L"      \"settlementType\": \"" << escapeJsonString(region.getSettlementType()) << L"\",\n";
      file << L"      \"peasantType\": \"" << escapeJsonString(region.getPeasantType()) << L"\",\n";
      file << L"      \"peasantNumber\": " << region.getPeasantNumber() << L",\n";
      file << L"      \"wages\": " << std::fixed << std::setprecision(2) << region.getWages() << L",\n";
      file << L"      \"wagesMax\": " << region.getWagesMax() << L",\n";
      file << L"      \"taxableIncome\": " << region.getTaxableIncome() << L",\n";
      file << L"      \"entertainment\": " << region.getEntertainment() << L",\n";
      file << L"      \"resources\": {";
      const auto& resources = region.getResources();
      size_t resIndex = 0;
      for (const auto& [resToken, resAmount] : resources)
      {
        if (resIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(resToken) << L"\": " << resAmount;
        ++resIndex;
      }
      file << L"},\n";
      file << L"      \"wanted\": {";
      const auto& wanted = region.getWanted();
      size_t wantedIndex = 0;
      for (const auto& [wantedToken, wantedPair] : wanted)
      {
        if (wantedIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(wantedToken) << L"\": [" << wantedPair.first << L", " << wantedPair.second << L"]";
        ++wantedIndex;
      }
      file << L"},\n";
      file << L"      \"forSale\": {";
      const auto& forSale = region.getForSale();
      size_t forSaleIndex = 0;
      for (const auto& [forSaleToken, forSalePair] : forSale)
      {
        if (forSaleIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(forSaleToken) << L"\": [" << forSalePair.first << L", " << forSalePair.second << L"]";
        ++forSaleIndex;
      }
      file << L"},\n";
      file << L"      \"exitDirections\": [";
      const auto& exitDirections = region.getExitDirections();
      for (size_t j = 0; j < exitDirections.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(exitDirections[j]) << L"\"";
      }
      file << L"],\n";
      file << L"      \"visited\": " << (region.getVisited() ? L"true" : L"false") << L",\n";
      file << L"      \"month\": " << region.getMonth() << L",\n";
      file << L"      \"year\": " << region.getYear() << L"\n";
      file << L"    }";
    }

    file << L"\n  ],\n";
    file << L"  \"units\": [\n";

    const UnitRepository& unitRepo = appData.unitRepository();
    for (size_t i = 0; i < unitRepo.size(); ++i)
    {
      const Unit& unit = unitRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"number\": " << unit.getUnitNumber() << L",\n";
      file << L"      \"name\": \"" << escapeJsonString(unit.getUnitName()) << L"\",\n";
      file << L"      \"factionNumber\": " << unit.getFactionNumber() << L",\n";
      file << L"      \"structureId\": " << unit.getStructureId() << L",\n";
      file << L"      \"x\": " << unit.getXCoordinate() << L",\n";
      file << L"      \"y\": " << unit.getYCoordinate() << L",\n";
      file << L"      \"z\": " << unit.getZCoordinate() << L",\n";
      file << L"      \"month\": " << unit.getMonth() << L",\n";
      file << L"      \"year\": " << unit.getYear() << L",\n";
      file << L"      \"weight\": " << unit.getWeight() << L",\n";
      file << L"      \"capacityWalk\": " << unit.getCapacityWalk() << L",\n";
      file << L"      \"capacityRide\": " << unit.getCapacityRide() << L",\n";
      file << L"      \"capacityFly\": " << unit.getCapacityFly() << L",\n";
      file << L"      \"capacitySwim\": " << unit.getCapacitySwim() << L",\n";
      file << L"      \"flags\": [";
      const auto& flags = unit.getFlags();
      for (size_t j = 0; j < flags.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(flags[j]) << L"\"";
      }
      file << L"],\n";
      file << L"      \"items\": {";
      const auto& itemCounts = unit.getItems();
      size_t itemIndex = 0;
      for (const auto& [itemToken, itemAmount] : itemCounts)
      {
        if (itemIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(itemToken) << L"\": " << itemAmount;
        ++itemIndex;
      }
      file << L"},\n";
      file << L"      \"skills\": [";
      const auto& skills = unit.getSkills();
      bool firstSkill = true;
      for (const auto& [skillToken, days] : skills)
      {
        if (!firstSkill) file << L", ";
        firstSkill = false;
        const std::wstring skillText = skillToken + L" [" + skillToken + L"] " +
                                       std::to_wstring(days);
        file << L"\"" << escapeJsonString(skillText) << L"\"";
      }
      file << L"],\n";
      file << L"      \"canStudySkills\": [";
      const auto& canStudySkills = unit.getCanStudySkillTokens();
      for (size_t j = 0; j < canStudySkills.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(canStudySkills[j]) << L"\"";
      }
      file << L"],\n";
      file << L"      \"orders\": [";
      const auto& orders = unit.getOrders();
      for (size_t j = 0; j < orders.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(orders[j]) << L"\"";
      }
      file << L"]\n";
      file << L"    }";
    }

    file << L"\n  ],\n";

    // Serialize UnitNew objects separately from normal units.
    file << L"  \"unitNews\": [\n";
    const UnitNewRepository& unitNewRepo = appData.unitNewRepository();
    for (size_t i = 0; i < unitNewRepo.size(); ++i)
    {
      const UnitNew& unitNew = unitNewRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"number\": " << unitNew.getUnitNumber() << L",\n";
      file << L"      \"name\": \"" << escapeJsonString(unitNew.getUnitName()) << L"\",\n";
      file << L"      \"structureId\": " << unitNew.getStructureId() << L",\n";
      file << L"      \"futureStructureId\": " << unitNew.getFutureStructureId() << L",\n";
      file << L"      \"x\": " << unitNew.getXCoordinate() << L",\n";
      file << L"      \"y\": " << unitNew.getYCoordinate() << L",\n";
      file << L"      \"z\": " << unitNew.getZCoordinate() << L",\n";
      file << L"      \"month\": " << unitNew.getMonth() << L",\n";
      file << L"      \"year\": " << unitNew.getYear() << L",\n";
      file << L"      \"originUnit\": " << unitNew.getOriginUnit() << L",\n";
      file << L"      \"weight\": " << unitNew.getWeight() << L",\n";
      file << L"      \"capacityWalk\": " << unitNew.getCapacityWalk() << L",\n";
      file << L"      \"capacityRide\": " << unitNew.getCapacityRide() << L",\n";
      file << L"      \"capacityFly\": " << unitNew.getCapacityFly() << L",\n";
      file << L"      \"capacitySwim\": " << unitNew.getCapacitySwim() << L",\n";
      file << L"      \"flags\": [";
      const auto& flags = unitNew.getFlags();
      for (size_t j = 0; j < flags.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(flags[j]) << L"\"";
      }
      file << L"],\n";
      file << L"      \"items\": {";
      const auto& itemCounts = unitNew.getItems();
      size_t itemIndex = 0;
      for (const auto& [itemToken, itemAmount] : itemCounts)
      {
        if (itemIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(itemToken) << L"\": " << itemAmount;
        ++itemIndex;
      }
      file << L"},\n";
      file << L"      \"skills\": [";
      const auto& skills = unitNew.getSkills();
      bool firstSkill = true;
      for (const auto& [skillToken, days] : skills)
      {
        if (!firstSkill) file << L", ";
        firstSkill = false;
        const std::wstring skillText = skillToken + L" [" + skillToken + L"] " + std::to_wstring(days);
        file << L"\"" << escapeJsonString(skillText) << L"\"";
      }
      file << L"],\n";
      file << L"      \"canStudySkills\": [";
      const auto& canStudySkills = unitNew.getCanStudySkillTokens();
      for (size_t j = 0; j < canStudySkills.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(canStudySkills[j]) << L"\"";
      }
      file << L"],\n";
      file << L"      \"warnings\": [";
      const auto& warnings = unitNew.getWarnings();
      for (size_t j = 0; j < warnings.size(); ++j)
      {
        if (j > 0) file << L", ";
        file << L"\"" << escapeJsonString(warnings[j]) << L"\"";
      }
      file << L"]\n";
      file << L"    }";
    }
    file << L"\n  ],\n";
    file << L"  \"events\": [\n";

    const EventRepository& eventRepo = appData.eventRepository();
    for (size_t i = 0; i < eventRepo.size(); ++i)
    {
      const Event& eventValue = eventRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"unitId\": " << eventValue.getUnitId() << L",\n";
      file << L"      \"message\": \"" << escapeJsonString(eventValue.getMessage()) << L"\"\n";
      file << L"    }";
    }

    file << L"\n  ],\n";
    file << L"  \"settings\": {\n";
    file << L"    \"shipStructureIdThreshold\": " << appData.getShipStructureIdThreshold() << L",\n";
    file << L"    \"flyingShipsCsv\": \"" << escapeJsonString(appData.getFlyingShipsCsv()) << L"\",\n";
    file << L"    \"onlyLeaderCanTeach\": " << (appData.getOnlyLeaderCanTeach() ? L"true" : L"false") << L",\n";
    file << L"    \"leaderMages\": " << (appData.getLeaderMages() ? L"true" : L"false") << L"\n";
    file << L"  },\n";


    // structures
    file << L"  \"structures\": [\n";
    const StructureRepository& structureRepo = appData.structureRepository();
    for (size_t i = 0; i < structureRepo.size(); ++i)
    {
      const Structure& structure = structureRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"id\": " << structure.getStructureId() << L",\n";
      file << L"      \"x\": " << structure.getXCoordinate() << L",\n";
      file << L"      \"y\": " << structure.getYCoordinate() << L",\n";
      file << L"      \"z\": " << structure.getZCoordinate() << L",\n";
      file << L"      \"type\": \"" << escapeJsonString(structure.getStructureType()) << L"\",\n";
      file << L"      \"name\": \"" << escapeJsonString(structure.getStructureName()) << L"\",\n";
      file << L"      \"month\": " << structure.getMonth() << L",\n";
      file << L"      \"year\": " << structure.getYear() << L",\n";
      file << L"      \"isClosed\": " << (structure.isClosed() ? L"true" : L"false") << L",\n";
      file << L"      \"ownerUnitId\": " << structure.getOwnerUnitId() << L"\n";
      file << L"    }";
    }
    file << L"\n  ],\n";


    // structure info
    file << L"  \"structureInfo\": [\n";
    const StructInfoRepository& structInfoRepo = appData.structInfoRepository();
    for (size_t i = 0; i < structInfoRepo.size(); ++i)
    {
      const StructInfo& structInfo = structInfoRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"type\": \"" << escapeJsonString(structInfo.getStructureType()) << L"\",\n";
      file << L"      \"needs\": " << structInfo.getNeeds() << L",\n";
      file << L"      \"magesCapacity\": " << structInfo.getMagesCapacity() << L",\n";
      file << L"      \"isShip\": " << (structInfo.isShip() ? L"true" : L"false") << L",\n";
      file << L"      \"isFlying\": " << (structInfo.protoIsFlying() ? L"true" : L"false") << L",\n";
      file << L"      \"itemIdentifierToken\": \"" << escapeJsonString(structInfo.getItemIdentifierToken()) << L"\"\n";
      file << L"    }";
    } 
    file << L"\n  ],\n";


    // item repo
    file << L"  \"itemRepository\": [\n";

    const ItemRepository& itemRepo = appData.itemRepository();
    for (size_t i = 0; i < itemRepo.size(); ++i)
    {
      const Item& item = itemRepo.at(i);
      if (i > 0) file << L",\n";
      file << L"    {\n";
      file << L"      \"token\": \"" << escapeJsonString(item.getIdentifierToken()) << L"\",\n";
      file << L"      \"name\": \"" << escapeJsonString(item.getItemName()) << L"\",\n";
      file << L"      \"weight\": " << item.getWeight() << L",\n";
      file << L"      \"meeleWeapon\": " << (item.isMeeleWeapon() ? L"true" : L"false") << L",\n";
      file << L"      \"rangedWeapon\": " << (item.isRangedWeapon() ? L"true" : L"false") << L",\n";
      file << L"      \"armour\": " << (item.isArmour() ? L"true" : L"false") << L",\n";
      file << L"      \"resource\": " << (item.isResource() ? L"true" : L"false") << L",\n";
      file << L"      \"mount\": " << (item.isMount() ? L"true" : L"false") << L",\n";
      file << L"      \"moves\": " << item.getMoves() << L",\n";
      file << L"      \"walkCapacity\": " << item.getWalkCapacity() << L",\n";
      file << L"      \"rideCapacity\": " << item.getRideCapacity() << L",\n";
      file << L"      \"swimCapacity\": " << item.getSwimCapacity() << L",\n";
      file << L"      \"flyCapacity\": " << item.getFlyCapacity() << L",\n";
      file << L"      \"shipSpeedHexesPerMonth\": " << item.getShipSpeedHexesPerMonth() << L",\n";
      file << L"      \"shipSailingSkillRequired\": " << item.getShipSailingSkillRequired() << L",\n";
      file << L"      \"magesStudy\": " << item.getMagesStudy() << L",\n";
      file << L"      \"man\": " << (item.isMan() ? L"true" : L"false") << L",\n";
      file << L"      \"defaultSkillMax\": " << item.getDefaultSkillMax() << L",\n";
      file << L"      \"fullText\": \"" << escapeJsonString(item.getFullText()) << L"\",\n";
      file << L"      \"resources\": {";
      const auto& resources = item.getResources();
      size_t resourceIndex = 0;
      for (const auto& [resourceToken, resourceAmount] : resources)
      {
        if (resourceIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(resourceToken) << L"\": " << resourceAmount;
        ++resourceIndex;
      }
      file << L"},\n";
      file << L"      \"skillsMax\": {";
      const auto& skillsMax = item.getSkillsMax();
      size_t skillsMaxIndex = 0;
      for (const auto& [skillToken, maxLevel] : skillsMax)
      {
        if (skillsMaxIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(skillToken) << L"\": " << maxLevel;
        ++skillsMaxIndex;
      }
      file << L"},\n";
      file << L"      \"productionSkill\": {";
      const auto& productionSkill = item.getProductionSkill();
      size_t productionSkillIndex = 0;
      for (const auto& [skillToken, skillLevel] : productionSkill)
      {
        if (productionSkillIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(skillToken) << L"\": " << skillLevel;
        ++productionSkillIndex;
      }
      file << L"},\n";
      file << L"      \"productionHelp\": {";
      const auto& productionHelp = item.getProductionHelp();
      size_t productionHelpIndex = 0;
      for (const auto& [helpToken, helpAmount] : productionHelp)
      {
        if (productionHelpIndex > 0) file << L", ";
        file << L"\"" << escapeJsonString(helpToken) << L"\": " << helpAmount;
        ++productionHelpIndex;
      }
      file << L"}\n";
      file << L"    }";
    }

    file << L"\n  ],\n";
    file << L"  \"skillRepository\": [\n";

    const SkillRepository& skillRepo = appData.skillRepository();
    bool firstSkillEntry = true;
    for (size_t i = 0; i < skillRepo.size(); ++i)
    {
      const Skill& skill = skillRepo.at(i);
      for (int lv = Skill::kMinLevel; lv <= Skill::kMaxLevel; ++lv)
      {
        const SkillLevelData* ld = skill.getLevelData(lv);
        if (!ld) continue;
        if (!firstSkillEntry) file << L",\n";
        firstSkillEntry = false;
        file << L"    {\n";
        file << L"      \"token\": \"" << escapeJsonString(skill.getIdentifierToken()) << L"\",\n";
        file << L"      \"level\": " << lv << L",\n";
        file << L"      \"name\": \"" << escapeJsonString(skill.getName()) << L"\",\n";
        file << L"      \"productionItems\": {";
        size_t mmIndex = 0;
        for (const auto& [itemToken, manMonths] : ld->productionItems)
        {
          if (mmIndex > 0) file << L", ";
          file << L"\"" << escapeJsonString(itemToken) << L"\": " << manMonths;
          ++mmIndex;
        }
        file << L"},\n";
        file << L"      \"magic\": " << (skill.isMagic() ? L"true" : L"false") << L",\n";
        file << L"      \"magicFoundation\": " << (skill.isMagicFoundation() ? L"true" : L"false") << L",\n";
        file << L"      \"description\": \"" << escapeJsonString(ld->description) << L"\",\n";
        file << L"      \"prerequisites\": [";
        const auto& prerequisites = skill.getPrerequisites();
        for (size_t prerequisiteIndex = 0; prerequisiteIndex < prerequisites.size(); ++prerequisiteIndex)
        {
          if (prerequisiteIndex > 0) file << L", ";
          file << L"{\"token\": \"" << escapeJsonString(prerequisites[prerequisiteIndex].token)
               << L"\", \"requiredLevel\": " << prerequisites[prerequisiteIndex].requiredLevel << L"}";
        }
        file << L"],\n";
        file << L"      \"studyCost\": " << skill.getStudyCost() << L"\n";
        file << L"    }";
      }
    }

    file << L"\n  ]\n";
    file << L"}\n";

    file.close();
    lastError_ = L"";
    return true;
  }
  catch (const std::exception& e)
  {
    lastError_ = L"Exception during save: ";
    lastError_ += std::wstring(e.what(), e.what() + std::strlen(e.what()));
    return false;
  }
}

namespace
{
  // Simple JSON parser helper functions
  // Extracts the value of a field, properly handling arrays and nested structures
  std::wstring extractFieldValue(const std::wstring& obj, size_t colonPos)
  {
    size_t valueStart = colonPos + 1;
    while (valueStart < obj.length() && (obj[valueStart] == L' ' || obj[valueStart] == L':'))
      valueStart++;

    if (valueStart >= obj.length())
      return L"";

    size_t valueEnd = valueStart;
    if (obj[valueStart] == L'[')
    {
      // Handle array
      int bracketCount = 0;
      bool inString = false;
      bool escapeNext = false;
      for (size_t i = valueStart; i < obj.length(); ++i)
      {
        if (escapeNext)
        {
          escapeNext = false;
          continue;
        }
        if (obj[i] == L'\\')
        {
          escapeNext = true;
          continue;
        }
        if (obj[i] == L'"')
        {
          inString = !inString;
          continue;
        }
        if (!inString)
        {
          if (obj[i] == L'[')
            bracketCount++;
          else if (obj[i] == L']')
          {
            bracketCount--;
            if (bracketCount == 0)
            {
              valueEnd = i + 1;
              break;
            }
          }
        }
      }
    }
    else if (obj[valueStart] == L'{')
    {
      // Handle object
      int braceCount = 0;
      bool inString = false;
      bool escapeNext = false;
      for (size_t i = valueStart; i < obj.length(); ++i)
      {
        if (escapeNext)
        {
          escapeNext = false;
          continue;
        }
        if (obj[i] == L'\\')
        {
          escapeNext = true;
          continue;
        }
        if (obj[i] == L'"')
        {
          inString = !inString;
          continue;
        }
        if (!inString)
        {
          if (obj[i] == L'{')
            braceCount++;
          else if (obj[i] == L'}')
          {
            braceCount--;
            if (braceCount == 0)
            {
              valueEnd = i + 1;
              break;
            }
          }
        }
      }
    }
    else
    {
      // Handle primitive (string, number, boolean, null)
      bool inString = false;
      bool escapeNext = false;
      for (size_t i = valueStart; i < obj.length(); ++i)
      {
        if (escapeNext)
        {
          escapeNext = false;
          continue;
        }
        if (obj[i] == L'\\')
        {
          escapeNext = true;
          continue;
        }
        if (obj[i] == L'"')
        {
          inString = !inString;
          if (!inString && i > valueStart)
          {
            valueEnd = i + 1;
            break;
          }
          continue;
        }
        if (!inString && (obj[i] == L',' || obj[i] == L'}'))
        {
          valueEnd = i;
          break;
        }
      }
    }

    return StringUtils::trimWhitespace(obj.substr(valueStart, valueEnd - valueStart));
  }

  bool parseJsonString(const std::wstring& jsonValue, std::wstring& result)
  {
    std::wstring trimmed = StringUtils::trimWhitespace(jsonValue);
    if (trimmed.size() < 2 || trimmed[0] != L'"' || trimmed[trimmed.size() - 1] != L'"')
    {
      return false;
    }

    return unescapeJsonString(trimmed.substr(1, trimmed.size() - 2), result);
  }

  bool parseJsonNumber(const std::wstring& jsonValue, int& result)
  {
    try
    {
      result = std::stoi(StringUtils::trimWhitespace(jsonValue));
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool parseJsonNumber(const std::wstring& jsonValue, double& result)
  {
    try
    {
      result = std::stod(StringUtils::trimWhitespace(jsonValue));
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool parseJsonBool(const std::wstring& jsonValue, bool& result)
  {
    std::wstring trimmed = StringUtils::trimWhitespace(jsonValue);
    if (trimmed == L"true")
    {
      result = true;
      return true;
    }
    if (trimmed == L"false")
    {
      result = false;
      return true;
    }
    return false;
  }

  std::vector<std::wstring> parseJsonStringArray(const std::wstring& jsonArray)
  {
    std::vector<std::wstring> result;
    std::wstring trimmed = StringUtils::trimWhitespace(jsonArray);
    if (trimmed.size() < 2 || trimmed[0] != L'[' || trimmed[trimmed.size() - 1] != L']')
    {
      return result;
    }

    const std::wstring content = trimmed.substr(1, trimmed.size() - 2);
    size_t pos = 0;
    while (pos < content.size())
    {
      while (pos < content.size() && iswspace(content[pos]))
      {
        ++pos;
      }

      if (pos >= content.size())
      {
        break;
      }

      if (content[pos] == L',')
      {
        ++pos;
        continue;
      }

      if (content[pos] != L'"')
      {
        break;
      }

      const size_t start = pos;
      ++pos;
      bool escapeNext = false;
      bool foundEnd = false;
      for (; pos < content.size(); ++pos)
      {
        if (escapeNext)
        {
          escapeNext = false;
          continue;
        }

        if (content[pos] == L'\\')
        {
          escapeNext = true;
          continue;
        }

        if (content[pos] == L'"')
        {
          foundEnd = true;
          break;
        }
      }

      if (!foundEnd)
      {
        break;
      }

      std::wstring item;
      if (!unescapeJsonString(content.substr(start + 1, pos - start - 1), item))
      {
        result.clear();
        return result;
      }
      result.push_back(item);
      ++pos;
    }

    return result;
  }

  std::map<std::wstring, int> parseJsonStringIntMap(const std::wstring& jsonObject)
  {
    std::map<std::wstring, int> result;
    std::wstring trimmed = StringUtils::trimWhitespace(jsonObject);
    if (trimmed.size() < 2 || trimmed.front() != L'{' || trimmed.back() != L'}')
    {
      return result;
    }

    std::wstring content = trimmed.substr(1, trimmed.size() - 2);
    size_t pos = 0;
    while (pos < content.size())
    {
      size_t keyStart = content.find(L'"', pos);
      if (keyStart == std::wstring::npos)
      {
        break;
      }

      size_t keyEnd = std::wstring::npos;
      bool escapeNext = false;
      for (size_t index = keyStart + 1; index < content.size(); ++index)
      {
        if (escapeNext)
        {
          escapeNext = false;
          continue;
        }

        if (content[index] == L'\\')
        {
          escapeNext = true;
          continue;
        }

        if (content[index] == L'"')
        {
          keyEnd = index;
          break;
        }
      }
      if (keyEnd == std::wstring::npos)
      {
        break;
      }

      std::wstring key;
      if (!unescapeJsonString(content.substr(keyStart + 1, keyEnd - keyStart - 1), key))
      {
        break;
      }
      const size_t colonPos = content.find(L':', keyEnd + 1);
      if (colonPos == std::wstring::npos)
      {
        break;
      }

      const size_t valueStart = colonPos + 1;
      size_t valueEnd = content.find(L',', valueStart);
      if (valueEnd == std::wstring::npos)
      {
        valueEnd = content.size();
      }

      int amount = 0;
      if (parseJsonNumber(content.substr(valueStart, valueEnd - valueStart), amount) && amount > 0 && !key.empty())
      {
        std::transform(key.begin(), key.end(), key.begin(),
                       [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
        result[key] = amount;
      }

      pos = (valueEnd < content.size()) ? valueEnd + 1 : content.size();
    }

    return result;
  }

  bool extractTopLevelArrayContent(const std::wstring& content,
                                  const std::wstring& key,
                                  std::wstring& arrayContent)
  {
    const size_t keyPos = content.find(key);
    if (keyPos == std::wstring::npos)
    {
      return false;
    }

    const size_t colonPos = content.find(L':', keyPos + key.size());
    if (colonPos == std::wstring::npos)
    {
      return false;
    }

    std::wstring fieldValue = extractFieldValue(content, colonPos);
    fieldValue = StringUtils::trimWhitespace(fieldValue);
    if (fieldValue.size() < 2 || fieldValue.front() != L'[' || fieldValue.back() != L']')
    {
      return false;
    }

    arrayContent = fieldValue.substr(1, fieldValue.size() - 2);
    return true;
  }

  // Returns the position of the '}' that matches the '{' at openBracePos,
  // or std::wstring::npos if not found.
  size_t findMatchingCloseBrace(const std::wstring& str, size_t openBracePos)
  {
    int depth = 0;
    bool inString = false;
    bool escapeNext = false;
    for (size_t i = openBracePos; i < str.size(); ++i)
    {
      if (escapeNext) { escapeNext = false; continue; }
      if (str[i] == L'\\') { escapeNext = true; continue; }
      if (str[i] == L'"') { inString = !inString; continue; }
      if (!inString)
      {
        if (str[i] == L'{') ++depth;
        else if (str[i] == L'}') { if (--depth == 0) return i; }
      }
    }
    return std::wstring::npos;
  }

  std::vector<SkillPrerequisite> parseJsonSkillPrerequisites(const std::wstring& jsonArray)
  {
    std::vector<SkillPrerequisite> result;
    std::wstring trimmed = StringUtils::trimWhitespace(jsonArray);
    if (trimmed.size() < 2 || trimmed.front() != L'[' || trimmed.back() != L']')
    {
      return result;
    }

    std::wstring content = trimmed.substr(1, trimmed.size() - 2);
    size_t pos = 0;
    while (true)
    {
      size_t objectStart = content.find(L'{', pos);
      if (objectStart == std::wstring::npos)
      {
        break;
      }

      size_t objectEnd = findMatchingCloseBrace(content, objectStart);
      if (objectEnd == std::wstring::npos)
      {
        break;
      }

      std::wstring objectContent = content.substr(objectStart + 1, objectEnd - objectStart - 1);
      std::wstring token;
      int requiredLevel = 0;

      auto extractPrerequisiteField = [&objectContent](const std::wstring& key, std::wstring& value) -> bool
      {
        const std::wstring searchKey = L"\"" + key + L"\"";
        const size_t keyPos = objectContent.find(searchKey);
        if (keyPos == std::wstring::npos) return false;
        const size_t colonPos = objectContent.find(L':', keyPos + searchKey.size());
        if (colonPos == std::wstring::npos) return false;
        value = extractFieldValue(objectContent, colonPos);
        return true;
      };

      std::wstring fieldValue;
      if (extractPrerequisiteField(L"token", fieldValue))
      {
        parseJsonString(fieldValue, token);
      }

      if (extractPrerequisiteField(L"requiredLevel", fieldValue))
      {
        parseJsonNumber(fieldValue, requiredLevel);
      }

      if (!token.empty() && requiredLevel > 0)
      {
        result.push_back(SkillPrerequisite{ token, requiredLevel });
      }

      pos = objectEnd + 1;
    }

    return result;
  }

  bool extractTopLevelObjectContent(const std::wstring& content,
                                    const std::wstring& key,
                                    std::wstring& objectContent)
  {
    const size_t keyPos = content.find(key);
    if (keyPos == std::wstring::npos)
    {
      return false;
    }

    const size_t colonPos = content.find(L':', keyPos + key.size());
    if (colonPos == std::wstring::npos)
    {
      return false;
    }

    std::wstring fieldValue = extractFieldValue(content, colonPos);
    fieldValue = StringUtils::trimWhitespace(fieldValue);
    if (fieldValue.size() < 2 || fieldValue.front() != L'{' || fieldValue.back() != L'}')
    {
      return false;
    }

    objectContent = fieldValue.substr(1, fieldValue.size() - 2);
    return true;
  }
}

bool DataSerializer::loadFromFile(AppData& appData, const std::wstring& filePath)
{
  try
  {
    std::wifstream file(filePath);
    if (!file.is_open())
    {
      lastError_ = L"Failed to open file for reading: " + filePath;
      return false;
    }

    std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                        std::istreambuf_iterator<wchar_t>());
    file.close();

    appData.clear();

    // Parse factions
    std::wstring factionsArray;
    if (extractTopLevelArrayContent(content, L"\"factions\"", factionsArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = factionsArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = factionsArray.find(L'}', objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = factionsArray.substr(objStart + 1, objEnd - objStart - 1);

        int number = 0, month = 0, year = 0;
        int taxedOrTradedRegionsCurrent = 0;
        int taxedOrTradedRegionsMax = 0;
        int quartermastersCurrent = 0;
        int quartermastersMax = 0;
        int magesCurrent = 0;
        int magesMax = 0;
        int apprenticesCurrent = 0;
        int apprenticesMax = 0;
        std::wstring name;
        std::wstring password;
        bool mainFaction = false;

        size_t fieldPos = 0;
        while ((fieldPos = obj.find(L'"', fieldPos)) != std::wstring::npos)
        {
          size_t fieldEnd = obj.find(L'"', fieldPos + 1);
          if (fieldEnd == std::wstring::npos)
            break;
          std::wstring fieldName = obj.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
          size_t colonPos = obj.find(L':', fieldEnd);
          if (colonPos == std::wstring::npos)
            break;
          std::wstring fieldValue = extractFieldValue(obj, colonPos);

          if (fieldName == L"number")
            parseJsonNumber(fieldValue, number);
          else if (fieldName == L"name")
            parseJsonString(fieldValue, name);
          else if (fieldName == L"mainFaction")
            parseJsonBool(fieldValue, mainFaction);
          else if (fieldName == L"password")
            parseJsonString(fieldValue, password);
          else if (fieldName == L"taxedOrTradedRegionsCurrent")
            parseJsonNumber(fieldValue, taxedOrTradedRegionsCurrent);
          else if (fieldName == L"taxedOrTradedRegionsMax")
            parseJsonNumber(fieldValue, taxedOrTradedRegionsMax);
          else if (fieldName == L"quartermastersCurrent")
            parseJsonNumber(fieldValue, quartermastersCurrent);
          else if (fieldName == L"quartermastersMax")
            parseJsonNumber(fieldValue, quartermastersMax);
          else if (fieldName == L"magesCurrent")
            parseJsonNumber(fieldValue, magesCurrent);
          else if (fieldName == L"magesMax")
            parseJsonNumber(fieldValue, magesMax);
          else if (fieldName == L"apprenticesCurrent")
            parseJsonNumber(fieldValue, apprenticesCurrent);
          else if (fieldName == L"apprenticesMax")
            parseJsonNumber(fieldValue, apprenticesMax);
          else if (fieldName == L"month")
            parseJsonNumber(fieldValue, month);
          else if (fieldName == L"year")
            parseJsonNumber(fieldValue, year);

          fieldPos = fieldEnd + 1;
        }

        appData.factionRepository().add(number,
                                        name,
                                        mainFaction,
                                        month,
                                        year,
                                        password,
                                        taxedOrTradedRegionsCurrent,
                                        taxedOrTradedRegionsMax,
                                        quartermastersCurrent,
                                        quartermastersMax,
                                        magesCurrent,
                                        magesMax,
                                        apprenticesCurrent,
                                        apprenticesMax);
        pos = objEnd + 1;
      }
    }

    // Parse regions
    std::wstring regionsArray;
    if (extractTopLevelArrayContent(content, L"\"regions\"", regionsArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = regionsArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = findMatchingCloseBrace(regionsArray, objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = regionsArray.substr(objStart + 1, objEnd - objStart - 1);

        int x = 0, y = 0, z = 1, peasantNumber = 0, wagesMax = 0, taxableIncome = 0, month = 0, year = 0, entertainment = 0;
        std::wstring type, province, settlementName, settlementType, peasantType;
        std::vector<std::wstring> exitDirections;
        std::map<std::wstring, int> resources;
        std::map<std::wstring, std::pair<int, int>> wanted;
        std::map<std::wstring, std::pair<int, int>> forSale;
        bool hasSettlement = false, visited = false;
        double wages = 0.0;

        size_t fieldPos = 0;
        while ((fieldPos = obj.find(L'"', fieldPos)) != std::wstring::npos)
        {
          size_t fieldEnd = obj.find(L'"', fieldPos + 1);
          if (fieldEnd == std::wstring::npos)
            break;
          std::wstring fieldName = obj.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
          size_t colonPos = obj.find(L':', fieldEnd);
          if (colonPos == std::wstring::npos)
            break;
          std::wstring fieldValue = extractFieldValue(obj, colonPos);

          if (fieldName == L"x")
            parseJsonNumber(fieldValue, x);
          else if (fieldName == L"y")
            parseJsonNumber(fieldValue, y);
          else if (fieldName == L"z")
            parseJsonNumber(fieldValue, z);
          else if (fieldName == L"type")
            parseJsonString(fieldValue, type);
          else if (fieldName == L"province")
            parseJsonString(fieldValue, province);
          else if (fieldName == L"hasSettlement")
            parseJsonBool(fieldValue, hasSettlement);
          else if (fieldName == L"settlementName")
            parseJsonString(fieldValue, settlementName);
          else if (fieldName == L"settlementType")
            parseJsonString(fieldValue, settlementType);
          else if (fieldName == L"peasantType")
            parseJsonString(fieldValue, peasantType);
          else if (fieldName == L"peasantNumber")
            parseJsonNumber(fieldValue, peasantNumber);
          else if (fieldName == L"wages")
            parseJsonNumber(fieldValue, wages);
          else if (fieldName == L"wagesMax")
            parseJsonNumber(fieldValue, wagesMax);
          else if (fieldName == L"taxableIncome")
            parseJsonNumber(fieldValue, taxableIncome);
          else if (fieldName == L"entertainment")
            parseJsonNumber(fieldValue, entertainment);
          else if (fieldName == L"resources")
          {
            // Parse resources map: {"TOKEN": amount, ...}
            resources.clear();
            size_t mapPos = fieldValue.find(L'{');
            if (mapPos != std::wstring::npos)
            {
              size_t mapEnd = fieldValue.find(L'}', mapPos);
              if (mapEnd != std::wstring::npos)
              {
                std::wstring mapContent = fieldValue.substr(mapPos + 1, mapEnd - mapPos - 1);
                size_t kvPos = 0;
                while ((kvPos = mapContent.find(L'"', kvPos)) != std::wstring::npos)
                {
                  size_t kvEnd = mapContent.find(L'"', kvPos + 1);
                  if (kvEnd == std::wstring::npos) break;
                  std::wstring resToken = mapContent.substr(kvPos + 1, kvEnd - kvPos - 1);
                  size_t resColonPos = mapContent.find(L':', kvEnd);
                  if (resColonPos == std::wstring::npos) break;
                  std::wstring resValue = extractFieldValue(mapContent, resColonPos);
                  int resAmount = 0;
                  try { resAmount = std::stoi(resValue); } catch (...) {}
                  resources[resToken] = resAmount;
                  kvPos = kvEnd + 1;
                }
              }
            }
          }
          else if (fieldName == L"wanted")
          {
            // Parse wanted map: {"TOKEN": [amount, price], ...}
            wanted.clear();
            size_t mapPos = fieldValue.find(L'{');
            if (mapPos != std::wstring::npos)
            {
              size_t mapEnd = fieldValue.find(L'}', mapPos);
              if (mapEnd != std::wstring::npos)
              {
                std::wstring mapContent = fieldValue.substr(mapPos + 1, mapEnd - mapPos - 1);
                size_t kvPos = 0;
                while ((kvPos = mapContent.find(L'"', kvPos)) != std::wstring::npos)
                {
                  size_t kvEnd = mapContent.find(L'"', kvPos + 1);
                  if (kvEnd == std::wstring::npos) break;
                  std::wstring wantedToken = mapContent.substr(kvPos + 1, kvEnd - kvPos - 1);
                  size_t wantedColonPos = mapContent.find(L':', kvEnd);
                  if (wantedColonPos == std::wstring::npos) break;
                  size_t bracketStart = mapContent.find(L'[', wantedColonPos);
                  size_t bracketEnd = mapContent.find(L']', bracketStart);
                  if (bracketStart != std::wstring::npos && bracketEnd != std::wstring::npos)
                  {
                    std::wstring arrayContent = mapContent.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                    size_t commaPos = arrayContent.find(L',');
                    int wantedAmount = 0, wantedPrice = 0;
                    try
                    {
                      wantedAmount = std::stoi(arrayContent.substr(0, commaPos));
                      wantedPrice = std::stoi(arrayContent.substr(commaPos + 1));
                    }
                    catch (...) {}
                    wanted[wantedToken] = {wantedAmount, wantedPrice};
                  }
                  kvPos = kvEnd + 1;
                }
              }
            }
          }
          else if (fieldName == L"forSale")
          {
            // Parse forSale map: {"TOKEN": [amount, price], ...}
            forSale.clear();
            size_t mapPos = fieldValue.find(L'{');
            if (mapPos != std::wstring::npos)
            {
              size_t mapEnd = fieldValue.find(L'}', mapPos);
              if (mapEnd != std::wstring::npos)
              {
                std::wstring mapContent = fieldValue.substr(mapPos + 1, mapEnd - mapPos - 1);
                size_t kvPos = 0;
                while ((kvPos = mapContent.find(L'"', kvPos)) != std::wstring::npos)
                {
                  size_t kvEnd = mapContent.find(L'"', kvPos + 1);
                  if (kvEnd == std::wstring::npos) break;
                  std::wstring forSaleToken = mapContent.substr(kvPos + 1, kvEnd - kvPos - 1);
                  size_t forSaleColonPos = mapContent.find(L':', kvEnd);
                  if (forSaleColonPos == std::wstring::npos) break;
                  size_t bracketStart = mapContent.find(L'[', forSaleColonPos);
                  size_t bracketEnd = mapContent.find(L']', bracketStart);
                  if (bracketStart != std::wstring::npos && bracketEnd != std::wstring::npos)
                  {
                    std::wstring arrayContent = mapContent.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                    size_t commaPos = arrayContent.find(L',');
                    int forSaleAmount = 0, forSalePrice = 0;
                    try
                    {
                      forSaleAmount = std::stoi(arrayContent.substr(0, commaPos));
                      forSalePrice = std::stoi(arrayContent.substr(commaPos + 1));
                    }
                    catch (...) {}
                    forSale[forSaleToken] = {forSaleAmount, forSalePrice};
                  }
                  kvPos = kvEnd + 1;
                }
              }
            }
          }
          else if (fieldName == L"exitDirections")
            exitDirections = parseJsonStringArray(fieldValue);
          else if (fieldName == L"visited")
            parseJsonBool(fieldValue, visited);
          else if (fieldName == L"month")
            parseJsonNumber(fieldValue, month);
          else if (fieldName == L"year")
            parseJsonNumber(fieldValue, year);

          fieldPos = fieldEnd + 1;
        }

        appData.regionRepository().upsertRegion(x, y, z, type, province, hasSettlement,
                                               settlementName, settlementType, peasantType,
                                               peasantNumber, wages, wagesMax, taxableIncome,
                                               month, year, visited);

        if (Region* loadedRegion = appData.regionRepository().findByCoordinates(x, y, z))
        {
          loadedRegion->clearExitDirections();
          for (const std::wstring& direction : exitDirections)
          {
            loadedRegion->addExitDirection(direction);
          }
          loadedRegion->setEntertainment(entertainment);
          loadedRegion->setResources(resources);
          loadedRegion->setWanted(wanted);
          loadedRegion->setForSale(forSale);
        }
        pos = objEnd + 1;
      }
    }

    // Parse units
    std::wstring unitsArray;
    if (extractTopLevelArrayContent(content, L"\"units\"", unitsArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = unitsArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = findMatchingCloseBrace(unitsArray, objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = unitsArray.substr(objStart + 1, objEnd - objStart - 1);

        int number = 0, factionNumber = 0, structureId = 0, x = 0, y = 0, z = 1, month = 0, year = 0, weight = 0;
        int capacityWalk = 0, capacityRide = 0, capacityFly = 0, capacitySwim = 0;
        std::wstring name;
        std::vector<std::wstring> flags, skillStrings, canStudySkills, orders;
        std::map<std::wstring, int> skills;
        std::map<std::wstring, int> itemCounts;

        auto extractObjectFieldValue = [&obj](const std::wstring& key, std::wstring& value) -> bool
        {
          const std::wstring token = L"\"" + key + L"\"";
          const size_t keyPos = obj.find(token);
          if (keyPos == std::wstring::npos)
          {
            return false;
          }

          const size_t colonPos = obj.find(L':', keyPos + token.size());
          if (colonPos == std::wstring::npos)
          {
            return false;
          }

          value = extractFieldValue(obj, colonPos);
          return true;
        };

        std::wstring fieldValue;
        if (extractObjectFieldValue(L"number", fieldValue))      parseJsonNumber(fieldValue, number);
        if (extractObjectFieldValue(L"name", fieldValue))        parseJsonString(fieldValue, name);
        if (extractObjectFieldValue(L"factionNumber", fieldValue)) parseJsonNumber(fieldValue, factionNumber);
        if (extractObjectFieldValue(L"structureId", fieldValue)) parseJsonNumber(fieldValue, structureId);
        if (extractObjectFieldValue(L"x", fieldValue))           parseJsonNumber(fieldValue, x);
        if (extractObjectFieldValue(L"y", fieldValue))           parseJsonNumber(fieldValue, y);
        if (extractObjectFieldValue(L"z", fieldValue))           parseJsonNumber(fieldValue, z);
        if (extractObjectFieldValue(L"month", fieldValue))       parseJsonNumber(fieldValue, month);
        if (extractObjectFieldValue(L"year", fieldValue))        parseJsonNumber(fieldValue, year);
        if (extractObjectFieldValue(L"weight", fieldValue))      parseJsonNumber(fieldValue, weight);
        if (extractObjectFieldValue(L"capacityWalk", fieldValue)) parseJsonNumber(fieldValue, capacityWalk);
        if (extractObjectFieldValue(L"capacityRide", fieldValue)) parseJsonNumber(fieldValue, capacityRide);
        if (extractObjectFieldValue(L"capacityFly", fieldValue)) parseJsonNumber(fieldValue, capacityFly);
        if (extractObjectFieldValue(L"capacitySwim", fieldValue)) parseJsonNumber(fieldValue, capacitySwim);
        if (extractObjectFieldValue(L"flags", fieldValue))       flags = parseJsonStringArray(fieldValue);
        if (extractObjectFieldValue(L"items", fieldValue))
        {
          itemCounts = parseJsonStringIntMap(fieldValue);
        }
        if (extractObjectFieldValue(L"skills", fieldValue))      skillStrings = parseJsonStringArray(fieldValue);
        if (extractObjectFieldValue(L"canStudySkills", fieldValue)) canStudySkills = parseJsonStringArray(fieldValue);
        if (extractObjectFieldValue(L"orders", fieldValue))      orders = parseJsonStringArray(fieldValue);

        static const std::wregex skillDetailPattern(L"(\\[([^\\]]+)\\]\\s+)?(\\d+)");
        for (const std::wstring& skillString : skillStrings)
        {
          std::wsmatch skillMatch;
          if (std::regex_search(skillString, skillMatch, skillDetailPattern))
          {
            // Try to extract token from [TOKEN] or fall back to the skill string itself
            std::wstring token;
            int days = 0;
            
            // Look for [TOKEN] pattern in the skill string
            size_t bracketStart = skillString.find(L"[");
            if (bracketStart != std::wstring::npos)
            {
              size_t bracketEnd = skillString.find(L"]", bracketStart);
              if (bracketEnd != std::wstring::npos)
              {
                token = skillString.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
              }
            }
            
            // Extract days value (the last number in the string)
            size_t lastNumberStart = skillString.find_last_not_of(L"0123456789");
            if (lastNumberStart != std::wstring::npos && lastNumberStart + 1 < skillString.size())
            {
              std::wstring daysStr = skillString.substr(lastNumberStart + 1);
              parseJsonNumber(daysStr, days);
            }
            
            if (!token.empty() && days > 0)
            {
              // Normalize token to uppercase for consistency with skill repository
              std::transform(token.begin(), token.end(), token.begin(),
                             [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
              skills[token] = days;
            }
          }
        }

        appData.unitRepository().add(number, name, factionNumber, structureId, x, y, z, flags, itemCounts,
                                    weight, capacityWalk, capacityRide, capacityFly,
                                    capacitySwim, skills, month, year);
        if (!orders.empty())
        {
          if (Unit* loadedUnit = appData.unitRepository().findByNumber(number))
          {
            loadedUnit->setOrders(std::move(orders));
          }
        }
        if (!canStudySkills.empty())
        {
          if (Unit* loadedUnit = appData.unitRepository().findByNumber(number))
          {
            loadedUnit->setCanStudySkillTokens(std::move(canStudySkills));
          }
        }
        pos = objEnd + 1;
      }
    }

    // Parse unitNews after loading standard units.
    // This block reads the additional UnitNew entries created after the main game units.
    std::wstring unitNewsArray;
    if (extractTopLevelArrayContent(content, L"\"unitNews\"", unitNewsArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = unitNewsArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = findMatchingCloseBrace(unitNewsArray, objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = unitNewsArray.substr(objStart + 1, objEnd - objStart - 1);

        int number = 0;
        int structureId = 0;
        int futureStructureId = 0;
        int x = 0;
        int y = 0;
        int z = 1;
        int month = 0;
        int year = 0;
        int originalUnit = 0;
        int weight = 0;
        int capacityWalk = 0;
        int capacityRide = 0;
        int capacityFly = 0;
        int capacitySwim = 0;
        std::wstring name;
        std::vector<std::wstring> flags;
        std::vector<std::wstring> canStudySkills;
        std::vector<std::wstring> warnings;
        std::map<std::wstring, int> skills;
        std::map<std::wstring, int> itemCounts;

        auto extractObjectFieldValue = [&obj](const std::wstring& key, std::wstring& value) -> bool
        {
          const std::wstring token = L"\"" + key + L"\"";
          const size_t keyPos = obj.find(token);
          if (keyPos == std::wstring::npos)
          {
            return false;
          }

          const size_t colonPos = obj.find(L':', keyPos + token.size());
          if (colonPos == std::wstring::npos)
          {
            return false;
          }

          value = extractFieldValue(obj, colonPos);
          return true;
        };

        std::wstring fieldValue;
        if (extractObjectFieldValue(L"number", fieldValue)) parseJsonNumber(fieldValue, number);
        if (extractObjectFieldValue(L"name", fieldValue)) parseJsonString(fieldValue, name);
        if (extractObjectFieldValue(L"structureId", fieldValue)) parseJsonNumber(fieldValue, structureId);
        if (extractObjectFieldValue(L"futureStructureId", fieldValue)) parseJsonNumber(fieldValue, futureStructureId);
        if (extractObjectFieldValue(L"x", fieldValue)) parseJsonNumber(fieldValue, x);
        if (extractObjectFieldValue(L"y", fieldValue)) parseJsonNumber(fieldValue, y);
        if (extractObjectFieldValue(L"z", fieldValue)) parseJsonNumber(fieldValue, z);
        if (extractObjectFieldValue(L"month", fieldValue)) parseJsonNumber(fieldValue, month);
        if (extractObjectFieldValue(L"year", fieldValue)) parseJsonNumber(fieldValue, year);
        if (extractObjectFieldValue(L"originUnit", fieldValue)) parseJsonNumber(fieldValue, originalUnit);
        if (extractObjectFieldValue(L"weight", fieldValue)) parseJsonNumber(fieldValue, weight);
        if (extractObjectFieldValue(L"capacityWalk", fieldValue)) parseJsonNumber(fieldValue, capacityWalk);
        if (extractObjectFieldValue(L"capacityRide", fieldValue)) parseJsonNumber(fieldValue, capacityRide);
        if (extractObjectFieldValue(L"capacityFly", fieldValue)) parseJsonNumber(fieldValue, capacityFly);
        if (extractObjectFieldValue(L"capacitySwim", fieldValue)) parseJsonNumber(fieldValue, capacitySwim);
        if (extractObjectFieldValue(L"flags", fieldValue)) flags = parseJsonStringArray(fieldValue);
        if (extractObjectFieldValue(L"items", fieldValue)) itemCounts = parseJsonStringIntMap(fieldValue);
        if (extractObjectFieldValue(L"skills", fieldValue))
        {
          std::vector<std::wstring> skillStrings = parseJsonStringArray(fieldValue);
          static const std::wregex skillDetailPattern(L"(\\[([^\\]]+)\\]\\s+)?(\\d+)");
          for (const std::wstring& skillString : skillStrings)
          {
            std::wsmatch skillMatch;
            if (std::regex_search(skillString, skillMatch, skillDetailPattern))
            {
              std::wstring token;
              int days = 0;
              size_t bracketStart = skillString.find(L"[");
              if (bracketStart != std::wstring::npos)
              {
                size_t bracketEnd = skillString.find(L"]", bracketStart);
                if (bracketEnd != std::wstring::npos)
                {
                  token = skillString.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                }
              }
              size_t lastNumberStart = skillString.find_last_not_of(L"0123456789");
              if (lastNumberStart != std::wstring::npos && lastNumberStart + 1 < skillString.size())
              {
                std::wstring daysStr = skillString.substr(lastNumberStart + 1);
                parseJsonNumber(daysStr, days);
              }
              if (!token.empty() && days > 0)
              {
                std::transform(token.begin(), token.end(), token.begin(),
                               [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
                skills[token] = days;
              }
            }
          }
        }
        if (extractObjectFieldValue(L"canStudySkills", fieldValue)) canStudySkills = parseJsonStringArray(fieldValue);
        if (extractObjectFieldValue(L"warnings", fieldValue)) warnings = parseJsonStringArray(fieldValue);

        appData.unitNewRepository().add(number, name, structureId, x, y, z, flags, itemCounts,
                                        weight, capacityWalk, capacityRide, capacityFly,
                                        capacitySwim, skills, month, year, originalUnit);
        if (UnitNew* loadedUnitNew = appData.unitNewRepository().findByNumberAndCoordinates(number, x, y, z))
        {
          loadedUnitNew->setFutureStructureId(futureStructureId);
          if (!canStudySkills.empty())
          {
            loadedUnitNew->setCanStudySkillTokens(std::move(canStudySkills));
          }
          for (const std::wstring& warning : warnings)
          {
            loadedUnitNew->addWarning(warning);
          }
        }

        pos = objEnd + 1;
      }
    }

    // Parse events
    std::wstring eventsArray;
    if (extractTopLevelArrayContent(content, L"\"events\"", eventsArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = eventsArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = eventsArray.find(L'}', objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = eventsArray.substr(objStart + 1, objEnd - objStart - 1);

        int unitId = 0;
        std::wstring message;

        size_t fieldPos = 0;
        while ((fieldPos = obj.find(L'"', fieldPos)) != std::wstring::npos)
        {
          size_t fieldEnd = obj.find(L'"', fieldPos + 1);
          if (fieldEnd == std::wstring::npos)
            break;
          std::wstring fieldName = obj.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
          size_t colonPos = obj.find(L':', fieldEnd);
          if (colonPos == std::wstring::npos)
            break;
          std::wstring fieldValue = extractFieldValue(obj, colonPos);

          if (fieldName == L"unitId")
            parseJsonNumber(fieldValue, unitId);
          else if (fieldName == L"message")
            parseJsonString(fieldValue, message);

          fieldPos = fieldEnd + 1;
        }

        if (unitId != 0 && !message.empty())
        {
          appData.eventRepository().add(unitId, message);
        }
        pos = objEnd + 1;
      }
    }

    // Parse settings
    std::wstring settingsObject;
    if (extractTopLevelObjectContent(content, L"\"settings\"", settingsObject))
    {
      int shipStructureIdThreshold = appData.getShipStructureIdThreshold();
      std::wstring flyingShipsCsv;
      bool onlyLeaderCanTeach = appData.getOnlyLeaderCanTeach();
      bool leaderMages = appData.getLeaderMages();

      size_t fieldPos = 0;
      while ((fieldPos = settingsObject.find(L'"', fieldPos)) != std::wstring::npos)
      {
        size_t fieldEnd = settingsObject.find(L'"', fieldPos + 1);
        if (fieldEnd == std::wstring::npos)
          break;

        std::wstring fieldName = settingsObject.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
        size_t colonPos = settingsObject.find(L':', fieldEnd);
        if (colonPos == std::wstring::npos)
          break;

        std::wstring fieldValue = extractFieldValue(settingsObject, colonPos);

        if (fieldName == L"shipStructureIdThreshold")
          parseJsonNumber(fieldValue, shipStructureIdThreshold);
        else if (fieldName == L"flyingShipsCsv")
          parseJsonString(fieldValue, flyingShipsCsv);
        else if (fieldName == L"onlyLeaderCanTeach")
          parseJsonBool(fieldValue, onlyLeaderCanTeach);
        else if (fieldName == L"leaderMages")
          parseJsonBool(fieldValue, leaderMages);

        fieldPos = fieldEnd + 1;
      }

      appData.setShipStructureIdThreshold(shipStructureIdThreshold);
      appData.setFlyingShipsCsv(flyingShipsCsv);
      appData.setOnlyLeaderCanTeach(onlyLeaderCanTeach);
      appData.setLeaderMages(leaderMages);
    }

    // Parse structures
    std::wstring structuresArray;
    if (extractTopLevelArrayContent(content, L"\"structures\"", structuresArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = structuresArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = structuresArray.find(L'}', objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = structuresArray.substr(objStart + 1, objEnd - objStart - 1);

        int id = 0, x = 0, y = 0, z = 1, month = 0, year = 0;
        int ownerUnitId = 0;
        std::wstring type, name;
        bool isClosed = false;

        size_t fieldPos = 0;
        while ((fieldPos = obj.find(L'"', fieldPos)) != std::wstring::npos)
        {
          size_t fieldEnd = obj.find(L'"', fieldPos + 1);
          if (fieldEnd == std::wstring::npos)
            break;
          std::wstring fieldName = obj.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
          size_t colonPos = obj.find(L':', fieldEnd);
          if (colonPos == std::wstring::npos)
            break;
          std::wstring fieldValue = extractFieldValue(obj, colonPos);

          if (fieldName == L"id")
            parseJsonNumber(fieldValue, id);
          else if (fieldName == L"x")
            parseJsonNumber(fieldValue, x);
          else if (fieldName == L"y")
            parseJsonNumber(fieldValue, y);
          else if (fieldName == L"z")
            parseJsonNumber(fieldValue, z);
          else if (fieldName == L"type")
            parseJsonString(fieldValue, type);
          else if (fieldName == L"name")
            parseJsonString(fieldValue, name);
          else if (fieldName == L"month")
            parseJsonNumber(fieldValue, month);
          else if (fieldName == L"year")
            parseJsonNumber(fieldValue, year);
          else if (fieldName == L"isClosed")
            parseJsonBool(fieldValue, isClosed);
          else if (fieldName == L"ownerUnitId")
            parseJsonNumber(fieldValue, ownerUnitId);

          fieldPos = fieldEnd + 1;
        }

        appData.structureRepository().add(id, x, y, z, type, name, isClosed, month, year);
        if (Structure* structure = appData.structureRepository().findByIdAndCoordinates(id, x, y, z))
        {
          structure->setOwnerUnitId(ownerUnitId);
        }
        pos = objEnd + 1;
      }

      // Ensure derived flags match loaded settings for all structures.
      appData.refreshStructureDerivedFlags();
    }

    // Parse structureInfo
    std::wstring structInfoArray;
    if (!extractTopLevelArrayContent(content, L"\"structureInfo\"", structInfoArray))
    {
      // Backward compatibility with older key name.
      extractTopLevelArrayContent(content, L"\"structInfo\"", structInfoArray);
    }
    if (!structInfoArray.empty())
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = structInfoArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = structInfoArray.find(L'}', objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = structInfoArray.substr(objStart + 1, objEnd - objStart - 1);

        int needs = 0, magesCapacity = 0;
        std::wstring type, itemIdentifierToken;
        bool isShip = false, isFlying = false;

        size_t fieldPos = 0;
        while ((fieldPos = obj.find(L'"', fieldPos)) != std::wstring::npos)
        {
          size_t fieldEnd = obj.find(L'"', fieldPos + 1);
          if (fieldEnd == std::wstring::npos)
            break;
          std::wstring fieldName = obj.substr(fieldPos + 1, fieldEnd - fieldPos - 1);
          size_t colonPos = obj.find(L':', fieldEnd);
          if (colonPos == std::wstring::npos)
            break;
          std::wstring fieldValue = extractFieldValue(obj, colonPos);

          if (fieldName == L"type")
            parseJsonString(fieldValue, type);
          else if (fieldName == L"needs")
            parseJsonNumber(fieldValue, needs);
          else if (fieldName == L"magesCapacity")
            parseJsonNumber(fieldValue, magesCapacity);
          else if (fieldName == L"isShip")
            parseJsonBool(fieldValue, isShip);
          else if (fieldName == L"isFlying")
            parseJsonBool(fieldValue, isFlying);
          else if (fieldName == L"itemIdentifierToken")
            parseJsonString(fieldValue, itemIdentifierToken);

          fieldPos = fieldEnd + 1;
        }

        if (!type.empty())
        {
          appData.structInfoRepository().addOrUpdateByType(type, needs, magesCapacity, isShip, isFlying, itemIdentifierToken);
        }
        pos = objEnd + 1;
      }
      appData.refreshStructureDerivedFlags();
    }

    // Parse itemRepository
    std::wstring itemRepositoryArray;
    if (extractTopLevelArrayContent(content, L"\"itemRepository\"", itemRepositoryArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = itemRepositoryArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = findMatchingCloseBrace(itemRepositoryArray, objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = itemRepositoryArray.substr(objStart + 1, objEnd - objStart - 1);

        std::wstring token, name, fullText;
        int weight = 0, moves = 0, walkCapacity = 0, rideCapacity = 0, swimCapacity = 0, flyCapacity = 0;
        int shipSpeedHexesPerMonth = 0, shipSailingSkillRequired = 0, magesStudy = 0, defaultSkillMax = 0;
        bool meeleWeapon = false, rangedWeapon = false, armour = false, resource = false, mount = false, man = false;
        std::map<std::wstring, int> resources;
        std::map<std::wstring, int> skillsMax;
        std::map<std::wstring, int> productionSkill;
        std::map<std::wstring, int> productionHelp;

        auto extractItemField = [&obj](const std::wstring& key, std::wstring& value) -> bool
        {
          const std::wstring searchKey = L"\"" + key + L"\"";
          const size_t keyPos = obj.find(searchKey);
          if (keyPos == std::wstring::npos) return false;
          const size_t colonPos = obj.find(L':', keyPos + searchKey.size());
          if (colonPos == std::wstring::npos) return false;
          value = extractFieldValue(obj, colonPos);
          return true;
        };

        std::wstring fv;
        if (extractItemField(L"token", fv))        parseJsonString(fv, token);
        if (extractItemField(L"name", fv))         parseJsonString(fv, name);
        if (extractItemField(L"weight", fv))       parseJsonNumber(fv, weight);
        if (extractItemField(L"meeleWeapon", fv))  parseJsonBool(fv, meeleWeapon);
        if (extractItemField(L"rangedWeapon", fv)) parseJsonBool(fv, rangedWeapon);
        if (extractItemField(L"armour", fv))       parseJsonBool(fv, armour);
        if (extractItemField(L"resource", fv))     parseJsonBool(fv, resource);
        if (extractItemField(L"mount", fv))        parseJsonBool(fv, mount);
        if (extractItemField(L"moves", fv))        parseJsonNumber(fv, moves);
        if (extractItemField(L"walkCapacity", fv)) parseJsonNumber(fv, walkCapacity);
        if (extractItemField(L"rideCapacity", fv)) parseJsonNumber(fv, rideCapacity);
        if (extractItemField(L"swimCapacity", fv)) parseJsonNumber(fv, swimCapacity);
        if (extractItemField(L"flyCapacity", fv))  parseJsonNumber(fv, flyCapacity);
        if (extractItemField(L"shipSpeedHexesPerMonth", fv)) parseJsonNumber(fv, shipSpeedHexesPerMonth);
        if (extractItemField(L"shipSailingSkillRequired", fv)) parseJsonNumber(fv, shipSailingSkillRequired);
        if (extractItemField(L"magesStudy", fv)) parseJsonNumber(fv, magesStudy);
        if (extractItemField(L"man", fv))          parseJsonBool(fv, man);
        if (extractItemField(L"defaultSkillMax", fv)) parseJsonNumber(fv, defaultSkillMax);
        if (extractItemField(L"fullText", fv))     parseJsonString(fv, fullText);
        if (extractItemField(L"resources", fv))    resources = parseJsonStringIntMap(fv);
        if (extractItemField(L"skillsMax", fv))    skillsMax = parseJsonStringIntMap(fv);
        if (extractItemField(L"productionSkill", fv)) productionSkill = parseJsonStringIntMap(fv);
        if (extractItemField(L"productionHelp", fv)) productionHelp = parseJsonStringIntMap(fv);

        if (!token.empty())
        {
          Item* existing = appData.itemRepository().findByIdentifierToken(token);
          if (existing)
          {
            if (!name.empty()) existing->setItemName(name);
            existing->setWeight(weight);
            existing->setMeeleWeapon(meeleWeapon);
            existing->setRangedWeapon(rangedWeapon);
            existing->setArmour(armour);
            existing->setResource(resource);
            existing->setMount(mount);
            existing->setMoves(moves);
            existing->setWalkCapacity(walkCapacity);
            existing->setRideCapacity(rideCapacity);
            existing->setSwimCapacity(swimCapacity);
            existing->setFlyCapacity(flyCapacity);
            existing->setShipSpeedHexesPerMonth(shipSpeedHexesPerMonth);
            existing->setShipSailingSkillRequired(shipSailingSkillRequired);
            existing->setMagesStudy(magesStudy);
            existing->setMan(man);
            existing->setDefaultSkillMax(defaultSkillMax);
            existing->setFullText(fullText);
            existing->setResources(resources);
            existing->setSkillsMax(skillsMax);
            existing->setProductionSkill(productionSkill);
            existing->setProductionHelp(productionHelp);
          }
          else
          {
            appData.itemRepository().add(token, name, weight, meeleWeapon, rangedWeapon,
                                        armour, resource, mount, moves,
                                        walkCapacity, rideCapacity, swimCapacity, flyCapacity, man);
            if (Item* added = appData.itemRepository().findByIdentifierToken(token))
            {
              added->setShipSpeedHexesPerMonth(shipSpeedHexesPerMonth);
              added->setShipSailingSkillRequired(shipSailingSkillRequired);
              added->setMagesStudy(magesStudy);
              added->setDefaultSkillMax(defaultSkillMax);
              added->setFullText(fullText);
              added->setResources(resources);
              added->setSkillsMax(skillsMax);
              added->setProductionSkill(productionSkill);
              added->setProductionHelp(productionHelp);
            }
          }
        }

        pos = objEnd + 1;
      }
    }

    // Parse skillRepository
    std::wstring skillRepositoryArray;
    if (extractTopLevelArrayContent(content, L"\"skillRepository\"", skillRepositoryArray))
    {
      size_t pos = 0;
      while (true)
      {
        size_t objStart = skillRepositoryArray.find(L'{', pos);
        if (objStart == std::wstring::npos)
          break;
        size_t objEnd = findMatchingCloseBrace(skillRepositoryArray, objStart);
        if (objEnd == std::wstring::npos)
          break;

        std::wstring obj = skillRepositoryArray.substr(objStart + 1, objEnd - objStart - 1);

        std::wstring token;
        std::wstring name;
        int level = 1;
        bool magic = false;
        bool magicFoundation = false;
        std::wstring description;
        int parsedStudyCost = 0;
        std::map<std::wstring, int> productionItems;
        std::vector<SkillPrerequisite> prerequisites;

        auto extractSkillField = [&obj](const std::wstring& key, std::wstring& value) -> bool
        {
          const std::wstring searchKey = L"\"" + key + L"\"";
          const size_t keyPos = obj.find(searchKey);
          if (keyPos == std::wstring::npos) return false;
          const size_t colonPos = obj.find(L':', keyPos + searchKey.size());
          if (colonPos == std::wstring::npos) return false;
          value = extractFieldValue(obj, colonPos);
          return true;
        };

        std::wstring fv;
        if (extractSkillField(L"token", fv)) parseJsonString(fv, token);
        if (extractSkillField(L"level", fv)) parseJsonNumber(fv, level);
        if (extractSkillField(L"name", fv)) parseJsonString(fv, name);
        if (extractSkillField(L"productionItems", fv)) productionItems = parseJsonStringIntMap(fv);
        if (extractSkillField(L"magic", fv)) parseJsonBool(fv, magic);
        if (extractSkillField(L"magicFoundation", fv)) parseJsonBool(fv, magicFoundation);
        if (extractSkillField(L"description", fv)) parseJsonString(fv, description);
        if (extractSkillField(L"studyCost", fv)) parseJsonNumber(fv, parsedStudyCost);
        if (extractSkillField(L"prerequisites", fv)) prerequisites = parseJsonSkillPrerequisites(fv);

        if (!token.empty())
        {
          const int studyCost = (std::max)(0, parsedStudyCost);

          const bool levelAdded = appData.skillRepository().addLevel(
            token, name, level, productionItems, description);

          if (levelAdded)
          {
            Skill* skill = appData.skillRepository().findByIdentifier(token);
            if (skill)
            {
              if (magic)
              {
                skill->setMagic(true);
              }

              if (magicFoundation)
              {
                skill->setMagicFoundation(true);
              }

              if (studyCost > 0)
              {
                skill->setStudyCost(studyCost);
              }

              if (!prerequisites.empty())
              {
                skill->setPrerequisites(prerequisites);
              }
            }
          }

          for (const auto& [itemToken, _] : productionItems)
          {
            Item* producedItem = appData.itemRepository().findByIdentifierToken(itemToken);
            if (producedItem)
            {
              producedItem->setProductionSkillLevel(token, level);
            }
          }
        }

        pos = objEnd + 1;
      }
    }

    // OrderRepository is derived data: rebuild it from loaded unit orders.
    appData.orderRepository().clear();
    for (std::size_t index = 0; index < appData.unitRepository().size(); ++index)
    {
      const int originUnitNumber = appData.unitRepository().at(index).getUnitNumber();
      OrderBusinessLogic::syncOrderRepositoryForSavedUnit(appData, originUnitNumber, false);
    }
    CommandSimulationService::recalculateAfterOrdersValues(appData);

    lastError_ = L"";
    return true;
  }
  catch (const std::exception& e)
  {
    lastError_ = L"Exception during load: ";
    lastError_ += std::wstring(e.what(), e.what() + std::strlen(e.what()));
    appData.clear();
    return false;
  }
}

const std::wstring& DataSerializer::getLastError()
{
  return lastError_;
}
