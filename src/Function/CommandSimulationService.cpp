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
 * File: CommandSimulationService.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/CommandSimulationService.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/Commands.hpp"
#include "Data/Region.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <set>
#include <tuple>

namespace
{
bool tryParseClaimAmount(const std::wstring& orderLine, int& claimAmount)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!OrderParsingUtils::tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return false;
  }

  std::size_t tokenIndex = 0;
  if (!tokens[0].empty() && tokens[0][0] == L'@')
  {
    if (tokens[0].size() > 1)
    {
      tokens[0] = tokens[0].substr(1);
    }
    else
    {
      ++tokenIndex;
    }
  }

  if (tokenIndex >= tokens.size() ||  StringUtils::toUpper(tokens[tokenIndex]) != L"CLAIM")
  {
    return false;
  }

  if ((tokenIndex + 1) >= tokens.size())
  {
    return false;
  }

  int parsedAmount = 0;
  if (!OrderParsingUtils::tryParseIntStrict(tokens[tokenIndex + 1], parsedAmount) || parsedAmount <= 0)
  {
    return false;
  }

  claimAmount = parsedAmount;
  return true;
}
}

int CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(const AppData& appData)
{
  const Faction* mainFaction = appData.factionRepository().getMainFaction();
  if (!mainFaction)
  {
    return 0;
  }

  return mainFaction->getUnclaimedSilverAfterOrders();
}

void CommandSimulationService::processMainFactionClaimEffects(AppData& appData)
{
  Faction* mainFaction = appData.factionRepository().getMainFaction();
  if (!mainFaction)
  {
    return;
  }

  int totalClaimAmount = 0;
  std::set<int> claimingUnitNumbers;
  std::set<std::tuple<int, int, int, int>> claimingNewUnitKeys;
  const auto& orderRepository = appData.orderRepository();

  const auto collectClaimsForOrders = [&](const std::vector<Order>* orders,
                                          int unitNumber,
                                          bool fromNewUnit,
                                          int xCoordinate,
                                          int yCoordinate,
                                          int zCoordinate)
  {
    if (!orders)
    {
      return;
    }

    bool hasClaimOrder = false;
    for (const Order& order : *orders)
    {
      int claimAmount = 0;
      if (!tryParseClaimAmount(order.getFullOrderText(), claimAmount))
      {
        continue;
      }

      totalClaimAmount += claimAmount;
      hasClaimOrder = true;
    }

    if (!hasClaimOrder)
    {
      return;
    }

    if (fromNewUnit)
    {
      claimingNewUnitKeys.insert(std::make_tuple(unitNumber, xCoordinate, yCoordinate, zCoordinate));
    }
    else
    {
      claimingUnitNumbers.insert(unitNumber);
    }
  };

  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& unit = unitRepository.at(index);
    collectClaimsForOrders(orderRepository.getOrdersForUnit(unit.getUnitNumber(), false),
                           unit.getUnitNumber(),
                           false,
                           unit.getXCoordinate(),
                           unit.getYCoordinate(),
                           unit.getZCoordinate());
  }

  auto& unitNewRepository = appData.unitNewRepository();
  for (std::size_t index = 0; index < unitNewRepository.size(); ++index)
  {
    const UnitNew& unitNew = unitNewRepository.at(index);
    collectClaimsForOrders(orderRepository.getOrdersForUnit(unitNew.getUnitNumber(), true),
                           unitNew.getUnitNumber(),
                           true,
                           unitNew.getXCoordinate(),
                           unitNew.getYCoordinate(),
                           unitNew.getZCoordinate());
  }

  const int remainingUnclaimedSilver = mainFaction->getUnclaimedSilver() - totalClaimAmount;
  mainFaction->setUnclaimedSilverAfterOrders(remainingUnclaimedSilver);

  if (totalClaimAmount <= mainFaction->getUnclaimedSilver())
  {
    return;
  }

  for (const int unitNumber : claimingUnitNumbers)
  {
    Unit* unit = unitRepository.findByNumber(unitNumber);
    if (unit)
    {
      unit->addWarning(L"CLAIM: insufficient unclaimed silver");
    }
  }

  for (const auto& [unitNumber, xCoordinate, yCoordinate, zCoordinate] : claimingNewUnitKeys)
  {
    UnitNew* unitNew = unitNewRepository.findByNumberAndCoordinates(unitNumber, xCoordinate, yCoordinate, zCoordinate);
    if (unitNew)
    {
      unitNew->addWarning(L"CLAIM: insufficient unclaimed silver");
    }
  }
}

void CommandSimulationService::recalculateAfterOrdersValues(AppData& appData)
{
  processMainFactionClaimEffects(appData);

  for (std::size_t unitIndex = 0; unitIndex < appData.unitRepository().size(); ++unitIndex)
  {
    const int unitNumber = appData.unitRepository().at(unitIndex).getUnitNumber();
    Unit* unit = appData.unitRepository().findByNumber(unitNumber);
    if (!unit)
    {
      continue;
    }

    unit->setUnitNameAfterOrders(Commands::calculateAfterCommandUnitNameForUnit(appData, *unit));
    unit->setItemsAfterOrders(Commands::calculateAfterCommandItemCountsForUnit(appData, *unit));
    unit->setSkillsAfterOrders(Commands::calculateAfterCommandSkillDaysForUnit(appData, *unit));
  }

  for (std::size_t unitNewIndex = 0; unitNewIndex < appData.unitNewRepository().size(); ++unitNewIndex)
  {
    const UnitNew& unitNewSnapshot = appData.unitNewRepository().at(unitNewIndex);
    UnitNew* unitNew = appData.unitNewRepository().findByNumberAndCoordinates(
      unitNewSnapshot.getUnitNumber(),
      unitNewSnapshot.getXCoordinate(),
      unitNewSnapshot.getYCoordinate(),
      unitNewSnapshot.getZCoordinate());
    if (!unitNew)
    {
      continue;
    }

    unitNew->setUnitNameAfterOrders(Commands::calculateAfterCommandUnitNameForUnitNew(appData, *unitNew));
    unitNew->setItemsAfterOrders(Commands::calculateAfterCommandItemCountsForUnitNew(appData, *unitNew));
    unitNew->setSkillsAfterOrders(Commands::calculateAfterCommandSkillDaysForUnitNew(appData, *unitNew));
  }

  for (std::size_t regionIndex = 0; regionIndex < appData.regionRepository().size(); ++regionIndex)
  {
    const Region& regionSnapshot = appData.regionRepository().at(regionIndex);
    Region* region = appData.regionRepository().findByCoordinates(
      regionSnapshot.getXCoordinate(),
      regionSnapshot.getYCoordinate(),
      regionSnapshot.getZCoordinate());
    if (!region)
    {
      continue;
    }

    region->setResourcesAfterOrders(Commands::calculateAfterCommandRegionResources(appData, *region));
    region->setForSaleAfterOrders(Commands::calculateAfterCommandRegionForSale(appData, *region));
    region->setWantedAfterOrders(Commands::calculateAfterCommandRegionWanted(appData, *region));

    const Commands::RegionEconomyAfterCommands economy =
      Commands::calculateAfterCommandRegionEconomy(appData, *region);
    region->setEntertainmentAfterOrders(economy.remainingEntertainment);
    region->setTaxableIncomeAfterOrders(economy.remainingTaxableIncome);
    region->setWagesAfterOrders(economy.remainingWorkWages);
  }

}
