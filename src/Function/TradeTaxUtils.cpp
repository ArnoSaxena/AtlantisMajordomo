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
 * File: TradeTaxUtils.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/TradeTaxUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/Report.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>

namespace
{
struct RegionKey
{
  int x { 0 };
  int y { 0 };
  int z { 0 };

  bool operator<(const RegionKey& other) const
  {
    if (z != other.z) return z < other.z;
    if (x != other.x) return x < other.x;
    return y < other.y;
  }
};

int resolveMainFactionNumber(const AppData& appData)
{
  return appData.factionRepository().getMainFactionNumber();
}

bool latestReportPeriod(const AppData& appData, int& latestMonth, int& latestYear)
{
  latestMonth = 0;
  latestYear = 0;
  const auto& reports = appData.reportRepository();
  for (std::size_t index = 0; index < reports.size(); ++index)
  {
    const Report& report = reports.at(index);
    const int month = report.getMonth();
    const int year = report.getYear();
    if (month < 1 || month > 12 || year <= 0)
    {
      continue;
    }

    if (year > latestYear || (year == latestYear && month > latestMonth))
    {
      latestMonth = month;
      latestYear = year;
    }
  }

  return latestMonth >= 1 && latestMonth <= 12 && latestYear > 0;
}

bool isCurrentPeriod(int month, int year, bool hasLatestPeriod, int latestMonth, int latestYear)
{
  return !hasLatestPeriod || (month == latestMonth && year == latestYear);
}

std::wstring extractKeywordUpper(const std::wstring& orderLine)
{
  std::wstring keyword;
  if (!OrderParsingUtils::tryExtractOrderKeywordUpper(orderLine, keyword))
  {
    return L"";
  }
  return keyword;
}

std::wstring extractProduceOperand(const std::wstring& orderLine)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!OrderParsingUtils::tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return L"";
  }

  std::size_t tokenIndex = 0;
  if (!tokens.front().empty() && tokens.front().front() == L'@')
  {
    if (tokens.front().size() == 1)
    {
      tokenIndex = 1;
    }
    else
    {
      tokens.front().erase(tokens.front().begin());
    }
  }

  if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"PRODUCE")
  {
    return L"";
  }

  ++tokenIndex;
  if (tokenIndex >= tokens.size())
  {
    return L"";
  }

  int ignoredAmount = 0;
  if (OrderParsingUtils::tryParseIntStrict(tokens[tokenIndex], ignoredAmount))
  {
    ++tokenIndex;
  }

  if (tokenIndex >= tokens.size())
  {
    return L"";
  }

  return tokens[tokenIndex];
}

std::wstring resolveResourceToken(const AppData& appData, const Region* region, const std::wstring& operand)
{
  if (operand.empty())
  {
    return L"";
  }

  const std::wstring normalizedOperand = StringUtils::normalizeToken(operand);
  const Item* item = appData.itemRepository().findByIdentifierToken(normalizedOperand);
  if (!item)
  {
    item = appData.itemRepository().findByItemName(operand);
  }
  if (item && item->isResource())
  {
    return StringUtils::normalizeToken(item->getIdentifierToken());
  }

  if (region)
  {
    for (const auto& [resourceToken, amount] : region->getResources())
    {
      if (amount <= 0)
      {
        continue;
      }

      const std::wstring normalizedResourceToken = StringUtils::normalizeToken(resourceToken);
      if (normalizedResourceToken == normalizedOperand)
      {
        return normalizedResourceToken;
      }
    }
  }

  return L"";
}

void collectOrders(const AppData& appData,
                   int mainFactionNumber,
                   bool hasLatestPeriod,
                   int latestMonth,
                   int latestYear,
                   std::set<RegionKey>& taxingRegions,
                   std::map<RegionKey, std::set<std::wstring>>& producedResources)
{
  const auto& units = appData.unitRepository();
  for (std::size_t index = 0; index < units.size(); ++index)
  {
    const Unit& unit = units.at(index);
    if (unit.getFactionNumber() != mainFactionNumber ||
        !isCurrentPeriod(unit.getMonth(), unit.getYear(), hasLatestPeriod, latestMonth, latestYear))
    {
      continue;
    }

    const RegionKey key { unit.getXCoordinate(), unit.getYCoordinate(), unit.getZCoordinate() };
    const Region* region = appData.regionRepository().findByCoordinates(key.x, key.y, key.z);
    if (!region)
    {
      region = appData.regionRepository().findByCoordinates(key.x, key.y);
    }
    const std::vector<std::wstring> ownOrders = OrderParsingUtils::filterOrdersIgnoringFormBlocks(unit.getOrders());
    for (const std::wstring& order : ownOrders)
    {
      const std::wstring keyword = extractKeywordUpper(order);
      if (keyword == L"TAX")
      {
        taxingRegions.insert(key);
      }
      else if (keyword == L"PRODUCE")
      {
        const std::wstring resourceToken = resolveResourceToken(appData, region, extractProduceOperand(order));
        if (!resourceToken.empty())
        {
          producedResources[key].insert(resourceToken);
        }
      }
    }
  }

  const auto& unitNews = appData.unitNewRepository();
  for (std::size_t index = 0; index < unitNews.size(); ++index)
  {
    const UnitNew& unitNew = unitNews.at(index);
    if (!isCurrentPeriod(unitNew.getMonth(), unitNew.getYear(), hasLatestPeriod, latestMonth, latestYear))
    {
      continue;
    }

    int unitNewFactionNumber = unitNew.getFactionNumber();
    const Unit* originUnit = appData.unitRepository().findByNumber(unitNew.getOriginUnit());
    if (unitNewFactionNumber <= 0 && originUnit)
    {
      unitNewFactionNumber = originUnit->getFactionNumber();
    }
    if (unitNewFactionNumber != mainFactionNumber || !originUnit)
    {
      continue;
    }

    const RegionKey key { unitNew.getXCoordinate(), unitNew.getYCoordinate(), unitNew.getZCoordinate() };
    const Region* region = appData.regionRepository().findByCoordinates(key.x, key.y, key.z);
    if (!region)
    {
      region = appData.regionRepository().findByCoordinates(key.x, key.y);
    }
    const std::vector<std::wstring> formOrders = OrderParsingUtils::extractFormNewUnitBlock(
      originUnit->getOrders(),
      unitNew.getUnitNumber());
    for (const std::wstring& order : formOrders)
    {
      const std::wstring keyword = extractKeywordUpper(order);
      if (keyword == L"TAX")
      {
        taxingRegions.insert(key);
      }
      else if (keyword == L"PRODUCE")
      {
        const std::wstring resourceToken = resolveResourceToken(appData, region, extractProduceOperand(order));
        if (!resourceToken.empty())
        {
          producedResources[key].insert(resourceToken);
        }
      }
    }
  }
}

TradeTaxUtils::RowSeverity taxSeverity(int untaxed)
{
  if (untaxed <= 0)
  {
    return TradeTaxUtils::RowSeverity::Green;
  }
  if (untaxed < 50)
  {
    return TradeTaxUtils::RowSeverity::Yellow;
  }
  return TradeTaxUtils::RowSeverity::Red;
}

int potentialTaxers(int untaxed)
{
  if (untaxed <= 0)
  {
    return 0;
  }
  return (untaxed + 49) / 50;
}

int resolveResourceAmount(const std::map<std::wstring, int>& resources,
                          const std::wstring& token,
                          int fallback)
{
  const std::wstring normalizedToken = StringUtils::normalizeToken(token);
  int amount = 0;
  bool found = false;
  for (const auto& [resourceToken, resourceAmount] : resources)
  {
    if (StringUtils::normalizeToken(resourceToken) == normalizedToken)
    {
      amount += resourceAmount;
      found = true;
    }
  }

  return found ? amount : fallback;
}
}

namespace TradeTaxUtils
{
std::vector<TaxRow> buildTaxRows(const AppData& appData)
{
  const int mainFactionNumber = resolveMainFactionNumber(appData);
  if (mainFactionNumber <= 0)
  {
    return {};
  }

  int latestMonth = 0;
  int latestYear = 0;
  const bool hasLatestPeriod = latestReportPeriod(appData, latestMonth, latestYear);

  std::set<RegionKey> taxingRegions;
  std::map<RegionKey, std::set<std::wstring>> producedResources;
  collectOrders(appData, mainFactionNumber, hasLatestPeriod, latestMonth, latestYear, taxingRegions, producedResources);

  std::vector<TaxRow> rows;
  for (const RegionKey& key : taxingRegions)
  {
    const Region* region = appData.regionRepository().findByCoordinates(key.x, key.y, key.z);
    if (!region || !isCurrentPeriod(region->getMonth(), region->getYear(), hasLatestPeriod, latestMonth, latestYear))
    {
      continue;
    }

    const int untaxed = region->getTaxableIncomeAfterOrders();
    rows.push_back(TaxRow{
      CoordinateFormattingUtils::formatCoordinates(key.x, key.y, key.z),
      region->getTaxableIncome(),
      untaxed,
      potentialTaxers(untaxed),
      taxSeverity(untaxed)
    });
  }

  return rows;
}

std::vector<ResourceRow> buildResourceRows(const AppData& appData)
{
  const int mainFactionNumber = resolveMainFactionNumber(appData);
  if (mainFactionNumber <= 0)
  {
    return {};
  }

  int latestMonth = 0;
  int latestYear = 0;
  const bool hasLatestPeriod = latestReportPeriod(appData, latestMonth, latestYear);

  std::set<RegionKey> taxingRegions;
  std::map<RegionKey, std::set<std::wstring>> producedResources;
  collectOrders(appData, mainFactionNumber, hasLatestPeriod, latestMonth, latestYear, taxingRegions, producedResources);

  std::vector<ResourceRow> rows;
  for (const auto& [key, tokens] : producedResources)
  {
    const Region* region = appData.regionRepository().findByCoordinates(key.x, key.y, key.z);
    if (!region || !isCurrentPeriod(region->getMonth(), region->getYear(), hasLatestPeriod, latestMonth, latestYear))
    {
      continue;
    }

    std::map<std::wstring, std::wstring> resourcesToDisplay;
    for (const auto& [resourceToken, amount] : region->getResources())
    {
      (void)amount;
      const std::wstring normalizedToken = StringUtils::normalizeToken(resourceToken);
      if (!normalizedToken.empty())
      {
        resourcesToDisplay.emplace(normalizedToken, resourceToken);
      }
    }

    for (const std::wstring& resourceToken : tokens)
    {
      const std::wstring normalizedToken = StringUtils::normalizeToken(resourceToken);
      if (!normalizedToken.empty())
      {
        resourcesToDisplay.emplace(normalizedToken, resourceToken);
      }
    }

    for (const auto& [normalizedToken, resourceToken] : resourcesToDisplay)
    {
      const int maxAmount = resolveResourceAmount(region->getResources(), normalizedToken, 0);
      const int afterCommandsAmount = resolveResourceAmount(region->getResourcesAfterOrders(), resourceToken, 0);
      rows.push_back(ResourceRow{
        CoordinateFormattingUtils::formatCoordinates(key.x, key.y, key.z),
        resourceToken,
        maxAmount,
        afterCommandsAmount,
        afterCommandsAmount <= 0
          ? RowSeverity::Green
          : afterCommandsAmount == maxAmount ? RowSeverity::Red : RowSeverity::Yellow
      });
    }
  }

  return rows;
}
}
