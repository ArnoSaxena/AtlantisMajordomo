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
 * File: UnitRepositoryTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/UnitRepository.hpp"

struct UnitRepositoryFixture
{
  UnitRepository repository;
  std::map<std::wstring, int> emptyItems;
  std::map<std::wstring, int> emptySkills;
  std::vector<std::wstring> emptyFlags;
};

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository starts empty", "[UnitRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository add creates unit", "[UnitRepository]")
{
  bool result = repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository findByNumber returns unit", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  Unit* unit = repository.findByNumber(1000);
  REQUIRE(unit != nullptr);
  CHECK(unit->getUnitName() == L"Fighters");
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository const findByNumber returns const unit", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  const UnitRepository& constRepo = repository;
  const Unit* unit = constRepo.findByNumber(1000);
  REQUIRE(unit != nullptr);
  CHECK(unit->getUnitName() == L"Fighters");
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository findByNumber returns null for unknown unit", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  Unit* unit = repository.findByNumber(2000);
  CHECK(unit == nullptr);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository at returns unit by index", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  const Unit& unit = repository.at(0);
  CHECK(unit.getUnitNumber() == 1000);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository at throws for out of range index", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  CHECK_THROWS_AS(repository.at(5), std::out_of_range);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository removeByNumber removes unit", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  CHECK(repository.size() == 1);
  
  bool result = repository.removeByNumber(1000);
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository removeByNumber fails for unknown unit", "[UnitRepositoryTests]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  bool result = repository.removeByNumber(2000);
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository clear removes all units", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(1001, L"Archers", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  
  CHECK(repository.size() == 2);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository can manage multiple units", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(1001, L"Archers", 5, 11, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  repository.add(1002, L"Mages", 6, 12, 1, 2, 1, emptyFlags, emptyItems, 300, 5000, 3000, 0, 0, emptySkills, 8, 7);
  
  CHECK(repository.size() == 3);
  CHECK(repository.findByNumber(1000) != nullptr);
  CHECK(repository.findByNumber(1001) != nullptr);
  CHECK(repository.findByNumber(1002) != nullptr);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository findSmallestUnitNumberForFaction", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(1005, L"Archers", 5, 11, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  repository.add(1003, L"Mages", 5, 12, 1, 2, 1, emptyFlags, emptyItems, 300, 5000, 3000, 0, 0, emptySkills, 8, 7);
  repository.add(2000, L"Enemy", 6, 20, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  int smallest = repository.findSmallestUnitNumberForFaction(5);
  CHECK(smallest == 1000);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository findSmallestUnitNumberForFaction returns 0 for unknown faction", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  int smallest = repository.findSmallestUnitNumberForFaction(99);
  CHECK(smallest == 0);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository addOrUpdateIfLater creates new unit", "[UnitRepository]")
{
  bool result = repository.addOrUpdateIfLater(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository addOrUpdateIfLater updates if later period", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  bool result = repository.addOrUpdateIfLater(1000, L"Updated Fighters", 5, 10, 2, 3, 1, emptyFlags, emptyItems, 600, 11000, 5500, 0, 0, emptySkills, 9, 7);
  CHECK(result);
  CHECK(repository.size() == 1);
  
  Unit* unit = repository.findByNumber(1000);
  CHECK(unit->getUnitName() == L"Updated Fighters");
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository addOrUpdateIfLater updates existing unit with later period", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  bool result = repository.addOrUpdateIfLater(1000, L"Updated Fighters", 5, 10, 2, 3, 1, emptyFlags, emptyItems, 600, 11000, 5500, 0, 0, emptySkills, 9, 7);
  CHECK(result);
  CHECK(repository.size() == 1);
  
  Unit* unit = repository.findByNumber(1000);
  CHECK(unit->getUnitName() == L"Updated Fighters");
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository addOrUpdateIfLater creates new unit if not exists", "[UnitRepository]")
{
  bool result = repository.addOrUpdateIfLater(1000, L"New Unit", 5, 10, 2, 3, 1, emptyFlags, emptyItems, 600, 11000, 5500, 0, 0, emptySkills, 8, 7);
  CHECK(result);
  CHECK(repository.size() == 1);
  
  Unit* unit = repository.findByNumber(1000);
  CHECK(unit->getUnitName() == L"New Unit");
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository getLastError", "[UnitRepository]")
{
  const auto& error = repository.getLastError();
  CHECK(error.empty());
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository findUnitNumbersWithWarnings", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(1001, L"Archers", 5, 11, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  
  Unit* unit1 = repository.findByNumber(1000);
  Unit* unit2 = repository.findByNumber(1001);
  
  REQUIRE(unit1 != nullptr);
  REQUIRE(unit2 != nullptr);
  
  unit1->addWarning(L"Low supplies");
  unit2->addWarning(L"Morale issue");
  
  std::vector<int> warnings = repository.findUnitNumbersWithWarnings();
  CHECK(warnings.size() == 2);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository supports copy construction", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(1001, L"Archers", 5, 11, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  
  UnitRepository copy(repository);
  CHECK(copy.size() == 2);
  CHECK(copy.findByNumber(1000) != nullptr);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository supports move construction", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  UnitRepository moved(std::move(repository));
  CHECK(moved.size() == 1);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository supports copy assignment", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  
  UnitRepository other;
  other = repository;
  
  CHECK(other.size() == 1);
  CHECK(other.findByNumber(1000) != nullptr);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository multiple units from different factions", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(2000, L"Enemy", 6, 20, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7);
  repository.add(2001, L"Enemy Archers", 6, 21, 1, 2, 1, emptyFlags, emptyItems, 400, 8000, 4000, 0, 0, emptySkills, 8, 7);
  
  CHECK(repository.findSmallestUnitNumberForFaction(5) == 1000);
  CHECK(repository.findSmallestUnitNumberForFaction(6) == 2000);
}

TEST_CASE_METHOD(UnitRepositoryFixture, "UnitRepository on guard units", "[UnitRepository]")
{
  repository.add(1000, L"Fighters", 5, 10, 1, 2, 1, emptyFlags, emptyItems, 500, 10000, 5000, 0, 0, emptySkills, 8, 7, true);
  
  Unit* unit = repository.findByNumber(1000);
  REQUIRE(unit != nullptr);
  CHECK(unit->isOnGuard());
}
