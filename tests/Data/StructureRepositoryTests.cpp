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
 * File: StructureRepositoryTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Data/StructureRepository.hpp"

TEST_CASE("StructureRepository add creates new structure successfully")
{
  StructureRepository repo;
  
  bool result = repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  CHECK(result == true);
  CHECK(repo.size() == 1);
  CHECK(repo.at(0).getStructureId() == 101);
}

TEST_CASE("StructureRepository add rejects duplicate ID and coordinates combination")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.add(101, 5, 10, 2, L"fort", L"New Fort", true, 4, 8);
  
  CHECK(result == false);
  CHECK(repo.size() == 1);
}

TEST_CASE("StructureRepository add allows same ID at different coordinates")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.add(101, 6, 11, 3, L"tower", L"Another Tower", false, 3, 7);
  
  CHECK(result == true);
  CHECK(repo.size() == 2);
}

TEST_CASE("StructureRepository add allows same coordinates with different IDs")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.add(102, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  
  CHECK(result == true);
  CHECK(repo.size() == 2);
}

TEST_CASE("StructureRepository add sets lastError on duplicate")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.add(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  
  CHECK(result == false);
  CHECK(!repo.getLastError().empty());
}

TEST_CASE("StructureRepository add clears lastError on success")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  
  bool result = repo.add(102, 6, 11, 3, L"tower", L"Another Tower", false, 3, 7);
  
  CHECK(result == true);
  CHECK(repo.getLastError().empty());
}

TEST_CASE("StructureRepository findByIdAndCoordinates finds existing structure")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  Structure* result = repo.findByIdAndCoordinates(101, 5, 10, 2);
  
  CHECK(result != nullptr);
  CHECK(result->getStructureId() == 101);
  CHECK(result->getStructureName() == L"Guard Tower");
}

TEST_CASE("StructureRepository findByIdAndCoordinates returns nullptr for non-existent")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  Structure* result = repo.findByIdAndCoordinates(999, 5, 10, 2);
  
  CHECK(result == nullptr);
}

TEST_CASE("StructureRepository findByIdAndCoordinates const version works")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  const StructureRepository& constRepo = repo;
  const Structure* result = constRepo.findByIdAndCoordinates(101, 5, 10, 2);
  
  CHECK(result != nullptr);
  CHECK(result->getStructureId() == 101);
}

TEST_CASE("StructureRepository removeByIdAndCoordinates deletes existing structure")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.removeByIdAndCoordinates(101, 5, 10, 2);
  
  CHECK(result == true);
  CHECK(repo.size() == 0);
}

TEST_CASE("StructureRepository removeByIdAndCoordinates returns false for non-existent")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.removeByIdAndCoordinates(999, 5, 10, 2);
  
  CHECK(result == false);
  CHECK(repo.size() == 1);
}

TEST_CASE("StructureRepository removeByIdAndCoordinates sets lastError on failure")
{
  StructureRepository repo;
  
  bool result = repo.removeByIdAndCoordinates(999, 5, 10, 2);
  
  CHECK(result == false);
  CHECK(!repo.getLastError().empty());
}

TEST_CASE("StructureRepository removeByIdAndCoordinates clears lastError on success")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.removeByIdAndCoordinates(101, 5, 10, 2);
  
  CHECK(result == true);
  CHECK(repo.getLastError().empty());
}

TEST_CASE("StructureRepository findByCoordinates returns all structures at location")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  repo.add(103, 6, 11, 3, L"tower", L"Another Tower", false, 3, 7);
  
  auto results = repo.findByCoordinates(5, 10, 2);
  
  CHECK(results.size() == 2);
  CHECK(results[0]->getStructureId() == 101);
  CHECK(results[1]->getStructureId() == 102);
}

TEST_CASE("StructureRepository findByCoordinates returns empty for non-existent location")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  auto results = repo.findByCoordinates(999, 999, 999);
  
  CHECK(results.empty());
}

TEST_CASE("StructureRepository findByCoordinatesAndType returns matching structures")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  repo.add(103, 5, 10, 2, L"tower", L"Another Tower", false, 3, 7);
  
  auto results = repo.findByCoordinatesAndType(5, 10, 2, L"tower");
  
  CHECK(results.size() == 2);
  CHECK(results[0]->getStructureId() == 101);
  CHECK(results[1]->getStructureId() == 103);
}

TEST_CASE("StructureRepository findByCoordinatesAndType is case-insensitive")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  
  auto results = repo.findByCoordinatesAndType(5, 10, 2, L"TOWER");
  
  CHECK(results.size() == 1);
  CHECK(results[0]->getStructureId() == 101);
}

TEST_CASE("StructureRepository findByCoordinatesAndType returns empty for non-matching type")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  auto results = repo.findByCoordinatesAndType(5, 10, 2, L"fort");
  
  CHECK(results.empty());
}

TEST_CASE("StructureRepository clear removes all structures")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 6, 11, 3, L"fort", L"New Fort", false, 3, 7);
  
  repo.clear();
  
  CHECK(repo.size() == 0);
}

TEST_CASE("StructureRepository size returns correct count")
{
  StructureRepository repo;
  
  CHECK(repo.size() == 0);
  
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  CHECK(repo.size() == 1);
  
  repo.add(102, 6, 11, 3, L"fort", L"New Fort", false, 3, 7);
  CHECK(repo.size() == 2);
}

TEST_CASE("StructureRepository at returns structure by index")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 6, 11, 3, L"fort", L"New Fort", false, 3, 7);
  
  CHECK(repo.at(0).getStructureId() == 101);
  CHECK(repo.at(1).getStructureId() == 102);
}

TEST_CASE("StructureRepository at const version works")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  const StructureRepository& constRepo = repo;
  const Structure& structure = constRepo.at(0);
  
  CHECK(structure.getStructureId() == 101);
}

TEST_CASE("StructureRepository addOrUpdateIfLater adds new structure")
{
  StructureRepository repo;
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7, false);
  
  CHECK(result == true);
  CHECK(repo.size() == 1);
}

TEST_CASE("StructureRepository addOrUpdateIfLater updates with later month")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"fort", L"New Fort", true, 5, 7, false);
  
  CHECK(result == true);
  CHECK(repo.size() == 1);
  Structure* found = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(found->getStructureType() == L"fort");
  CHECK(found->getStructureName() == L"New Fort");
  CHECK(found->isClosed() == true);
  CHECK(found->getMonth() == 5);
}

TEST_CASE("StructureRepository addOrUpdateIfLater updates with later year")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 8, false);
  
  CHECK(result == true);
  Structure* found = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(found->getStructureType() == L"fort");
  CHECK(found->getYear() == 8);
}

TEST_CASE("StructureRepository addOrUpdateIfLater ignores same month and year")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 7, false);
  
  CHECK(result == true);
  Structure* found = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(found->getStructureType() == L"tower");
}

TEST_CASE("StructureRepository addOrUpdateIfLater ignores earlier month same year")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 5, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 7, false);
  
  CHECK(result == true);
  Structure* found = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(found->getStructureType() == L"tower");
}

TEST_CASE("StructureRepository addOrUpdateIfLater ignores earlier year")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 8);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"fort", L"New Fort", false, 3, 7, false);
  
  CHECK(result == true);
  Structure* found = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(found->getStructureType() == L"tower");
}

TEST_CASE("StructureRepository addOrUpdateIfLater with globallyUniqueId removes old location")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"ship", L"HMS Victory", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 8, 12, 1, L"ship", L"HMS Victory", false, 4, 7, true);
  
  CHECK(result == true);
  CHECK(repo.size() == 1);
  Structure* found = repo.findByIdAndCoordinates(101, 8, 12, 1);
  CHECK(found != nullptr);
  CHECK(repo.findByIdAndCoordinates(101, 5, 10, 2) == nullptr);
}

TEST_CASE("StructureRepository addOrUpdateIfLater with globallyUniqueId ignores old location when not later")
{
  StructureRepository repo;
  repo.add(101, 8, 12, 1, L"ship", L"HMS Victory", false, 4, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 5, 10, 2, L"ship", L"HMS Victory", false, 3, 7, true);
  
  CHECK(result == true);
  CHECK(repo.size() == 2);
  Structure* oldLocation = repo.findByIdAndCoordinates(101, 8, 12, 1);
  CHECK(oldLocation != nullptr);
  Structure* newLocation = repo.findByIdAndCoordinates(101, 5, 10, 2);
  CHECK(newLocation != nullptr);
}

TEST_CASE("StructureRepository addOrUpdateIfLater with globallyUniqueId keeps both when adding different ID")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"ship", L"HMS Victory", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(102, 8, 12, 1, L"ship", L"HMS Endeavour", false, 4, 7, true);
  
  CHECK(result == true);
  CHECK(repo.size() == 2);
}

TEST_CASE("StructureRepository does not remove old location when globallyUniqueId is false")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  bool result = repo.addOrUpdateIfLater(101, 8, 12, 1, L"tower", L"Guard Tower", false, 4, 7, false);
  
  CHECK(result == true);
  CHECK(repo.size() == 2);
  CHECK(repo.findByIdAndCoordinates(101, 5, 10, 2) != nullptr);
  CHECK(repo.findByIdAndCoordinates(101, 8, 12, 1) != nullptr);
}

TEST_CASE("StructureRepository copy construction")
{
  StructureRepository original;
  original.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  StructureRepository copy = original;
  
  CHECK(copy.size() == 1);
  CHECK(copy.at(0).getStructureId() == 101);
}

TEST_CASE("StructureRepository copy assignment")
{
  StructureRepository original;
  original.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  StructureRepository target;
  target = original;
  
  CHECK(target.size() == 1);
  CHECK(target.at(0).getStructureId() == 101);
}

TEST_CASE("StructureRepository move construction")
{
  StructureRepository original;
  original.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  StructureRepository moved = std::move(original);
  
  CHECK(moved.size() == 1);
  CHECK(moved.at(0).getStructureId() == 101);
}

TEST_CASE("StructureRepository move assignment")
{
  StructureRepository original;
  original.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  StructureRepository target;
  target = std::move(original);
  
  CHECK(target.size() == 1);
  CHECK(target.at(0).getStructureId() == 101);
}

TEST_CASE("StructureRepository with negative coordinates")
{
  StructureRepository repo;
  repo.add(101, -5, -10, -1, L"tower", L"Guard Tower", false, 3, 7);
  
  Structure* found = repo.findByIdAndCoordinates(101, -5, -10, -1);
  CHECK(found != nullptr);
}

TEST_CASE("StructureRepository differentiates same ID at different Z coordinates")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 1, L"tower", L"Tower L1", false, 3, 7);
  repo.add(101, 5, 10, 2, L"tower", L"Tower L2", false, 3, 7);
  
  CHECK(repo.size() == 2);
  CHECK(repo.findByIdAndCoordinates(101, 5, 10, 1)->getStructureName() == L"Tower L1");
  CHECK(repo.findByIdAndCoordinates(101, 5, 10, 2)->getStructureName() == L"Tower L2");
}

TEST_CASE("StructureRepository large number of structures")
{
  StructureRepository repo;
  
  for (int i = 0; i < 100; ++i)
  {
    repo.add(i, i, i, i, L"tower", L"Tower", false, 1, 1);
  }
  
  CHECK(repo.size() == 100);
  Structure* found = repo.findByIdAndCoordinates(50, 50, 50, 50);
  CHECK(found != nullptr);
}

TEST_CASE("StructureRepository multiple structures same coordinates different IDs")
{
  StructureRepository repo;
  repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  repo.add(102, 5, 10, 2, L"fort", L"New Fort", false, 3, 7);
  repo.add(103, 5, 10, 2, L"mine", L"Silver Mine", false, 3, 7);
  
  auto results = repo.findByCoordinates(5, 10, 2);
  CHECK(results.size() == 3);
}

TEST_CASE("StructureRepository coordinates matter for uniqueness")
{
  StructureRepository repo;
  bool r1 = repo.add(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  bool r2 = repo.add(101, 5, 10, 3, L"tower", L"Guard Tower", false, 3, 7);
  
  CHECK(r1 == true);
  CHECK(r2 == true);
  CHECK(repo.size() == 2);
}
