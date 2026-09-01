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
 * File: UnitCapacityUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "Function/UnitCapacityUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/Skill.hpp"
#include "Data/StructInfo.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/Structure.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Data/UnitNewRepository.hpp"
#include "Data/UnitRepository.hpp"

#include <map>
#include <string>

namespace
{

// ---------------------------------------------------------------------------
// Determine which structure a unit will effectively be inside after orders.
// Returns:
//   futureStructureId  when > 0 (unit is boarding a new structure)
//   0                  when futureStructureId == 0 (unit is leaving its structure)
//   currentStructureId otherwise (unit stays put)
// ---------------------------------------------------------------------------
int effectiveStructureId(int currentStructureId, int futureStructureId)
{
    if (futureStructureId > 0)
        return futureStructureId;
    if (futureStructureId == 0)
        return 0;
    return currentStructureId;
}

// ---------------------------------------------------------------------------
// Sum of weights of every unit (existing and new) that will be aboard the
// given structure after orders.
// ---------------------------------------------------------------------------
int combinedStructureWeight(int structureId, int x, int y, int z,
                             const AppData& appData)
{
    int total = 0;
    const auto& itemRepo = appData.itemRepository();

    const auto& unitRepo = appData.unitRepository();
    for (std::size_t i = 0; i < unitRepo.size(); ++i)
    {
        const Unit& u = unitRepo.at(i);
        if (u.getXCoordinate() != x || u.getYCoordinate() != y || u.getZCoordinate() != z)
            continue;
        if (effectiveStructureId(u.getStructureId(), u.getFutureStructureId()) != structureId)
            continue;

        const auto& items = u.getItemsAfterOrders().empty()
                            ? u.getItems()
                            : u.getItemsAfterOrders();
        total += itemRepo.calculateTotalWeight(items);
    }

    const auto& unitNewRepo = appData.unitNewRepository();
    for (std::size_t i = 0; i < unitNewRepo.size(); ++i)
    {
        const UnitNew& u = unitNewRepo.at(i);
        if (u.getXCoordinate() != x || u.getYCoordinate() != y || u.getZCoordinate() != z)
            continue;
        if (u.getFutureStructureId() != structureId)
            continue;

        const auto& items = u.getItemsAfterOrders().empty()
                            ? u.getItems()
                            : u.getItemsAfterOrders();
        total += itemRepo.calculateTotalWeight(items);
    }

    return total;
}

// ---------------------------------------------------------------------------
// Compute fleet capacity and skill need from a structure's fleet items.
// Uses swim capacity unless the structure is currently flying.
// ---------------------------------------------------------------------------
struct FleetResult { int capacity; int skillNeed; bool hasValues; bool isCapableOfFlying; };

FleetResult computeFleetCapacity(const Structure& structure,
                                  const ItemRepository& itemRepo)
{
    FleetResult result{};
    const bool flying = structure.isFlying();
    const auto& fleetItems = structure.getFleetItems();

    if (fleetItems.empty())
        return result;

    for (const auto& entry : fleetItems)
    {
        const int amount = entry.second;
        if (amount <= 0)
            continue;
        const Item* item = itemRepo.findByIdentifierToken(entry.first);
        if (!item)
            continue;
        result.capacity          += (flying ? item->getFlyCapacity()  : item->getSwimCapacity()) * amount;
        result.skillNeed         += item->getShipSailingSkillRequired() * amount;
        result.hasValues          = true;
        if (item->getFlyCapacity() > 0)
            result.isCapableOfFlying = true;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Populate ShipCapacities for a structure (shared between Unit and UnitNew).
// ownerUnitNumber = unit number to check against structure.getOwnerUnitId();
//                   pass -1 for UnitNew (never an owner).
// ownerSkillDays  = unit's SAIL skill training days (ignored when ownerUnitNumber == -1).
// ownerManCount   = number of man-items the unit carries (ignored same condition).
// ---------------------------------------------------------------------------
UnitCapacityUtils::ShipCapacities buildShipCapacities(
    int ownerUnitNumber,
    int ownerSkillDays,
    int ownerManCount,
    int structureId, int x, int y, int z,
    const AppData& appData)
{
    UnitCapacityUtils::ShipCapacities result{};

    if (structureId <= 0)
        return result;

    const Structure* structure = appData.structureRepository()
        .findByIdAndCoordinates(structureId, x, y, z);
    if (!structure)
        return result;

    const StructInfo* structInfo = appData.structInfoRepository()
        .findByType(structure->getStructureType());
    if (!structInfo || !structInfo->isShip())
        return result;

    result.isFlying = structure->isFlying();

    const FleetResult fleet = computeFleetCapacity(*structure, appData.itemRepository());

    if (!fleet.hasValues)
    {
        // Fall back to the single ship-item definition from StructInfo.
        const std::wstring& token = structInfo->getItemIdentifierToken();
        const Item* shipItem = token.empty()
            ? appData.itemRepository().findByItemName(structure->getStructureName())
            : appData.itemRepository().findByIdentifierToken(token);

        if (shipItem)
        {
            result.shipCapacity      = result.isFlying
                                       ? shipItem->getFlyCapacity()
                                       : shipItem->getSwimCapacity();
            result.shipSkillNeed     = shipItem->getShipSailingSkillRequired();
            result.hasCapacityValues = true;
            result.isCapableOfFlying = (shipItem->getFlyCapacity() > 0);
        }
    }
    else
    {
        result.shipCapacity      = fleet.capacity;
        result.shipSkillNeed     = fleet.skillNeed;
        result.hasCapacityValues = true;
        result.isCapableOfFlying = fleet.isCapableOfFlying;
    }

    if (result.hasCapacityValues)
    {
        const int load           = combinedStructureWeight(structureId, x, y, z, appData);
        result.shipFreeCapacity  = result.shipCapacity - load;
    }

    // Owner sailing contribution (Unit only; UnitNew passes ownerUnitNumber == -1)
    if (ownerUnitNumber >= 0 && structure->getOwnerUnitId() == ownerUnitNumber)
    {
        result.ownerSailContrib    = ownerManCount * Skill::trainingDaysToLevel(ownerSkillDays);
        result.hasOwnerSkillValues = true;
    }

    return result;
}

// ---------------------------------------------------------------------------
// Build UnitCapacities by computing from the unit's projected item inventory.
//
// Gross capacity for type C = Σ (item.weight + item.C) × count
//                               for every item where item.C > 0.
// Free capacity for type C  = gross C − total weight.
//
// Uses getItemsAfterOrders() when non-empty so that order-simulation results
// are reflected; falls back to getItems() for units without pending orders.
// ---------------------------------------------------------------------------
template <typename UnitType>
UnitCapacityUtils::UnitCapacities buildUnitCapacities(const UnitType& unit,
                                                        const AppData& appData)
{
    const auto& itemRepo = appData.itemRepository();

    const auto& items = unit.getItemsAfterOrders().empty()
                        ? unit.getItems()
                        : unit.getItemsAfterOrders();

    UnitCapacityUtils::UnitCapacities result;
    result.totalWeight = itemRepo.calculateTotalWeight(items);

    int walkGross = 0;
    int rideGross = 0;
    int flyGross  = 0;
    int swimGross = 0;

    for (const auto& entry : items)
    {
        const int count = entry.second;
        if (count <= 0)
            continue;
        const Item* item = itemRepo.findByIdentifierToken(entry.first);
        if (!item)
            continue;

        const int w = item->getWeight();

        if (item->getWalkCapacity() > 0)
            walkGross += (w + item->getWalkCapacity()) * count;

        if (item->getRideCapacity() > 0)
        {
            rideGross += (w + item->getRideCapacity()) * count;
            result.hasRideSource = true;
        }
        if (item->getFlyCapacity() > 0)
        {
            flyGross += (w + item->getFlyCapacity()) * count;
            result.hasFlySource = true;
        }
        if (item->getSwimCapacity() > 0)
        {
            swimGross += (w + item->getSwimCapacity()) * count;
            result.hasSwimSource = true;
        }
    }

    result.walkCapacity = walkGross - result.totalWeight;
    result.rideCapacity = rideGross - result.totalWeight;
    result.flyCapacity  = flyGross  - result.totalWeight;
    result.swimCapacity = swimGross - result.totalWeight;

    return result;
}

} // namespace

namespace UnitCapacityUtils
{

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

UnitCapacities getUnitCapacities(const Unit& unit, const AppData& appData)
{
    return buildUnitCapacities(unit, appData);
}

UnitCapacities getUnitCapacities(const UnitNew& unit, const AppData& appData)
{
    return buildUnitCapacities(unit, appData);
}

ShipCapacities getShipCapacities(const Unit& unit, const AppData& appData)
{
    const auto& items = unit.getItemsAfterOrders().empty() ? unit.getItems() : unit.getItemsAfterOrders();
    const int manCount = appData.itemRepository().calculateManItemCount(items);
    const int sailDays = unit.getSkillDays(L"SAIL");
    const int structureId = unit.getFutureStructureId() > 0
        ? unit.getFutureStructureId()
        : unit.getStructureId();
    return buildShipCapacities(
        unit.getUnitNumber(),
        sailDays,
        manCount,
        structureId,
        unit.getXCoordinate(),
        unit.getYCoordinate(),
        unit.getZCoordinate(),
        appData);
}

ShipCapacities getShipCapacities(const UnitNew& unit, const AppData& appData)
{
    // UnitNew units cannot be the declared owner of a ship; pass -1 to skip
    // the owner-sailing branch.
    return buildShipCapacities(
        -1, 0, 0,
        unit.getStructureId(),
        unit.getXCoordinate(),
        unit.getYCoordinate(),
        unit.getZCoordinate(),
        appData);
}

} // namespace UnitCapacityUtils
