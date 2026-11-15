/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/ItemRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <map>
#include <string>

namespace
{
struct ItemRepositoryFixture
{
  ItemRepository repository;
};
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository accepts an item", "[ItemRepository]")
{
  CHECK(repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false));
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository stores the identifier token", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  CHECK(item->getIdentifierToken() == L"SWRD");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository stores the item name", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  CHECK(item->getItemName() == L"sword");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository stores all item properties", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  CHECK(item->getWeight() == 5);
  CHECK(item->isMeeleWeapon());
  CHECK_FALSE(item->isRangedWeapon());
  CHECK_FALSE(item->isArmour());
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository allows multiple items", "[ItemRepository]")
{
  CHECK(repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false));
  CHECK(repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false));
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository reports empty size", "[ItemRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository reports size after adding one item", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository reports size after adding multiple items", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ARM", L"plate armor", 10, false, false, true, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.size() == 3);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository rejects duplicate identifier tokens", "[ItemRepository]")
{
  CHECK(repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false));
  CHECK_FALSE(repository.add(L"SWRD", L"another sword", 6, true, false, false, false, false, 0, 0, 0, 0, 0, false));
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository preserves repository size when rejecting duplicates", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"SWRD", L"another sword", 6, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository provides indexed access", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ARM", L"plate armor", 10, false, false, true, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.at(0).getIdentifierToken() == L"SWRD");
  CHECK(repository.at(1).getIdentifierToken() == L"BOW");
  CHECK(repository.at(2).getIdentifierToken() == L"ARM");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository finds an item by identifier token", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"SWRD");
  CHECK(item != nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository returns nullptr for a missing identifier token", "[ItemRepository]")
{
  CHECK(repository.findByIdentifierToken(L"MISSING") == nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository finds an item by identifier token in a populated repository", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ARM", L"plate armor", 10, false, false, true, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"BOW");
  REQUIRE(item != nullptr);
  CHECK(item->getItemName() == L"bow");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository const find returns nullptr for a missing token", "[ItemRepository]")
{
  const auto& constRepo = repository;
  CHECK(constRepo.findByIdentifierToken(L"MISSING") == nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository const find locates an existing item", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto& constRepo = repository;
  const auto* item = constRepo.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  CHECK(item->getItemName() == L"sword");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository finds an item by name", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto* item = repository.findByItemName(L"sword");
  REQUIRE(item != nullptr);
  CHECK(item->getIdentifierToken() == L"SWRD");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository returns nullptr for a missing item name", "[ItemRepository]")
{
  CHECK(repository.findByItemName(L"missing") == nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository resolves exact token to item", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.resolveRaceNameToToken(L"SWRD") == L"SWRD");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository resolves exact name to token", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.resolveRaceNameToToken(L"sword") == L"SWRD");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository de-pluralizes singular name to resolve token", "[ItemRepository]")
{
  repository.add(L"ELF", L"elf", 10, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  auto* item = repository.findByIdentifierToken(L"ELF");
  REQUIRE(item != nullptr);
  item->setItemNamePlural(L"elves");
  CHECK(repository.resolveRaceNameToToken(L"elves") == L"ELF");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository de-pluralizes men to man for token resolution", "[ItemRepository]")
{
  repository.add(L"LMAN", L"lizardman", 15, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  auto* item = repository.findByIdentifierToken(L"LMAN");
  REQUIRE(item != nullptr);
  item->setItemNamePlural(L"lizardmen");
  CHECK(repository.resolveRaceNameToToken(L"lizardmen") == L"LMAN");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository de-pluralizes ves to f for token resolution", "[ItemRepository]")
{
  repository.add(L"DWARF", L"dwarf", 10, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  auto* item = repository.findByIdentifierToken(L"DWARF");
  REQUIRE(item != nullptr);
  item->setItemNamePlural(L"dwarves");
  CHECK(repository.resolveRaceNameToToken(L"dwarves") == L"DWARF");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository returns unresolved name unchanged", "[ItemRepository]")
{
  CHECK(repository.resolveRaceNameToToken(L"unknown") == L"unknown");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository removes an item by identifier token", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  REQUIRE(repository.removeByIdentifierToken(L"SWRD"));
  CHECK(repository.findByIdentifierToken(L"SWRD") == nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository reports failure when removing a missing token", "[ItemRepository]")
{
  CHECK_FALSE(repository.removeByIdentifierToken(L"MISSING"));
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository decreases size after removal", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  REQUIRE(repository.size() == 2);
  REQUIRE(repository.removeByIdentifierToken(L"SWRD"));
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository removes an item from indexed access", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ARM", L"plate armor", 10, false, false, true, false, false, 0, 0, 0, 0, 0, false);
  repository.removeByIdentifierToken(L"BOW");
  CHECK(repository.at(1).getIdentifierToken() == L"ARM");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository can be cleared", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository clearing resets item lookup", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.clear();
  CHECK(repository.findByIdentifierToken(L"SWRD") == nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository clearing allows a previously used token again", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.clear();
  CHECK(repository.add(L"SWRD", L"long sword", 6, true, false, false, false, false, 0, 0, 0, 0, 0, false));
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository allows updates to stored item name", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  auto* item = repository.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  item->setItemName(L"long sword");
  CHECK(repository.findByIdentifierToken(L"SWRD")->getItemName() == L"long sword");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository allows updates to stored item properties", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  auto* item = repository.findByIdentifierToken(L"SWRD");
  REQUIRE(item != nullptr);
  item->setWeight(7);
  CHECK(repository.findByIdentifierToken(L"SWRD")->getWeight() == 7);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository calculates total weight from item counts", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", 3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  std::map<std::wstring, int> counts { { L"SWRD", 2 }, { L"BOW", 1 } };
  CHECK(repository.calculateTotalWeight(counts) == 13);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository returns zero total weight for empty counts", "[ItemRepository]")
{
  std::map<std::wstring, int> counts;
  CHECK(repository.calculateTotalWeight(counts) == 0);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository ignores missing items in weight calculation", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  std::map<std::wstring, int> counts { { L"SWRD", 2 }, { L"MISSING", 1 } };
  CHECK(repository.calculateTotalWeight(counts) == 10);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository handles negative weights in calculation", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"BOW", L"bow", -3, false, true, false, false, false, 0, 0, 0, 0, 0, false);
  std::map<std::wstring, int> counts { { L"SWRD", 2 }, { L"BOW", 1 } };
  CHECK(repository.calculateTotalWeight(counts) == 7);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository counts man items", "[ItemRepository]")
{
  repository.add(L"MAN", L"man", 10, false, false, false, false, false, 0, 0, 0, 0, 0, true);
  std::map<std::wstring, int> counts { { L"MAN", 5 } };
  CHECK(repository.calculateManItemCount(counts) == 5);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository returns zero for no man items", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  std::map<std::wstring, int> counts { { L"SWRD", 2 } };
  CHECK(repository.calculateManItemCount(counts) == 0);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository ignores missing items in man count", "[ItemRepository]")
{
  repository.add(L"MAN", L"man", 10, false, false, false, false, false, 0, 0, 0, 0, 0, true);
  std::map<std::wstring, int> counts { { L"MAN", 5 }, { L"MISSING", 10 } };
  CHECK(repository.calculateManItemCount(counts) == 5);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository persists all item properties through lookup", "[ItemRepository]")
{
  repository.add(L"HORS", L"horse", 500, false, false, false, false, true, 2, 0, 750, 0, 0, false);
  const auto* item = repository.findByIdentifierToken(L"HORS");
  REQUIRE(item != nullptr);
  CHECK(item->getMoves() == 2);
  CHECK(item->getRideCapacity() == 750);
  CHECK(item->isMount());
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository reports no error on successful operation", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.getLastError() == L"");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository stores item with empty name", "[ItemRepository]")
{
  CHECK(repository.add(L"TEST", L"", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false));
  const auto* item = repository.findByIdentifierToken(L"TEST");
  REQUIRE(item != nullptr);
  CHECK(item->getItemName() == L"");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository handles insertion order", "[ItemRepository]")
{
  repository.add(L"ITEM3", L"third", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ITEM1", L"first", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  repository.add(L"ITEM2", L"second", 1, false, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.at(0).getIdentifierToken() == L"ITEM3");
  CHECK(repository.at(1).getIdentifierToken() == L"ITEM1");
  CHECK(repository.at(2).getIdentifierToken() == L"ITEM2");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository const at provides read-only access", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  const auto& constRepo = repository;
  CHECK(constRepo.at(0).getIdentifierToken() == L"SWRD");
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository token matching is case-insensitive", "[ItemRepository]")
{
  repository.add(L"SWRD", L"sword", 5, true, false, false, false, false, 0, 0, 0, 0, 0, false);
  CHECK(repository.findByIdentifierToken(L"swrd") != nullptr);
}

TEST_CASE_METHOD(ItemRepositoryFixture, "ItemRepository handles man item with zero count", "[ItemRepository]")
{
  repository.add(L"MAN", L"man", 10, false, false, false, false, false, 0, 0, 0, 0, 0, true);
  std::map<std::wstring, int> counts { { L"MAN", 0 } };
  CHECK(repository.calculateManItemCount(counts) == 0);
}
