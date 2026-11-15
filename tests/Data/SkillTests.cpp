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
 * File: SkillTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/Skill.hpp"

struct SkillFixture
{
  Skill skill { L"FIRE", L"Fire" };
};

TEST_CASE_METHOD(SkillFixture, "Skill can be constructed with token and name", "[Skill]")
{
  CHECK(skill.getIdentifierToken() == L"FIRE");
  CHECK(skill.getName() == L"Fire");
}

TEST_CASE_METHOD(SkillFixture, "Skill name can be set", "[Skill]")
{
  skill.setName(L"Advanced Fire");
  CHECK(skill.getName() == L"Advanced Fire");
}

TEST_CASE_METHOD(SkillFixture, "Skill defaults to not magic", "[Skill]")
{
  CHECK_FALSE(skill.isMagic());
}

TEST_CASE_METHOD(SkillFixture, "Skill can set magic flag", "[Skill]")
{
  skill.setMagic(true);
  CHECK(skill.isMagic());
}

TEST_CASE_METHOD(SkillFixture, "Skill defaults to not magic foundation", "[Skill]")
{
  CHECK_FALSE(skill.isMagicFoundation());
}

TEST_CASE_METHOD(SkillFixture, "Skill can set magic foundation flag", "[Skill]")
{
  skill.setMagicFoundation(true);
  CHECK(skill.isMagicFoundation());
}

TEST_CASE_METHOD(SkillFixture, "Skill magic foundation implies magic", "[Skill]")
{
  skill.setMagicFoundation(true);
  CHECK(skill.isMagic());
  CHECK(skill.isMagicFoundation());
}

TEST_CASE_METHOD(SkillFixture, "Skill defaults to zero study cost", "[Skill]")
{
  CHECK(skill.getStudyCost() == 0);
}

TEST_CASE_METHOD(SkillFixture, "Skill can set study cost", "[Skill]")
{
  skill.setStudyCost(5);
  CHECK(skill.getStudyCost() == 5);
}

TEST_CASE_METHOD(SkillFixture, "Skill rejects negative study cost", "[Skill]")
{
  skill.setStudyCost(-5);
  CHECK(skill.getStudyCost() == 0);
}

TEST_CASE_METHOD(SkillFixture, "Skill has no levels by default", "[Skill]")
{
  auto levels = skill.getLevels();
  CHECK(levels.empty());
}

TEST_CASE_METHOD(SkillFixture, "Skill rejects level data for invalid level", "[Skill]")
{
  bool result = skill.setLevelData(0, {}, L"Invalid");
  CHECK_FALSE(result);
  
  result = skill.setLevelData(6, {}, L"Invalid");
  CHECK_FALSE(result);
}

TEST_CASE_METHOD(SkillFixture, "Skill accepts level data for valid level", "[Skill]")
{
  std::map<std::wstring, int> items { { L"SILV", 10 } };
  bool result = skill.setLevelData(1, items, L"Level 1 description");
  CHECK(result);
}

TEST_CASE_METHOD(SkillFixture, "Skill tracks levels after setting level data", "[Skill]")
{
  skill.setLevelData(1, {}, L"Level 1");
  skill.setLevelData(3, {}, L"Level 3");
  
  auto levels = skill.getLevels();
  CHECK(levels.size() == 2);
  CHECK(levels[0] == 1);
  CHECK(levels[1] == 3);
}

TEST_CASE_METHOD(SkillFixture, "Skill reports if level exists", "[Skill]")
{
  skill.setLevelData(2, {}, L"Level 2");
  CHECK(skill.hasLevel(2));
  CHECK_FALSE(skill.hasLevel(1));
  CHECK_FALSE(skill.hasLevel(6));
}

TEST_CASE_METHOD(SkillFixture, "Skill returns null for invalid level data", "[Skill]")
{
  CHECK(skill.getLevelData(0) == nullptr);
  CHECK(skill.getLevelData(6) == nullptr);
}

TEST_CASE_METHOD(SkillFixture, "Skill returns level data for existing level", "[Skill]")
{
  skill.setLevelData(2, {}, L"Test description");
  const auto* data = skill.getLevelData(2);
  REQUIRE(data != nullptr);
  CHECK(data->description == L"Test description");
}

TEST_CASE_METHOD(SkillFixture, "Skill level data contains production items", "[Skill]")
{
  std::map<std::wstring, int> items { { L"SILV", 100 }, { L"IRON", 50 } };
  skill.setLevelData(1, items, L"");
  
  const auto* data = skill.getLevelData(1);
  REQUIRE(data != nullptr);
  CHECK(data->productionItems.size() == 2);
  CHECK(data->productionItems.at(L"SILV") == 100);
  CHECK(data->productionItems.at(L"IRON") == 50);
}

TEST_CASE_METHOD(SkillFixture, "Skill is production when level has items", "[Skill]")
{
  std::map<std::wstring, int> items { { L"SILV", 10 } };
  skill.setLevelData(1, items, L"");
  
  CHECK(skill.isProduction(1));
  CHECK_FALSE(skill.isProduction(2));
}

TEST_CASE_METHOD(SkillFixture, "Skill production items for level can be retrieved", "[Skill]")
{
  std::map<std::wstring, int> items { { L"SILV", 10 } };
  skill.setLevelData(1, items, L"");
  
  const auto& retrieved = skill.getProductionItems(1);
  CHECK(retrieved.size() == 1);
  CHECK(retrieved.at(L"SILV") == 10);
}

TEST_CASE_METHOD(SkillFixture, "Skill production items for non-existent level returns empty", "[Skill]")
{
  const auto& retrieved = skill.getProductionItems(1);
  CHECK(retrieved.empty());
}

TEST_CASE_METHOD(SkillFixture, "Skill can set production items for existing level", "[Skill]")
{
  skill.setLevelData(1, {}, L"");
  std::map<std::wstring, int> newItems { { L"IRON", 20 } };
  skill.setProductionItems(1, newItems);
  
  const auto* data = skill.getLevelData(1);
  REQUIRE(data != nullptr);
  CHECK(data->productionItems.at(L"IRON") == 20);
}

TEST_CASE_METHOD(SkillFixture, "Skill description can be retrieved for level", "[Skill]")
{
  skill.setLevelData(1, {}, L"Level 1 description");
  CHECK(skill.getDescription(1) == L"Level 1 description");
}

TEST_CASE_METHOD(SkillFixture, "Skill description for non-existent level returns empty", "[Skill]")
{
  CHECK(skill.getDescription(1).empty());
}

TEST_CASE_METHOD(SkillFixture, "Skill can set description for existing level", "[Skill]")
{
  skill.setLevelData(1, {}, L"Original");
  skill.setDescription(1, L"Updated");
  CHECK(skill.getDescription(1) == L"Updated");
}

TEST_CASE_METHOD(SkillFixture, "Skill combines all level descriptions", "[Skill]")
{
  skill.setLevelData(1, {}, L"First level");
  skill.setLevelData(2, {}, L"Second level");
  skill.setLevelData(4, {}, L"Fourth level");
  
  const auto combined = skill.getAllLevelDescriptions();
  CHECK(combined.find(L"First level") != std::wstring::npos);
  CHECK(combined.find(L"Second level") != std::wstring::npos);
  CHECK(combined.find(L"Fourth level") != std::wstring::npos);
}

TEST_CASE_METHOD(SkillFixture, "Skill returns empty string when no descriptions", "[Skill]")
{
  CHECK(skill.getAllLevelDescriptions().empty());
}

TEST_CASE_METHOD(SkillFixture, "Skill defaults to no prerequisites", "[Skill]")
{
  const auto& prereqs = skill.getPrerequisites();
  CHECK(prereqs.empty());
}

TEST_CASE_METHOD(SkillFixture, "Skill can set prerequisites", "[Skill]")
{
  std::vector<SkillPrerequisite> prereqs {
    SkillPrerequisite { L"HEAT", 1 },
    SkillPrerequisite { L"MAGIC", 2 }
  };
  skill.setPrerequisites(prereqs);
  
  const auto& stored = skill.getPrerequisites();
  CHECK(stored.size() == 2);
  CHECK(stored[0].token == L"HEAT");
  CHECK(stored[0].requiredLevel == 1);
  CHECK(stored[1].token == L"MAGIC");
  CHECK(stored[1].requiredLevel == 2);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for less than 30 days", "[Skill]")
{
  CHECK(Skill::trainingDaysToLevel(0) == 0);
  CHECK(Skill::trainingDaysToLevel(29) == 0);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for level 1", "[Skill]")
{
  // cumulative at i=1 is 30, at i=2 is 90, so 30-89 days = level 1
  CHECK(Skill::trainingDaysToLevel(30) == 1);
  CHECK(Skill::trainingDaysToLevel(89) == 1);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for level 2", "[Skill]")
{
  // cumulative at i=2 is 90, at i=3 is 180, so 90-179 days = level 2
  CHECK(Skill::trainingDaysToLevel(90) == 2);
  CHECK(Skill::trainingDaysToLevel(179) == 2);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for level 3", "[Skill]")
{
  // cumulative at i=3 is 180, at i=4 is 300, so 180-299 days = level 3
  CHECK(Skill::trainingDaysToLevel(180) == 3);
  CHECK(Skill::trainingDaysToLevel(299) == 3);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for level 4", "[Skill]")
{
  // cumulative at i=4 is 300, at i=5 is 450, so 300-449 days = level 4
  CHECK(Skill::trainingDaysToLevel(300) == 4);
  CHECK(Skill::trainingDaysToLevel(449) == 4);
}

TEST_CASE_METHOD(SkillFixture, "Skill trainingDaysToLevel for level 5", "[Skill]")
{
  // cumulative at i=5 is 450, so 450+ days = level 5
  CHECK(Skill::trainingDaysToLevel(450) == 5);
  CHECK(Skill::trainingDaysToLevel(600) == 5);
}

TEST_CASE_METHOD(SkillFixture, "Skill supports copy construction", "[Skill]")
{
  skill.setLevelData(1, { { L"SILV", 10 } }, L"Fire level 1");
  skill.setMagic(true);
  skill.setStudyCost(3);
  
  Skill copy(skill);
  CHECK(copy.getIdentifierToken() == L"FIRE");
  CHECK(copy.getName() == L"Fire");
  CHECK(copy.isMagic());
  CHECK(copy.getStudyCost() == 3);
  CHECK(copy.hasLevel(1));
}

TEST_CASE_METHOD(SkillFixture, "Skill supports move construction", "[Skill]")
{
  skill.setLevelData(1, {}, L"Fire level 1");
  skill.setMagic(true);
  
  Skill moved(std::move(skill));
  CHECK(moved.getIdentifierToken() == L"FIRE");
  CHECK(moved.hasLevel(1));
  CHECK(moved.isMagic());
}

TEST_CASE_METHOD(SkillFixture, "Skill supports copy assignment", "[Skill]")
{
  skill.setLevelData(1, {}, L"Fire level 1");
  skill.setMagic(true);
  
  Skill other(L"COLD", L"Cold");
  other = skill;
  
  CHECK(other.getIdentifierToken() == L"FIRE");
  CHECK(other.getName() == L"Fire");
  CHECK(other.isMagic());
  CHECK(other.hasLevel(1));
}
