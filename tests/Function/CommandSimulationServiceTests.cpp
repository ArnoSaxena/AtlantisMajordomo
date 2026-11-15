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
 * File: CommandSimulationServiceTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Function/CommandSimulationService.hpp"
#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitRepository.hpp"
#include "Data/UnitNew.hpp"
#include "Data/UnitNewRepository.hpp"
#include "Data/Order.hpp"
#include "Data/OrderRepository.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/Structure.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/ReportRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructInfoRepository.hpp"

struct CommandSimulationServiceFixture
{
  BattleRepository battles;
  EventRepository events;
  FactionRepository factions;
  ItemRepository items;
  OrderRepository orders;
  RegionRepository regions;
  ReportRepository reports;
  SkillRepository skills;
  StructureRepository structures;
  StructInfoRepository structureInfo;
  UnitRepository units;
  UnitNewRepository newUnits;

  std::vector<std::wstring> emptyFlags;
  std::map<std::wstring, int> emptyItems;
  std::map<std::wstring, int> emptySkills;

  AppData appData{
    AppData::Repositories{
      reports,
      factions,
      items,
      skills,
      regions,
      units,
      newUnits,
      battles,
      events,
      structures,
      structureInfo,
      orders
    }
  };

  Faction* createMainFaction(int unclaimedSilver = 1000)
  {
    factions.add(1, L"Main Faction", true, 0, 0, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, {}, unclaimedSilver);
    return factions.getMainFaction();
  }

  Faction* createSecondaryFaction(int unclaimedSilver = 500)
  {
    factions.add(2, L"Secondary Faction", false, 0, 0, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, {}, unclaimedSilver);
    return factions.findByNumber(2);
  }

  Unit* createUnit(int unitNumber, int factionNumber, int x = 10, int y = 20, int z = 1, int structureId = 0)
  {
    units.add(unitNumber, L"Test Unit", factionNumber, structureId, x, y, z, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
    return units.findByNumber(unitNumber);
  }

  UnitNew* createUnitNew(int unitNumber, int x = 10, int y = 20, int z = 1)
  {
    newUnits.add(unitNumber, L"New Unit", 0, x, y, z, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, 99);
    return newUnits.findByNumberAndCoordinates(unitNumber, x, y, z);
  }

  Region* createRegion(int x = 10, int y = 20, int z = 1)
  {
    regions.add(x, y, z, L"Plain", L"Test Province", false, L"", L"", L"Peasants", 100, 50.0, 25, 0, false);
    return regions.findByCoordinates(x, y, z);
  }

  Structure* createStructure(int id, int ownerId, int x = 10, int y = 20, int z = 1)
  {
    structures.add(id, x, y, z, L"Castle", L"Test Structure", false, 0, 0);
    Structure* structure = structures.findByIdAndCoordinates(id, x, y, z);
    if (structure != nullptr)
    {
      structure->setOwnerUnitId(ownerId);
    }
    return structure;
  }

  void addOrder(int unitNumber, bool isNewUnit, int xCoordinate, int yCoordinate, int zCoordinate, const std::wstring& orderToken, const std::wstring& fullOrderText)
  {
    orders.addOrder(unitNumber, isNewUnit, xCoordinate, yCoordinate, zCoordinate, orderToken, fullOrderText);
    
    if (!isNewUnit)
    {
      Unit* unit = units.findByNumber(unitNumber);
      if (unit)
      {
        unit->addOrder(fullOrderText);
      }
    }
    // Note: UnitNew does not currently support addOrder method - orders are stored in OrderRepository only
  }
};

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService calculateMainFactionUnclaimedSilverAfterCommands no main faction")
{
  int result = CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(appData);
  
  CHECK(result == 0);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService calculateMainFactionUnclaimedSilverAfterCommands with main faction")
{
  Faction* mainFaction = createMainFaction(5000);
  mainFaction->setUnclaimedSilverAfterOrders(4500);
  
  int result = CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(appData);
  
  CHECK(result == 4500);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService calculateMainFactionUnclaimedSilverAfterCommands zero unclaimed silver")
{
  Faction* mainFaction = createMainFaction(1000);
  mainFaction->setUnclaimedSilverAfterOrders(0);
  
  int result = CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(appData);
  
  CHECK(result == 0);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService calculateMainFactionUnclaimedSilverAfterCommands large values")
{
  Faction* mainFaction = createMainFaction(999999);
  mainFaction->setUnclaimedSilverAfterOrders(999000);
  
  int result = CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(appData);
  
  CHECK(result == 999000);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService calculateMainFactionUnclaimedSilverAfterCommands negative after orders")
{
  Faction* mainFaction = createMainFaction(1000);
  mainFaction->setUnclaimedSilverAfterOrders(-500);
  
  int result = CommandSimulationService::calculateMainFactionUnclaimedSilverAfterCommands(appData);
  
  CHECK(result == -500);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects no main faction")
{
  CommandSimulationService::processMainFactionClaimEffects(appData);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects no claims")
{
  Faction* mainFaction = createMainFaction(1000);
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 1000);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects with sufficient silver")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit = createUnit(101, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 200", L"CLAIM 200");
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 800);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects with insufficient silver")
{
  Faction* mainFaction = createMainFaction(500);
  Unit* unit = createUnit(101, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 1000", L"CLAIM 1000");
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == -500);
  REQUIRE(unit != nullptr);
  auto warnings = unit->getWarnings();
  CHECK(std::find(warnings.begin(), warnings.end(), L"CLAIM: insufficient unclaimed silver") != warnings.end());
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects multiple units claiming")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit1 = createUnit(101, 1);
  Unit* unit2 = createUnit(102, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 400", L"CLAIM 400");
  addOrder(102, false, 10, 20, 1, L"CLAIM 400", L"CLAIM 400");
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 200);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects zero claim")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit = createUnit(101, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 0", L"CLAIM 0");
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 1000);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects new unit claiming insufficient")
{
  Faction* mainFaction = createMainFaction(100);
  UnitNew* unitNew = createUnitNew(1000);
  
  addOrder(1000, true, 10, 20, 1, L"CLAIM 500", L"CLAIM 500");
  
  // Debug: verify unitNew was created
  REQUIRE(unitNew != nullptr);
  CHECK(unitNew->getUnitNumber() == 1000);
  
  // Debug: verify the order was added
  auto ordersForUnitNew = appData.orderRepository().getOrderReferencesForUnit(1000, true);
  CHECK(!ordersForUnitNew.empty());
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == -400);
  auto warnings = unitNew->getWarnings();
  CHECK(std::find(warnings.begin(), warnings.end(), L"CLAIM: insufficient unclaimed silver") != warnings.end());
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService processMainFactionClaimEffects exact amount sufficient")
{
  Faction* mainFaction = createMainFaction(500);
  Unit* unit = createUnit(101, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 500", L"CLAIM 500");
  
  CommandSimulationService::processMainFactionClaimEffects(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 0);
  auto warnings = unit->getWarnings();
  CHECK(std::find(warnings.begin(), warnings.end(), L"CLAIM: insufficient unclaimed silver") == warnings.end());
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues no units")
{
  Faction* mainFaction = createMainFaction(1000);
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 1000);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues processes claims")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit = createUnit(101, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM 300", L"CLAIM 300");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 700);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues updates unit derived fields")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit = createUnit(101, 1);
  
  unit->setUnitNameAfterOrders(L"Old Name");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  REQUIRE(unit != nullptr);
  std::wstring name = unit->getUnitNameAfterOrders();
  CHECK(!name.empty());
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues updates new unit derived fields")
{
  Faction* mainFaction = createMainFaction(1000);
  UnitNew* unitNew = createUnitNew(1000, 1);
  
  unitNew->setUnitNameAfterOrders(L"Old Name");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  REQUIRE(unitNew != nullptr);
  std::wstring name = unitNew->getUnitNameAfterOrders();
  CHECK(!name.empty());
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues handles promote order")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Unit* unit2 = createUnit(102, 1, 10, 20, 1, 0);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  REQUIRE(structure->getOwnerUnitId() == 101);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 102");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 102);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote different faction")
{
  Faction* faction1 = createMainFaction(1000);
  Faction* faction2 = createSecondaryFaction(500);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Unit* unit2 = createUnit(102, 2, 10, 20, 1, 0);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 102");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote different location")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Unit* unit2 = createUnit(102, 1, 15, 25, 1, 0);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 102");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote non-existent unit")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 999");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote non-owned structure")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 0);
  Unit* unit2 = createUnit(102, 1, 10, 20, 1, 0);
  Structure* structure = createStructure(50, 999, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 102");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 999);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote invalid unit id")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE -1");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote zero unit id")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 0");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues ignores promote non-integer unit id")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1, 50);
  Structure* structure = createStructure(50, 101, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE abc");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(structure->getOwnerUnitId() == 101);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues with no structure")
{
  Faction* mainFaction = createMainFaction(1000);
  Region* region = createRegion(10, 20, 1);
  Unit* unit1 = createUnit(101, 1, 10, 20, 1);
  Unit* unit2 = createUnit(102, 1, 10, 20, 1);
  
  addOrder(101, false, 10, 20, 1, L"PROMOTE", L"PROMOTE 102");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues processes multiple units")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit1 = createUnit(101, 1);
  Unit* unit2 = createUnit(102, 1);
  Unit* unit3 = createUnit(103, 1);
  
  addOrder(101, false, 10, 20, 1, L"CLAIM", L"CLAIM 100");
  addOrder(102, false, 10, 20, 1, L"CLAIM", L"CLAIM 200");
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
  
  CHECK(mainFaction->getUnclaimedSilverAfterOrders() == 700);
}

TEST_CASE_METHOD(CommandSimulationServiceFixture, "CommandSimulationService recalculateAfterOrdersValues with new units")
{
  Faction* mainFaction = createMainFaction(1000);
  Unit* unit = createUnit(101, 1);
  UnitNew* unitNew = createUnitNew(1000, 1);
  
  CommandSimulationService::recalculateAfterOrdersValues(appData);
}
