/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Item.hpp"

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>

namespace
{
struct ItemFixture
{
  Item item { L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false };
};
}

TEST_CASE_METHOD(ItemFixture, "Item stores the identifier token", "[Item]")
{
  CHECK(item.getIdentifierToken() == L"SWRD");
}

TEST_CASE_METHOD(ItemFixture, "Item stores the item name", "[Item]")
{
  CHECK(item.getItemName() == L"sword");
}

TEST_CASE_METHOD(ItemFixture, "Item stores the weight", "[Item]")
{
  CHECK(item.getWeight() == 5);
}

TEST_CASE_METHOD(ItemFixture, "Item stores melee weapon status", "[Item]")
{
  CHECK(item.isMeeleWeapon());
}

TEST_CASE_METHOD(ItemFixture, "Item stores ranged weapon status", "[Item]")
{
  Item bow(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(bow.isRangedWeapon());
}

TEST_CASE_METHOD(ItemFixture, "Item stores armour status", "[Item]")
{
  Item armor(L"ARM", L"plate armor", 10, false, false, true, false, false, 0, 0, 0, 0, 0, false);
  CHECK(armor.isArmour());
}

TEST_CASE_METHOD(ItemFixture, "Item stores resource status", "[Item]")
{
  Item grain(L"GRAIN", L"grain", 1, false, false, false, true, false, 0, 0, 0, 0, 0, false);
  CHECK(grain.isResource());
}

TEST_CASE_METHOD(ItemFixture, "Item stores mount status", "[Item]")
{
  Item horse(L"HORS", L"horse", 500, false, false, false, false, true, 2, 0, 750, 0, 0, false);
  CHECK(horse.isMount());
}

TEST_CASE_METHOD(ItemFixture, "Item stores melee weapon false status", "[Item]")
{
  CHECK_FALSE(item.isRangedWeapon());
}

TEST_CASE_METHOD(ItemFixture, "Item stores man status", "[Item]")
{
  Item man(L"MAN", L"man", 10, false, false, false, false, false, 0, 0, 0, 0, 0, true);
  CHECK(man.isMan());
}

TEST_CASE_METHOD(ItemFixture, "Item is not provisional by default", "[Item]")
{
  CHECK_FALSE(item.isProvisional());
}

TEST_CASE("Item stores zero capacities by default", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(item.getWalkCapacity() == 0);
  CHECK(item.getRideCapacity() == 0);
  CHECK(item.getSwimCapacity() == 0);
  CHECK(item.getFlyCapacity() == 0);
}

TEST_CASE("Item stores walk capacity", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, 100, 0, 0, 0, false);
  CHECK(item.getWalkCapacity() == 100);
}

TEST_CASE("Item stores ride capacity", "[Item]")
{
  Item item(L"HORS", L"horse", 500, false, false, false, false, true, 2, 0, 750, 0, 0, false);
  CHECK(item.getRideCapacity() == 750);
}

TEST_CASE("Item stores swim capacity", "[Item]")
{
  Item item(L"BOAT", L"boat", 100, false, false, false, false, false, 0, 0, 0, 500, 0, false);
  CHECK(item.getSwimCapacity() == 500);
}

TEST_CASE("Item stores fly capacity", "[Item]")
{
  Item item(L"DRAG", L"dragon", 1000, false, false, false, false, true, 3, 0, 500, 0, 300, false);
  CHECK(item.getFlyCapacity() == 300);
}

TEST_CASE_METHOD(ItemFixture, "Item stores moves", "[Item]")
{
  Item horse(L"HORS", L"horse", 500, false, false, false, false, true, 2, 0, 750, 0, 0, false);
  CHECK(horse.getMoves() == 2);
}

TEST_CASE_METHOD(ItemFixture, "Item can change its name", "[Item]")
{
  item.setItemName(L"long sword");
  CHECK(item.getItemName() == L"long sword");
}

TEST_CASE_METHOD(ItemFixture, "Item can set a plural name", "[Item]")
{
  item.setItemNamePlural(L"swords");
  CHECK(item.getItemNamePlural() == L"swords");
}

TEST_CASE_METHOD(ItemFixture, "Item can change its weight", "[Item]")
{
  item.setWeight(6);
  CHECK(item.getWeight() == 6);
}

TEST_CASE_METHOD(ItemFixture, "Item can change melee weapon status", "[Item]")
{
  item.setMeeleWeapon(false);
  CHECK_FALSE(item.isMeeleWeapon());
}

TEST_CASE_METHOD(ItemFixture, "Item can change ranged weapon status", "[Item]")
{
  item.setRangedWeapon(true);
  CHECK(item.isRangedWeapon());
}

TEST_CASE_METHOD(ItemFixture, "Item can change armour status", "[Item]")
{
  item.setArmour(true);
  CHECK(item.isArmour());
}

TEST_CASE_METHOD(ItemFixture, "Item can change resource status", "[Item]")
{
  item.setResource(true);
  CHECK(item.isResource());
}

TEST_CASE_METHOD(ItemFixture, "Item can change mount status", "[Item]")
{
  item.setMount(true);
  CHECK(item.isMount());
}

TEST_CASE_METHOD(ItemFixture, "Item can change provisional status", "[Item]")
{
  item.setProvisional(true);
  CHECK(item.isProvisional());
}

TEST_CASE_METHOD(ItemFixture, "Item can change moves", "[Item]")
{
  item.setMoves(3);
  CHECK(item.getMoves() == 3);
}

TEST_CASE_METHOD(ItemFixture, "Item can change walk capacity", "[Item]")
{
  item.setWalkCapacity(200);
  CHECK(item.getWalkCapacity() == 200);
}

TEST_CASE_METHOD(ItemFixture, "Item can change ride capacity", "[Item]")
{
  item.setRideCapacity(800);
  CHECK(item.getRideCapacity() == 800);
}

TEST_CASE_METHOD(ItemFixture, "Item can change swim capacity", "[Item]")
{
  item.setSwimCapacity(600);
  CHECK(item.getSwimCapacity() == 600);
}

TEST_CASE_METHOD(ItemFixture, "Item can change fly capacity", "[Item]")
{
  item.setFlyCapacity(400);
  CHECK(item.getFlyCapacity() == 400);
}

TEST_CASE_METHOD(ItemFixture, "Item stores zero ship speed by default", "[Item]")
{
  CHECK(item.getShipSpeedHexesPerMonth() == 0);
}

TEST_CASE_METHOD(ItemFixture, "Item can change ship speed", "[Item]")
{
  item.setShipSpeedHexesPerMonth(8);
  CHECK(item.getShipSpeedHexesPerMonth() == 8);
}

TEST_CASE_METHOD(ItemFixture, "Item stores zero ship sailing skill by default", "[Item]")
{
  CHECK(item.getShipSailingSkillRequired() == 0);
}

TEST_CASE_METHOD(ItemFixture, "Item can change ship sailing skill required", "[Item]")
{
  item.setShipSailingSkillRequired(2);
  CHECK(item.getShipSailingSkillRequired() == 2);
}

TEST_CASE_METHOD(ItemFixture, "Item stores zero mages study by default", "[Item]")
{
  CHECK(item.getMagesStudy() == 0);
}

TEST_CASE_METHOD(ItemFixture, "Item can change mages study", "[Item]")
{
  item.setMagesStudy(5);
  CHECK(item.getMagesStudy() == 5);
}

TEST_CASE_METHOD(ItemFixture, "Item stores zero default skill max by default", "[Item]")
{
  CHECK(item.getDefaultSkillMax() == 0);
}

TEST_CASE_METHOD(ItemFixture, "Item can change default skill max", "[Item]")
{
  item.setDefaultSkillMax(3);
  CHECK(item.getDefaultSkillMax() == 3);
}

TEST_CASE_METHOD(ItemFixture, "Item stores empty full text by default", "[Item]")
{
  CHECK(item.getFullText() == L"");
}

TEST_CASE_METHOD(ItemFixture, "Item can set full text", "[Item]")
{
  item.setFullText(L"A fine sword with intricate engravings");
  CHECK(item.getFullText() == L"A fine sword with intricate engravings");
}

TEST_CASE_METHOD(ItemFixture, "Item stores empty resources by default", "[Item]")
{
  CHECK(item.getResources().empty());
}

TEST_CASE_METHOD(ItemFixture, "Item can set resources", "[Item]")
{
  std::map<std::wstring, int> resources { { L"IRON", 10 }, { L"MITHRIL", 2 } };
  item.setResources(resources);
  CHECK(item.getResources() == resources);
}

TEST_CASE_METHOD(ItemFixture, "Item stores empty skills max by default", "[Item]")
{
  CHECK(item.getSkillsMax().empty());
}

TEST_CASE_METHOD(ItemFixture, "Item can set skills max", "[Item]")
{
  std::map<std::wstring, int> skillsMax { { L"WMES", 5 }, { L"COMB", 3 } };
  item.setSkillsMax(skillsMax);
  CHECK(item.getSkillsMax() == skillsMax);
}

TEST_CASE_METHOD(ItemFixture, "Item stores empty production skill by default", "[Item]")
{
  CHECK(item.getProductionSkill().empty());
}

TEST_CASE_METHOD(ItemFixture, "Item can set production skill", "[Item]")
{
  std::map<std::wstring, int> prodSkill { { L"SMIT", 2 } };
  item.setProductionSkill(prodSkill);
  CHECK(item.getProductionSkill() == prodSkill);
}

TEST_CASE_METHOD(ItemFixture, "Item can set production skill level", "[Item]")
{
  item.setProductionSkillLevel(L"SMIT", 2);
  CHECK(item.getProductionSkill().at(L"SMIT") == 2);
}

TEST_CASE_METHOD(ItemFixture, "Item stores empty production help by default", "[Item]")
{
  CHECK(item.getProductionHelp().empty());
}

TEST_CASE_METHOD(ItemFixture, "Item can set production help", "[Item]")
{
  std::map<std::wstring, int> prodHelp { { L"TROW", 1 } };
  item.setProductionHelp(prodHelp);
  CHECK(item.getProductionHelp() == prodHelp);
}

TEST_CASE("Item pluralizes simple nouns", "[Item]")
{
  CHECK(Item::pluralizeName(L"sword") == L"swords");
  CHECK(Item::pluralizeName(L"horse") == L"horses");
}

TEST_CASE("Item pluralizes words ending in f to ves", "[Item]")
{
  CHECK(Item::pluralizeName(L"elf") == L"elves");
  CHECK(Item::pluralizeName(L"dwarf") == L"dwarves");
}

TEST_CASE("Item pluralizes man to men", "[Item]")
{
  CHECK(Item::pluralizeName(L"man") == L"men");
}

TEST_CASE("Item pluralizes compound words ending in man", "[Item]")
{
  CHECK(Item::pluralizeName(L"lizardman") == L"lizardmen");
  CHECK(Item::pluralizeName(L"nobleman") == L"noblemen");
}

TEST_CASE("Item handles empty string in pluralization", "[Item]")
{
  CHECK(Item::pluralizeName(L"") == L"");
}

TEST_CASE("Item stores man status false", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK_FALSE(item.isMan());
}

TEST_CASE("Item stores identifier token with special characters", "[Item]")
{
  Item item(L"TEST_123", L"test", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(item.getIdentifierToken() == L"TEST_123");
}

TEST_CASE("Item stores item name with special characters", "[Item]")
{
  Item item(L"TEST", L"fey-forged blade", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(item.getItemName() == L"fey-forged blade");
}

TEST_CASE("Item stores negative weight", "[Item]")
{
  Item item(L"TEST", L"test", -5, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(item.getWeight() == -5);
}

TEST_CASE("Item stores negative capacity values", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, -100, -200, -300, -400, false);
  CHECK(item.getWalkCapacity() == -100);
  CHECK(item.getRideCapacity() == -200);
  CHECK(item.getSwimCapacity() == -300);
  CHECK(item.getFlyCapacity() == -400);
}

TEST_CASE("Item can set and get man status", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  item.setMan(true);
  CHECK(item.isMan());
}

TEST_CASE("Item can set man status to false", "[Item]")
{
  Item item(L"TEST", L"test", 1, false, false, false, false, false, 0, 0, 0, 0, 0, true);
  item.setMan(false);
  CHECK_FALSE(item.isMan());
}
