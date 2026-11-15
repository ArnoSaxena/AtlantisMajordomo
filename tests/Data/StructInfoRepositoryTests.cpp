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
 * File: StructInfoRepositoryTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/StructInfoRepository.hpp"

struct StructInfoRepositoryFixture
{
  StructInfoRepository repository;
};

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository starts empty", "[StructInfoRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository add creates new structure info", "[StructInfoRepository]")
{
  bool result = repository.add(L"Tower", 50, 2, false, false);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository add with item identifier token", "[StructInfoRepository]")
{
  bool result = repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  CHECK(result);
  
  StructInfo* info = repository.findByType(L"Tower");
  REQUIRE(info != nullptr);
  CHECK(info->getItemIdentifierToken() == L"TOWE");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository add rejects duplicate type", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  bool result = repository.add(L"Tower", 100, 3, false, false);
  
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository add clears error on success", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  repository.add(L"Tower", 100, 3, false, false);
  CHECK_FALSE(repository.getLastError().empty());
  
  bool result = repository.add(L"Castle", 100, 3, false, false);
  CHECK(result);
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository addOrUpdateByType creates new structure", "[StructInfoRepository]")
{
  bool result = repository.addOrUpdateByType(L"Tower", 50, 2, false, false);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository addOrUpdateByType updates existing structure", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  bool result = repository.addOrUpdateByType(L"Tower", 100, 5, true, false, L"TOWE2");
  
  CHECK(result);
  CHECK(repository.size() == 1);
  
  StructInfo* info = repository.findByType(L"Tower");
  REQUIRE(info != nullptr);
  CHECK(info->getNeeds() == 100);
  CHECK(info->getMagesCapacity() == 5);
  CHECK(info->isShip());
  CHECK(info->getItemIdentifierToken() == L"TOWE2");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByType returns structure for existing type", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfo* info = repository.findByType(L"Tower");
  REQUIRE(info != nullptr);
  CHECK(info->getNeeds() == 50);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByType case insensitive", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfo* info1 = repository.findByType(L"tower");
  StructInfo* info2 = repository.findByType(L"TOWER");
  StructInfo* info3 = repository.findByType(L"ToWeR");
  
  CHECK(info1 != nullptr);
  CHECK(info2 != nullptr);
  CHECK(info3 != nullptr);
  CHECK(info1 == info2);
  CHECK(info2 == info3);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByType returns null for unknown type", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfo* info = repository.findByType(L"Castle");
  CHECK(info == nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository const findByType returns const structure", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  const StructInfoRepository& constRepo = repository;
  const StructInfo* info = constRepo.findByType(L"Tower");
  REQUIRE(info != nullptr);
  CHECK(info->getNeeds() == 50);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByItemIdentifierToken returns structure", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  
  StructInfo* info = repository.findByItemIdentifierToken(L"TOWE");
  REQUIRE(info != nullptr);
  CHECK(info->getStructureType() == L"Tower");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByItemIdentifierToken case sensitive", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  
  StructInfo* info = repository.findByItemIdentifierToken(L"towe");
  CHECK(info == nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository findByItemIdentifierToken returns null for unknown token", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  
  StructInfo* info = repository.findByItemIdentifierToken(L"CAST");
  CHECK(info == nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository const findByItemIdentifierToken returns const structure", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  
  const StructInfoRepository& constRepo = repository;
  const StructInfo* info = constRepo.findByItemIdentifierToken(L"TOWE");
  REQUIRE(info != nullptr);
  CHECK(info->getStructureType() == L"Tower");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository removeByType removes structure", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  CHECK(repository.size() == 1);
  
  bool result = repository.removeByType(L"Tower");
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository removeByType case insensitive", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  bool result = repository.removeByType(L"tower");
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository removeByType fails for unknown type", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  bool result = repository.removeByType(L"Castle");
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository removeByType clears error on success", "[StructInfoRepository]")
{
  repository.removeByType(L"Unknown");
  CHECK_FALSE(repository.getLastError().empty());
  
  repository.add(L"Tower", 50, 2, false, false);
  bool result = repository.removeByType(L"Tower");
  CHECK(result);
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository clear removes all structures", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  repository.add(L"Castle", 100, 3, false, false);
  repository.add(L"Keep", 150, 4, false, false);
  
  CHECK(repository.size() == 3);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository at returns structure by index", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  repository.add(L"Castle", 100, 3, false, false);
  
  const StructInfo& info = repository.at(0);
  CHECK(info.getStructureType() == L"Tower");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository const at returns const structure by index", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  const StructInfoRepository& constRepo = repository;
  const StructInfo& info = constRepo.at(0);
  CHECK(info.getStructureType() == L"Tower");
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository at throws for out of range index", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  CHECK_THROWS_AS(repository.at(5), std::out_of_range);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository can manage multiple structures", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  repository.add(L"Castle", 100, 3, false, false, L"CAST");
  repository.add(L"Longship", 0, 1, true, false, L"SHIP");
  repository.add(L"Airship", 100, 2, true, true, L"AIRSHIP");
  
  CHECK(repository.size() == 4);
  
  CHECK(repository.findByType(L"Tower") != nullptr);
  CHECK(repository.findByType(L"Castle") != nullptr);
  CHECK(repository.findByItemIdentifierToken(L"SHIP") != nullptr);
  CHECK(repository.findByItemIdentifierToken(L"AIRSHIP") != nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository getLastError returns error message", "[StructInfoRepository]")
{
  const auto& error = repository.getLastError();
  CHECK(error.empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository supports copy construction", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  repository.add(L"Castle", 100, 3, false, false, L"CAST");
  
  StructInfoRepository copy(repository);
  CHECK(copy.size() == 2);
  CHECK(copy.findByType(L"Tower") != nullptr);
  CHECK(copy.findByType(L"Castle") != nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository supports move construction", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfoRepository moved(std::move(repository));
  CHECK(moved.size() == 1);
  CHECK(moved.findByType(L"Tower") != nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository supports copy assignment", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfoRepository other;
  other = repository;
  
  CHECK(other.size() == 1);
  CHECK(other.findByType(L"Tower") != nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository supports move assignment", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  
  StructInfoRepository other;
  other = std::move(repository);
  
  CHECK(other.size() == 1);
  CHECK(other.findByType(L"Tower") != nullptr);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository ship structures", "[StructInfoRepository]")
{
  repository.add(L"Longship", 0, 1, true, false);
  repository.add(L"Galley", 100, 2, true, false);
  
  StructInfo* ship1 = repository.findByType(L"Longship");
  StructInfo* ship2 = repository.findByType(L"Galley");
  
  REQUIRE(ship1 != nullptr);
  REQUIRE(ship2 != nullptr);
  CHECK(ship1->isShip());
  CHECK(ship2->isShip());
  CHECK_FALSE(ship1->protoIsFlying());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository flying ship structures", "[StructInfoRepository]")
{
  repository.add(L"Airship", 150, 3, true, true);
  
  StructInfo* flying = repository.findByType(L"Airship");
  REQUIRE(flying != nullptr);
  CHECK(flying->isShip());
  CHECK(flying->protoIsFlying());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository non-flying land structures", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false);
  repository.add(L"Forest", 0, 0, false, false);
  
  StructInfo* tower = repository.findByType(L"Tower");
  StructInfo* forest = repository.findByType(L"Forest");
  
  REQUIRE(tower != nullptr);
  REQUIRE(forest != nullptr);
  CHECK_FALSE(tower->isShip());
  CHECK_FALSE(tower->protoIsFlying());
  CHECK_FALSE(forest->isShip());
  CHECK_FALSE(forest->protoIsFlying());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository update preserves item token if not provided", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 2, false, false, L"TOWE");
  repository.addOrUpdateByType(L"Tower", 75, 3, false, false);
  
  StructInfo* info = repository.findByType(L"Tower");
  REQUIRE(info != nullptr);
  CHECK(info->getNeeds() == 75);
  CHECK(info->getItemIdentifierToken().empty());
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository multiple structures different needs", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 0, false, false);
  repository.add(L"Castle", 500, 2, false, false);
  repository.add(L"Palisade", 10, 0, false, false);
  
  CHECK(repository.findByType(L"Tower")->getNeeds() == 50);
  CHECK(repository.findByType(L"Castle")->getNeeds() == 500);
  CHECK(repository.findByType(L"Palisade")->getNeeds() == 10);
}

TEST_CASE_METHOD(StructInfoRepositoryFixture, "StructInfoRepository multiple structures different mages capacities", "[StructInfoRepository]")
{
  repository.add(L"Tower", 50, 1, false, false);
  repository.add(L"Mage Tower", 100, 5, false, false);
  repository.add(L"Fortress", 200, 3, false, false);
  
  CHECK(repository.findByType(L"Tower")->getMagesCapacity() == 1);
  CHECK(repository.findByType(L"Mage Tower")->getMagesCapacity() == 5);
  CHECK(repository.findByType(L"Fortress")->getMagesCapacity() == 3);
}
