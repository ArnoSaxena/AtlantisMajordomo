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

  auto& unitRepository = appData.unitRepository();
  auto& unitNewRepository = appData.unitNewRepository();
  // Reuse Commands-level CLAIM parsing/aggregation so there is one source of truth.
  const Commands::ClaimSummary claimSummary = Commands::summarizeClaims(appData);
  const int totalClaimAmount = claimSummary.totalClaimAmount;

  // Apply post-order value even when insufficient, so UI can reflect deficit state.
  const int remainingUnclaimedSilver = mainFaction->getUnclaimedSilver() - totalClaimAmount;
  mainFaction->setUnclaimedSilverAfterOrders(remainingUnclaimedSilver);

  // If claims fit within available unclaimed silver, no warnings are needed.
  if (totalClaimAmount <= mainFaction->getUnclaimedSilver())
  {
    return;
  }

  // Otherwise, warn every participating issuer.
  for (const Commands::ClaimIssuer& issuer : claimSummary.issuers)
  {
    if (!issuer.isNewUnit)
    {
      Unit* unit = unitRepository.findByNumber(issuer.unitNumber);
      if (unit)
      {
        unit->addWarning(L"CLAIM: insufficient unclaimed silver");
      }
      continue;
    }

    UnitNew* unitNew = unitNewRepository.findByNumberAndCoordinates(
      issuer.unitNumber,
      issuer.xCoordinate,
      issuer.yCoordinate,
      issuer.zCoordinate);
    if (unitNew)
    {
      unitNew->addWarning(L"CLAIM: insufficient unclaimed silver");
    }
  }
}

void CommandSimulationService::recalculateAfterOrdersValues(AppData& appData)
{
  // CLAIM directly affects faction-level after-orders silver and warning state.
  processMainFactionClaimEffects(appData);

  // Recompute unit-level derived fields from current data + parsed orders.
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

  // Recompute UnitNew derived fields; lookup by (number, coordinates) to avoid ambiguity.
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

  // Recompute region-level resources, market, and economy after all commands.
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
    // Write economy breakdown from one calculation pass to keep values consistent.
    region->setEntertainmentAfterOrders(economy.remainingEntertainment);
    region->setTaxableIncomeAfterOrders(economy.remainingTaxableIncome);
    region->setWagesAfterOrders(economy.remainingWorkWages);
  }

}
