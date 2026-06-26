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
 * File: UnitCapacityUtils.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

class AppData;
class Unit;
class UnitNew;

namespace UnitCapacityUtils
{

/**
 * @brief Weight and movement-capacity values for a single unit.
 *
 * Weight and the four capacity figures are read directly from the unit's
 * stored report data (Unit::getWeight() / getCapacityWalk() etc.).
 * The three @c has*Source flags indicate whether any item in the unit's
 * current inventory grants that capacity type — this determines which
 * capacity rows the GUI should display.
 */
struct UnitCapacities
{
    int  totalWeight   { 0 };
    int  walkCapacity  { 0 };
    int  rideCapacity  { 0 };
    int  flyCapacity   { 0 };
    int  swimCapacity  { 0 };
    bool hasRideSource { false }; ///< any carried item grants non-zero ride capacity
    bool hasFlySource  { false }; ///< any carried item grants non-zero fly  capacity
    bool hasSwimSource { false }; ///< any carried item grants non-zero swim capacity
};

/**
 * @brief Ship-level capacity figures for a unit that is aboard a ship structure.
 *
 * @c hasCapacityValues is false when the unit is not inside a ship structure
 * (in that case all numeric fields are zero).
 * @c hasOwnerSkillValues is false for UnitNew instances and for units that are
 * not the declared owner of the structure.
 */
struct ShipCapacities
{
    int  shipCapacity          { 0 };   ///< total capacity of the fleet (swim or fly)
    int  shipFreeCapacity      { 0 };   ///< shipCapacity minus combined load of all units aboard
    int  shipSkillNeed         { 0 };   ///< total SAIL skill required to crew the fleet
    int  ownerSailContrib      { 0 };   ///< owner's effective sailing: manCount × SAIL level
    bool hasCapacityValues     { false };
    bool hasOwnerSkillValues   { false }; ///< true when the unit is the declared ship owner
    bool isFlying              { false }; ///< structure->isFlying() — ship is currently airborne
    bool isCapableOfFlying     { false }; ///< ship item has fly capacity > 0 (can ever fly)
};

/**
 * @brief Computes @ref UnitCapacities for an existing unit.
 *
 * Weight and capacity figures are taken from the unit's stored report data.
 * The item repository is consulted only to determine @c hasRideSource,
 * @c hasFlySource, and @c hasSwimSource.
 */
UnitCapacities getUnitCapacities(const Unit& unit, const AppData& appData);

/**
 * @brief Computes @ref UnitCapacities for a new (order-simulated) unit.
 *
 * Identical semantics to the @c Unit overload; uses the UnitNew weight /
 * capacity getters and iterates the new unit's items.
 */
UnitCapacities getUnitCapacities(const UnitNew& unit, const AppData& appData);

/**
 * @brief Computes @ref ShipCapacities for an existing unit that is aboard a ship.
 *
 * Looks up the structure the unit currently occupies, checks whether it is a
 * ship, sums fleet-item capacities and skill requirements from the item
 * repository, and accumulates the combined weight of every unit and new-unit
 * that will be aboard the ship after orders.  If the unit is the declared
 * owner of the structure the @c ownerSailContrib field is populated.
 *
 * Returns a @ref ShipCapacities with @c hasCapacityValues == false when the
 * unit is not inside a ship structure.
 */
ShipCapacities getShipCapacities(const Unit& unit, const AppData& appData);

/**
 * @brief Computes @ref ShipCapacities for a new unit boarding a ship.
 *
 * Identical logic to the @c Unit overload except that @c hasOwnerSkillValues
 * is always false (new units cannot be the declared ship owner).
 */
ShipCapacities getShipCapacities(const UnitNew& unit, const AppData& appData);

} // namespace UnitCapacityUtils
