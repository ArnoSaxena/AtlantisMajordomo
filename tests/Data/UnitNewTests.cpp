/*
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo - UnitNew Tests
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
 * File: UnitNewTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/UnitNew.hpp"

struct UnitNewFixture
{
  int unitNumber = 1000;
  std::wstring unitName = L"New Fighters";
  int structureId = 5;
  int x = 10;
  int y = 20;
  int z = 1;
  std::vector<std::wstring> emptyFlags;
  std::map<std::wstring, int> emptyItems;
  int weight = 100;
  int capWalk = 500;
  int capRide = 1000;
  int capFly = 0;
  int capSwim = 200;
  std::map<std::wstring, int> emptySkills;
  int month = 1;
  int year = 1;
  int originUnit = 1001;
};

TEST_CASE_METHOD(UnitNewFixture, "UnitNew constructor initializes members", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems, 
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  
  CHECK(unit.getUnitNumber() == unitNumber);
  CHECK(unit.getUnitName() == unitName);
  CHECK(unit.getStructureId() == structureId);
  CHECK(unit.getXCoordinate() == x);
  CHECK(unit.getYCoordinate() == y);
  CHECK(unit.getZCoordinate() == z);
  CHECK(unit.getOriginUnit() == originUnit);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getUnitName returns name", "[UnitNew]")
{
  UnitNew unit(unitNumber, L"Archers", structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getUnitName() == L"Archers");
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setUnitName sets name", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setUnitName(L"Updated Fighters");
  CHECK(unit.getUnitName() == L"Updated Fighters");
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getXCoordinate returns x", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, 15, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getXCoordinate() == 15);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setCoordinates sets coordinates", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setCoordinates(25, 35, 2);
  CHECK(unit.getXCoordinate() == 25);
  CHECK(unit.getYCoordinate() == 35);
  CHECK(unit.getZCoordinate() == 2);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getYCoordinate returns y", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, 30, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getYCoordinate() == 30);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getZCoordinate returns z", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, 2, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getZCoordinate() == 2);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getOriginUnit returns origin unit", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, 1002);
  CHECK(unit.getOriginUnit() == 1002);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setOriginUnit sets origin unit", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setOriginUnit(1005);
  CHECK(unit.getOriginUnit() == 1005);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getStructureId returns structure", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, 10, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getStructureId() == 10);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setStructureId sets structure", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setStructureId(20);
  CHECK(unit.getStructureId() == 20);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getFlags returns flags", "[UnitNew]")
{
  std::vector<std::wstring> flags{L"Guard", L"Hold"};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, flags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getFlags().size() == 2);
  CHECK(((unit.getFlags()[0] == L"Guard") || (unit.getFlags()[1] == L"Guard")));
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew addFlag adds flag", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.addFlag(L"Hold");
  
  const auto& flags = unit.getFlags();
  CHECK(std::find(flags.begin(), flags.end(), L"Hold") != flags.end());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew clearFlags removes all flags", "[UnitNew]")
{
  std::vector<std::wstring> flags{L"Guard", L"Hold"};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, flags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.clearFlags();
  CHECK(unit.getFlags().empty());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getItems returns items", "[UnitNew]")
{
  std::map<std::wstring, int> items{{L"Horse", 10}, {L"Sword", 20}};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, items,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getItems().size() == 2);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setItem sets item quantity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setItem(L"Horse", 15);
  
  CHECK(unit.getItems().count(L"Horse") == 1);
  CHECK(unit.getItems().at(L"Horse") == 15);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew clearItems removes all items", "[UnitNew]")
{
  std::map<std::wstring, int> items{{L"Horse", 10}, {L"Sword", 20}};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, items,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.clearItems();
  CHECK(unit.getItems().empty());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getWeight returns weight", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               250, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getWeight() == 250);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setWeight sets weight", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setWeight(300);
  CHECK(unit.getWeight() == 300);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getCapacityWalk returns walk capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, 1000, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getCapacityWalk() == 1000);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setCapacityWalk sets walk capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setCapacityWalk(1500);
  CHECK(unit.getCapacityWalk() == 1500);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getCapacityRide returns ride capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, 2000, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getCapacityRide() == 2000);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setCapacityRide sets ride capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setCapacityRide(2500);
  CHECK(unit.getCapacityRide() == 2500);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getCapacityFly returns fly capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, 3000, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getCapacityFly() == 3000);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setCapacityFly sets fly capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setCapacityFly(3500);
  CHECK(unit.getCapacityFly() == 3500);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getCapacitySwim returns swim capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, 4000, emptySkills, month, year, originUnit);
  CHECK(unit.getCapacitySwim() == 4000);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setCapacitySwim sets swim capacity", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setCapacitySwim(450);
  CHECK(unit.getCapacitySwim() == 450);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getSkills returns skills", "[UnitNew]")
{
  std::map<std::wstring, int> skills{{L"Combat", 3}, {L"Magic", 1}};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, skills, month, year, originUnit);
  CHECK(unit.getSkills().size() == 2);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew addSkill adds skill", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.addSkill(L"Combat", 30);
  
  CHECK(unit.getSkills().count(L"Combat") == 1);
  CHECK(unit.getSkills().at(L"Combat") == 30);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew clearSkills removes all skills", "[UnitNew]")
{
  std::map<std::wstring, int> skills{{L"Combat", 3}, {L"Magic", 1}};
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, skills, month, year, originUnit);
  unit.clearSkills();
  CHECK(unit.getSkills().empty());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getMonth returns month", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, 6, year, originUnit);
  CHECK(unit.getMonth() == 6);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setMonth sets month", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setMonth(8);
  CHECK(unit.getMonth() == 8);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getYear returns year", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, 50, originUnit);
  CHECK(unit.getYear() == 50);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setYear sets year", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setYear(75);
  CHECK(unit.getYear() == 75);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew isOnGuard returns false by default", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK_FALSE(unit.isOnGuard());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew getWarnings returns warnings", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.addWarning(L"Low supplies");
  
  const auto& warnings = unit.getWarnings();
  CHECK(std::find(warnings.begin(), warnings.end(), L"Low supplies") != warnings.end());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew addWarning adds warning", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.addWarning(L"Morale issue");
  
  CHECK(unit.getWarnings().size() == 1);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew clearWarnings removes all warnings", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.addWarning(L"Issue 1");
  unit.addWarning(L"Issue 2");
  unit.clearWarnings();
  CHECK(unit.getWarnings().empty());
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew copy constructor", "[UnitNew]")
{
  std::vector<std::wstring> flags{L"Hold"};
  std::map<std::wstring, int> items{{L"Horse", 10}};
  std::map<std::wstring, int> skills{{L"Combat", 3}};
  
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, flags, items,
               weight, capWalk, capRide, capFly, capSwim, skills, month, year, originUnit);
  
  UnitNew copy(unit);
  
  CHECK(copy.getUnitNumber() == unitNumber);
  CHECK(copy.getUnitName() == unitName);
  CHECK(copy.getXCoordinate() == x);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew move constructor", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  
  UnitNew moved(std::move(unit));
  CHECK(moved.getUnitNumber() == unitNumber);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew copy assignment", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  
  UnitNew other(2000, L"Other", structureId, 5, 5, 1, emptyFlags, emptyItems,
                weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, 2001);
  other = unit;
  
  CHECK(other.getUnitNumber() == unitNumber);
  CHECK(other.getUnitName() == unitName);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew default z coordinate is 1", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, 1, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getZCoordinate() == 1);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew supports higher z coordinates", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, 5, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  CHECK(unit.getZCoordinate() == 5);
}

TEST_CASE_METHOD(UnitNewFixture, "UnitNew setFutureStructureId sets future structure", "[UnitNew]")
{
  UnitNew unit(unitNumber, unitName, structureId, x, y, z, emptyFlags, emptyItems,
               weight, capWalk, capRide, capFly, capSwim, emptySkills, month, year, originUnit);
  unit.setFutureStructureId(100);
  CHECK(unit.getFutureStructureId() == 100);
}
