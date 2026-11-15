/*
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo - UnitNewRepository Tests
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
 * File: UnitNewRepositoryTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/UnitNewRepository.hpp"

struct UnitNewRepositoryFixture
{
  UnitNewRepository repository;
  std::vector<std::wstring> emptyFlags;
  std::map<std::wstring, int> emptyItems;
  std::map<std::wstring, int> emptySkills;
  int weight = 100;
  int capWalk = 500;
  int capRide = 1000;
  int capFly = 0;
  int capSwim = 200;
};

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository starts empty", "[UnitNewRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository add creates unit", "[UnitNewRepository]")
{
  bool result = repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                               weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository findByNumberAndCoordinates returns unit", "[UnitNewRepository]")
{
  repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  UnitNew* unit = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(unit != nullptr);
  CHECK(unit->getUnitName() == L"New Fighters");
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository findByNumberAndCoordinates returns null for unknown", "[UnitNewRepository]")
{
  repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  UnitNew* unit = repository.findByNumberAndCoordinates(1000, 15, 25, 1);
  CHECK(unit == nullptr);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository const findByNumberAndCoordinates returns const unit", "[UnitNewRepository]")
{
  repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  const UnitNewRepository& constRepo = repository;
  const UnitNew* unit = constRepo.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(unit != nullptr);
  CHECK(unit->getUnitName() == L"New Fighters");
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository removeByNumberAndCoordinates removes unit", "[UnitNewRepository]")
{
  repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK(repository.size() == 1);
  
  bool result = repository.removeByNumberAndCoordinates(1000, 10, 20, 1);
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository removeByNumberAndCoordinates fails for unknown", "[UnitNewRepository]")
{
  repository.add(1000, L"New Fighters", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  bool result = repository.removeByNumberAndCoordinates(1000, 15, 25, 1);
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository findByOriginUnit returns units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1002, L"Unit C", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  std::vector<UnitNew*> units = repository.findByOriginUnit(1001);
  CHECK(units.size() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository findByOriginUnit returns empty for unknown", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  std::vector<UnitNew*> units = repository.findByOriginUnit(2000);
  CHECK(units.empty());
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository removeByOriginUnit removes units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1002, L"Unit C", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  CHECK(repository.size() == 3);
  
  repository.removeByOriginUnit(1001);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository coordinates and number must be unique", "[UnitNewRepository]")
{
  bool result1 = repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                                weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK(result1);
  
  bool result2 = repository.add(1000, L"Unit B", 5, 10, 20, 1, emptyFlags, emptyItems,
                                weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK_FALSE(result2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository allows same coordinates with different unit numbers", "[UnitNewRepository]")
{
  bool result1 = repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                                weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK(result1);
  
  bool result2 = repository.add(1001, L"Unit B", 5, 10, 20, 1, emptyFlags, emptyItems,
                                weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  CHECK(result2);
  CHECK(repository.size() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository at returns unit by index", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  const UnitNew& unit = repository.at(0);
  CHECK(unit.getUnitNumber() == 1000);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository at throws for out of range", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  CHECK_THROWS_AS(repository.at(5), std::out_of_range);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository clear removes all units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  CHECK(repository.size() == 2);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository getLastError", "[UnitNewRepository]")
{
  const auto& error = repository.getLastError();
  CHECK(error.empty());
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository supports copy construction", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  UnitNewRepository copy(repository);
  CHECK(copy.size() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository supports move construction", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  UnitNewRepository moved(std::move(repository));
  CHECK(moved.size() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository supports copy assignment", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  UnitNewRepository other;
  other = repository;
  
  CHECK(other.size() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository hasUnitWithNumberAtCoordinates", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  CHECK(repository.hasUnitWithNumberAtCoordinates(1000, 10, 20, 1));
  CHECK_FALSE(repository.hasUnitWithNumberAtCoordinates(1000, 15, 25, 1));
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository can manage multiple units from different origins", "[UnitNewRepository]")
{
  repository.add(1000, L"Origin 1 Unit", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(2000, L"Origin 2 Unit", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 2001);
  
  CHECK(repository.size() == 2);
  CHECK(repository.findByNumberAndCoordinates(1000, 10, 20, 1) != nullptr);
  CHECK(repository.findByNumberAndCoordinates(2000, 10, 20, 1) != nullptr);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository findByOriginUnit returns multiple units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1002, L"Unit C", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  std::vector<UnitNew*> units = repository.findByOriginUnit(1001);
  CHECK(units.size() == 3);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository const findByOriginUnit returns const units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  const UnitNewRepository& constRepo = repository;
  std::vector<const UnitNew*> units = constRepo.findByOriginUnit(1001);
  CHECK(units.size() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository different z coordinates are distinct locations", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit Level 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1000, L"Unit Level 2", 5, 10, 20, 2, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  UnitNew* unit1 = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  UnitNew* unit2 = repository.findByNumberAndCoordinates(1000, 10, 20, 2);
  
  REQUIRE(unit1 != nullptr);
  REQUIRE(unit2 != nullptr);
  CHECK(unit1->getUnitName() == L"Unit Level 1");
  CHECK(unit2->getUnitName() == L"Unit Level 2");
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository large dataset", "[UnitNewRepository]")
{
  for (int i = 0; i < 50; ++i)
  {
    repository.add(1000 + i, L"Unit " + std::to_wstring(i), 5, i, i * 2, 1, emptyFlags, emptyItems,
                   weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001 + i / 10);
  }
  
  CHECK(repository.size() == 50);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository edge case negative coordinates", "[UnitNewRepository]")
{
  bool result = repository.add(1000, L"Unit", 5, -10, -20, 1, emptyFlags, emptyItems,
                               weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  CHECK(result);
  
  UnitNew* unit = repository.findByNumberAndCoordinates(1000, -10, -20, 1);
  REQUIRE(unit != nullptr);
  CHECK(unit->getXCoordinate() == -10);
  CHECK(unit->getYCoordinate() == -20);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository countTotalWarnings", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1002);
  
  UnitNew* unit1 = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  UnitNew* unit2 = repository.findByNumberAndCoordinates(1001, 11, 21, 1);
  
  REQUIRE(unit1 != nullptr);
  REQUIRE(unit2 != nullptr);
  
  unit1->addWarning(L"Low supplies");
  unit2->addWarning(L"Morale issue");
  
  int totalWarnings = repository.countTotalWarnings();
  CHECK(totalWarnings >= 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository old units from earlier month are retained", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit from month 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit from month 2", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1002);
  
  CHECK(repository.size() == 2);
  
  UnitNew* unit1 = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  UnitNew* unit2 = repository.findByNumberAndCoordinates(1001, 11, 21, 1);
  
  CHECK(unit1 != nullptr);
  CHECK(unit2 != nullptr);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository can identify oldest units in repository", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit from month 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit from month 3", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 3, 1, 1002);
  repository.add(1002, L"Unit from month 2", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1003);
  
  CHECK(repository.size() == 3);
  
  const UnitNew& first = repository.at(0);
  CHECK(first.getMonth() == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository units from month 1 and month 3 have different periods", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit month 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit month 3", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 3, 1, 1002);
  
  UnitNew* unit1 = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  UnitNew* unit2 = repository.findByNumberAndCoordinates(1001, 11, 21, 1);
  
  REQUIRE(unit1 != nullptr);
  REQUIRE(unit2 != nullptr);
  
  CHECK(unit1->getMonth() == 1);
  CHECK(unit2->getMonth() == 3);
  CHECK(((unit1->getMonth()) < (unit2->getMonth())));
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository can find units by earlier month period", "[UnitNewRepository]")
{
  repository.add(1000, L"Old unit", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Newer unit", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1002);
  
  UnitNew* oldUnit = nullptr;
  UnitNew* newUnit = nullptr;
  
  for (size_t i = 0; i < repository.size(); ++i)
  {
    const UnitNew& unit = repository.at(i);
    if (unit.getMonth() == 1)
      oldUnit = const_cast<UnitNew*>(&unit);
    if (unit.getMonth() == 2)
      newUnit = const_cast<UnitNew*>(&unit);
  }
  
  CHECK(oldUnit != nullptr);
  CHECK(newUnit != nullptr);
  CHECK(oldUnit->getMonth() == 1);
  CHECK(newUnit->getMonth() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository preserves units across different years", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit year 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit year 2", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 2, 1002);
  
  CHECK(repository.size() == 2);
  
  UnitNew* year1 = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  UnitNew* year2 = repository.findByNumberAndCoordinates(1001, 11, 21, 1);
  
  REQUIRE(year1 != nullptr);
  REQUIRE(year2 != nullptr);
  
  CHECK(year1->getYear() == 1);
  CHECK(year2->getYear() == 2);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository units with same month different years are distinguishable", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit from year 1 month 5", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 5, 1, 1001);
  repository.add(1001, L"Unit from year 2 month 5", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 5, 2, 1002);
  
  int year1Count = 0;
  int year2Count = 0;
  
  for (size_t i = 0; i < repository.size(); ++i)
  {
    const UnitNew& unit = repository.at(i);
    if (unit.getYear() == 1 && unit.getMonth() == 5)
      year1Count++;
    if (unit.getYear() == 2 && unit.getMonth() == 5)
      year2Count++;
  }
  
  CHECK(year1Count == 1);
  CHECK(year2Count == 1);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository clear empties all units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit 2", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1002);
  repository.add(1002, L"Unit 3", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 3, 1, 1003);
  
  CHECK(repository.size() == 3);
  
  repository.clear();
  
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository cleared repository no longer finds units", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit 2", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1002);
  
  CHECK(repository.size() == 2);
  
  repository.clear();
  
  CHECK(repository.findByNumberAndCoordinates(1000, 10, 20, 1) == nullptr);
  CHECK(repository.findByNumberAndCoordinates(1001, 11, 21, 1) == nullptr);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository can re-add units after clearing", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit 1", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  repository.clear();
  
  CHECK(repository.size() == 0);
  
  bool result = repository.add(2000, L"Unit 2", 5, 15, 25, 1, emptyFlags, emptyItems,
                               weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 2001);
  
  CHECK(result == true);
  CHECK(repository.size() == 1);
  UnitNew* unit = repository.findByNumberAndCoordinates(2000, 15, 25, 1);
  REQUIRE(unit != nullptr);
  CHECK(unit->getUnitName() == L"Unit 2");
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository clear discards old unit data completely", "[UnitNewRepository]")
{
  repository.add(1000, L"Old Unit", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  
  UnitNew* oldUnit = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(oldUnit != nullptr);
  
  repository.clear();
  
  UnitNew* shouldBeNull = repository.findByNumberAndCoordinates(1000, 10, 20, 1);
  CHECK(shouldBeNull == nullptr);
  
  bool canAddWithSameNumber = repository.add(1000, L"New Unit", 5, 30, 40, 1, emptyFlags, emptyItems,
                                             weight, capWalk, capRide, capFly, capSwim, emptySkills, 5, 1, 1005);
  CHECK(canAddWithSameNumber == true);
}

TEST_CASE_METHOD(UnitNewRepositoryFixture, "UnitNewRepository lifecycle: populate, clear, verify empty", "[UnitNewRepository]")
{
  repository.add(1000, L"Unit A", 5, 10, 20, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 1, 1, 1001);
  repository.add(1001, L"Unit B", 5, 11, 21, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 2, 1, 1002);
  repository.add(1002, L"Unit C", 5, 12, 22, 1, emptyFlags, emptyItems,
                 weight, capWalk, capRide, capFly, capSwim, emptySkills, 3, 1, 1003);
  
  CHECK(repository.size() == 3);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1000, 10, 20, 1) == true);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1001, 11, 21, 1) == true);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1002, 12, 22, 1) == true);
  
  repository.clear();
  
  CHECK(repository.size() == 0);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1000, 10, 20, 1) == false);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1001, 11, 21, 1) == false);
  CHECK(repository.hasUnitWithNumberAtCoordinates(1002, 12, 22, 1) == false);
}
