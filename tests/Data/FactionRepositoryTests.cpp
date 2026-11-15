/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/FactionRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <map>
#include <string>

namespace
{
struct FactionRepositoryFixture
{
  FactionRepository repository;
};
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository accepts a faction", "[FactionRepository]")
{
  CHECK(repository.add(42, L"Dwarves", true, 8, 7));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores the faction number", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getFactionNumber() == 42);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores the faction name", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getName() == L"Dwarves");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores main faction status", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->isMainFaction());
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores period information", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getMonth() == 8);
  CHECK(faction->getYear() == 7);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores password", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"secret");
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getPassword() == L"secret");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores taxed regions information", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 5, 10);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getTaxedOrTradedRegionsCurrent() == 5);
  CHECK(faction->getTaxedOrTradedRegionsMax() == 10);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores quartermasters information", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 2, 4);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getQuartermastersCurrent() == 2);
  CHECK(faction->getQuartermastersMax() == 4);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores mages information", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 3, 5);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getMagesCurrent() == 3);
  CHECK(faction->getMagesMax() == 5);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores apprentices information", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 7, 10);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getApprenticesCurrent() == 7);
  CHECK(faction->getApprenticesMax() == 10);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores command unit number", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 123);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getCommandUnitNumber() == 123);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores default attitude", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Friendly);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getDefaultAttitude() == Faction::Attitude::Friendly);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores declared attitudes", "[FactionRepository]")
{
  std::map<int, Faction::Attitude> attitudes { { 1, Faction::Attitude::Ally }, { 2, Faction::Attitude::Hostile } };
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, attitudes);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getDeclaredAttitudes() == attitudes);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores unclaimed silver", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, {}, 500);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getUnclaimedSilver() == 500);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository allows multiple factions", "[FactionRepository]")
{
  CHECK(repository.add(42, L"Dwarves", true));
  CHECK(repository.add(43, L"Elves", false));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository reports empty size", "[FactionRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository reports size after adding one faction", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository reports size after adding multiple factions", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  repository.add(44, L"Men", false);
  CHECK(repository.size() == 3);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository rejects duplicate faction numbers", "[FactionRepository]")
{
  CHECK(repository.add(42, L"Dwarves", true));
  CHECK_FALSE(repository.add(42, L"Another", false));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository preserves repository size when rejecting duplicates", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(42, L"Another", false);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository provides indexed access", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  repository.add(44, L"Men", false);
  CHECK(repository.at(0).getFactionNumber() == 42);
  CHECK(repository.at(1).getFactionNumber() == 43);
  CHECK(repository.at(2).getFactionNumber() == 44);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository finds a faction by number", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto* faction = repository.findByNumber(42);
  CHECK(faction != nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository returns nullptr for a missing faction", "[FactionRepository]")
{
  CHECK(repository.findByNumber(999) == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository finds a faction by number in a populated repository", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  repository.add(44, L"Men", false);
  const auto* faction = repository.findByNumber(43);
  REQUIRE(faction != nullptr);
  CHECK(faction->getName() == L"Elves");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository const find returns nullptr for a missing faction", "[FactionRepository]")
{
  const auto& constRepo = repository;
  CHECK(constRepo.findByNumber(999) == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository const find locates an existing faction", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto& constRepo = repository;
  const auto* faction = constRepo.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getName() == L"Dwarves");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository finds the main faction", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  repository.add(43, L"Elves", true);
  repository.add(44, L"Men", false);
  const auto* main = repository.getMainFaction();
  REQUIRE(main != nullptr);
  CHECK(main->getFactionNumber() == 43);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository returns nullptr for main faction when none exists", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  repository.add(43, L"Elves", false);
  CHECK(repository.getMainFaction() == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository returns the main faction number", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  repository.add(43, L"Elves", true);
  CHECK(repository.getMainFactionNumber() == 43);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository returns zero for main faction number when none exists", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  CHECK(repository.getMainFactionNumber() == 0);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository const getMainFaction returns nullptr when none exists", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  const auto& constRepo = repository;
  CHECK(constRepo.getMainFaction() == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository const getMainFaction locates the main faction", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  repository.add(43, L"Elves", true);
  const auto& constRepo = repository;
  const auto* main = constRepo.getMainFaction();
  REQUIRE(main != nullptr);
  CHECK(main->getFactionNumber() == 43);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository updates a faction name", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  REQUIRE(repository.updateName(42, L"Dwarven Folk"));
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getName() == L"Dwarven Folk");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository fails to update name for missing faction", "[FactionRepository]")
{
  CHECK_FALSE(repository.updateName(999, L"Other"));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository sets main faction status", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  REQUIRE(repository.setMainFaction(42, true));
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->isMainFaction());
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository fails to set main faction for missing faction", "[FactionRepository]")
{
  CHECK_FALSE(repository.setMainFaction(999, true));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository unsets main faction status", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  REQUIRE(repository.setMainFaction(42, false));
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK_FALSE(faction->isMainFaction());
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository removes a faction by number", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  REQUIRE(repository.removeByNumber(42));
  CHECK(repository.findByNumber(42) == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository reports failure when removing a missing faction", "[FactionRepository]")
{
  CHECK_FALSE(repository.removeByNumber(999));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository decreases size after removal", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  REQUIRE(repository.size() == 2);
  REQUIRE(repository.removeByNumber(42));
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository removes a faction from indexed access", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  repository.add(44, L"Men", false);
  repository.removeByNumber(43);
  CHECK(repository.at(1).getFactionNumber() == 44);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository can be cleared", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.add(43, L"Elves", false);
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository clearing resets faction lookup", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.clear();
  CHECK(repository.findByNumber(42) == nullptr);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository clearing allows a previously used number again", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  repository.clear();
  CHECK(repository.add(42, L"Another", false));
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository allows updates to stored faction name", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  faction->setName(L"Dwarven Kingdom");
  CHECK(repository.findByNumber(42)->getName() == L"Dwarven Kingdom");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository allows updates to stored faction main status", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", false);
  auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  faction->setMainFaction(true);
  CHECK(repository.getMainFactionNumber() == 42);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository persists period information through lookup", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true, 8, 7);
  const auto* faction = repository.findByNumber(42);
  REQUIRE(faction != nullptr);
  CHECK(faction->getMonth() == 8);
  CHECK(faction->getYear() == 7);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository reports no error on successful operation", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  CHECK(repository.getLastError() == L"");
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores faction with zero faction number", "[FactionRepository]")
{
  CHECK(repository.add(0, L"Test", true));
  const auto* faction = repository.findByNumber(0);
  REQUIRE(faction != nullptr);
  CHECK(faction->getFactionNumber() == 0);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository stores faction with negative faction number", "[FactionRepository]")
{
  CHECK(repository.add(-1, L"Test", false));
  const auto* faction = repository.findByNumber(-1);
  REQUIRE(faction != nullptr);
  CHECK(faction->getFactionNumber() == -1);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository handles faction insertion order", "[FactionRepository]")
{
  repository.add(100, L"Alpha", false);
  repository.add(50, L"Beta", false);
  repository.add(150, L"Gamma", false);
  CHECK(repository.at(0).getFactionNumber() == 100);
  CHECK(repository.at(1).getFactionNumber() == 50);
  CHECK(repository.at(2).getFactionNumber() == 150);
}

TEST_CASE_METHOD(FactionRepositoryFixture, "FactionRepository const at provides read-only access", "[FactionRepository]")
{
  repository.add(42, L"Dwarves", true);
  const auto& constRepo = repository;
  CHECK(constRepo.at(0).getFactionNumber() == 42);
}
