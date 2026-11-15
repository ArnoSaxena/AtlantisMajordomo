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
 * File: SkillRepositoryTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/SkillRepository.hpp"

struct SkillRepositoryFixture
{
  SkillRepository repository;
};

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository starts empty", "[SkillRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel creates skill for new token", "[SkillRepository]")
{
  bool result = repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel normalizes token to uppercase", "[SkillRepository]")
{
  repository.addLevel(L"fire", L"Fire", 1, {}, L"");
  
  Skill* skill = repository.findByIdentifier(L"FIRE");
  REQUIRE(skill != nullptr);
  CHECK(skill->getIdentifierToken() == L"FIRE");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel rejects token shorter than 3 characters", "[SkillRepository]")
{
  bool result = repository.addLevel(L"FI", L"Fire", 1, {}, L"");
  CHECK_FALSE(result);
  CHECK(repository.size() == 0);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel rejects invalid skill level", "[SkillRepository]")
{
  bool result = repository.addLevel(L"FIRE", L"Fire", 0, {}, L"");
  CHECK_FALSE(result);
  CHECK_FALSE(repository.getLastError().empty());
  
  result = repository.addLevel(L"FIRE", L"Fire", 6, {}, L"");
  CHECK_FALSE(result);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel rejects duplicate level", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"Level 1");
  bool result = repository.addLevel(L"fire", L"Fire", 1, {}, L"Duplicate");
  CHECK_FALSE(result);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel accepts additional levels for same token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"Level 1");
  bool result = repository.addLevel(L"fire", L"Fire", 2, {}, L"Level 2");
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel stores production items", "[SkillRepository]")
{
  std::map<std::wstring, int> items { { L"SILV", 100 }, { L"IRON", 50 } };
  repository.addLevel(L"FIRE", L"Fire", 1, items, L"Produces something");
  
  Skill* skill = repository.findByIdentifier(L"FIRE");
  REQUIRE(skill != nullptr);
  const auto& retrieved = skill->getProductionItems(1);
  CHECK(retrieved.size() == 2);
  CHECK(retrieved.at(L"SILV") == 100);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository addLevel clears error on success", "[SkillRepository]")
{
  repository.addLevel(L"AB", L"Short", 1, {}, L"");
  CHECK_FALSE(repository.getLastError().empty());
  
  bool result = repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  CHECK(result);
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository findByIdentifier returns skill for valid token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  Skill* skill = repository.findByIdentifier(L"FIRE");
  REQUIRE(skill != nullptr);
  CHECK(skill->getName() == L"Fire");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository findByIdentifier normalizes token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  Skill* skill = repository.findByIdentifier(L"fire");
  REQUIRE(skill != nullptr);
  CHECK(skill->getIdentifierToken() == L"FIRE");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository findByIdentifier returns null for unknown token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  Skill* skill = repository.findByIdentifier(L"COLD");
  CHECK(skill == nullptr);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository const findByIdentifier returns const skill", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  const SkillRepository& constRepo = repository;
  const Skill* skill = constRepo.findByIdentifier(L"FIRE");
  REQUIRE(skill != nullptr);
  CHECK(skill->getName() == L"Fire");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository removeByIdentifier removes skill", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  CHECK(repository.size() == 1);
  
  bool result = repository.removeByIdentifier(L"FIRE");
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository removeByIdentifier normalizes token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  bool result = repository.removeByIdentifier(L"fire");
  CHECK(result);
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository removeByIdentifier fails for unknown token", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  bool result = repository.removeByIdentifier(L"COLD");
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository removeByIdentifier clears error on success", "[SkillRepository]")
{
  repository.removeByIdentifier(L"NOTFOUND");
  CHECK_FALSE(repository.getLastError().empty());
  
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  bool result = repository.removeByIdentifier(L"FIRE");
  CHECK(result);
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository clear removes all skills", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  repository.addLevel(L"COLD", L"Cold", 1, {}, L"");
  repository.addLevel(L"LITE", L"Lightning", 1, {}, L"");
  
  CHECK(repository.size() == 3);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository at returns skill by index", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  repository.addLevel(L"COLD", L"Cold", 1, {}, L"");
  
  const Skill& skill = repository.at(0);
  CHECK(skill.getIdentifierToken() == L"FIRE");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository const at returns const skill by index", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  const SkillRepository& constRepo = repository;
  const Skill& skill = constRepo.at(0);
  CHECK(skill.getIdentifierToken() == L"FIRE");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository at throws for out of range index", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  CHECK_THROWS_AS(repository.at(5), std::out_of_range);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository can manage multiple skills", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  repository.addLevel(L"FIRE", L"Fire", 2, {}, L"");
  repository.addLevel(L"COLD", L"Cold", 1, {}, L"");
  repository.addLevel(L"LITE", L"Lightning", 1, {}, L"");
  
  CHECK(repository.size() == 3);
  CHECK(repository.findByIdentifier(L"FIRE")->getLevels().size() == 2);
  CHECK(repository.findByIdentifier(L"COLD")->getLevels().size() == 1);
  CHECK(repository.findByIdentifier(L"LITE")->getLevels().size() == 1);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository trainingDaysToLevel delegates to Skill", "[SkillRepository]")
{
  CHECK(repository.trainingDaysToLevel(30) == 1);
  CHECK(repository.trainingDaysToLevel(90) == 2);
  CHECK(repository.trainingDaysToLevel(180) == 3);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository getLastError returns last error message", "[SkillRepository]")
{
  const auto& error = repository.getLastError();
  CHECK(error.empty());
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository supports copy construction", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  repository.addLevel(L"COLD", L"Cold", 1, {}, L"");
  
  SkillRepository copy(repository);
  CHECK(copy.size() == 2);
  CHECK(copy.findByIdentifier(L"FIRE") != nullptr);
  CHECK(copy.findByIdentifier(L"COLD") != nullptr);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository supports move construction", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  SkillRepository moved(std::move(repository));
  CHECK(moved.size() == 1);
  CHECK(moved.findByIdentifier(L"FIRE") != nullptr);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository supports copy assignment", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  SkillRepository other;
  other = repository;
  
  CHECK(other.size() == 1);
  CHECK(other.findByIdentifier(L"FIRE") != nullptr);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository supports move assignment", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"");
  
  SkillRepository other;
  other = std::move(repository);
  
  CHECK(other.size() == 1);
  CHECK(other.findByIdentifier(L"FIRE") != nullptr);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository add level with production items", "[SkillRepository]")
{
  std::map<std::wstring, int> items { { L"SILV", 50 }, { L"IRON", 25 } };
  bool result = repository.addLevel(L"SMITH", L"Smithing", 1, items, L"Basic smithing");
  
  CHECK(result);
  Skill* skill = repository.findByIdentifier(L"SMITH");
  REQUIRE(skill != nullptr);
  const auto& retrieved = skill->getProductionItems(1);
  CHECK(retrieved.size() == 2);
  CHECK(retrieved.at(L"SILV") == 50);
  CHECK(retrieved.at(L"IRON") == 25);
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository preserves skill name across levels", "[SkillRepository]")
{
  repository.addLevel(L"FIRE", L"Fire", 1, {}, L"Level 1");
  repository.addLevel(L"FIRE", L"Fire", 2, {}, L"Level 2");
  repository.addLevel(L"FIRE", L"Advanced Fire", 3, {}, L"Level 3");
  
  Skill* skill = repository.findByIdentifier(L"FIRE");
  REQUIRE(skill != nullptr);
  CHECK(skill->getName() == L"Fire");
}

TEST_CASE_METHOD(SkillRepositoryFixture, "SkillRepository case insensitive lookup", "[SkillRepository]")
{
  repository.addLevel(L"FiRe", L"Fire", 1, {}, L"");
  
  Skill* skill1 = repository.findByIdentifier(L"fire");
  Skill* skill2 = repository.findByIdentifier(L"FIRE");
  Skill* skill3 = repository.findByIdentifier(L"FiRe");
  
  CHECK(skill1 == skill2);
  CHECK(skill2 == skill3);
}
