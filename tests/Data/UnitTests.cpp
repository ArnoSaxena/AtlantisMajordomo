/*
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo - Unit Tests
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
 * File: UnitTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/Unit.hpp"

struct UnitFixture
{
  std::map<std::wstring, int> items { { L"SILV", 500 }, { L"IRON", 100 } };
  std::map<std::wstring, int> skills { { L"FIRE", 30 } };
  std::vector<std::wstring> flags { L"behind" };
  
  Unit unit { 1000, L"Fighters", 5, 10, 1, 2, 1, flags, items, 500, 10000, 5000, 0, 0, skills, 8, 7, false };
};

TEST_CASE_METHOD(UnitFixture, "Unit can be constructed with all parameters", "[Unit]")
{
  CHECK(unit.getUnitNumber() == 1000);
  CHECK(unit.getUnitName() == L"Fighters");
  CHECK(unit.getFactionNumber() == 5);
  CHECK(unit.getStructureId() == 10);
  CHECK(unit.getXCoordinate() == 1);
  CHECK(unit.getYCoordinate() == 2);
  CHECK(unit.getZCoordinate() == 1);
  CHECK(unit.getWeight() == 500);
  CHECK(unit.getCapacityWalk() == 10000);
  CHECK(unit.getCapacityRide() == 5000);
  CHECK(unit.getMonth() == 8);
  CHECK(unit.getYear() == 7);
  CHECK_FALSE(unit.isOnGuard());
}

TEST_CASE_METHOD(UnitFixture, "Unit flags are stored", "[Unit]")
{
  const auto& flags = unit.getFlags();
  CHECK(flags.size() == 1);
  CHECK(flags[0] == L"behind");
}

TEST_CASE_METHOD(UnitFixture, "Unit items are stored", "[Unit]")
{
  const auto& items = unit.getItems();
  CHECK(items.size() == 2);
  CHECK(items.at(L"SILV") == 500);
  CHECK(items.at(L"IRON") == 100);
}

TEST_CASE_METHOD(UnitFixture, "Unit skills are stored", "[Unit]")
{
  const auto& skills = unit.getSkills();
  CHECK(skills.size() == 1);
  CHECK(skills.at(L"FIRE") == 30);
}

TEST_CASE_METHOD(UnitFixture, "Unit name can be set", "[Unit]")
{
  unit.setUnitName(L"Archers");
  CHECK(unit.getUnitName() == L"Archers");
}

TEST_CASE_METHOD(UnitFixture, "Unit name after orders can be set", "[Unit]")
{
  unit.setUnitNameAfterOrders(L"Disbanded");
  CHECK(unit.getUnitNameAfterOrders() == L"Disbanded");
}

TEST_CASE_METHOD(UnitFixture, "Unit faction number can be set", "[Unit]")
{
  unit.setFactionNumber(10);
  CHECK(unit.getFactionNumber() == 10);
}

TEST_CASE_METHOD(UnitFixture, "Unit structure id can be set", "[Unit]")
{
  unit.setStructureId(20);
  CHECK(unit.getStructureId() == 20);
}

TEST_CASE_METHOD(UnitFixture, "Unit future structure id can be set", "[Unit]")
{
  unit.setFutureStructureId(30);
  CHECK(unit.getFutureStructureId() == 30);
}

TEST_CASE_METHOD(UnitFixture, "Unit coordinates can be set", "[Unit]")
{
  unit.setCoordinates(3, 4, 2);
  CHECK(unit.getXCoordinate() == 3);
  CHECK(unit.getYCoordinate() == 4);
  CHECK(unit.getZCoordinate() == 2);
}

TEST_CASE_METHOD(UnitFixture, "Unit weight can be set", "[Unit]")
{
  unit.setWeight(1000);
  CHECK(unit.getWeight() == 1000);
}

TEST_CASE_METHOD(UnitFixture, "Unit capacities can be set", "[Unit]")
{
  unit.setCapacityWalk(15000);
  unit.setCapacityRide(8000);
  unit.setCapacityFly(2000);
  unit.setCapacitySwim(5000);
  
  CHECK(unit.getCapacityWalk() == 15000);
  CHECK(unit.getCapacityRide() == 8000);
  CHECK(unit.getCapacityFly() == 2000);
  CHECK(unit.getCapacitySwim() == 5000);
}

TEST_CASE_METHOD(UnitFixture, "Unit month and year can be set", "[Unit]")
{
  unit.setMonth(9);
  unit.setYear(10);
  
  CHECK(unit.getMonth() == 9);
  CHECK(unit.getYear() == 10);
}

TEST_CASE_METHOD(UnitFixture, "Unit on guard flag can be set", "[Unit]")
{
  unit.setOnGuard(true);
  CHECK(unit.isOnGuard());
}

TEST_CASE_METHOD(UnitFixture, "Unit can add flag", "[Unit]")
{
  unit.clearFlags();
  unit.addFlag(L"avoid");
  
  const auto& flags = unit.getFlags();
  CHECK(flags.size() == 1);
  CHECK(flags[0] == L"avoid");
}

TEST_CASE_METHOD(UnitFixture, "Unit can add multiple flags", "[Unit]")
{
  unit.clearFlags();
  unit.addFlag(L"avoid");
  unit.addFlag(L"behind");
  
  const auto& flags = unit.getFlags();
  CHECK(flags.size() == 2);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear flags", "[Unit]")
{
  unit.clearFlags();
  CHECK(unit.getFlags().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit can set items", "[Unit]")
{
  std::map<std::wstring, int> newItems { { L"GOLD", 1000 } };
  unit.setItems(newItems);
  
  const auto& items = unit.getItems();
  CHECK(items.size() == 1);
  CHECK(items.at(L"GOLD") == 1000);
}

TEST_CASE_METHOD(UnitFixture, "Unit can set single item", "[Unit]")
{
  unit.setItem(L"SILV", 1000);
  
  const auto& items = unit.getItems();
  CHECK(items.at(L"SILV") == 1000);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear items", "[Unit]")
{
  unit.clearItems();
  CHECK(unit.getItems().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit can set skills", "[Unit]")
{
  std::map<std::wstring, int> newSkills { { L"COMBAT", 60 }, { L"ARCHERY", 45 } };
  unit.setSkills(newSkills);
  
  const auto& skills = unit.getSkills();
  CHECK(skills.size() == 2);
  CHECK(skills.at(L"COMBAT") == 60);
}

TEST_CASE_METHOD(UnitFixture, "Unit can add skill", "[Unit]")
{
  unit.addSkill(L"ARCHERY", 30);
  
  const auto& skills = unit.getSkills();
  CHECK(skills.count(L"ARCHERY") > 0);
  CHECK(skills.at(L"ARCHERY") == 30);
}

TEST_CASE_METHOD(UnitFixture, "Unit can get skill days", "[Unit]")
{
  CHECK(unit.getSkillDays(L"FIRE") == 30);
  CHECK(unit.getSkillDays(L"UNKNOWN") == 0);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear skills", "[Unit]")
{
  unit.clearSkills();
  CHECK(unit.getSkills().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit can add order", "[Unit]")
{
  unit.addOrder(L"MOVE 1 2 1");
  
  const auto& orders = unit.getOrders();
  CHECK(orders.size() >= 1);
}

TEST_CASE_METHOD(UnitFixture, "Unit can add warning", "[Unit]")
{
  unit.addWarning(L"Not enough silver");
  
  const auto& warnings = unit.getWarnings();
  CHECK(warnings.size() >= 1);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear warnings", "[Unit]")
{
  unit.clearWarnings();
  CHECK(unit.getWarnings().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit can set can study skill tokens", "[Unit]")
{
  std::vector<std::wstring> skills { L"FIRE", L"COLD" };
  unit.setCanStudySkillTokens(skills);
  
  const auto& tokens = unit.getCanStudySkillTokens();
  CHECK(tokens.size() == 2);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear can study skill tokens", "[Unit]")
{
  unit.clearCanStudySkillTokens();
  CHECK(unit.getCanStudySkillTokens().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit can set orders", "[Unit]")
{
  std::vector<std::wstring> orders { L"MOVE 1 2 1", L"STUDY FIRE" };
  unit.setOrders(orders);
  
  const auto& retrieved = unit.getOrders();
  CHECK(retrieved.size() == 2);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear orders", "[Unit]")
{
  unit.clearOrders();
  CHECK(unit.getOrders().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit items after orders can be set", "[Unit]")
{
  std::map<std::wstring, int> itemsAfter { { L"SILV", 400 } };
  unit.setItemsAfterOrders(itemsAfter);
  
  const auto& items = unit.getItemsAfterOrders();
  CHECK(items.size() == 1);
  CHECK(items.at(L"SILV") == 400);
}

TEST_CASE_METHOD(UnitFixture, "Unit can set single item after orders", "[Unit]")
{
  unit.setItemAfterOrders(L"SILV", 600);
  
  const auto& items = unit.getItemsAfterOrders();
  CHECK(items.at(L"SILV") == 600);
}

TEST_CASE_METHOD(UnitFixture, "Unit skills after orders can be set", "[Unit]")
{
  std::map<std::wstring, int> skillsAfter { { L"FIRE", 40 } };
  unit.setSkillsAfterOrders(skillsAfter);
  
  const auto& skills = unit.getSkillsAfterOrders();
  CHECK(skills.at(L"FIRE") == 40);
}

TEST_CASE_METHOD(UnitFixture, "Unit can add skill after orders", "[Unit]")
{
  unit.addSkillAfterOrders(L"ARCHERY", 25);
  
  const auto& skills = unit.getSkillsAfterOrders();
  CHECK(skills.count(L"ARCHERY") > 0);
}

TEST_CASE_METHOD(UnitFixture, "Unit can get skill days after orders", "[Unit]")
{
  unit.setSkillsAfterOrders({ { L"FIRE", 40 } });
  CHECK(unit.getSkillsAfterOrdersDays(L"FIRE") == 40);
}

TEST_CASE_METHOD(UnitFixture, "Unit can clear skills after orders", "[Unit]")
{
  unit.clearSkillsAfterOrders();
  CHECK(unit.getSkillsAfterOrders().empty());
}

TEST_CASE_METHOD(UnitFixture, "Unit supports copy construction", "[Unit]")
{
  Unit copy(unit);
  CHECK(copy.getUnitNumber() == 1000);
  CHECK(copy.getUnitName() == L"Fighters");
  CHECK(copy.getFactionNumber() == 5);
  CHECK(copy.getWeight() == 500);
}

TEST_CASE_METHOD(UnitFixture, "Unit supports move construction", "[Unit]")
{
  Unit moved(std::move(unit));
  CHECK(moved.getUnitNumber() == 1000);
  CHECK(moved.getUnitName() == L"Fighters");
}

TEST_CASE_METHOD(UnitFixture, "Unit supports copy assignment", "[Unit]")
{
  std::map<std::wstring, int> emptyItems;
  std::map<std::wstring, int> emptySkills;
  std::vector<std::wstring> emptyFlags;
  
  Unit other { 2000, L"Archers", 6, 20, 3, 4, 2, emptyFlags, emptyItems, 400, 8000, 3000, 0, 0, emptySkills, 9, 8, true };
  
  other = unit;
  
  CHECK(other.getUnitNumber() == 1000);
  CHECK(other.getUnitName() == L"Fighters");
}

TEST_CASE_METHOD(UnitFixture, "Unit with zero capacity values", "[Unit]")
{
  unit.setCapacityWalk(0);
  unit.setCapacityRide(0);
  unit.setCapacityFly(0);
  unit.setCapacitySwim(0);
  
  CHECK(unit.getCapacityWalk() == 0);
  CHECK(unit.getCapacityRide() == 0);
  CHECK(unit.getCapacityFly() == 0);
  CHECK(unit.getCapacitySwim() == 0);
}

TEST_CASE_METHOD(UnitFixture, "Unit with multiple items", "[Unit]")
{
  std::map<std::wstring, int> items {
    { L"SILV", 1000 },
    { L"IRON", 500 },
    { L"WOOD", 200 },
    { L"STONE", 150 }
  };
  unit.setItems(items);
  
  CHECK(unit.getItems().size() == 4);
}

TEST_CASE_METHOD(UnitFixture, "Unit with multiple skills", "[Unit]")
{
  std::map<std::wstring, int> skills {
    { L"FIRE", 30 },
    { L"COMBAT", 50 },
    { L"ARCHERY", 45 },
    { L"TACTICS", 20 }
  };
  unit.setSkills(skills);
  
  CHECK(unit.getSkills().size() == 4);
  CHECK(unit.getSkillDays(L"COMBAT") == 50);
  CHECK(unit.getSkillDays(L"ARCHERY") == 45);
}
