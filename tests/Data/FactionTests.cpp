/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Faction.hpp"
#include "Data/UnitRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>

namespace
{
struct FactionFixture
{
  Faction faction { 42, L"Dwarves", true, 8, 7 };
};
}

TEST_CASE_METHOD(FactionFixture, "Faction stores the faction number", "[Faction]")
{
  CHECK(faction.getFactionNumber() == 42);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores the faction name", "[Faction]")
{
  CHECK(faction.getName() == L"Dwarves");
}

TEST_CASE_METHOD(FactionFixture, "Faction stores main faction status", "[Faction]")
{
  CHECK(faction.isMainFaction());
}

TEST_CASE_METHOD(FactionFixture, "Faction stores the report month", "[Faction]")
{
  CHECK(faction.getMonth() == 8);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores the report year", "[Faction]")
{
  CHECK(faction.getYear() == 7);
}

TEST_CASE("Faction construction accepts period defaults", "[Faction]")
{
  Faction faction(42, L"Dwarves", true);
  CHECK(faction.getMonth() == 0);
  CHECK(faction.getYear() == 0);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores the password", "[Faction]")
{
  Faction f(42, L"Dwarves", true, 8, 7, L"secret");
  CHECK(f.getPassword() == L"secret");
}

TEST_CASE("Faction stores empty password by default", "[Faction]")
{
  Faction faction(42, L"Dwarves", true);
  CHECK(faction.getPassword() == L"");
}

TEST_CASE_METHOD(FactionFixture, "Faction stores taxed regions current count", "[Faction]")
{
  Faction f(42, L"Dwarves", true, 8, 7, L"", 5);
  CHECK(f.getTaxedOrTradedRegionsCurrent() == 5);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores taxed regions max count", "[Faction]")
{
  Faction f(42, L"Dwarves", true, 8, 7, L"", 5, 10);
  CHECK(f.getTaxedOrTradedRegionsMax() == 10);
}

TEST_CASE("Faction stores quartermasters current count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 2);
  CHECK(faction.getQuartermastersCurrent() == 2);
}

TEST_CASE("Faction stores quartermasters max count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 2, 4);
  CHECK(faction.getQuartermastersMax() == 4);
}

TEST_CASE("Faction stores mages current count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 3);
  CHECK(faction.getMagesCurrent() == 3);
}

TEST_CASE("Faction stores mages max count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 3, 5);
  CHECK(faction.getMagesMax() == 5);
}

TEST_CASE("Faction stores apprentices current count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 7);
  CHECK(faction.getApprenticesCurrent() == 7);
}

TEST_CASE("Faction stores apprentices max count", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 7, 10);
  CHECK(faction.getApprenticesMax() == 10);
}

TEST_CASE("Faction stores command unit number", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 123);
  CHECK(faction.getCommandUnitNumber() == 123);
}

TEST_CASE("Faction stores default attitude", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Friendly);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Friendly);
}

TEST_CASE("Faction default attitude is neutral by default", "[Faction]")
{
  Faction faction(42, L"Dwarves", true);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Neutral);
}

TEST_CASE("Faction stores declared attitudes", "[Faction]")
{
  std::map<int, Faction::Attitude> attitudes { { 1, Faction::Attitude::Ally }, { 2, Faction::Attitude::Hostile } };
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, attitudes);
  CHECK(faction.getDeclaredAttitudes() == attitudes);
}

TEST_CASE("Faction stores unclaimed silver", "[Faction]")
{
  Faction faction(42, L"Dwarves", true, 8, 7, L"", 0, 0, 0, 0, 0, 0, 0, 0, 0, Faction::Attitude::Neutral, {}, 500);
  CHECK(faction.getUnclaimedSilver() == 500);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores zero unclaimed silver by default", "[Faction]")
{
  CHECK(faction.getUnclaimedSilver() == 0);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change its name", "[Faction]")
{
  faction.setName(L"Elves");
  CHECK(faction.getName() == L"Elves");
}

TEST_CASE_METHOD(FactionFixture, "Faction can change main faction status", "[Faction]")
{
  faction.setMainFaction(false);
  CHECK_FALSE(faction.isMainFaction());
}

TEST_CASE_METHOD(FactionFixture, "Faction can change its month", "[Faction]")
{
  faction.setMonth(9);
  CHECK(faction.getMonth() == 9);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change its year", "[Faction]")
{
  faction.setYear(8);
  CHECK(faction.getYear() == 8);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change its password", "[Faction]")
{
  faction.setPassword(L"newpass");
  CHECK(faction.getPassword() == L"newpass");
}

TEST_CASE_METHOD(FactionFixture, "Faction can change taxed regions current count", "[Faction]")
{
  faction.setTaxedOrTradedRegionsCurrent(7);
  CHECK(faction.getTaxedOrTradedRegionsCurrent() == 7);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change taxed regions max count", "[Faction]")
{
  faction.setTaxedOrTradedRegionsMax(15);
  CHECK(faction.getTaxedOrTradedRegionsMax() == 15);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change quartermasters current count", "[Faction]")
{
  faction.setQuartermastersCurrent(4);
  CHECK(faction.getQuartermastersCurrent() == 4);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change quartermasters max count", "[Faction]")
{
  faction.setQuartermastersMax(8);
  CHECK(faction.getQuartermastersMax() == 8);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change mages current count", "[Faction]")
{
  faction.setMagesCurrent(2);
  CHECK(faction.getMagesCurrent() == 2);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change mages max count", "[Faction]")
{
  faction.setMagesMax(6);
  CHECK(faction.getMagesMax() == 6);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change apprentices current count", "[Faction]")
{
  faction.setApprenticesCurrent(5);
  CHECK(faction.getApprenticesCurrent() == 5);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change apprentices max count", "[Faction]")
{
  faction.setApprenticesMax(12);
  CHECK(faction.getApprenticesMax() == 12);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change unclaimed silver", "[Faction]")
{
  faction.setUnclaimedSilver(1000);
  CHECK(faction.getUnclaimedSilver() == 1000);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change unclaimed silver after orders", "[Faction]")
{
  faction.setUnclaimedSilverAfterOrders(750);
  CHECK(faction.getUnclaimedSilverAfterOrders() == 750);
}

TEST_CASE_METHOD(FactionFixture, "Faction stores zero silver after orders by default", "[Faction]")
{
  CHECK(faction.getUnclaimedSilverAfterOrders() == 0);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change command unit number", "[Faction]")
{
  faction.setCommandUnitNumber(456);
  CHECK(faction.getCommandUnitNumber() == 456);
}

TEST_CASE_METHOD(FactionFixture, "Faction can change default attitude", "[Faction]")
{
  faction.setDefaultAttitude(Faction::Attitude::Hostile);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Hostile);
}

TEST_CASE_METHOD(FactionFixture, "Faction can declare an attitude toward another faction", "[Faction]")
{
  faction.setDeclaredAttitude(1, Faction::Attitude::Ally);
  CHECK(faction.getDeclaredAttitudes().at(1) == Faction::Attitude::Ally);
}

TEST_CASE_METHOD(FactionFixture, "Faction can update a declared attitude", "[Faction]")
{
  faction.setDeclaredAttitude(1, Faction::Attitude::Ally);
  faction.setDeclaredAttitude(1, Faction::Attitude::Hostile);
  CHECK(faction.getDeclaredAttitudes().at(1) == Faction::Attitude::Hostile);
}

TEST_CASE_METHOD(FactionFixture, "Faction can declare multiple attitudes", "[Faction]")
{
  faction.setDeclaredAttitude(1, Faction::Attitude::Ally);
  faction.setDeclaredAttitude(2, Faction::Attitude::Hostile);
  faction.setDeclaredAttitude(3, Faction::Attitude::Friendly);
  CHECK(faction.getDeclaredAttitudes().size() == 3);
}

TEST_CASE_METHOD(FactionFixture, "Faction can remove a declared attitude", "[Faction]")
{
  faction.setDeclaredAttitude(1, Faction::Attitude::Ally);
  faction.removeDeclaredAttitude(1);
  CHECK(faction.getDeclaredAttitudes().find(1) == faction.getDeclaredAttitudes().end());
}

TEST_CASE_METHOD(FactionFixture, "Faction can clear all declared attitudes", "[Faction]")
{
  faction.setDeclaredAttitude(1, Faction::Attitude::Ally);
  faction.setDeclaredAttitude(2, Faction::Attitude::Hostile);
  faction.clearDeclaredAttitudes();
  CHECK(faction.getDeclaredAttitudes().empty());
}

TEST_CASE("Faction handles all attitude enum values", "[Faction]")
{
  Faction faction(42, L"Test", true);
  faction.setDefaultAttitude(Faction::Attitude::Hostile);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Hostile);
  faction.setDefaultAttitude(Faction::Attitude::Unfriendly);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Unfriendly);
  faction.setDefaultAttitude(Faction::Attitude::Neutral);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Neutral);
  faction.setDefaultAttitude(Faction::Attitude::Friendly);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Friendly);
  faction.setDefaultAttitude(Faction::Attitude::Ally);
  CHECK(faction.getDefaultAttitude() == Faction::Attitude::Ally);
}

TEST_CASE("Faction stores negative faction number", "[Faction]")
{
  Faction faction(-1, L"Test", true);
  CHECK(faction.getFactionNumber() == -1);
}

TEST_CASE("Faction stores faction number zero", "[Faction]")
{
  Faction faction(0, L"Test", true);
  CHECK(faction.getFactionNumber() == 0);
}

TEST_CASE("Faction stores empty name", "[Faction]")
{
  Faction faction(42, L"", true);
  CHECK(faction.getName() == L"");
}

TEST_CASE("Faction stores special characters in name", "[Faction]")
{
  Faction faction(42, L"Dwarf \"Tribe\" & Kin", true);
  CHECK(faction.getName() == L"Dwarf \"Tribe\" & Kin");
}

TEST_CASE("Faction can resolve command unit number from empty repository", "[Faction]")
{
  Faction faction(42, L"Test", true);
  faction.setCommandUnitNumber(101);
  UnitRepository unitRepo;
  const int resolved = faction.resolveCommandUnitNumber(unitRepo);
  CHECK(resolved == 101);
}

TEST_CASE("Faction resolves command unit number correctly", "[Faction]")
{
  Faction faction(42, L"Test", true);
  faction.setCommandUnitNumber(202);
  UnitRepository unitRepo;
  const int resolved = faction.resolveCommandUnitNumber(unitRepo);
  CHECK(resolved == 202);
}
