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
 * File: BattleFormattingUtilsTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Function/BattleFormattingUtils.hpp"
#include "Data/Battle.hpp"

struct BattleFormattingUtilsFixture
{
  Battle createSimpleBattle()
  {
    Battle battle;
    battle.setAttackerUnitId(101);
    battle.setAttackerUnitName(L"Attacker Unit");
    battle.setDefenderUnitId(201);
    battle.setDefenderUnitName(L"Defender Unit");
    battle.setRegionCoordinates(10, 20, 1);
    battle.setMonth(3);
    battle.setYear(2026);
    battle.setRegionType(L"Plain");
    battle.setProvinceName(L"Test Province");
    battle.setAttackerLosses(5);
    battle.setDefenderLosses(3);
    return battle;
  }

  Battle createBattleWithDamagedUnits()
  {
    Battle battle = createSimpleBattle();
    battle.addAttackerDamagedUnitId(102);
    battle.addAttackerDamagedUnitId(103);
    battle.addDefenderDamagedUnitId(202);
    return battle;
  }

  Battle createBattleWithSpoils()
  {
    Battle battle = createSimpleBattle();
    BattleSpoil spoil1;
    spoil1.amount = 100;
    spoil1.token = L"SILV";
    battle.addSpoil(spoil1);
    
    BattleSpoil spoil2;
    spoil2.amount = 50;
    spoil2.token = L"GOLD";
    battle.addSpoil(spoil2);
    
    return battle;
  }

  Battle createBattleWithMultipleDamagedAndSpoils()
  {
    Battle battle = createSimpleBattle();
    
    for (int i = 102; i <= 105; ++i)
    {
      battle.addAttackerDamagedUnitId(i);
    }
    
    for (int i = 202; i <= 204; ++i)
    {
      battle.addDefenderDamagedUnitId(i);
    }
    
    BattleSpoil spoil1;
    spoil1.amount = 150;
    spoil1.token = L"SILV";
    battle.addSpoil(spoil1);
    
    BattleSpoil spoil2;
    spoil2.amount = 75;
    spoil2.token = L"IRON";
    battle.addSpoil(spoil2);
    
    BattleSpoil spoil3;
    spoil3.amount = 200;
    spoil3.token = L"WOOD";
    battle.addSpoil(spoil3);
    
    return battle;
  }
};

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod single digit month")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(3, 2026);
  
  CHECK(result == L"03/2026");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod double digit month")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(11, 2026);
  
  CHECK(result == L"11/2026");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod month 1")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(1, 2026);
  
  CHECK(result == L"01/2026");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod month 12")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(12, 2026);
  
  CHECK(result == L"12/2026");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod single digit year")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(5, 1);
  
  CHECK(result == L"05/1");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod large year")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(7, 9999);
  
  CHECK(result == L"07/9999");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatPeriod zero month")
{
  std::wstring result = BattleFormattingUtils::formatPeriod(0, 2026);
  
  CHECK(result == L"00/2026");
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleCoordinates standard coordinates")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatBattleCoordinates(battle);
  
  CHECK(result.find(L"10") != std::string::npos);
  CHECK(result.find(L"20") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleCoordinates negative coordinates")
{
  Battle battle = createSimpleBattle();
  battle.setRegionCoordinates(-5, -10, 2);
  
  std::wstring result = BattleFormattingUtils::formatBattleCoordinates(battle);
  
  CHECK(!result.empty());
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleCoordinates zero coordinates")
{
  Battle battle = createSimpleBattle();
  battle.setRegionCoordinates(0, 0, 1);
  
  std::wstring result = BattleFormattingUtils::formatBattleCoordinates(battle);
  
  CHECK(!result.empty());
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleCoordinates large coordinates")
{
  Battle battle = createSimpleBattle();
  battle.setRegionCoordinates(9999, 9999, 3);
  
  std::wstring result = BattleFormattingUtils::formatBattleCoordinates(battle);
  
  CHECK(result.find(L"9999") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry standard battle")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  CHECK(result.find(L"Attacker Unit") != std::string::npos);
  CHECK(result.find(L"(101)") != std::string::npos);
  CHECK(result.find(L"attacks") != std::string::npos);
  CHECK(result.find(L"Defender Unit") != std::string::npos);
  CHECK(result.find(L"(201)") != std::string::npos);
  CHECK(result.find(L"in") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry contains coordinates")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  CHECK(result.find(L"10") != std::string::npos);
  CHECK(result.find(L"20") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry with single digit IDs")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerUnitId(1);
  battle.setDefenderUnitId(2);
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  CHECK(result.find(L"(1)") != std::string::npos);
  CHECK(result.find(L"(2)") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry with large IDs")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerUnitId(999999);
  battle.setDefenderUnitId(888888);
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  CHECK(result.find(L"(999999)") != std::string::npos);
  CHECK(result.find(L"(888888)") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry with special characters in names")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerUnitName(L"Unit's \"Name\" [Test]");
  battle.setDefenderUnitName(L"Defender & Unit");
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  CHECK(result.find(L"Unit's \"Name\" [Test]") != std::string::npos);
  CHECK(result.find(L"Defender & Unit") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with no damaged units")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Attacker Attacker Unit (101) loses 5") != std::string::npos);
  CHECK(result.find(L"Damaged units: none") != std::string::npos);
  CHECK(result.find(L"Defender Defender Unit (201) loses 3") != std::string::npos);
  CHECK(result.find(L"Spoils: none") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with damaged units")
{
  Battle battle = createBattleWithDamagedUnits();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Damaged units: 102, 103") != std::string::npos);
  CHECK(result.find(L"Damaged units: 202") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with spoils")
{
  Battle battle = createBattleWithSpoils();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Spoils:") != std::string::npos);
  CHECK(result.find(L"100 [SILV]") != std::string::npos);
  CHECK(result.find(L"50 [GOLD]") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with multiple damaged and spoils")
{
  Battle battle = createBattleWithMultipleDamagedAndSpoils();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"102, 103, 104, 105") != std::string::npos);
  CHECK(result.find(L"202, 203, 204") != std::string::npos);
  CHECK(result.find(L"150 [SILV]") != std::string::npos);
  CHECK(result.find(L"75 [IRON]") != std::string::npos);
  CHECK(result.find(L"200 [WOOD]") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary contains expected format")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Attacker") != std::string::npos);
  CHECK(result.find(L"loses") != std::string::npos);
  CHECK(result.find(L"Damaged units:") != std::string::npos);
  CHECK(result.find(L"Defender") != std::string::npos);
  CHECK(result.find(L"Spoils:") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with zero losses")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerLosses(0);
  battle.setDefenderLosses(0);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"loses 0") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with large losses")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerLosses(9999);
  battle.setDefenderLosses(8888);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"loses 9999") != std::string::npos);
  CHECK(result.find(L"loses 8888") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary contains carriage return line feeds")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"\r\n") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with single damaged unit per side")
{
  Battle battle = createSimpleBattle();
  battle.addAttackerDamagedUnitId(150);
  battle.addDefenderDamagedUnitId(250);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Damaged units: 150") != std::string::npos);
  CHECK(result.find(L"Damaged units: 250") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary damaged units in correct order")
{
  Battle battle = createSimpleBattle();
  battle.addAttackerDamagedUnitId(105);
  battle.addAttackerDamagedUnitId(102);
  battle.addAttackerDamagedUnitId(103);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"105, 102, 103") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with spoil amount 1")
{
  Battle battle = createSimpleBattle();
  
  BattleSpoil spoil;
  spoil.amount = 1;
  spoil.token = L"HORSE";
  battle.addSpoil(spoil);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"1 [HORSE]") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with spoil amount 0")
{
  Battle battle = createSimpleBattle();
  
  BattleSpoil spoil;
  spoil.amount = 0;
  spoil.token = L"ITEM";
  battle.addSpoil(spoil);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"0 [ITEM]") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with spoil large amount")
{
  Battle battle = createSimpleBattle();
  
  BattleSpoil spoil;
  spoil.amount = 999999;
  spoil.token = L"TREASURE";
  battle.addSpoil(spoil);
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"999999 [TREASURE]") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary with special characters in unit names")
{
  Battle battle = createSimpleBattle();
  battle.setAttackerUnitName(L"Unit's Army");
  battle.setDefenderUnitName(L"Enemy & Friends");
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  CHECK(result.find(L"Unit's Army") != std::string::npos);
  CHECK(result.find(L"Enemy & Friends") != std::string::npos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary many spoil types")
{
  Battle battle = createSimpleBattle();
  
  std::vector<std::wstring> tokens = {L"SILV", L"GOLD", L"IRON", L"WOOD", L"STONE", L"HORSE"};
  int amount = 10;
  
  for (const auto& token : tokens)
  {
    BattleSpoil spoil;
    spoil.amount = amount;
    spoil.token = token;
    battle.addSpoil(spoil);
    amount += 10;
  }
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  for (const auto& token : tokens)
  {
    CHECK(result.find(token) != std::string::npos);
  }
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatBattleListEntry preserves order of information")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatBattleListEntry(battle);
  
  size_t attacker_pos = result.find(L"Attacker Unit");
  size_t attacks_pos = result.find(L"attacks");
  size_t defender_pos = result.find(L"Defender Unit");
  size_t in_pos = result.find(L"in");
  
  CHECK(attacker_pos < attacks_pos);
  CHECK(attacks_pos < defender_pos);
  CHECK(defender_pos < in_pos);
}

TEST_CASE_METHOD(BattleFormattingUtilsFixture, "BattleFormattingUtils formatSummary maintains side separation")
{
  Battle battle = createSimpleBattle();
  
  std::wstring result = BattleFormattingUtils::formatSummary(battle);
  
  size_t attacker_pos = result.find(L"Attacker");
  size_t defender_pos = result.find(L"Defender");
  
  CHECK(attacker_pos < defender_pos);
}
