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
 * File: Report.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Report.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>

#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/UnitRepository.hpp"
#include "Function/MonthUtils.hpp"
#include "Function/StringUtils.hpp"

bool Report::loadFromFile(const std::wstring& filePath)
{
  clear();
  filePath_ = filePath;

  std::wifstream file(filePath);
  if (!file.is_open())
  {
    lastError_ = L"Failed to open file: " + filePath;
    return false;
  }

  // Set up UTF-8 codecvt for proper wide character handling
  std::wstringstream buffer;
  buffer << file.rdbuf();
  content_ = buffer.str();
  file.close();

  if (content_.empty())
  {
    lastError_ = L"File is empty or cannot be read";
    return false;
  }

  splitIntoLines();
  parseAtlantisHeader();
  return true;
}

const std::wstring& Report::getContent() const
{
  return content_;
}

const std::vector<std::wstring>& Report::getLines() const
{
  return lines_;
}

const std::wstring& Report::getFilePath() const
{
  return filePath_;
}

int Report::getFactionNumber() const
{
  return factionNumber_;
}

const std::wstring& Report::getFactionName() const
{
  return factionName_;
}

Faction* Report::getFaction() const
{
  return faction_;
}

int Report::getFactionTaxedOrTradedRegionsCurrent() const
{
  return factionTaxedOrTradedRegionsCurrent_;
}

int Report::getFactionTaxedOrTradedRegionsMax() const
{
  return factionTaxedOrTradedRegionsMax_;
}

int Report::getFactionQuartermastersCurrent() const
{
  return factionQuartermastersCurrent_;
}

int Report::getFactionQuartermastersMax() const
{
  return factionQuartermastersMax_;
}

int Report::getFactionMagesCurrent() const
{
  return factionMagesCurrent_;
}

int Report::getFactionMagesMax() const
{
  return factionMagesMax_;
}

int Report::getFactionApprenticesCurrent() const
{
  return factionApprenticesCurrent_;
}

int Report::getFactionApprenticesMax() const
{
  return factionApprenticesMax_;
}

int Report::getFactionUnclaimedSilver() const
{
  return factionUnclaimedSilver_;
}

const std::wstring& Report::getFactionDefaultAttitudeText() const
{
  return factionDefaultAttitudeText_;
}

const std::map<int, std::wstring>& Report::getFactionDeclaredAttitudesText() const
{
  return factionDeclaredAttitudesText_;
}

const std::map<int, std::wstring>& Report::getFactionDeclaredFactionNames() const
{
  return factionDeclaredFactionNames_;
}

int Report::getMonth() const
{
  return month_;
}

int Report::getYear() const
{
  return year_;
}

bool Report::hasContent() const
{
  return !content_.empty();
}

const std::wstring& Report::getLastError() const
{
  return lastError_;
}

void Report::setMonth(int month)
{
  month_ = month;
}

void Report::setYear(int year)
{
  year_ = year;
}

void Report::setFactionNumber(int number)
{
  factionNumber_ = number;
}

void Report::setFactionName(std::wstring name)
{
  factionName_ = std::move(name);
}

void Report::setFaction(Faction* faction)
{
  faction_ = faction;
}

void Report::setFactionTaxedOrTradedRegionsCurrent(int value)
{
  factionTaxedOrTradedRegionsCurrent_ = value;
}

void Report::setFactionTaxedOrTradedRegionsMax(int value)
{
  factionTaxedOrTradedRegionsMax_ = value;
}

void Report::setFactionQuartermastersCurrent(int value)
{
  factionQuartermastersCurrent_ = value;
}

void Report::setFactionQuartermastersMax(int value)
{
  factionQuartermastersMax_ = value;
}

void Report::setFactionMagesCurrent(int value)
{
  factionMagesCurrent_ = value;
}

void Report::setFactionMagesMax(int value)
{
  factionMagesMax_ = value;
}

void Report::setFactionApprenticesCurrent(int value)
{
  factionApprenticesCurrent_ = value;
}

void Report::setFactionApprenticesMax(int value)
{
  factionApprenticesMax_ = value;
}

void Report::setFactionUnclaimedSilver(int value)
{
  factionUnclaimedSilver_ = value;
}

int Report::getFoundRegions() const
{
  return foundRegions_;
}

int Report::getVisitedRegions() const
{
  return visitedRegions_;
}

void Report::clear()
{
  content_.clear();
  lines_.clear();
  filePath_.clear();
  lastError_.clear();
  month_ = 0;
  year_ = 0;
  factionNumber_ = 0;
  factionName_.clear();
  faction_ = nullptr;
  factionTaxedOrTradedRegionsCurrent_ = 0;
  factionTaxedOrTradedRegionsMax_ = 0;
  factionQuartermastersCurrent_ = 0;
  factionQuartermastersMax_ = 0;
  factionMagesCurrent_ = 0;
  factionMagesMax_ = 0;
  factionApprenticesCurrent_ = 0;
  factionApprenticesMax_ = 0;
  factionUnclaimedSilver_ = 0;
  factionDefaultAttitudeText_ = L"Neutral";
  factionDeclaredAttitudesText_.clear();
  factionDeclaredFactionNames_.clear();
  foundRegions_ = 0;
  visitedRegions_ = 0;
}

void Report::splitIntoLines()
{
  lines_.clear();
  std::wistringstream stream(content_);
  std::wstring line;

  while (std::getline(stream, line))
  {
    // Strip trailing carriage return so Windows line endings don't affect parsing
    if (!line.empty() && line.back() == L'\r')
    {
      line.pop_back();
    }
    lines_.push_back(line);
  }
}

void Report::parseAtlantisHeader()
{
  static const std::wstring kMarker = L"Atlantis Report For:";
  static const std::wstring kFactionStatusMarker = L"Faction Status:";
  static const std::wregex factionStatusEntryPattern(LR"(^([A-Za-z ]+):\s*(\d+)\s*\((\d+)\)\s*$)");
  static const std::wregex declaredAttitudesHeaderPattern(
    LR"(^\s*Declared Attitudes\s*\(default\s+(Hostile|Unfriendly|Neutral|Friendly|Ally)\):\s*$)"
  );
  static const std::wregex declaredAttitudeLinePattern(
    LR"(^\s*(Hostile|Unfriendly|Neutral|Friendly|Ally)\s*:\s*(.*)\s*$)"
  );
  static const std::wregex trailingPeriodPattern(LR"(\.+\s*$)");
  static const std::wregex factionIdPattern(LR"(\((\d+)\)\s*$)");
  static const std::wregex unclaimedSilverPattern(LR"(^\s*Unclaimed silver:\s*(\d+)\.\s*$)");

  for (std::size_t i = 0; i + 2 < lines_.size(); ++i)
  {
    if (lines_[i] != kMarker)
    {
      continue;
    }

    // --- Line i+1: "<Name> (<number>) [optional extra text]" ---
    const std::wstring& factionLine = lines_[i + 1];
    const auto firstOpen = factionLine.find(L'(');
    const auto firstClose = factionLine.find(L')', firstOpen);
    if (firstOpen == std::wstring::npos || firstClose == std::wstring::npos)
    {
      break;
    }

    // Faction name: text before the first '(', trailing spaces stripped
    std::wstring name = factionLine.substr(0, firstOpen);
    while (!name.empty() && name.back() == L' ')
    {
      name.pop_back();
    }

    // Faction number: digits between first '(' and ')'
    std::wstring numberStr = factionLine.substr(firstOpen + 1, firstClose - firstOpen - 1);
    bool allDigits = !numberStr.empty() &&
      std::all_of(numberStr.begin(), numberStr.end(), ::iswdigit);
    if (!allDigits)
    {
      break;
    }
    const int factionNum = std::stoi(numberStr);

    // --- Line i+2: "<Month>, Year <number>" ---
    const std::wstring& dateLine = lines_[i + 2];
    const std::wstring kYearPrefix = L", Year ";
    const auto yearPos = dateLine.find(kYearPrefix);
    if (yearPos == std::wstring::npos)
    {
      break;
    }

    const std::wstring monthStr = dateLine.substr(0, yearPos);
    const std::wstring yearStr  = dateLine.substr(yearPos + kYearPrefix.size());

    int parsedMonth = 0;
    if (!MonthUtils::tryParseMonthName(monthStr, parsedMonth))
    {
      break;
    }

    bool yearAllDigits = !yearStr.empty() &&
      std::all_of(yearStr.begin(), yearStr.end(), ::iswdigit);
    if (!yearAllDigits)
    {
      break;
    }

    factionNumber_ = factionNum;
    factionName_   = name;
    month_         = parsedMonth;
    year_          = std::stoi(yearStr);
    break;
  }

  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    if (lines_[index] != kFactionStatusMarker)
    {
      continue;
    }

    for (std::size_t statusIndex = index + 1; statusIndex < lines_.size(); ++statusIndex)
    {
      const std::wstring& line = lines_[statusIndex];
      if (line.find_first_not_of(L" \t") == std::wstring::npos)
      {
        break;
      }

      std::wsmatch statusMatch;
      if (!std::regex_match(line, statusMatch, factionStatusEntryPattern))
      {
        break;
      }

      const std::wstring label = statusMatch[1].str();
      const int currentValue = std::stoi(statusMatch[2].str());
      const int maxValue = std::stoi(statusMatch[3].str());

      if (label == L"Regions")
      {
        factionTaxedOrTradedRegionsCurrent_ = currentValue;
        factionTaxedOrTradedRegionsMax_ = maxValue;
      }
      else if (label == L"Quartermasters")
      {
        factionQuartermastersCurrent_ = currentValue;
        factionQuartermastersMax_ = maxValue;
      }
      else if (label == L"Mages")
      {
        factionMagesCurrent_ = currentValue;
        factionMagesMax_ = maxValue;
      }
      else if (label == L"Apprentices")
      {
        factionApprenticesCurrent_ = currentValue;
        factionApprenticesMax_ = maxValue;
      }
    }

    break;
  }

  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    std::wsmatch headerMatch;
    if (!std::regex_match(lines_[index], headerMatch, declaredAttitudesHeaderPattern))
    {
      continue;
    }

    factionDefaultAttitudeText_ = headerMatch[1].str();
    factionDeclaredAttitudesText_.clear();
    factionDeclaredFactionNames_.clear();

    for (std::size_t lineIndex = index + 1; lineIndex < lines_.size(); ++lineIndex)
    {
      std::wstring line = StringUtils::trimWhitespace(lines_[lineIndex]);
      if (line.empty())
      {
        break;
      }

      std::wsmatch attitudeLineMatch;
      if (!std::regex_match(line, attitudeLineMatch, declaredAttitudeLinePattern))
      {
        break;
      }

      std::wstring attitudeText = attitudeLineMatch[1].str();
      std::wstring listText = StringUtils::trimWhitespace(attitudeLineMatch[2].str());
      listText = std::regex_replace(listText, trailingPeriodPattern, L"");
      listText = StringUtils::trimWhitespace(listText);

      if (listText.empty() || listText == L"none")
      {
        continue;
      }

      std::wstringstream listStream(listText);
      std::wstring entry;
      while (std::getline(listStream, entry, L','))
      {
        entry = StringUtils::trimWhitespace(entry);
        if (entry.empty())
        {
          continue;
        }

        std::wsmatch idMatch;
        if (!std::regex_search(entry, idMatch, factionIdPattern))
        {
          continue;
        }

        const int targetFaction = std::stoi(idMatch[1].str());
        factionDeclaredAttitudesText_[targetFaction] = attitudeText;

        const std::size_t idTokenPos = entry.rfind(L'(');
        if (idTokenPos != std::wstring::npos)
        {
          const std::wstring targetFactionName = StringUtils::trimWhitespace(entry.substr(0, idTokenPos));
          if (!targetFactionName.empty())
          {
            factionDeclaredFactionNames_[targetFaction] = targetFactionName;
          }
        }
      }
    }

    break;
  }

  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    std::wsmatch unclaimedMatch;
    if (!std::regex_match(lines_[index], unclaimedMatch, unclaimedSilverPattern))
    {
      continue;
    }

    factionUnclaimedSilver_ = std::stoi(unclaimedMatch[1].str());
    break;
  }
}

void Report::parseRegions(RegionRepository& regionRepository,
                          UnitRepository& unitRepository,
                          StructureRepository& structureRepository,
                          ItemRepository& itemRepository,
                          FactionRepository& factionRepository,
                          int shipStructureIdThreshold,
                          const std::vector<std::wstring>& flyingShipTypeTokens)
{
  (void)shipStructureIdThreshold;
  (void)flyingShipTypeTokens;

  foundRegions_ = 0;
  visitedRegions_ = 0;

  // Pattern to match region lines:
  // "<type> (<x>,<y>) in <province>, [contains <settlement> [<bracket_info>], ]<peasants> peasants (<peasant_type>), $<amount>."
  // "<type> (<x>,<y>) in <province>, [contains <settlement> [<bracket_info>], ]$<amount>."
  // "<type> (<x>,<y>) in <province>." (income omitted on header line)
  // 
  // Examples:
  //   plain (54,22) in Baquaudblir, 2669 peasants (humans), $1601.
  //   ocean (49,47) in Atlantis Ocean, $2025.
  //   underforest (31,17,underworld) in Hesscot, contains Tanost [town], $1234.
  
  // Regex pattern explanation:
  // ^(\w+)\s+                                  - Region type (plain, ocean, etc)
  // \((\d+),(\d+)(?:,([^)]+))?\)              - Coordinates with optional third component token
  // \s+in\s+                                   - "in" separator
  // ([^,]+?)                                    - Province name (stops at comma)
  // ,\s*                                       - Comma and optional spaces
  // (?:contains\s+(.+?)\s+\[[^\]]+\],\s*)?  - Optional settlement name before bracketed kind
  // (?:(\d+)\s+peasants\s+\(([^)]+)\),\s*)? - Optional peasant number and type
  // (?:,\s*\$(\d+))?                          - Optional dollar amount
  // \.?                                         - Optional period

  std::wregex regionPattern(
    L"^(\\w+)\\s+\\((\\d+),(\\d+)(?:,([^)]+))?\\)\\s+in\\s+([^,.]+?)(?:,\\s*(?:contains\\s+(.+?)\\s+\\[([^\\]]+)\\],\\s*)?(?:(\\d+)\\s+peasants\\s+\\(([^)]+)\\))?(?:,\\s*\\$(\\d+))?|(?:,\\s*\\$(\\d+))?)?\\.?$"
  );
 
  std::wregex regionHeaderStartPattern(
    L"^\\w+\\s+\\(\\d+,\\d+(?:,[^)]+)?\\)\\s+in\\s+[^,.]+[,.].*$"
  );

  /*
  std::wregex regionHeaderStartPattern(
    L"^\\w+\\s+\\(\\d+,\\d+(?:,[^)]+)?\\)\\s+in\\s+[^,]+,.*$"
  );
  */

  std::wregex exitsHeaderPattern(L"^\\s*Exits:\\s*$");
  std::wregex exitDirectionPrefixPattern(L"^\\s*[A-Za-z]+\\s*:\\s*.*$");
  std::wregex exitRegionPattern(
    L"^\\s*([A-Za-z]+)\\s*:\\s*(\\w+)\\s+\\((\\d+),(\\d+)(?:,([^)]+))?\\)\\s+in\\s+([^,\\.]+?)(?:,\\s*contains\\s+(.+?)\\s+\\[([^\\]]+)\\])?\\s*\\.\\s*$"
  );

  std::wregex wagesPattern(L"^\\s*Wages:\\s*\\$(\\d+(?:\\.\\d+)?)\\s*\\(Max:\\s*\\$(\\d+)\\)\\.\\s*$");
  std::wregex entertainmentPattern(L"^\\s*Entertainment available:\\s*\\$(\\d+)\\.\\s*$");
  std::wregex productEntryPattern(L"(\\d+)\\s+[^,\\[]+\\s+\\[([^\\]]+)\\]");
  std::wregex wantedForSaleEntryPattern(L"(\\d+)\\s+[^,\\[]+\\s+\\[([^\\]]+)\\]\\s+at\\s+\\$(\\d+)");

  std::wregex unitEntryStartPattern(L"^\\s*[-*]\\s+(.+)$");
  std::wregex unitHeaderPattern(L"^\\s*[-*]\\s*(.+?)\\s*\\((\\d+)\\)\\s*,\\s*(.*)$");
  std::wregex leadingFactionPattern(L"^(.+?)\\s*\\((\\d+)\\)\\s*,\\s*(.*)$");
  std::wregex skillTokenPattern(L"([^,]+?\\[[^\\]]+\\]\\s+\\d+\\s*\\(\\d+\\))");
  std::wregex weightPattern(L"Weight:\\s*(\\d+)\\s*\\.");
  std::wregex capacityPattern(L"Capacity:\\s*(\\d+)\\s*/\\s*(\\d+)\\s*/\\s*(\\d+)\\s*/\\s*(\\d+)\\s*\\.");
  std::wregex structureEntryStartPattern(L"^\\s*\\+\\s+(.+)$");

  auto trim = [](std::wstring value)
  {
    if (!value.empty() && value.front() == 0xFEFF)
    {
      value.erase(value.begin());
    }

    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  auto parseFleetItems = [&](const std::wstring& fleetList)
  {
    std::map<std::wstring, int> parsedFleetItems;
    std::wstring remaining = fleetList;

    while (!remaining.empty())
    {
      const std::size_t commaPos = remaining.find(L',');
      std::wstring entry = trim(remaining.substr(0, commaPos));
      if (!entry.empty() && entry.back() == L'.')
      {
        entry.pop_back();
      }
      entry = trim(entry);

      std::size_t amountDigits = 0;
      while (amountDigits < entry.size() && iswdigit(entry[amountDigits]))
      {
        ++amountDigits;
      }

      if (amountDigits > 0)
      {
        int amount = 0;
        try
        {
          amount = std::stoi(entry.substr(0, amountDigits));
        }
        catch (const std::exception&)
        {
          amount = 0;
        }

        if (amount > 0)
        {
          std::wstring shipType = trim(entry.substr(amountDigits));
          if (!shipType.empty())
          {
            const Item* item = itemRepository.findByItemName(shipType);
            if (item == nullptr && !shipType.empty() && shipType.back() == L's')
            {
              shipType.pop_back();
              item = itemRepository.findByItemName(trim(shipType));
            }
            if (item != nullptr)
            {
              parsedFleetItems[item->getIdentifierToken()] += amount;
            }
          }
        }
      }

      if (commaPos == std::wstring::npos)
      {
        break;
      }

      remaining = trim(remaining.substr(commaPos + 1));
    }

    return parsedFleetItems;
  };

  auto splitByComma = [&trim](const std::wstring& text)
  {
    std::vector<std::wstring> parts;
    std::wstring current;
    for (wchar_t ch : text)
    {
      if (ch == L',')
      {
        const std::wstring part = trim(current);
        if (!part.empty())
        {
          parts.push_back(part);
        }
        current.clear();
      }
      else
      {
        current.push_back(ch);
      }
    }

    const std::wstring tail = trim(current);
    if (!tail.empty())
    {
      parts.push_back(tail);
    }

    return parts;
  };

  auto startsWithIgnoreCase = [](const std::wstring& text, const std::wstring& prefix)
  {
    if (text.size() < prefix.size())
    {
      return false;
    }

    for (std::size_t i = 0; i < prefix.size(); ++i)
    {
      if (towlower(text[i]) != towlower(prefix[i]))
      {
        return false;
      }
    }

    return true;
  };

  auto parseZCoordinate = [](const std::wstring& zTokenRaw)
  {
    if (zTokenRaw.empty())
    {
      return 1;
    }

    std::wstring zToken = zTokenRaw;
    std::transform(zToken.begin(), zToken.end(), zToken.begin(), towlower);
    if (zToken == L"underworld")
    {
      return 2;
    }
    if (zToken == L"nexus")
    {
      return 0;
    }

    return 1;
  };

  bool inBattleSection = false;

  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    const std::wstring topLevelLine = trim(lines_[index]);
    if (startsWithIgnoreCase(topLevelLine, L"Battles during turn:") ||
        startsWithIgnoreCase(topLevelLine, L"Battle reports:") ||
        startsWithIgnoreCase(topLevelLine, L"Battle statistics:"))
    {
      inBattleSection = true;
      continue;
    }

    if (inBattleSection)
    {
      // Resume parsing once the next top-level region header is reached.
      std::wsmatch resumedRegionMatch;
      if (std::regex_match(topLevelLine, resumedRegionMatch, regionHeaderStartPattern))
      {
        inBattleSection = false;
      }
      else
      {
        continue;
      }
    }

    std::wstring line = lines_[index];
    std::size_t continuationLineCount = 0;

    while (line.find(L'$') == std::wstring::npos &&
          index + continuationLineCount + 1 < lines_.size())
    {
      const std::wstring& continuationLine = lines_[index + continuationLineCount + 1];
      if (continuationLine.empty() || continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
      {
        break;
      }

      const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
      if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
      {
        break;
      }

      if (continuationLine[firstNonWhitespace] == L'-')
      {
        break;
      }

      line += L" ";
      line += continuationLine.substr(firstNonWhitespace);
      ++continuationLineCount;
    }

    std::wsmatch match;
    if (std::regex_match(line, match, regionPattern))
    {
      // match[0] = full line
      // match[1] = region type
      // match[2] = x coordinate
      // match[3] = y coordinate
      // match[4] = optional third coordinate token
      // match[5] = province name
      // match[6] = settlement name (optional, may be empty)
      // match[7] = settlement type (optional, may be empty)
      // match[8] = peasant number (optional, may be empty)
      // match[9] = peasant type (optional, may be empty)
      // match[10] = dollar amount (with contains/peasants clause)
      // match[11] = dollar amount (without contains/peasants clause)

      try
      {
        const int xCoord = std::stoi(match[2].str());
        const int yCoord = std::stoi(match[3].str());
        const int zCoord = parseZCoordinate(match[4].matched ? match[4].str() : L"");
        const std::wstring regionType = match[1].str();
        const std::wstring provinceName = match[5].str();
        const bool containsSettlement = match[6].matched && !match[6].str().empty();
        const std::wstring settlementName = containsSettlement ? match[6].str() : L"";
        const std::wstring settlementType = containsSettlement && match[7].matched ? match[7].str() : L"";
        double wages = 0.0;
        int wagesMax = 0;
        int taxableIncome = 0;
        const std::wstring taxableIncomeText =
          match[10].matched ? match[10].str() : (match[11].matched ? match[11].str() : L"");
        if (!taxableIncomeText.empty())
        {
          taxableIncome = std::stoi(taxableIncomeText);
        }
        
        int peasantNumber = 0;
        if (match[8].matched && !match[8].str().empty())
        {
          peasantNumber = std::stoi(match[8].str());
        }

        const std::wstring peasantType = match[9].matched ? match[9].str() : L"unknown";

        const std::size_t wagesLineIndex = index + continuationLineCount + 2;
        if (wagesLineIndex < lines_.size())
        {
          std::wsmatch wagesMatch;
          if (std::regex_match(lines_[wagesLineIndex], wagesMatch, wagesPattern))
          {
            wages = std::stod(wagesMatch[1].str());
            wagesMax = std::stoi(wagesMatch[2].str());
          }
        }
        
        // Count each successfully parsed main-region line as visited for this report.
        visitedRegions_++;

        const bool regionAlreadyExists =
          regionRepository.findByCoordinates(xCoord, yCoord, zCoord) != nullptr;

        if (regionRepository.upsertRegion(
              xCoord,
              yCoord,
            zCoord,
              regionType,
              provinceName,
              containsSettlement,
              settlementName,
              settlementType,
              peasantType,
              peasantNumber,
              wages,
              wagesMax,
              taxableIncome,
              month_,
              year_,
              true))
        {
          if (!regionAlreadyExists)
          {
            foundRegions_++;
          }
        }

        Region* parsedRegion = regionRepository.findByCoordinates(xCoord, yCoord, zCoord);
        if (parsedRegion != nullptr)
        {
          parsedRegion->clearExitDirections();
          parsedRegion->setResources({});
        }

        const std::size_t regionBodyStart = index + continuationLineCount + 1;
        std::size_t exitsHeaderIndex = lines_.size();
        for (std::size_t scanIndex = regionBodyStart; scanIndex < lines_.size(); ++scanIndex)
        {
          if (scanIndex > regionBodyStart)
          {
            std::wsmatch nextRegionMatch;
            if (std::regex_match(lines_[scanIndex], nextRegionMatch, regionHeaderStartPattern))
            {
              break;
            }
          }

          std::wsmatch exitsHeaderMatch;
          if (std::regex_match(lines_[scanIndex], exitsHeaderMatch, exitsHeaderPattern))
          {
            exitsHeaderIndex = scanIndex;
            break;
          }

          std::wsmatch entertainmentMatch;
          if (parsedRegion != nullptr &&
              std::regex_match(lines_[scanIndex], entertainmentMatch, entertainmentPattern))
          {
            try
            {
              parsedRegion->setEntertainment(std::stoi(entertainmentMatch[1].str()));
            }
            catch (const std::exception&) {}
          }

          const std::wstring trimmedScanLine = trim(lines_[scanIndex]);
          if (parsedRegion != nullptr && trimmedScanLine.rfind(L"Products:", 0) == 0)
          {
            std::wstring productsText = trim(trimmedScanLine.substr(9));
            std::size_t consumedProductLines = 0;

            while (scanIndex + consumedProductLines + 1 < lines_.size() &&
                   productsText.find(L'.') == std::wstring::npos)
            {
              const std::wstring& continuationLine = lines_[scanIndex + consumedProductLines + 1];
              if (continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
              {
                break;
              }

              std::wsmatch stopAtRegion;
              if (std::regex_match(continuationLine, stopAtRegion, regionHeaderStartPattern))
              {
                break;
              }

              const std::wstring trimmedContinuation = trim(continuationLine);
              if (trimmedContinuation.rfind(L"Exits:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Wages:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Wanted:", 0) == 0 ||
                  trimmedContinuation.rfind(L"For Sale:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Products:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Entertainment available:", 0) == 0)
              {
                break;
              }

              const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
              if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
              {
                break;
              }

              productsText += L" ";
              productsText += trimmedContinuation;
              ++consumedProductLines;
            }

            std::map<std::wstring, int> regionResources;
            for (std::wsregex_iterator it(productsText.begin(), productsText.end(), productEntryPattern),
                 end; it != end; ++it)
            {
              int amount = 0;
              const std::wstring amountText = (*it)[1].str();
              const std::wstring token = trim((*it)[2].str());
              try
              {
                amount = std::stoi(amountText);
              }
              catch (const std::exception&)
              {
                amount = 0;
              }

              if (!token.empty() && amount > 0)
              {
                regionResources[token] += amount;
              }
            }

            parsedRegion->setResources(std::move(regionResources));
            scanIndex += consumedProductLines;
          }

          if (parsedRegion != nullptr && trimmedScanLine.rfind(L"Wanted:", 0) == 0)
          {
            std::wstring wantedText = trim(trimmedScanLine.substr(7));
            
            // Skip "none." entries
            if (wantedText != L"none.")
            {
              std::size_t consumedWantedLines = 0;

              while (scanIndex + consumedWantedLines + 1 < lines_.size() &&
                     wantedText.find(L'.') == std::wstring::npos)
              {
                const std::wstring& continuationLine = lines_[scanIndex + consumedWantedLines + 1];
                if (continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
                {
                  break;
                }

                std::wsmatch stopAtRegion;
                if (std::regex_match(continuationLine, stopAtRegion, regionHeaderStartPattern))
                {
                  break;
                }

                const std::wstring trimmedContinuation = trim(continuationLine);
                if (trimmedContinuation.rfind(L"Exits:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Wages:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Wanted:", 0) == 0 ||
                    trimmedContinuation.rfind(L"For Sale:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Products:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Entertainment available:", 0) == 0)
                {
                  break;
                }

                const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
                if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
                {
                  break;
                }

                wantedText += L" ";
                wantedText += trimmedContinuation;
                ++consumedWantedLines;
              }

              std::map<std::wstring, std::pair<int, int>> regionWanted;
              for (std::wsregex_iterator it(wantedText.begin(), wantedText.end(), wantedForSaleEntryPattern),
                   end; it != end; ++it)
              {
                int amount = 0;
                const std::wstring amountText = (*it)[1].str();
                const std::wstring token = trim((*it)[2].str());
                int price = 0;
                const std::wstring priceText = (*it)[3].str();
                try
                {
                  amount = std::stoi(amountText);
                  price = std::stoi(priceText);
                }
                catch (const std::exception&) {}

                if (!token.empty() && amount > 0)
                {
                  regionWanted[token] = {amount, price};
                }
              }

              parsedRegion->setWanted(std::move(regionWanted));
              scanIndex += consumedWantedLines;
            }
          }

          if (parsedRegion != nullptr && trimmedScanLine.rfind(L"For Sale:", 0) == 0)
          {
            std::wstring forSaleText = trim(trimmedScanLine.substr(9));
            
            // Skip "none." entries
            if (forSaleText != L"none.")
            {
              std::size_t consumedForSaleLines = 0;

              while (scanIndex + consumedForSaleLines + 1 < lines_.size() &&
                     forSaleText.find(L'.') == std::wstring::npos)
              {
                const std::wstring& continuationLine = lines_[scanIndex + consumedForSaleLines + 1];
                if (continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
                {
                  break;
                }

                std::wsmatch stopAtRegion;
                if (std::regex_match(continuationLine, stopAtRegion, regionHeaderStartPattern))
                {
                  break;
                }

                const std::wstring trimmedContinuation = trim(continuationLine);
                if (trimmedContinuation.rfind(L"Exits:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Wages:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Wanted:", 0) == 0 ||
                    trimmedContinuation.rfind(L"For Sale:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Products:", 0) == 0 ||
                    trimmedContinuation.rfind(L"Entertainment available:", 0) == 0)
                {
                  break;
                }

                const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
                if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
                {
                  break;
                }

                forSaleText += L" ";
                forSaleText += trimmedContinuation;
                ++consumedForSaleLines;
              }

              std::map<std::wstring, std::pair<int, int>> regionForSale;
              for (std::wsregex_iterator it(forSaleText.begin(), forSaleText.end(), wantedForSaleEntryPattern),
                   end; it != end; ++it)
              {
                int amount = 0;
                const std::wstring amountText = (*it)[1].str();
                const std::wstring token = trim((*it)[2].str());
                int price = 0;
                const std::wstring priceText = (*it)[3].str();
                try
                {
                  amount = std::stoi(amountText);
                  price = std::stoi(priceText);
                }
                catch (const std::exception&) {}

                if (!token.empty() && amount > 0)
                {
                  regionForSale[token] = {amount, price};
                }
              }

              parsedRegion->setForSale(std::move(regionForSale));
              scanIndex += consumedForSaleLines;
            }
          }
        }

        if (exitsHeaderIndex < lines_.size())
        {
          int exitsParsed = 0;
          for (std::size_t exitIndex = exitsHeaderIndex + 1;
              exitIndex < lines_.size() && exitsParsed < 6;
              ++exitIndex)
          {
            const std::wstring& rawExitLine = lines_[exitIndex];

            std::wsmatch nextRegionMatch;
            if (std::regex_match(rawExitLine, nextRegionMatch, regionHeaderStartPattern))
            {
              break;
            }

            if (rawExitLine.find_first_not_of(L" \t") == std::wstring::npos)
            {
              if (exitsParsed > 0)
              {
                break;
              }
              continue;
            }

            std::wstring parsedExitLine = rawExitLine;
            std::size_t consumedContinuationLines = 0;

            // Some reports wrap long exit lines. Join indented continuation lines
            // until this entry can be parsed or we hit another section/entry.
            while (exitIndex + consumedContinuationLines + 1 < lines_.size())
            {
              const std::wstring& continuationLine = lines_[exitIndex + consumedContinuationLines + 1];
              if (continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
              {
                break;
              }

              std::wsmatch stopAtRegion;
              if (std::regex_match(continuationLine, stopAtRegion, regionHeaderStartPattern))
              {
                break;
              }

              std::wsmatch nextExitPrefixMatch;
              if (std::regex_match(continuationLine, nextExitPrefixMatch, exitDirectionPrefixPattern))
              {
                break;
              }

              const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
              if (firstNonWhitespace == std::wstring::npos)
              {
                break;
              }

              const wchar_t firstToken = continuationLine[firstNonWhitespace];
              if (firstToken == L'+' || firstToken == L'*' || firstToken == L'-')
              {
                break;
              }

              parsedExitLine += L" ";
              parsedExitLine += continuationLine.substr(firstNonWhitespace);
              ++consumedContinuationLines;

              std::wsmatch completeExitMatch;
              if (std::regex_match(parsedExitLine, completeExitMatch, exitRegionPattern))
              {
                break;
              }
            }

            std::wsmatch exitMatch;
            if (!std::regex_match(parsedExitLine, exitMatch, exitRegionPattern))
            {
              if (exitsParsed > 0)
              {
                break;
              }
              continue;
            }

            try
            {
              if (parsedRegion != nullptr)
              {
                parsedRegion->addExitDirection(exitMatch[1].str());
              }

              const int exitX = std::stoi(exitMatch[3].str());
              const int exitY = std::stoi(exitMatch[4].str());
              const int exitZ = parseZCoordinate(exitMatch[5].matched ? exitMatch[5].str() : L"");

              // Extract settlement information if present
              // exitMatch[7] = settlement name (optional)
              // exitMatch[8] = settlement type (optional)
              const bool exitHasSettlement = exitMatch[7].matched && !exitMatch[7].str().empty();
              const std::wstring exitSettlementName = exitHasSettlement ? exitMatch[7].str() : L"";
              const std::wstring exitSettlementType = exitHasSettlement && exitMatch[8].matched ? exitMatch[8].str() : L"";

              if (regionRepository.findByCoordinates(exitX, exitY, exitZ) == nullptr)
              {
                if (regionRepository.add(
                  exitX,
                  exitY,
                  exitZ,
                  exitMatch[2].str(),
                  exitMatch[6].str(),
                  exitHasSettlement,
                  exitSettlementName,
                  exitSettlementType,
                  L"unknown",
                  0,
                  0.0,
                  0,
                  0,
                  false
                ))
                {
                  foundRegions_++;
                }
              }

              ++exitsParsed;
              exitIndex += consumedContinuationLines;
            }
            catch (const std::exception&)
            {
              continue;
            }
          }

          int currentStructureId = 0;
          bool currentStructureOwnerAssigned = false;
          for (std::size_t bodyIndex = exitsHeaderIndex + 1; bodyIndex < lines_.size(); ++bodyIndex)
          {
            std::wsmatch nextRegionMatch;
            if (std::regex_match(lines_[bodyIndex], nextRegionMatch, regionHeaderStartPattern))
            {
              break;
            }

            std::wsmatch structureStartMatch;
            if (std::regex_match(lines_[bodyIndex], structureStartMatch, structureEntryStartPattern))
            {
              try
              {
                std::wstring structureText = trim(lines_[bodyIndex]);
                if (structureText.empty() || structureText[0] != L'+')
                {
                  continue;
                }

                structureText = trim(structureText.substr(1));

                const std::size_t idOpenPos = structureText.find(L'[');
                const std::size_t idClosePos = structureText.find(L']', idOpenPos == std::wstring::npos ? 0 : idOpenPos + 1);
                const std::size_t colonPos = structureText.find(L':', idClosePos == std::wstring::npos ? 0 : idClosePos + 1);
                if (idOpenPos == std::wstring::npos ||
                    idClosePos == std::wstring::npos ||
                    colonPos == std::wstring::npos)
                {
                  continue;
                }

                const std::wstring structureName = trim(structureText.substr(0, idOpenPos));
                const int structureId = std::stoi(trim(structureText.substr(idOpenPos + 1, idClosePos - idOpenPos - 1)));

                const std::wstring afterColon = trim(structureText.substr(colonPos + 1));
                const std::size_t commaPos = afterColon.find(L',');
                const std::size_t periodPos = afterColon.find(L'.');

                std::size_t nameEndPos = std::wstring::npos;
                if (commaPos != std::wstring::npos && periodPos != std::wstring::npos)
                {
                  nameEndPos = std::min(commaPos, periodPos);
                }
                else if (commaPos != std::wstring::npos)
                {
                  nameEndPos = commaPos;
                }
                else
                {
                  nameEndPos = periodPos;
                }

                if (nameEndPos == std::wstring::npos)
                {
                  continue;
                }

                const wchar_t nameTerminator = afterColon[nameEndPos];
                const std::wstring structureType = trim(afterColon.substr(0, nameEndPos));
                std::wstring lowerStructureType = structureType;
                std::transform(lowerStructureType.begin(), lowerStructureType.end(), lowerStructureType.begin(), towlower);
                std::wstring suffix = trim(afterColon.substr(nameEndPos + 1));
                std::wstring lowerSuffix = suffix;
                std::transform(lowerSuffix.begin(), lowerSuffix.end(), lowerSuffix.begin(), towlower);

                const bool isClosed = (nameTerminator == L',') &&
                                      (lowerSuffix.find(L"closed to player units") != std::wstring::npos);

                structureRepository.addOrUpdateIfLater(structureId,
                                                      xCoord,
                                                      yCoord,
                                                      zCoord,
                                                      structureType,
                                                      structureName,
                                                      isClosed,
                                                      month_,
                                                      year_);

                if (Structure* parsedStructure =
                      structureRepository.findByIdAndCoordinates(structureId, xCoord, yCoord, zCoord))
                {
                  if (lowerStructureType == L"fleet")
                  {
                    parsedStructure->setFleetItems(parseFleetItems(suffix));
                  }

                  if (parsedStructure->getYear() == year_ && parsedStructure->getMonth() == month_)
                  {
                    parsedStructure->setOwnerUnitId(0);
                  }
                }

                currentStructureId = structureId;
                currentStructureOwnerAssigned = false;
              }
              catch (const std::exception&)
              {
                continue;
              }

              continue;
            }

            std::wsmatch unitStartMatch;
            if (!std::regex_match(lines_[bodyIndex], unitStartMatch, unitEntryStartPattern))
            {
              continue;
            }

            std::wstring unitEntryText = trim(lines_[bodyIndex]);
            std::size_t consumedLines = 0;

            while (bodyIndex + consumedLines + 1 < lines_.size())
            {
              const std::wstring& continuationLine = lines_[bodyIndex + consumedLines + 1];

              if (continuationLine.empty() ||
                  continuationLine.find_first_not_of(L" \t") == std::wstring::npos)
              {
                break;
              }

              std::wsmatch stopAtRegion;
              if (std::regex_match(continuationLine, stopAtRegion, regionHeaderStartPattern))
              {
                break;
              }

              std::wsmatch stopAtNextUnit;
              if (std::regex_match(continuationLine, stopAtNextUnit, unitEntryStartPattern))
              {
                break;
              }

              const std::size_t firstNonWhitespace = continuationLine.find_first_not_of(L" \t");
              if (firstNonWhitespace == std::wstring::npos)
              {
                break;
              }

              const std::wstring trimmedContinuation = trim(continuationLine);
              if (trimmedContinuation.rfind(L"Exits:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Wages:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Wanted:", 0) == 0 ||
                  trimmedContinuation.rfind(L"For Sale:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Products:", 0) == 0 ||
                  trimmedContinuation.rfind(L"Entertainment available:", 0) == 0 ||
                  (!trimmedContinuation.empty() && trimmedContinuation[0] == L'+'))
              {
                break;
              }

              unitEntryText += L" ";
              unitEntryText += trimmedContinuation;
              ++consumedLines;
            }

            bodyIndex += consumedLines;

            std::wsmatch unitHeaderMatch;
            if (!std::regex_match(unitEntryText, unitHeaderMatch, unitHeaderPattern))
            {
              continue;
            }

            try
            {
              const std::wstring unitName = trim(unitHeaderMatch[1].str());
              const int unitNumber = std::stoi(unitHeaderMatch[2].str());
              const int parsedStructureId = currentStructureId;
              std::wstring remainder = trim(unitHeaderMatch[3].str());

              // Check for "on guard" flag
              bool onGuard = false;
              std::wregex onGuardPattern(L"^on guard\\s*,\\s*(.*)$");
              std::wsmatch onGuardMatch;
              if (std::regex_match(remainder, onGuardMatch, onGuardPattern))
              {
                onGuard = true;
                remainder = trim(onGuardMatch[1].str());
              }

              int parsedFactionNumber = 0;
              std::wstring parsedFactionName;
              std::wsmatch parsedFactionMatch;
              if (std::regex_match(remainder, parsedFactionMatch, leadingFactionPattern))
              {
                parsedFactionName = trim(parsedFactionMatch[1].str());
                parsedFactionNumber = std::stoi(parsedFactionMatch[2].str());
                remainder = trim(parsedFactionMatch[3].str());
              }

              // Validate/create faction in repository
              if (parsedFactionNumber > 0)
              {
                if (!factionRepository.findByNumber(parsedFactionNumber))
                {
                  // Faction doesn't exist, create it with the parsed name
                  factionRepository.add(parsedFactionNumber, parsedFactionName, false);
                }
              }

              int parsedWeight = 0;
              std::wsmatch weightMatch;
              if (std::regex_search(unitEntryText, weightMatch, weightPattern))
              {
                parsedWeight = std::stoi(weightMatch[1].str());
              }

              int capacityWalk = 0;
              int capacityRide = 0;
              int capacityFly = 0;
              int capacitySwim = 0;
              std::wsmatch capacityMatch;
              if (std::regex_search(unitEntryText, capacityMatch, capacityPattern))
              {
                capacityWalk = std::stoi(capacityMatch[1].str());
                capacityRide = std::stoi(capacityMatch[2].str());
                capacityFly = std::stoi(capacityMatch[3].str());
                capacitySwim = std::stoi(capacityMatch[4].str());
              }

              std::wstring flagsSource = remainder;
              const std::size_t weightPos = flagsSource.find(L"Weight:");
              if (weightPos != std::wstring::npos)
              {
                flagsSource = flagsSource.substr(0, weightPos);
              }

              std::vector<std::wstring> parsedFlags;
              std::map<std::wstring, int> parsedItemCounts;
              std::map<std::wstring, int> parsedSkills;
              std::vector<std::wstring> parsedCanStudySkillTokens;

              for (const std::wstring& token : splitByComma(flagsSource))
              {
                std::wstring cleaned = trim(token);
                if (!cleaned.empty() && cleaned.back() == L'.')
                {
                  cleaned.pop_back();
                  cleaned = trim(cleaned);
                }

                if (token.find(L'[') != std::wstring::npos)
                {
                  if (!cleaned.empty() && cleaned != L"none")
                  {
                    const std::size_t openBracket = cleaned.find(L'[');
                    const std::size_t closeBracket = cleaned.find(L']', openBracket == std::wstring::npos ? 0 : openBracket + 1);
                    if (openBracket != std::wstring::npos && closeBracket != std::wstring::npos && closeBracket > openBracket + 1)
                    {
                      const std::wstring itemToken = trim(cleaned.substr(openBracket + 1, closeBracket - openBracket - 1));
                      int amount = 1;

                      const std::wstring prefix = trim(cleaned.substr(0, openBracket));
                      std::size_t digitLength = 0;
                      while (digitLength < prefix.size() && iswdigit(prefix[digitLength]))
                      {
                        ++digitLength;
                      }

                      if (digitLength > 0)
                      {
                        try
                        {
                          amount = std::stoi(prefix.substr(0, digitLength));
                        }
                        catch (const std::exception&)
                        {
                          amount = 1;
                        }
                      }

                      if (!itemToken.empty() && amount > 0)
                      {
                        parsedItemCounts[itemToken] += amount;

                        // Register the item name in the repository if not already known.
                        if (!itemRepository.findByIdentifierToken(itemToken))
                        {
                          const std::wstring itemNamePart = trim(prefix.substr(digitLength));
                          itemRepository.add(itemToken, itemNamePart,
                                            0, false, false, false, false, false,
                                            0, 0, 0, 0, 0, false);
                        }
                      }
                    }
                  }
                  continue;
                }

                if (token.find(L':') != std::wstring::npos)
                {
                  continue;
                }

                if (!cleaned.empty() && cleaned != L"none")
                {
                  parsedFlags.push_back(cleaned);
                }
              }

              const std::size_t skillsPos = unitEntryText.find(L"Skills:");
              if (skillsPos != std::wstring::npos)
              {
                std::wstring skillsSource = unitEntryText.substr(skillsPos + 7);
                const std::size_t skillPeriodPos = skillsSource.find(L'.');
                if (skillPeriodPos != std::wstring::npos)
                {
                  skillsSource = skillsSource.substr(0, skillPeriodPos);
                }

                skillsSource = trim(skillsSource);
                if (skillsSource != L"none")
                {
                  // Pattern: skill_name [TOKEN] level (days)
                  std::wregex skillDetailPattern(L"\\[([^\\]]+)\\]\\s+(\\d+)\\s*\\((\\d+)\\)");
                  
                  for (std::wsregex_iterator skillIt(skillsSource.begin(), skillsSource.end(), skillTokenPattern),
                                              skillEnd;
                      skillIt != skillEnd;
                      ++skillIt)
                  {
                    const std::wstring skillValue = trim((*skillIt)[1].str());
                    if (!skillValue.empty())
                    {
                      // Parse the skill: skill_name [TOKEN] level (days)
                      std::wsmatch skillMatch;
                      if (std::regex_search(skillValue, skillMatch, skillDetailPattern))
                      {
                        std::wstring token = skillMatch[1].str();
                        // Normalize token to uppercase for consistency with skill repository
                        std::transform(token.begin(), token.end(), token.begin(),
                                      [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
                        int days = std::stoi(skillMatch[3].str());
                        
                        if (!token.empty())
                        {
                          parsedSkills[token] = days;
                        }
                      }
                    }
                  }
                }
              }

              const std::size_t canStudyPos = unitEntryText.find(L"Can Study:");
              if (canStudyPos != std::wstring::npos)
              {
                std::wstring canStudySource = unitEntryText.substr(canStudyPos + 10);
                const std::size_t canStudyPeriodPos = canStudySource.find(L'.');
                if (canStudyPeriodPos != std::wstring::npos)
                {
                  canStudySource = canStudySource.substr(0, canStudyPeriodPos);
                }

                static const std::wregex canStudyTokenPattern(L"\\[([A-Za-z0-9]{3,})\\]");
                std::set<std::wstring> seenCanStudyTokens;
                for (std::wsregex_iterator it(canStudySource.begin(), canStudySource.end(), canStudyTokenPattern), end;
                     it != end;
                     ++it)
                {
                  const std::wstring token = trim((*it)[1].str());
                  if (token.empty())
                  {
                    continue;
                  }

                  if (seenCanStudyTokens.insert(token).second)
                  {
                    parsedCanStudySkillTokens.push_back(token);
                  }
                }
              }

              unitRepository.addOrUpdateIfLater(unitNumber,
                                                unitName,
                                                parsedFactionNumber,
                                                parsedStructureId,
                                                xCoord,
                                                yCoord,
                                                zCoord,
                                                std::move(parsedFlags),
                                                std::move(parsedItemCounts),
                                                parsedWeight,
                                                capacityWalk,
                                                capacityRide,
                                                capacityFly,
                                                capacitySwim,
                                                std::move(parsedSkills),
                                                month_,
                                                year_,
                                                onGuard);

              if (Unit* parsedUnit = unitRepository.findByNumber(unitNumber))
              {
                if (parsedUnit->getYear() == year_ && parsedUnit->getMonth() == month_)
                {
                  parsedUnit->setCanStudySkillTokens(std::move(parsedCanStudySkillTokens));
                }
              }

              if (currentStructureId != 0 && !currentStructureOwnerAssigned)
              {
                if (Structure* currentStructure =
                      structureRepository.findByIdAndCoordinates(currentStructureId, xCoord, yCoord, zCoord))
                {
                  if (currentStructure->getYear() == year_ && currentStructure->getMonth() == month_)
                  {
                    currentStructure->setOwnerUnitId(unitNumber);
                    currentStructureOwnerAssigned = true;
                  }
                }
              }
            }
            catch (const std::exception&)
            {
              continue;
            }
          }
        }
      }
      catch (const std::exception&)
      {
        // Skip lines that fail to parse
        continue;
      }
    }
  }
}

void Report::parseBattles(BattleRepository& battleRepository,
                          RegionRepository& regionRepository,
                          UnitRepository& unitRepository)
{
  auto trim = [](std::wstring value)
  {
    return StringUtils::trimWhitespace(value);
  };

  auto startsWithIgnoreCase = [](const std::wstring& text, const std::wstring& prefix)
  {
    if (text.size() < prefix.size())
    {
      return false;
    }

    for (std::size_t index = 0; index < prefix.size(); ++index)
    {
      if (towlower(text[index]) != towlower(prefix[index]))
      {
        return false;
      }
    }

    return true;
  };

  auto parseZCoordinate = [](const std::wstring& zTokenRaw)
  {
    if (zTokenRaw.empty())
    {
      return 1;
    }

    std::wstring zToken = zTokenRaw;
    std::transform(zToken.begin(), zToken.end(), zToken.begin(), towlower);
    if (zToken == L"underworld")
    {
      return 2;
    }
    if (zToken == L"nexus")
    {
      return 0;
    }

    return 1;
  };

  auto splitByComma = [&trim](const std::wstring& text)
  {
    std::vector<std::wstring> parts;
    std::wstring current;

    for (wchar_t ch : text)
    {
      if (ch == L',')
      {
        std::wstring part = trim(current);
        if (!part.empty())
        {
          parts.push_back(std::move(part));
        }
        current.clear();
      }
      else
      {
        current.push_back(ch);
      }
    }

    std::wstring tail = trim(current);
    if (!tail.empty())
    {
      parts.push_back(std::move(tail));
    }

    return parts;
  };

  auto joinLines = [](const std::vector<std::wstring>& sourceLines)
  {
    std::wstring text;
    for (std::size_t index = 0; index < sourceLines.size(); ++index)
    {
      if (index != 0)
      {
        text += L"\r\n";
      }
      text += sourceLines[index];
    }
    return text;
  };

  auto ensureBattleUnit = [&unitRepository](int unitId,
                                            const std::wstring& unitName,
                                            int month,
                                            int year)
  {
    if (unitId <= 0 || unitName.empty())
    {
      return;
    }

    Unit* unit = unitRepository.findByNumber(unitId);
    if (unit)
    {
      if (unit->getUnitName().empty())
      {
        unit->setUnitName(unitName);
      }
      return;
    }

    unitRepository.add(unitId,
                       unitName,
                       0,
                       0,
                       0,
                       0,
                       1,
                       {},
                       {},
                       0,
                       0,
                       0,
                       0,
                       0,
                       {},
                       month,
                       year);
  };

  auto storeBattle = [&](const std::vector<std::wstring>& battleLines)
  {
    if (battleLines.empty())
    {
      return;
    }

    static const std::wregex battleHeaderPattern(
      L"^\\s*(.+?)\\s*\\((\\d+)\\)\\s+attacks\\s+(.+?)\\s*\\((\\d+)\\)\\s+in\\s+(.+?)\\s+\\((\\d+),(\\d+)(?:,([^\\)]+))?\\)\\s+in\\s+(.+?)!\\s*$"
    );
    static const std::wregex casualtyPattern(L"^\\s*(.+?)\\s*\\((\\d+)\\)\\s+loses\\s+(\\d+)\\.\\s*$");
    static const std::wregex damagedUnitsPattern(L"^\\s*Damaged units:\\s*(.*?)\\.\\s*$");
    static const std::wregex spoilsPattern(L"^\\s*Spoils:\\s*(.*)\\.\\s*$");
    static const std::wregex spoilItemPattern(L"^(?:(\\d+)\\s+)?(.+?)\\s+\\[([^\\]]+)\\]$");

    try
    {
      const std::wstring headerLine = trim(battleLines.front());
      std::wsmatch headerMatch;
      if (!std::regex_match(headerLine, headerMatch, battleHeaderPattern))
      {
        return;
      }

      Battle battle;
      battle.setFullText(joinLines(battleLines));
      battle.setAttackerUnitName(trim(headerMatch[1].str()));
      battle.setAttackerUnitId(std::stoi(headerMatch[2].str()));
      battle.setDefenderUnitName(trim(headerMatch[3].str()));
      battle.setDefenderUnitId(std::stoi(headerMatch[4].str()));
      battle.setRegionType(trim(headerMatch[5].str()));
      battle.setRegionCoordinates(std::stoi(headerMatch[6].str()),
                                  std::stoi(headerMatch[7].str()),
                                  headerMatch[8].matched ? parseZCoordinate(headerMatch[8].str()) : 1);
      battle.setMonth(month_);
      battle.setYear(year_);
      battle.setProvinceName(trim(headerMatch[9].str()));

      const Region* region = regionRepository.findByCoordinates(battle.getRegionXCoordinate(),
                                                               battle.getRegionYCoordinate(),
                                                               battle.getRegionZCoordinate());
      battle.setRegionFoundInRepository(region != nullptr);

      ensureBattleUnit(battle.getAttackerUnitId(), battle.getAttackerUnitName(), month_, year_);
      ensureBattleUnit(battle.getDefenderUnitId(), battle.getDefenderUnitName(), month_, year_);

      int pendingDamagedSide = 0;
      for (std::size_t index = 1; index < battleLines.size(); ++index)
      {
        const std::wstring line = trim(battleLines[index]);
        if (line.empty() || startsWithIgnoreCase(line, L"Round ") ||
            startsWithIgnoreCase(line, L"Battle statistics:") ||
            startsWithIgnoreCase(line, L"Army ") ||
            startsWithIgnoreCase(line, L"Attackers:") ||
            startsWithIgnoreCase(line, L"Defenders:"))
        {
          continue;
        }

        std::wsmatch casualtyMatch;
        if (std::regex_match(line, casualtyMatch, casualtyPattern))
        {
          const int sideUnitId = std::stoi(casualtyMatch[2].str());
          const int losses = std::stoi(casualtyMatch[3].str());
          if (sideUnitId == battle.getAttackerUnitId())
          {
            battle.setAttackerLosses(losses);
            pendingDamagedSide = 1;
          }
          else if (sideUnitId == battle.getDefenderUnitId())
          {
            battle.setDefenderLosses(losses);
            pendingDamagedSide = 2;
          }
          else
          {
            pendingDamagedSide = 0;
          }

          continue;
        }

        std::wsmatch damagedMatch;
        if (std::regex_match(line, damagedMatch, damagedUnitsPattern))
        {
          const std::vector<std::wstring> damagedIds = splitByComma(damagedMatch[1].str());
          for (const std::wstring& damagedIdText : damagedIds)
          {
            if (damagedIdText.empty())
            {
              continue;
            }

            const int damagedId = std::stoi(damagedIdText);
            if (pendingDamagedSide == 1)
            {
              battle.addAttackerDamagedUnitId(damagedId);
            }
            else if (pendingDamagedSide == 2)
            {
              battle.addDefenderDamagedUnitId(damagedId);
            }
          }

          pendingDamagedSide = 0;
          continue;
        }

        std::wsmatch spoilsMatch;
        if (std::regex_match(line, spoilsMatch, spoilsPattern))
        {
          std::wstring spoilsText = spoilsMatch[1].str();
          for (std::size_t continuationIndex = index + 1;
               continuationIndex < battleLines.size();
               ++continuationIndex)
          {
            const std::wstring continuationLine = trim(battleLines[continuationIndex]);
            if (continuationLine.empty())
            {
              break;
            }

            const std::size_t firstNonWhitespace = battleLines[continuationIndex].find_first_not_of(L" \t");
            if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
            {
              break;
            }

            spoilsText += L" ";
            spoilsText += continuationLine;
            index = continuationIndex;
          }

          for (const std::wstring& spoilText : splitByComma(spoilsText))
          {
            if (spoilText.empty())
            {
              continue;
            }

            std::wsmatch spoilMatch;
            if (!std::regex_match(spoilText, spoilMatch, spoilItemPattern))
            {
              continue;
            }

            BattleSpoil spoil;
            spoil.amount = spoilMatch[1].matched ? std::stoi(spoilMatch[1].str()) : 1;
            spoil.token = trim(spoilMatch[3].str());
            battle.addSpoil(std::move(spoil));
          }

          break;
        }
      }

      battleRepository.add(std::move(battle));
    }
    catch (const std::exception&)
    {
      return;
    }
  };

  static const std::wregex regionHeaderStartPattern(
    L"^\\w+\\s+\\(\\d+,\\d+(?:,[^)]+)?\\)\\s+in\\s+[^,]+,.*$"
  );

  bool inBattleSection = false;
  for (std::size_t index = 0; index < lines_.size();)
  {
    const std::wstring topLevelLine = trim(lines_[index]);
    if (startsWithIgnoreCase(topLevelLine, L"Battles during turn:") ||
        startsWithIgnoreCase(topLevelLine, L"Battle reports:") ||
        startsWithIgnoreCase(topLevelLine, L"Battle statistics:"))
    {
      inBattleSection = true;
      ++index;
      continue;
    }

    if (!inBattleSection)
    {
      ++index;
      continue;
    }

    if (std::regex_match(topLevelLine, regionHeaderStartPattern))
    {
      inBattleSection = false;
      ++index;
      continue;
    }

    static const std::wregex battleHeaderPattern(
      L"^\\s*(.+?)\\s*\\((\\d+)\\)\\s+attacks\\s+(.+?)\\s*\\((\\d+)\\)\\s+in\\s+(.+?)\\s+\\((\\d+),(\\d+)(?:,([^\\)]+))?\\)\\s+in\\s+(.+?)!\\s*$"
    );

    if (!std::regex_match(topLevelLine, battleHeaderPattern))
    {
      ++index;
      continue;
    }

    std::vector<std::wstring> battleLines;
    battleLines.push_back(lines_[index]);
    ++index;

    while (index < lines_.size())
    {
      battleLines.push_back(lines_[index]);

      const std::wstring currentTrim = trim(lines_[index]);
      if (startsWithIgnoreCase(currentTrim, L"Spoils:"))
      {
        std::size_t continuationIndex = index + 1;
        while (continuationIndex < lines_.size())
        {
          const std::wstring continuationTrim = trim(lines_[continuationIndex]);
          if (continuationTrim.empty())
          {
            break;
          }

          const std::size_t firstNonWhitespace = lines_[continuationIndex].find_first_not_of(L" \t");
          if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace == 0)
          {
            break;
          }

          battleLines.push_back(lines_[continuationIndex]);
          ++continuationIndex;
        }

        index = continuationIndex;
        break;
      }

      ++index;
    }

    storeBattle(battleLines);
  }
}

void Report::parseEvents(EventRepository& eventRepository)
{
  auto trim = [](std::wstring value)
  {
    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  static const std::wstring errorsHeader { L"Errors during turn:" };
  static const std::wstring eventsHeader { L"Events during turn:" };
  static const std::wregex eventPattern(L"^\\s*(.+?)\\s*\\((\\d+)\\):\\s*(.+)$");
  static const std::wregex sectionHeaderPattern(L"^[A-Za-z][^\\r\\n]*:\\s*$");

  const int reportMonth { getMonth() };
  const int reportYear { getYear() };

  int latestEventMonth {0};
  int latestEventYear {0};
  bool hasLatestEventPeriod = eventRepository.getLatestPeriod(latestEventMonth, latestEventYear);

  if (hasLatestEventPeriod && ((reportYear < latestEventYear) ||
                   (reportYear == latestEventYear && reportMonth <= latestEventMonth)))
  {
    eventRepository.clear();
  }

  auto addParsedEvent = [&eventRepository, reportMonth, reportYear](int unitId, const std::wstring& message, bool isErrorEvent)
  {
    if (unitId == 0 || message.empty())
    {
      return;
    }

    Event eventValue(unitId, eventRepository.getNextEventId(), message, reportMonth, reportYear);
    eventValue.setErrorEvent(isErrorEvent);
    eventRepository.add(std::move(eventValue));
  };

  auto parseEventSection = [&](const std::wstring& sectionHeader, bool isErrorEvent)
  {
    std::size_t sectionStartIndex { lines_.size() };
    for (std::size_t index = 0; index < lines_.size(); ++index)
    {
      if (trim(lines_[index]) == sectionHeader)
      {
        sectionStartIndex = index + 1;
        break;
      }
    }

    if (sectionStartIndex >= lines_.size())
    {
      return;
    }

    int currentUnitId {0};
    std::wstring currentMessage;

    auto flushCurrentEvent = [&]()
    {
      addParsedEvent(currentUnitId, currentMessage, isErrorEvent);
      currentUnitId = 0;
      currentMessage.clear();
    };

    for (std::size_t index = sectionStartIndex; index < lines_.size(); ++index)
    {
      const std::wstring& line = lines_[index];
      const std::wstring trimmedLine = trim(line);
      if (trimmedLine.empty())
      {
        flushCurrentEvent();
        continue;
      }

      std::wsmatch eventMatch;
      if (std::regex_match(line, eventMatch, eventPattern))
      {
        flushCurrentEvent();
        try
        {
          currentUnitId = std::stoi(eventMatch[2].str());
          currentMessage = trim(eventMatch[3].str());
        }
        catch (const std::exception&)
        {
          currentUnitId = 0;
          currentMessage.clear();
        }
        continue;
      }

      const std::size_t firstNonWhitespace = line.find_first_not_of(L" \t");
      if (currentUnitId != 0 && firstNonWhitespace != std::wstring::npos && firstNonWhitespace > 0)
      {
        if (!currentMessage.empty())
        {
          currentMessage += L" ";
        }
        currentMessage += trimmedLine;
        continue;
      }

      if (std::regex_match(trimmedLine, sectionHeaderPattern))
      {
        flushCurrentEvent();
        break;
      }

      flushCurrentEvent();
    }

    flushCurrentEvent();
  };

  parseEventSection(errorsHeader, true);
  parseEventSection(eventsHeader, false);
}

void Report::parseOrders(FactionRepository& factionRepository, UnitRepository& unitRepository)
{
  auto trim = [](std::wstring value)
  {
    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  static const std::wregex atlantisLinePattern(L"^#atlantis\\s+(\\d+)\\s+\"([^\"]*)\"\\s*$", std::regex::icase);
  static const std::wregex unitLinePattern(L"^unit\\s+(\\d+)\\b.*$", std::regex::icase);

  std::size_t ordersStartIndex = lines_.size();
  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    const std::wstring trimmedLine = trim(lines_[index]);
    if (trimmedLine.rfind(L"Orders Template", 0) == 0)
    {
      ordersStartIndex = index + 1;
      break;
    }
  }

  if (ordersStartIndex >= lines_.size())
  {
    return;
  }

  std::size_t index = ordersStartIndex;
  std::wsmatch atlantisMatch;
  bool foundAtlantisLine = false;
  for (; index < lines_.size(); ++index)
  {
    const std::wstring trimmedLine = trim(lines_[index]);
    if (trimmedLine.empty())
    {
      continue;
    }

    if (std::regex_match(trimmedLine, atlantisMatch, atlantisLinePattern))
    {
      foundAtlantisLine = true;
      break;
    }

    return;
  }

  if (!foundAtlantisLine)
  {
    return;
  }

  try
  {
    const int passwordFactionId = std::stoi(atlantisMatch[1].str());
    const std::wstring password = atlantisMatch[2].str();
    if (Faction* faction = factionRepository.findByNumber(passwordFactionId))
    {
      faction->setPassword(password);
    }
  }
  catch (const std::exception&)
  {
    return;
  }

  int currentUnitId = 0;
  std::vector<std::wstring> currentOrders;

  auto flushOrders = [&]()
  {
    if (currentUnitId == 0)
    {
      currentOrders.clear();
      return;
    }

    Unit* unit = unitRepository.findByNumber(currentUnitId);
    if (unit != nullptr)
    {
      const bool canApplyOrders = (year_ > unit->getYear()) ||
                                  (year_ == unit->getYear() && month_ >= unit->getMonth());
      if (canApplyOrders)
      {
        unit->setOrders(currentOrders);
      }
    }

    currentUnitId = 0;
    currentOrders.clear();
  };

  for (++index; index < lines_.size(); ++index)
  {
    const std::wstring trimmedLine = trim(lines_[index]);
    if (trimmedLine.empty())
    {
      continue;
    }

    if (trimmedLine.rfind(L"#end", 0) == 0)
    {
      flushOrders();
      break;
    }

    if (trimmedLine[0] == L';')
    {
      continue;
    }

    std::wsmatch unitMatch;
    if (std::regex_match(trimmedLine, unitMatch, unitLinePattern))
    {
      flushOrders();
      try
      {
        currentUnitId = std::stoi(unitMatch[1].str());
      }
      catch (const std::exception&)
      {
        currentUnitId = 0;
      }
      continue;
    }

    if (currentUnitId != 0)
    {
      // Preserve tagged comment commands (e.g. "@;study ridi") verbatim.
      if (trimmedLine.rfind(L"@;", 0) == 0)
      {
        currentOrders.push_back(trimmedLine);
        continue;
      }

      // Strip inline comment: everything from the first ';' that is not inside a quoted string.
      std::wstring orderLine;
      orderLine.reserve(trimmedLine.size());
      bool inQuotes = false;
      for (size_t i = 0; i < trimmedLine.size(); ++i)
      {
        const wchar_t ch = trimmedLine[i];
        if (ch == L'"')
        {
          inQuotes = !inQuotes;
          orderLine += ch;
        }
        else if (ch == L';' && !inQuotes)
        {
          break; // rest is a comment
        }
        else
        {
          orderLine += ch;
        }
      }

      // Trim trailing whitespace left after comment removal.
      while (!orderLine.empty() && iswspace(orderLine.back()))
      {
        orderLine.pop_back();
      }

      if (!orderLine.empty())
      {
        currentOrders.push_back(std::move(orderLine));
      }
    }
  }
}

void Report::parseItems(ItemRepository& itemRepository)
{
  auto trim = [](std::wstring value)
  {
    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  static const std::wstring kItemReportsMarker = L"Item reports:";
  static const std::wregex itemHeaderPattern(
    L"^([^\\[]+?)\\s*\\[(\\w{3,})\\],\\s*weight\\s+(\\d+)(.*)$"
  );
  static const std::wregex shipHeaderPattern(
    L"^([^\\[]+?)\\s*\\[(\\w{3,})\\]\\.\\s*This is(.*)$"
  );
  static const std::wregex capacityWalkPattern(L"walking capacity\\s+(\\d+)");
  static const std::wregex capacityRidePattern(L"riding capacity\\s+(\\d+)");
  static const std::wregex capacitySwimPattern(L"swimming capacity\\s+(\\d+)");
  static const std::wregex capacityFlyPattern(L"flying capacity\\s+(\\d+)");
  static const std::wregex shipCapacityPattern(L"capacity of\\s+(\\d+)");
  static const std::wregex shipSpeedPattern(L"speed of\\s+(\\d+)\\s+hexes per month");
  static const std::wregex shipSailingSkillPattern(L"(?:requires|require)\\s+(?:a\\s+total\\s+of\\s+)?(\\d+)\\s+levels?\\s+of\\s+sailing\\s+skill", std::regex::icase);
  static const std::wregex shipMagesStudyPattern(L"This ship will allow up to\\s+(\\d+)\\s+mages?\\s+to study above level 2", std::regex::icase);
  static const std::wregex shipFlyingPattern(L"flying\\s+'ship'");
  static const std::wregex movesPattern(L"moves\\s+(\\d+)\\s+hexes per month");
  static const std::wregex raceStudyPattern(L"This race may study");
  static const std::wregex raceStudyCapturePattern(L"This race may study\\s+(.+?)(?:\\.|$)");
  static const std::wregex allSkillsLevelPattern(L"all\\s+skills\\s+to\\s+level\\s+(\\d+)", std::regex::icase);
  static const std::wregex listedAndDefaultLevelPattern(
    L"(.+?)\\s+to\\s+level\\s+(\\d+)\\s+and\\s+all\\s+other(?:\\s+non-magic)?\\s+skills\\s+to\\s+level\\s+(\\d+)",
    std::regex::icase);
  static const std::wregex skillTokenPattern(L"\\[(\\w{3,})\\]");
  static const std::wregex mountPattern(L"This is a mount", std::regex::icase);
  static const std::wregex productionHelpPattern(L"This item increases the production of\\s+(.+?)(?:\\.|$)", std::regex::icase);

  std::size_t itemReportsIndex = lines_.size();
  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    if (lines_[index] == kItemReportsMarker)
    {
      itemReportsIndex = index;
      break;
    }
  }

  if (itemReportsIndex >= lines_.size())
  {
    return;
  }

  for (std::size_t index = itemReportsIndex + 1; index < lines_.size(); ++index)
  {
    const std::wstring& line = lines_[index];
    const std::size_t firstNonWhitespace = line.find_first_not_of(L" \t");

    if (firstNonWhitespace == std::wstring::npos)
    {
      continue;
    }

    if (firstNonWhitespace == 0)
    {
      std::wsmatch itemHeaderMatch;
      std::wsmatch shipHeaderMatch;
      const bool hasRegularHeader = std::regex_match(line, itemHeaderMatch, itemHeaderPattern);
      const bool hasShipHeader = std::regex_match(line, shipHeaderMatch, shipHeaderPattern);
      if (!hasRegularHeader && !hasShipHeader)
      {
        continue;
      }

      std::wstring itemName;
      std::wstring identifierToken;
      int weight = 0;
      if (hasRegularHeader)
      {
        itemName = trim(itemHeaderMatch[1].str());
        identifierToken = itemHeaderMatch[2].str();
        weight = std::stoi(itemHeaderMatch[3].str());
      }
      else
      {
        itemName = trim(shipHeaderMatch[1].str());
        identifierToken = shipHeaderMatch[2].str();
      }

      int walkCapacity = 0;
      int rideCapacity = 0;
      int swimCapacity = 0;
      int flyCapacity = 0;
      int moves = 0;
      int shipSpeedHexesPerMonth = 0;
      int shipSailingSkillRequired = 0;
      int magesStudy = 0;
      bool man = false;
      bool mount = false;
      int defaultSkillMax = 0;
      std::map<std::wstring, int> skillsMax;
      std::wstring fullText = line;

      std::size_t continuationLineCount = 0;
      while (index + continuationLineCount + 1 < lines_.size())
      {
        const std::wstring& continuationLine = lines_[index + continuationLineCount + 1];
        const std::size_t contFirstNonWS = continuationLine.find_first_not_of(L" \t");

        if (contFirstNonWS == std::wstring::npos)
        {
          break;
        }

        if (contFirstNonWS == 0)
        {
          break;
        }

        fullText += L" ";
        fullText += trim(continuationLine);

        ++continuationLineCount;
      }

      // Parse race study skills from the full wrapped item text to support multi-line entries.
      if (std::regex_search(fullText, raceStudyPattern))
      {
        man = true;
        std::wsmatch raceStudyMatch;
        if (std::regex_search(fullText, raceStudyMatch, raceStudyCapturePattern) && raceStudyMatch.size() > 1)
        {
          const std::wstring skillsClause = trim(raceStudyMatch[1].str());

          std::wsmatch listedAndDefaultMatch;
          if (std::regex_search(skillsClause, listedAndDefaultMatch, listedAndDefaultLevelPattern)
            && listedAndDefaultMatch.size() > 3)
          {
            const std::wstring listedSkills = listedAndDefaultMatch[1].str();
            const int listedMaxLevel = std::stoi(listedAndDefaultMatch[2].str());
            defaultSkillMax = std::stoi(listedAndDefaultMatch[3].str());

            for (std::wsregex_iterator it(listedSkills.begin(), listedSkills.end(), skillTokenPattern), end; it != end; ++it)
            {
              skillsMax[(*it)[1].str()] = listedMaxLevel;
            }
          }
          else
          {
            std::wsmatch allSkillsMatch;
            if (std::regex_search(skillsClause, allSkillsMatch, allSkillsLevelPattern) && allSkillsMatch.size() > 1)
            {
              defaultSkillMax = std::stoi(allSkillsMatch[1].str());
            }
          }
        }
      }

      std::wsmatch walkMatch;
      if (std::regex_search(fullText, walkMatch, capacityWalkPattern))
      {
        walkCapacity = std::stoi(walkMatch[1].str());
      }

      std::wsmatch rideMatch;
      if (std::regex_search(fullText, rideMatch, capacityRidePattern))
      {
        rideCapacity = std::stoi(rideMatch[1].str());
      }

      std::wsmatch swimMatch;
      if (std::regex_search(fullText, swimMatch, capacitySwimPattern))
      {
        swimCapacity = std::stoi(swimMatch[1].str());
      }

      std::wsmatch flyMatch;
      if (std::regex_search(fullText, flyMatch, capacityFlyPattern))
      {
        flyCapacity = std::stoi(flyMatch[1].str());
      }

      std::wsmatch movesMatch;
      if (std::regex_search(fullText, movesMatch, movesPattern))
      {
        moves = std::stoi(movesMatch[1].str());
      }

      if (std::regex_search(fullText, mountPattern))
      {
        mount = true;
      }

      // Parse production help items from the full wrapped item text.
      std::map<std::wstring, int> productionHelp;
      std::wsmatch productionHelpMatch;
      if (std::regex_search(fullText, productionHelpMatch, productionHelpPattern) && productionHelpMatch.size() > 1)
      {
        const std::wstring itemsList = productionHelpMatch[1].str();
        
        // Pattern to extract individual items: "item name [TOKEN] by NUMBER"
        static const std::wregex itemEntryPattern(L"([^\\[]+?)\\s*\\[(\\w{3,})\\]\\s+by\\s+(\\d+)");
        
        auto itemsBegin = std::wsregex_iterator(itemsList.begin(), itemsList.end(), itemEntryPattern);
        auto itemsEnd = std::wsregex_iterator();
        for (auto it = itemsBegin; it != itemsEnd; ++it)
        {
          const std::wsmatch& itemMatch = *it;
          if (itemMatch.size() > 3)
          {
            const std::wstring itemToken = itemMatch[2].str();
            const int bonusAmount = std::stoi(itemMatch[3].str());
            productionHelp[itemToken] = bonusAmount;
          }
        }
      }

      if (hasShipHeader)
      {
        std::wsmatch shipCapacityMatch;
        if (std::regex_search(fullText, shipCapacityMatch, shipCapacityPattern))
        {
          const int shipCapacity = std::stoi(shipCapacityMatch[1].str());
          if (std::regex_search(fullText, shipFlyingPattern))
          {
            flyCapacity = shipCapacity;
          }
          else
          {
            swimCapacity = shipCapacity;
          }
        }

        std::wsmatch shipSpeedMatch;
        if (std::regex_search(fullText, shipSpeedMatch, shipSpeedPattern))
        {
          shipSpeedHexesPerMonth = std::stoi(shipSpeedMatch[1].str());
          moves = shipSpeedHexesPerMonth;
        }

        std::wsmatch shipSailingSkillMatch;
        if (std::regex_search(fullText, shipSailingSkillMatch, shipSailingSkillPattern))
        {
          shipSailingSkillRequired = std::stoi(shipSailingSkillMatch[1].str());
        }

        std::wsmatch shipMagesStudyMatch;
        if (std::regex_search(fullText, shipMagesStudyMatch, shipMagesStudyPattern))
        {
          magesStudy = std::stoi(shipMagesStudyMatch[1].str());
        }
      }

      if (!itemRepository.add(identifierToken,
              itemName,
                              weight,
                              false,
                              false,
                              false,
                              false,
                  mount,
                              moves,
                              walkCapacity,
                              rideCapacity,
                              swimCapacity,
                              flyCapacity,
                              man))
      {
        Item* existingItem = itemRepository.findByIdentifierToken(identifierToken);
        if (existingItem)
        {
          existingItem->setItemName(itemName);
          existingItem->setWeight(weight);
          existingItem->setMoves(moves);
          existingItem->setWalkCapacity(walkCapacity);
          existingItem->setRideCapacity(rideCapacity);
          existingItem->setSwimCapacity(swimCapacity);
          existingItem->setFlyCapacity(flyCapacity);
          existingItem->setMount(mount);
          existingItem->setMan(man);
          existingItem->setShipSpeedHexesPerMonth(shipSpeedHexesPerMonth);
          existingItem->setShipSailingSkillRequired(shipSailingSkillRequired);
          existingItem->setMagesStudy(magesStudy);
          existingItem->setDefaultSkillMax(defaultSkillMax);
          existingItem->setSkillsMax(skillsMax);
          existingItem->setFullText(fullText);
          existingItem->setProductionHelp(productionHelp);
        }
      }
      else
      {
        Item* newItem = itemRepository.findByIdentifierToken(identifierToken);
        if (newItem)
        {
          newItem->setShipSpeedHexesPerMonth(shipSpeedHexesPerMonth);
          newItem->setShipSailingSkillRequired(shipSailingSkillRequired);
          newItem->setMagesStudy(magesStudy);
          newItem->setDefaultSkillMax(defaultSkillMax);
          newItem->setSkillsMax(skillsMax);
          newItem->setFullText(fullText);
          newItem->setProductionHelp(productionHelp);
        }
      }

      index += continuationLineCount;
    }
  }
}

void Report::parseStructures(StructInfoRepository& structInfoRepository, ItemRepository& itemRepository)
{
  // Local utility to trim BOM, leading/trailing whitespace and normalize report text.
  auto trim = [](std::wstring value)
  {
    if (!value.empty() && value.front() == 0xFEFF)
    {
      value.erase(value.begin());
    }

    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  // We locate the "Object reports:" section and parse top-level structure entries from it.
  static const std::wstring kObjectReportsMarker = L"Object reports:";
  static const std::wregex objectHeaderPattern(L"^([^:]+):\\s*(.+)$");
  static const std::wregex structureMagesStudyPattern(
    L"This structure will allow up to\\s+(\\d+)\\s+mages?\\s+to study above level 2",
    std::regex_constants::icase);
  static const std::wregex needsPattern(
    L"needs\\s+(\\d+)",
    std::regex_constants::icase);

  std::size_t objectReportsIndex = lines_.size();
  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    if (trim(lines_[index]) == kObjectReportsMarker)
    {
      objectReportsIndex = index;
      break;
    }
  }

  if (objectReportsIndex < lines_.size())
  {
    for (std::size_t index = objectReportsIndex + 1; index < lines_.size(); ++index)
    {
      const std::wstring& line = lines_[index];
      const std::size_t firstNonWhitespace = line.find_first_not_of(L" \t");
      if (firstNonWhitespace == std::wstring::npos)
      {
        continue;
      }

      if (firstNonWhitespace > 0)
      {
        continue;
      }

      std::wsmatch headerMatch;
      if (!std::regex_match(line, headerMatch, objectHeaderPattern))
      {
        continue;
      }

      std::wstring structureType = trim(headerMatch[1].str());
      if (structureType.empty())
      {
        continue;
      }

      std::wstring fullText = trim(headerMatch[2].str());
      std::size_t continuationLineCount = 0;
      while (index + continuationLineCount + 1 < lines_.size())
      {
        const std::wstring& continuationLine = lines_[index + continuationLineCount + 1];
        const std::size_t contFirstNonWS = continuationLine.find_first_not_of(L" \t");
        if (contFirstNonWS == std::wstring::npos)
        {
          break;
        }

        // Stop consuming lines when we hit another top-level object header.
        if (contFirstNonWS == 0)
        {
          std::wsmatch nextHeaderMatch;
          if (std::regex_match(continuationLine, nextHeaderMatch, objectHeaderPattern))
          {
            break;
          }
        }

        // Append continuation lines to the current structure description.
        fullText += L" ";
        fullText += trim(continuationLine);
        ++continuationLineCount;
      }

      std::wstring lowerFullText = fullText;
      std::transform(lowerFullText.begin(), lowerFullText.end(), lowerFullText.begin(), towlower);
      std::wstring lowerStructureType = structureType;
      std::transform(lowerStructureType.begin(), lowerStructureType.end(), lowerStructureType.begin(), towlower);

      // Detect the structure kind from its description or from known ship-like types.
      const bool isShip =
        lowerFullText.find(L"this is a ship") != std::wstring::npos ||
        lowerStructureType == L"fleet";
      const bool isFlying = lowerFullText.find(L"flying") != std::wstring::npos;
      int needs = 0;
      int magesCapacity = 0;
      std::wstring itemIdentifierToken;

      // Parse a numeric "needs N" clause, if present.
      std::wsmatch needsMatch;
      if (std::regex_search(fullText, needsMatch, needsPattern) && needsMatch.size() > 1)
      {
        try
        {
          needs = std::stoi(needsMatch[1].str());
        }
        catch (const std::exception&)
        {
          needs = 0;
        }
      }

      // Only ship structures are assigned an item identifier token.
      // This avoids incorrectly using tokens found inside building descriptions.
      if (isShip)
      {
        if (const Item* shipItem = itemRepository.findByItemName(structureType))
        {
          itemIdentifierToken = shipItem->getIdentifierToken();
        }
      }

      std::wsmatch structureMagesStudyMatch;
      if (std::regex_search(fullText, structureMagesStudyMatch, structureMagesStudyPattern) && structureMagesStudyMatch.size() > 1)
      {
        try
        {
          magesCapacity = std::stoi(structureMagesStudyMatch[1].str());
        }
        catch (const std::exception&)
        {
          magesCapacity = 0;
        }
      }

      structInfoRepository.addOrUpdateByType(structureType,
                           needs,
                           magesCapacity,
                           isShip,
                           isFlying,
                           itemIdentifierToken);

      index += continuationLineCount;
    }
  }

  // Some ship structure types are not listed in Object reports at all.
  // Mirror all ship-capable items from the Item repository into StructInfoRepository,
  // so ship structures like "Cog" or "Longship" still have metadata available.
  for (std::size_t itemIndex = 0; itemIndex < itemRepository.size(); ++itemIndex)
  {
    const Item& item = itemRepository.at(itemIndex);
    const int shipCapacity = std::max(item.getSwimCapacity(), item.getFlyCapacity());
    if (shipCapacity <= 0)
    {
      continue;
    }

    structInfoRepository.addOrUpdateByType(item.getItemName(),
                                           shipCapacity,
                                           item.getMagesStudy(),
                                           true,
                                           item.getFlyCapacity() > 0,
                                           item.getIdentifierToken());
  }
}

void Report::parseSkills(SkillRepository& skillRepository,
                         ItemRepository& itemRepository,
                         const std::vector<std::wstring>& magicSkillTriggerPhrases)
{
  auto trim = [](std::wstring value)
  {
    if (!value.empty() && value.front() == 0xFEFF)
    {
      value.erase(value.begin());
    }

    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  static const std::wstring kSkillReportsMarker = L"Skill reports:";
  static const std::vector<std::wstring> kDefaultMagicSkillMarkers = {
    L"a mage with this skill",
    L"a mage with",
    L"forms of magic",
    L"allows a mage"
  };
  static const std::wstring kMagicFoundationMarker = L"foundation skills on which other magical skills are based";
  static const std::wstring kPrerequisitesPrefix = L"this skill requires";
  static const std::wregex skillHeaderPattern(L"^([^\\[]+?)\\s*\\[(\\w{3,})\\]\\s+(\\d+):.*$");
  static const std::wregex prerequisiteSkillPattern(
    L"\\[([A-Za-z0-9]{3,})\\]\\s*(\\d+)",
    std::regex_constants::icase);

  // Find "Skill reports:" marker
  std::size_t skillReportsIndex = lines_.size();
  for (std::size_t index = 0; index < lines_.size(); ++index)
  {
    if (trim(lines_[index]) == kSkillReportsMarker)
    {
      skillReportsIndex = index;
      break;
    }
  }

  if (skillReportsIndex >= lines_.size())
  {
    return;
  }

  std::vector<std::wstring> normalizedMagicMarkers;
  const std::vector<std::wstring>& sourceMarkers = magicSkillTriggerPhrases.empty()
    ? kDefaultMagicSkillMarkers
    : magicSkillTriggerPhrases;
  normalizedMagicMarkers.reserve(sourceMarkers.size());
  for (const std::wstring& marker : sourceMarkers)
  {
    const std::wstring trimmedMarker = trim(marker);
    if (!trimmedMarker.empty())
    {
      normalizedMagicMarkers.push_back(StringUtils::toLower(trimmedMarker));
    }
  }

  // Parse each skill entry
  for (std::size_t index = skillReportsIndex + 1; index < lines_.size(); ++index)
  {
    const std::wstring& line = lines_[index];
    const std::size_t firstNonWhitespace = line.find_first_not_of(L" \t");

    // Skip empty lines or continuation lines
    if (firstNonWhitespace == std::wstring::npos || firstNonWhitespace > 0)
    {
      continue;
    }

    // Try to match skill header: "name [TOKEN] level:"
    std::wsmatch headerMatch;
    if (!std::regex_match(line, headerMatch, skillHeaderPattern))
    {
      continue;
    }

    std::wstring skillName = trim(headerMatch[1].str());
    std::wstring skillToken = headerMatch[2].str();
    int skillLevel = std::stoi(headerMatch[3].str());

    // Collect description from this line and continuation lines
    std::wstring fullDescription = line;
    std::size_t continuationLineCount = 0;

    while (index + continuationLineCount + 1 < lines_.size())
    {
      const std::wstring& continuationLine = lines_[index + continuationLineCount + 1];
      const std::size_t contFirstNonWS = continuationLine.find_first_not_of(L" \t");

      // Stop if we hit another skill header (non-indented line with skill pattern)
      if (contFirstNonWS == 0)
      {
        std::wsmatch nextHeaderMatch;
        if (std::regex_match(continuationLine, nextHeaderMatch, skillHeaderPattern))
        {
          break;
        }
      }

      // If empty line, stop collecting description
      if (contFirstNonWS == std::wstring::npos)
      {
        break;
      }

      fullDescription += L" " + trim(continuationLine);
      ++continuationLineCount;
    }

    // Check if this is a production skill
    bool isProduction = fullDescription.find(L"A unit with this skill may PRODUCE") != std::wstring::npos;
    std::map<std::wstring, int> productionItems;

    if (isProduction)
    {
      // Parse production items: "PRODUCE item [TOKEN] ... at a rate of N per man-month"
      size_t producePos = fullDescription.find(L"A unit with this skill may PRODUCE");
      if (producePos != std::wstring::npos)
      {
        std::wstring produceText = fullDescription.substr(producePos + 34); // Skip "A unit with this skill may PRODUCE"

        // Find the period that ends the produce section
        size_t endPos = produceText.find(L". A unit");
        if (endPos == std::wstring::npos)
        {
          endPos = produceText.find(L".");
        }
        if (endPos != std::wstring::npos)
        {
          produceText = produceText.substr(0, endPos);
        }

        // Parse multiple items separated by commas or "and"
        std::wregex itemPattern(L"([^,]+?)\\s*\\[([\\w]{3,})\\]");
        for (std::wsregex_iterator it(produceText.begin(), produceText.end(), itemPattern),
             end; it != end; ++it)
        {
          std::wstring itemName = trim((*it)[1].str());
          std::wstring itemToken = (*it)[2].str();

          // Check if this item has a "from" clause for resources
          size_t itemStartPos = produceText.find(itemName);
          if (itemStartPos != std::wstring::npos)
          {
            size_t fromPos = produceText.find(L"from", itemStartPos);
            size_t nextItemPos = produceText.find(L"[", itemStartPos + itemName.length());

            // Check if there's a "from" clause before the next item
            if (fromPos != std::wstring::npos && (nextItemPos == std::wstring::npos || fromPos < nextItemPos))
            {
              size_t ratePos = produceText.find(L"at a rate", fromPos);
              if (ratePos != std::wstring::npos)
              {
                std::wstring fromText = produceText.substr(fromPos + 4, ratePos - fromPos - 4);

                // Parse resource items: "amount item_name [TOKEN]"
                std::wregex resourcePattern(L"(\\d+)?\\s*([^\\[]+?)\\s*\\[([\\w]{3,})\\]");
                std::wsmatch resourceMatch;
                if (std::regex_search(fromText, resourceMatch, resourcePattern))
                {
                  int resourceAmount = 1;
                  if (resourceMatch[1].matched)
                  {
                    resourceAmount = std::stoi(resourceMatch[1].str());
                  }
                  std::wstring resourceToken = resourceMatch[3].str();

                  // Set the resource in the produced item
                  Item* producedItem = itemRepository.findByIdentifierToken(itemToken);
                  if (producedItem)
                  {
                    std::map<std::wstring, int> resources = producedItem->getResources();
                    resources[resourceToken] = resourceAmount;
                    producedItem->setResources(resources);
                  }
                }
              }
            }
          }

          // Parse the rate: "at a rate of N per man-month"
          size_t ratePos = produceText.find(L"at a rate of", itemStartPos);
          if (ratePos != std::wstring::npos)
          {
            std::wstring rateText = produceText.substr(ratePos + 12);
            std::wregex ratePattern(L"(\\d+)\\s+per");
            std::wsmatch rateMatch;
            if (std::regex_search(rateText, rateMatch, ratePattern))
            {
              int rate = std::stoi(rateMatch[1].str());
              productionItems[itemToken] = rate;
            }
          }
        }
      }
    }

    int studyCost = 0;
    static const std::wregex studyCostPattern(
      L"This skill costs\\s+(\\d+)\\s+silver per month of study",
      std::regex_constants::icase);
    std::wsmatch studyCostMatch;
    if (std::regex_search(fullDescription, studyCostMatch, studyCostPattern))
    {
      try
      {
        studyCost = std::stoi(studyCostMatch[1].str());
      }
      catch (const std::exception&)
      {
        studyCost = 0;
      }
    }

    if (studyCost == 0 && skillLevel > 1)
    {
      const Skill* existingSkill = skillRepository.findByIdentifier(skillToken);
      if (existingSkill)
      {
        studyCost = existingSkill->getStudyCost();
      }
    }

    const std::wstring lowerDescription = StringUtils::toLower(fullDescription);
    bool hasMagicTrigger = false;
    for (const std::wstring& marker : normalizedMagicMarkers)
    {
      if (lowerDescription.find(marker) != std::wstring::npos)
      {
        hasMagicTrigger = true;
        break;
      }
    }

    const bool isMagic = skillLevel == 1
      && hasMagicTrigger;
    const bool isMagicFoundation = skillLevel == 1
      && lowerDescription.find(kMagicFoundationMarker) != std::wstring::npos;

    std::vector<SkillPrerequisite> prerequisites;
    const std::size_t prerequisitesPrefixPos = lowerDescription.find(kPrerequisitesPrefix);
    if (prerequisitesPrefixPos != std::wstring::npos)
    {
      std::size_t clauseEndPos = lowerDescription.find(L"to begin to study", prerequisitesPrefixPos);
      if (clauseEndPos == std::wstring::npos)
      {
        clauseEndPos = lowerDescription.find(L".", prerequisitesPrefixPos);
      }

      if (clauseEndPos != std::wstring::npos && clauseEndPos > prerequisitesPrefixPos)
      {
        std::wstring clause = fullDescription.substr(
          prerequisitesPrefixPos + kPrerequisitesPrefix.length(),
          clauseEndPos - (prerequisitesPrefixPos + kPrerequisitesPrefix.length()));

        for (std::wsregex_iterator it(clause.begin(), clause.end(), prerequisiteSkillPattern), end;
             it != end;
             ++it)
        {
          // Store prerequisites token-only. Any display name must be resolved
          // later from SkillRepository via this token.
          SkillPrerequisite prerequisite;
          prerequisite.token = trim((*it)[1].str());
          try
          {
            prerequisite.requiredLevel = std::stoi((*it)[2].str());
          }
          catch (const std::exception&)
          {
            prerequisite.requiredLevel = 0;
          }

          if (!prerequisite.token.empty() && prerequisite.requiredLevel > 0)
          {
            prerequisites.push_back(std::move(prerequisite));
          }
        }
      }
    }

    // Add skill to repository
    const bool levelAdded = skillRepository.addLevel(
      skillToken,
      skillName,
      skillLevel,
      productionItems,
      fullDescription);

    if (levelAdded)
    {
      Skill* parsedSkill = skillRepository.findByIdentifier(skillToken);
      if (parsedSkill)
      {
        if (isMagic)
        {
          parsedSkill->setMagic(true);
        }

        if (isMagicFoundation)
        {
          parsedSkill->setMagicFoundation(true);
        }

        if (studyCost > 0)
        {
          parsedSkill->setStudyCost(studyCost);
        }

        if (!prerequisites.empty())
        {
          parsedSkill->setPrerequisites(std::move(prerequisites));
        }
      }
    }

    // Keep reverse linkage in Item model for production lookups.
    if (levelAdded)
    {
      for (const auto& [itemToken, _] : productionItems)
      {
        Item* producedItem = itemRepository.findByIdentifierToken(itemToken);
        if (producedItem)
        {
          producedItem->setProductionSkillLevel(skillToken, skillLevel);
        }
      }
    }

    index += continuationLineCount;
  }
}
