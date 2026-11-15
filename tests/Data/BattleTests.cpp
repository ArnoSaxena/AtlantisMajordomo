/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Battle.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct IdentifierWordsMock
{
  std::array<std::uint64_t, 2> words;
  std::size_t calls { 0 };

  std::uint64_t next() { return words.at(calls++); }
};

struct BattleFixture
{
  Battle battle { []() { return std::uint64_t { 0 }; } };
};
}

TEST_CASE("Battle formats identifier words into ordered lowercase hexadecimal fields", "[Battle]")
{
  IdentifierWordsMock source {{ 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL }};
  Battle battle([&]() { return source.next(); });
  CHECK(battle.getIdentifier() == L"01234567-89ab-cdef-fedc-ba9876543210");
  CHECK(source.calls == 2);
}

TEST_CASE("Battle pads every identifier field with leading zeros", "[Battle]")
{
  IdentifierWordsMock source {{ 0x0000000100020003ULL, 0x0004000000000005ULL }};
  Battle battle([&]() { return source.next(); });
  CHECK(battle.getIdentifier() == L"00000001-0002-0003-0004-000000000005");
}

TEST_CASE("Battle formats an all zero identifier source", "[Battle]")
{
  IdentifierWordsMock source {{ 0, 0 }};
  Battle battle([&]() { return source.next(); });
  CHECK(battle.getIdentifier() == L"00000000-0000-0000-0000-000000000000");
}

TEST_CASE("Battle preserves all identifier bits at the unsigned maximum", "[Battle]")
{
  IdentifierWordsMock source {{
    std::numeric_limits<std::uint64_t>::max(),
    std::numeric_limits<std::uint64_t>::max()
  }};
  Battle battle([&]() { return source.next(); });
  CHECK(battle.getIdentifier() == L"ffffffff-ffff-ffff-ffff-ffffffffffff");
}

TEST_CASE("Battle propagates identifier source failures", "[Battle]")
{
  const Battle::IdentifierWordSource source = []() -> std::uint64_t {
    throw std::runtime_error("Identifier source unavailable");
  };
  CHECK_THROWS_WITH(Battle(source), "Identifier source unavailable");
}

TEST_CASE("Battle propagates failure while obtaining the second identifier word", "[Battle]")
{
  int calls = 0;
  const Battle::IdentifierWordSource source = [&]() -> std::uint64_t {
    if (++calls == 2)
    {
      throw std::runtime_error("Second identifier word unavailable");
    }
    return 0;
  };
  CHECK_THROWS_WITH(Battle(source), "Second identifier word unavailable");
}

TEST_CASE_METHOD(BattleFixture, "Battle attacker damage does not change defender damage", "[Battle]")
{
  battle.addDefenderDamagedUnitId(202);
  battle.addAttackerDamagedUnitId(101);
  CHECK(battle.getDefenderDamagedUnitIds() == std::vector<int> { 202 });
}

TEST_CASE_METHOD(BattleFixture, "Battle defender damage does not change attacker damage", "[Battle]")
{
  battle.addAttackerDamagedUnitId(101);
  battle.addDefenderDamagedUnitId(202);
  CHECK(battle.getAttackerDamagedUnitIds() == std::vector<int> { 101 });
}

TEST_CASE_METHOD(BattleFixture, "Battle attacker participants do not change defender participants", "[Battle]")
{
  battle.addDefenderParticipantUnitId(202);
  battle.addAttackerParticipantUnitId(101);
  CHECK(battle.getDefenderParticipantUnitIds() == std::vector<int> { 202 });
}

TEST_CASE_METHOD(BattleFixture, "Battle defender participants do not change attacker participants", "[Battle]")
{
  battle.addAttackerParticipantUnitId(101);
  battle.addDefenderParticipantUnitId(202);
  CHECK(battle.getAttackerParticipantUnitIds() == std::vector<int> { 101 });
}

TEST_CASE_METHOD(BattleFixture, "Battle clearing attacker damage preserves other battle collections", "[Battle]")
{
  battle.addAttackerDamagedUnitId(101);
  battle.addAttackerDamagedUnitId(102);
  battle.addDefenderDamagedUnitId(202);
  battle.addAttackerParticipantUnitId(101);
  battle.clearAttackerDamagedUnitIds();
  CHECK(battle.getAttackerDamagedUnitIds().empty());
  CHECK(battle.getDefenderDamagedUnitIds() == std::vector<int> { 202 });
  CHECK(battle.getAttackerParticipantUnitIds() == std::vector<int> { 101 });
}

TEST_CASE_METHOD(BattleFixture, "Battle clearing defender damage preserves other battle collections", "[Battle]")
{
  battle.addAttackerDamagedUnitId(101);
  battle.addDefenderDamagedUnitId(201);
  battle.addDefenderDamagedUnitId(202);
  battle.addDefenderParticipantUnitId(202);
  battle.clearDefenderDamagedUnitIds();
  CHECK(battle.getDefenderDamagedUnitIds().empty());
  CHECK(battle.getAttackerDamagedUnitIds() == std::vector<int> { 101 });
  CHECK(battle.getDefenderParticipantUnitIds() == std::vector<int> { 202 });
}

TEST_CASE_METHOD(BattleFixture, "Battle appends spoils without replacing earlier entries", "[Battle]")
{
  battle.addSpoil({ 75, L"SILV" });
  battle.addSpoil({ 2, L"SWOR" });
  const BattleSpoil& first = battle.getSpoils().at(0);
  const BattleSpoil& second = battle.getSpoils().at(1);
  CHECK(std::make_pair(first.amount, first.token) == std::make_pair(75, std::wstring(L"SILV")));
  CHECK(std::make_pair(second.amount, second.token) == std::make_pair(2, std::wstring(L"SWOR")));
}

TEST_CASE_METHOD(BattleFixture, "Battle clearing spoils preserves both participant lists", "[Battle]")
{
  battle.addAttackerParticipantUnitId(101);
  battle.addDefenderParticipantUnitId(202);
  battle.addSpoil({ 75, L"SILV" });
  battle.addSpoil({ 2, L"SWOR" });
  battle.clearSpoils();
  CHECK(battle.getSpoils().empty());
  CHECK(battle.getAttackerParticipantUnitIds() == std::vector<int> { 101 });
  CHECK(battle.getDefenderParticipantUnitIds() == std::vector<int> { 202 });
}

TEST_CASE_METHOD(BattleFixture, "Battle cloning preserves a restored identifier and participants", "[Battle]")
{
  battle.setIdentifier(L"restored-identifier");
  battle.addAttackerParticipantUnitId(101);
  const auto copy = battle.clone();
  CHECK(copy->getIdentifier() == L"restored-identifier");
  CHECK(copy->getAttackerParticipantUnitIds() == std::vector<int> { 101 });
}

TEST_CASE_METHOD(BattleFixture, "Battle cloned damage is independent of later source changes", "[Battle]")
{
  battle.addAttackerDamagedUnitId(101);
  const auto copy = battle.clone();
  battle.clearAttackerDamagedUnitIds();
  CHECK(copy->getAttackerDamagedUnitIds() == std::vector<int> { 101 });
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get full text", "[Battle]")
{
  battle.setFullText(L"A great battle ensued");
  CHECK(battle.getFullText() == L"A great battle ensued");
}

TEST_CASE_METHOD(BattleFixture, "Battle stores empty full text", "[Battle]")
{
  battle.setFullText(L"");
  CHECK(battle.getFullText().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle full text defaults to empty", "[Battle]")
{
  CHECK(battle.getFullText().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get attacker unit ID", "[Battle]")
{
  battle.setAttackerUnitId(101);
  CHECK(battle.getAttackerUnitId() == 101);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero attacker unit ID", "[Battle]")
{
  battle.setAttackerUnitId(0);
  CHECK(battle.getAttackerUnitId() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle attacker unit ID defaults to zero", "[Battle]")
{
  CHECK(battle.getAttackerUnitId() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get attacker unit name", "[Battle]")
{
  battle.setAttackerUnitName(L"Elven Archers");
  CHECK(battle.getAttackerUnitName() == L"Elven Archers");
}

TEST_CASE_METHOD(BattleFixture, "Battle stores empty attacker unit name", "[Battle]")
{
  battle.setAttackerUnitName(L"");
  CHECK(battle.getAttackerUnitName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle attacker unit name defaults to empty", "[Battle]")
{
  CHECK(battle.getAttackerUnitName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get defender unit ID", "[Battle]")
{
  battle.setDefenderUnitId(202);
  CHECK(battle.getDefenderUnitId() == 202);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero defender unit ID", "[Battle]")
{
  battle.setDefenderUnitId(0);
  CHECK(battle.getDefenderUnitId() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle defender unit ID defaults to zero", "[Battle]")
{
  CHECK(battle.getDefenderUnitId() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get defender unit name", "[Battle]")
{
  battle.setDefenderUnitName(L"Dwarven Guard");
  CHECK(battle.getDefenderUnitName() == L"Dwarven Guard");
}

TEST_CASE_METHOD(BattleFixture, "Battle stores empty defender unit name", "[Battle]")
{
  battle.setDefenderUnitName(L"");
  CHECK(battle.getDefenderUnitName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle defender unit name defaults to empty", "[Battle]")
{
  CHECK(battle.getDefenderUnitName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get region coordinates", "[Battle]")
{
  battle.setRegionCoordinates(5, 10, 2);
  CHECK(battle.getRegionXCoordinate() == 5);
  CHECK(battle.getRegionYCoordinate() == 10);
  CHECK(battle.getRegionZCoordinate() == 2);
}

TEST_CASE_METHOD(BattleFixture, "Battle rejects negative coordinates", "[Battle]")
{
  battle.setRegionCoordinates(-5, -10, -1);
  CHECK(battle.getRegionXCoordinate() == 0);
  CHECK(battle.getRegionYCoordinate() == 0);
  CHECK(battle.getRegionZCoordinate() == 1);
}

TEST_CASE_METHOD(BattleFixture, "Battle coordinates default to zero X and Y, one for Z", "[Battle]")
{
  CHECK(battle.getRegionXCoordinate() == 0);
  CHECK(battle.getRegionYCoordinate() == 0);
  CHECK(battle.getRegionZCoordinate() == 1);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get month", "[Battle]")
{
  battle.setMonth(6);
  CHECK(battle.getMonth() == 6);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero month", "[Battle]")
{
  battle.setMonth(0);
  CHECK(battle.getMonth() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle month defaults to zero", "[Battle]")
{
  CHECK(battle.getMonth() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get year", "[Battle]")
{
  battle.setYear(1);
  CHECK(battle.getYear() == 1);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero year", "[Battle]")
{
  battle.setYear(0);
  CHECK(battle.getYear() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle year defaults to zero", "[Battle]")
{
  CHECK(battle.getYear() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get region type", "[Battle]")
{
  battle.setRegionType(L"Plain");
  CHECK(battle.getRegionType() == L"Plain");
}

TEST_CASE_METHOD(BattleFixture, "Battle stores empty region type", "[Battle]")
{
  battle.setRegionType(L"");
  CHECK(battle.getRegionType().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle region type defaults to empty", "[Battle]")
{
  CHECK(battle.getRegionType().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get province name", "[Battle]")
{
  battle.setProvinceName(L"Thalassa");
  CHECK(battle.getProvinceName() == L"Thalassa");
}

TEST_CASE_METHOD(BattleFixture, "Battle stores empty province name", "[Battle]")
{
  battle.setProvinceName(L"");
  CHECK(battle.getProvinceName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle province name defaults to empty", "[Battle]")
{
  CHECK(battle.getProvinceName().empty());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get region found in repository flag", "[Battle]")
{
  battle.setRegionFoundInRepository(true);
  CHECK(battle.isRegionFoundInRepository());
}

TEST_CASE_METHOD(BattleFixture, "Battle region found flag can be set to false", "[Battle]")
{
  battle.setRegionFoundInRepository(false);
  CHECK_FALSE(battle.isRegionFoundInRepository());
}

TEST_CASE_METHOD(BattleFixture, "Battle region found flag defaults to false", "[Battle]")
{
  CHECK_FALSE(battle.isRegionFoundInRepository());
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get attacker losses", "[Battle]")
{
  battle.setAttackerLosses(15);
  CHECK(battle.getAttackerLosses() == 15);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero attacker losses", "[Battle]")
{
  battle.setAttackerLosses(0);
  CHECK(battle.getAttackerLosses() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle attacker losses default to zero", "[Battle]")
{
  CHECK(battle.getAttackerLosses() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle can set and get defender losses", "[Battle]")
{
  battle.setDefenderLosses(8);
  CHECK(battle.getDefenderLosses() == 8);
}

TEST_CASE_METHOD(BattleFixture, "Battle stores zero defender losses", "[Battle]")
{
  battle.setDefenderLosses(0);
  CHECK(battle.getDefenderLosses() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle defender losses default to zero", "[Battle]")
{
  CHECK(battle.getDefenderLosses() == 0);
}

TEST_CASE_METHOD(BattleFixture, "Battle metadata remains independent", "[Battle]")
{
  battle.setAttackerUnitId(101);
  battle.setDefenderUnitId(202);
  battle.setMonth(6);
  battle.setYear(1);
  battle.setAttackerLosses(10);
  battle.setDefenderLosses(5);
  CHECK(battle.getAttackerUnitId() == 101);
  CHECK(battle.getDefenderUnitId() == 202);
  CHECK(battle.getMonth() == 6);
  CHECK(battle.getYear() == 1);
  CHECK(battle.getAttackerLosses() == 10);
  CHECK(battle.getDefenderLosses() == 5);
}
