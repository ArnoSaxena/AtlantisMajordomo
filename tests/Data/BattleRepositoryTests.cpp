/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/BattleRepository.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct BattleMock : Battle
{
  explicit BattleMock(std::wstring identifierValue)
    : Battle([]() { return std::uint64_t { 0 }; }),
      identifier(std::move(identifierValue))
  {
  }

  std::unique_ptr<Battle> clone() const override
  {
    if (failClone)
    {
      throw std::runtime_error("Mock clone failure");
    }
    if (returnNullClone)
    {
      return nullptr;
    }
    return std::make_unique<BattleMock>(*this);
  }

  const std::wstring& getIdentifier() const override { return identifier; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  int getRegionXCoordinate() const override { return x; }
  int getRegionYCoordinate() const override { return y; }
  int getRegionZCoordinate() const override { return z; }
  const std::vector<int>& getAttackerDamagedUnitIds() const override { return attackerDamage; }
  const std::vector<int>& getDefenderDamagedUnitIds() const override { return defenderDamage; }
  const std::vector<int>& getAttackerParticipantUnitIds() const override { return attackerParticipants; }
  const std::vector<int>& getDefenderParticipantUnitIds() const override { return defenderParticipants; }

  std::wstring identifier;
  int month { 8 };
  int year { 7 };
  int x { -4 };
  int y { 6 };
  int z { 1 };
  std::vector<int> attackerDamage;
  std::vector<int> defenderDamage;
  std::vector<int> attackerParticipants;
  std::vector<int> defenderParticipants;
  bool failClone { false };
  bool returnNullClone { false };
};

struct BattleRepositoryFixture
{
  BattleMock& addBattle(std::wstring identifier = L"battle", int month = 8, int year = 7)
  {
    auto battle = std::make_unique<BattleMock>(std::move(identifier));
    battle->month = month;
    battle->year = year;
    BattleMock& result = *battle;
    REQUIRE(repository.add(std::move(battle)));
    return result;
  }

  BattleRepository repository;
};
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository accepts and retains the supplied polymorphic battle", "[BattleRepository]")
{
  const BattleMock& battle = addBattle();
  CHECK(repository.findByIdentifier(L"battle") == &battle);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository rejects duplicates without changing its contents", "[BattleRepository]")
{
  const BattleMock& original = addBattle();
  REQUIRE_FALSE(repository.add(std::make_unique<BattleMock>(L"battle")));
  CHECK(repository.findByIdentifier(L"battle") == &original);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository allows different identifiers in the same period", "[BattleRepository]")
{
  addBattle(L"first");
  CHECK(repository.add(std::make_unique<BattleMock>(L"second")));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository compares identifiers case sensitively", "[BattleRepository]")
{
  addBattle(L"battle");
  CHECK(repository.add(std::make_unique<BattleMock>(L"Battle")));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository rejects null owned battles explicitly", "[BattleRepository]")
{
  CHECK_THROWS_AS(repository.add(std::unique_ptr<Battle> {}), std::invalid_argument);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository identifier lookup returns null when empty", "[BattleRepository]")
{
  CHECK(repository.findByIdentifier(L"missing") == nullptr);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository identifier lookup rejects a different identifier", "[BattleRepository]")
{
  addBattle();
  CHECK(repository.findByIdentifier(L"missing") == nullptr);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository identifier lookup searches beyond the first battle", "[BattleRepository]")
{
  addBattle(L"first");
  const BattleMock& second = addBattle(L"second");
  CHECK(repository.findByIdentifier(L"second") == &second);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository reports no available periods when empty", "[BattleRepository]")
{
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository reports periods as month year pairs", "[BattleRepository]")
{
  addBattle();
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository sorts years before months in descending periods", "[BattleRepository]")
{
  addBattle(L"older", 12, 6);
  addBattle(L"newer", 1, 7);
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 1, 7 }, { 12, 6 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository sorts months descending within a year", "[BattleRepository]")
{
  addBattle(L"early", 1, 7);
  addBattle(L"late", 12, 7);
  addBattle(L"middle", 6, 7);
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 12, 7 }, { 6, 7 }, { 1, 7 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository lists a shared period only once", "[BattleRepository]")
{
  addBattle(L"first");
  addBattle(L"second");
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository accepts the first positive year in period listings", "[BattleRepository]")
{
  addBattle(L"first-year", 1, 1);
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 1, 1 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository omits month zero from available periods", "[BattleRepository]")
{
  addBattle(L"invalid", 0, 7);
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository omits negative months from available periods", "[BattleRepository]")
{
  addBattle(L"invalid", -1, 7);
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository omits months above twelve from available periods", "[BattleRepository]")
{
  addBattle(L"invalid", 13, 7);
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository omits year zero from available periods", "[BattleRepository]")
{
  addBattle(L"invalid", 8, 0);
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository omits negative years from available periods", "[BattleRepository]")
{
  addBattle(L"invalid", 8, -1);
  CHECK(repository.getAvailablePeriodsDescending().empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository retains valid periods alongside invalid ones", "[BattleRepository]")
{
  addBattle(L"invalid", 13, 9);
  addBattle(L"valid", 8, 7);
  CHECK(repository.getAvailablePeriodsDescending() == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository latest period fails without changing outputs when empty", "[BattleRepository]")
{
  int month = 4;
  int year = 5;
  REQUIRE_FALSE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(4, 5));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository latest period reports failure with only invalid periods", "[BattleRepository]")
{
  addBattle(L"invalid", 13, 7);
  int month = 4;
  int year = 5;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository latest period selects the newest year", "[BattleRepository]")
{
  addBattle(L"older", 12, 6);
  addBattle(L"newer", 1, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(1, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository latest period selects the newest month within a year", "[BattleRepository]")
{
  addBattle(L"early", 1, 7);
  addBattle(L"late", 12, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(12, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository latest period ignores invalid newer entries", "[BattleRepository]")
{
  addBattle(L"invalid", 13, 9);
  addBattle(L"valid", 8, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository period lookup returns no battles when empty", "[BattleRepository]")
{
  CHECK(repository.findByPeriod(8, 7).empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository period lookup returns matching battles in insertion order", "[BattleRepository]")
{
  const BattleMock& first = addBattle(L"first");
  addBattle(L"other", 9, 7);
  const BattleMock& second = addBattle(L"second");
  CHECK(repository.findByPeriod(8, 7) == std::vector<const Battle*> { &first, &second });
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository period lookup rejects a different month", "[BattleRepository]")
{
  addBattle();
  CHECK(repository.findByPeriod(9, 7).empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository period lookup rejects a different year", "[BattleRepository]")
{
  addBattle();
  CHECK(repository.findByPeriod(8, 8).empty());
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository period lookup preserves exact matching for unvalidated dates", "[BattleRepository]")
{
  const BattleMock& battle = addBattle(L"unvalidated", 0, 0);
  CHECK(repository.findByPeriod(0, 0) == std::vector<const Battle*> { &battle });
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup returns false when empty", "[BattleRepository]")
{
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-4, 6, 1, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup accepts matching coordinates and period", "[BattleRepository]")
{
  addBattle();
  CHECK(repository.hasBattleInRegionForPeriod(-4, 6, 1, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup rejects a different x coordinate", "[BattleRepository]")
{
  addBattle();
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-3, 6, 1, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup rejects a different y coordinate", "[BattleRepository]")
{
  addBattle();
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-4, 5, 1, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup rejects a different z coordinate", "[BattleRepository]")
{
  addBattle();
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-4, 6, 2, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup rejects a different month", "[BattleRepository]")
{
  addBattle();
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-4, 6, 1, 9, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup rejects a different year", "[BattleRepository]")
{
  addBattle();
  CHECK_FALSE(repository.hasBattleInRegionForPeriod(-4, 6, 1, 8, 8));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository region lookup searches beyond the first battle", "[BattleRepository]")
{
  addBattle(L"other").x = 0;
  addBattle(L"matching");
  CHECK(repository.hasBattleInRegionForPeriod(-4, 6, 1, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup returns false when empty", "[BattleRepository]")
{
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup searches the attacker list", "[BattleRepository]")
{
  addBattle().attackerDamage = { 11, 42 };
  CHECK(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup searches the defender list", "[BattleRepository]")
{
  addBattle().defenderDamage = { 11, 42 };
  CHECK(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup rejects an absent unit", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerDamage = { 11 };
  battle.defenderDamage = { 22 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup rejects a different month", "[BattleRepository]")
{
  addBattle().attackerDamage = { 42 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(42, 9, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup rejects a different year", "[BattleRepository]")
{
  addBattle().defenderDamage = { 42 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 8));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup rejects unit zero even when listed", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerDamage = { 0 };
  battle.defenderDamage = { 0 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(0, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup rejects negative unit identifiers even when listed", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerDamage = { -42 };
  battle.defenderDamage = { -42 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(-42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup searches beyond a battle without damage", "[BattleRepository]")
{
  addBattle(L"undamaged");
  addBattle(L"damaged").defenderDamage = { 42 };
  CHECK(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository damage lookup continues after a different period", "[BattleRepository]")
{
  addBattle(L"other-period", 9, 7).attackerDamage = { 42 };
  addBattle(L"matching").defenderDamage = { 42 };
  CHECK(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository does not treat participation alone as damage", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerParticipants = { 42 };
  battle.defenderParticipants = { 42 };
  CHECK_FALSE(repository.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup returns false when empty", "[BattleRepository]")
{
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup searches the attacker list", "[BattleRepository]")
{
  addBattle().attackerParticipants = { 11, 42 };
  CHECK(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup searches the defender list", "[BattleRepository]")
{
  addBattle().defenderParticipants = { 11, 42 };
  CHECK(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup rejects an absent unit", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerParticipants = { 11 };
  battle.defenderParticipants = { 22 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup rejects a different month", "[BattleRepository]")
{
  addBattle().attackerParticipants = { 42 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(42, 9, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup rejects a different year", "[BattleRepository]")
{
  addBattle().defenderParticipants = { 42 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(42, 8, 8));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup rejects unit zero even when listed", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerParticipants = { 0 };
  battle.defenderParticipants = { 0 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(0, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup rejects negative unit identifiers even when listed", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerParticipants = { -42 };
  battle.defenderParticipants = { -42 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(-42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup searches beyond a battle without participants", "[BattleRepository]")
{
  addBattle(L"unrelated");
  addBattle(L"matching").defenderParticipants = { 42 };
  CHECK(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository participant lookup continues after a different period", "[BattleRepository]")
{
  addBattle(L"other-period", 9, 7).attackerParticipants = { 42 };
  addBattle(L"matching").defenderParticipants = { 42 };
  CHECK(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository does not treat damage alone as participation", "[BattleRepository]")
{
  BattleMock& battle = addBattle();
  battle.attackerDamage = { 42 };
  battle.defenderDamage = { 42 };
  CHECK_FALSE(repository.isParticipantInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository clearing removes battles and releases their identifiers", "[BattleRepository]")
{
  addBattle();
  repository.clear();
  CHECK(repository.findByIdentifier(L"battle") == nullptr);
  CHECK(repository.add(std::make_unique<BattleMock>(L"battle")));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copies own independent polymorphic battles", "[BattleRepository]")
{
  BattleMock& original = addBattle();
  original.attackerDamage = { 42 };
  BattleRepository copy(repository);
  REQUIRE(copy.findByIdentifier(L"battle") != nullptr);
  CHECK(copy.findByIdentifier(L"battle") != &original);
  CHECK(copy.isUnitDamagedInAnyBattleForPeriod(42, 8, 7));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copies preserve battle insertion order", "[BattleRepository]")
{
  addBattle(L"first");
  addBattle(L"second");
  BattleRepository copy(repository);
  REQUIRE(copy.size() == 2);
  CHECK(std::make_pair(copy.at(0).getIdentifier(), copy.at(1).getIdentifier()) ==
        std::make_pair(std::wstring(L"first"), std::wstring(L"second")));
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository clearing a copy preserves the source", "[BattleRepository]")
{
  const BattleMock& original = addBattle();
  BattleRepository copy(repository);
  copy.clear();
  CHECK(repository.findByIdentifier(L"battle") == &original);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copy assignment replaces existing contents", "[BattleRepository]")
{
  addBattle(L"replacement");
  BattleRepository target;
  REQUIRE(target.add(std::make_unique<BattleMock>(L"old")));
  target = repository;
  REQUIRE(target.size() == 1);
  CHECK(target.at(0).getIdentifier() == L"replacement");
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copy assignment owns independent battle objects", "[BattleRepository]")
{
  const BattleMock& original = addBattle();
  BattleRepository target;
  target = repository;
  REQUIRE(target.findByIdentifier(L"battle") != nullptr);
  CHECK(target.findByIdentifier(L"battle") != &original);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copy construction propagates clone failures", "[BattleRepository]")
{
  addBattle().failClone = true;
  CHECK_THROWS_WITH(BattleRepository { repository }, "Mock clone failure");
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository copy construction rejects null clones explicitly", "[BattleRepository]")
{
  addBattle().returnNullClone = true;
  CHECK_THROWS_AS(BattleRepository { repository }, std::logic_error);
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository failed copy assignment preserves its previous contents", "[BattleRepository]")
{
  addBattle(L"first");
  addBattle(L"failing").failClone = true;
  BattleRepository target;
  auto original = std::make_unique<BattleMock>(L"original");
  const Battle* originalAddress = original.get();
  REQUIRE(target.add(std::move(original)));
  REQUIRE_THROWS_WITH(target = repository, "Mock clone failure");
  CHECK(target.findByPeriod(8, 7) == std::vector<const Battle*> { originalAddress });
}

TEST_CASE_METHOD(BattleRepositoryFixture, "BattleRepository self assignment does not attempt to clone battles", "[BattleRepository]")
{
  addBattle().failClone = true;
  CHECK_NOTHROW(repository = repository);
}
