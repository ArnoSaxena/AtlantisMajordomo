/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/AppData.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace
{
template<typename Repository>
class RepositoryMock : public Repository
{
public:
  int clearCalls { 0 };

  void clear() override
  {
    ++clearCalls;
  }
};

class StructInfoMock : public StructInfo
{
public:
  StructInfoMock(std::wstring type, bool ship)
    : StructInfo(L"", 0, 0, false, false), type_(std::move(type)), ship_(ship)
  {
  }

  const std::wstring& getStructureType() const override { return type_; }
  bool isShip() const override { return ship_; }
  void setIsFlying(bool flying) override { flyingWrites.push_back(flying); }

  std::vector<bool> flyingWrites;

private:
  std::wstring type_;
  bool ship_;
};

class StructInfoRepositoryMock : public RepositoryMock<StructInfoRepository>
{
public:
  std::vector<StructInfo*> entries;

  std::size_t size() const override { return entries.size(); }
  StructInfo& at(std::size_t index) override { return *entries.at(index); }
};

struct AppDataFixture
{
  RepositoryMock<ReportRepository> reports;
  RepositoryMock<FactionRepository> factions;
  RepositoryMock<ItemRepository> items;
  RepositoryMock<SkillRepository> skills;
  RepositoryMock<RegionRepository> regions;
  RepositoryMock<UnitRepository> units;
  RepositoryMock<UnitNewRepository> newUnits;
  RepositoryMock<BattleRepository> battles;
  RepositoryMock<EventRepository> events;
  RepositoryMock<StructureRepository> structures;
  StructInfoRepositoryMock structureInfo;
  RepositoryMock<OrderRepository> orders;
  AppData data {{
    reports, factions, items, skills, regions, units, newUnits,
    battles, events, structures, structureInfo, orders
  }};
};
}

TEST_CASE_METHOD(AppDataFixture, "AppData lowercases flying ship tokens", "[AppData]")
{
  data.setFlyingShipsCsv(L"Cloud GALLEY");
  const std::vector<std::wstring> expected { L"cloud galley" };
  CHECK(data.getFlyingShipTypeTokens() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData trims flying ship tokens", "[AppData]")
{
  data.setFlyingShipsCsv(L" \tcloud galley\n ");
  const std::vector<std::wstring> expected { L"cloud galley" };
  CHECK(data.getFlyingShipTypeTokens() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData ignores empty flying ship CSV entries", "[AppData]")
{
  data.setFlyingShipsCsv(L", ,galley,, \t,");
  const std::vector<std::wstring> expected { L"galley" };
  CHECK(data.getFlyingShipTypeTokens() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData returns no flying ship tokens for empty configuration", "[AppData]")
{
  data.setFlyingShipsCsv(L"");
  CHECK(data.getFlyingShipTypeTokens().empty());
}

TEST_CASE_METHOD(AppDataFixture, "AppData lowercases magic trigger phrases", "[AppData]")
{
  data.setMagicSkillTriggersCsv(L"A Mage With This Skill");
  const std::vector<std::wstring> expected { L"a mage with this skill" };
  CHECK(data.getMagicSkillTriggerPhrases() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData trims magic trigger phrases", "[AppData]")
{
  data.setMagicSkillTriggersCsv(L" \ta mage with this skill\n ");
  const std::vector<std::wstring> expected { L"a mage with this skill" };
  CHECK(data.getMagicSkillTriggerPhrases() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData ignores empty magic trigger CSV entries", "[AppData]")
{
  data.setMagicSkillTriggersCsv(L", ,a mage,, \t,");
  const std::vector<std::wstring> expected { L"a mage" };
  CHECK(data.getMagicSkillTriggerPhrases() == expected);
}

TEST_CASE_METHOD(AppDataFixture, "AppData returns no magic triggers for empty configuration", "[AppData]")
{
  data.setMagicSkillTriggersCsv(L"");
  CHECK(data.getMagicSkillTriggerPhrases().empty());
}

TEST_CASE_METHOD(AppDataFixture, "AppData marks a ship flying for a case insensitive type substring", "[AppData]")
{
  StructInfoMock ship(L"Cloud GALLEY", true);
  structureInfo.entries = { &ship };
  data.setFlyingShipsCsv(L"galley");
  CHECK(ship.flyingWrites == std::vector<bool> { true });
}

TEST_CASE_METHOD(AppDataFixture, "AppData marks an unmatched ship nonflying", "[AppData]")
{
  StructInfoMock ship(L"Longboat", true);
  structureInfo.entries = { &ship };
  data.setFlyingShipsCsv(L"galley");
  CHECK(ship.flyingWrites == std::vector<bool> { false });
}

TEST_CASE_METHOD(AppDataFixture, "AppData never marks a matching nonship flying", "[AppData]")
{
  StructInfoMock tower(L"Galley Tower", false);
  structureInfo.entries = { &tower };
  data.setFlyingShipsCsv(L"galley");
  CHECK(tower.flyingWrites == std::vector<bool> { false });
}

TEST_CASE_METHOD(AppDataFixture, "AppData removes a flying flag when its token is removed", "[AppData]")
{
  StructInfoMock ship(L"Galley", true);
  structureInfo.entries = { &ship };
  data.setFlyingShipsCsv(L"galley");
  ship.flyingWrites.clear();
  data.setFlyingShipsCsv(L"");
  CHECK(ship.flyingWrites == std::vector<bool> { false });
}

TEST_CASE_METHOD(AppDataFixture, "AppData clear dispatches once to every repository", "[AppData]")
{
  data.clear();
  CHECK(reports.clearCalls == 1);
  CHECK(factions.clearCalls == 1);
  CHECK(items.clearCalls == 1);
  CHECK(skills.clearCalls == 1);
  CHECK(regions.clearCalls == 1);
  CHECK(units.clearCalls == 1);
  CHECK(newUnits.clearCalls == 1);
  CHECK(battles.clearCalls == 1);
  CHECK(events.clearCalls == 1);
  CHECK(structures.clearCalls == 1);
  CHECK(structureInfo.clearCalls == 1);
  CHECK(orders.clearCalls == 1);
}

TEST_CASE_METHOD(AppDataFixture, "AppData clear preserves configured settings", "[AppData]")
{
  data.setShipStructureIdThreshold(250);
  data.setFlyingShipsCsv(L"galley");
  data.setMagicSkillTriggersCsv(L"a mage");
  data.setOnlyLeaderCanTeach(true);
  data.setLeaderMages(false);
  data.clear();
  CHECK(data.getShipStructureIdThreshold() == 250);
  CHECK(data.getFlyingShipsCsv() == L"galley");
  CHECK(data.getMagicSkillTriggersCsv() == L"a mage");
  CHECK(data.getOnlyLeaderCanTeach());
  CHECK_FALSE(data.getLeaderMages());
}

TEST_CASE_METHOD(AppDataFixture, "AppData can set and get ship structure ID threshold", "[AppData]")
{
  data.setShipStructureIdThreshold(500);
  CHECK(data.getShipStructureIdThreshold() == 500);
}

TEST_CASE_METHOD(AppDataFixture, "AppData stores zero ship structure ID threshold", "[AppData]")
{
  data.setShipStructureIdThreshold(0);
  CHECK(data.getShipStructureIdThreshold() == 0);
}

TEST_CASE_METHOD(AppDataFixture, "AppData stores negative ship structure ID threshold", "[AppData]")
{
  data.setShipStructureIdThreshold(-1);
  CHECK(data.getShipStructureIdThreshold() == -1);
}

TEST_CASE_METHOD(AppDataFixture, "AppData can set and get only leader can teach flag", "[AppData]")
{
  data.setOnlyLeaderCanTeach(true);
  CHECK(data.getOnlyLeaderCanTeach());
  data.setOnlyLeaderCanTeach(false);
  CHECK_FALSE(data.getOnlyLeaderCanTeach());
}

TEST_CASE_METHOD(AppDataFixture, "AppData defaults only leader can teach to false", "[AppData]")
{
  CHECK_FALSE(data.getOnlyLeaderCanTeach());
}

TEST_CASE_METHOD(AppDataFixture, "AppData can set and get leader mages flag", "[AppData]")
{
  data.setLeaderMages(true);
  CHECK(data.getLeaderMages());
  data.setLeaderMages(false);
  CHECK_FALSE(data.getLeaderMages());
}

TEST_CASE_METHOD(AppDataFixture, "AppData defaults leader mages to true", "[AppData]")
{
  CHECK(data.getLeaderMages());
}

TEST_CASE_METHOD(AppDataFixture, "AppData report repository is accessible", "[AppData]")
{
  // Verify we can access the report repository through the AppData instance
  CHECK(std::addressof(data.reportRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData faction repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.factionRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData item repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.itemRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData skill repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.skillRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData region repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.regionRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData unit repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.unitRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData new unit repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.unitNewRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData battle repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.battleRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData event repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.eventRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData structure repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.structureRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData structure info repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.structInfoRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData order repository is accessible", "[AppData]")
{
  CHECK(std::addressof(data.orderRepository()) != nullptr);
}

TEST_CASE_METHOD(AppDataFixture, "AppData refreshStructureDerivedFlags updates flying status", "[AppData]")
{
  StructInfoMock ship(L"Galley", true);
  structureInfo.entries = { &ship };
  data.setFlyingShipsCsv(L"galley");
  ship.flyingWrites.clear();
  data.refreshStructureDerivedFlags();
  CHECK(ship.flyingWrites == std::vector<bool> { true });
}

TEST_CASE_METHOD(AppDataFixture, "AppData refreshStructureDerivedFlags clears flying status for non-matching ships", "[AppData]")
{
  StructInfoMock ship(L"Longboat", true);
  structureInfo.entries = { &ship };
  data.setFlyingShipsCsv(L"galley");
  ship.flyingWrites.clear();
  data.refreshStructureDerivedFlags();
  CHECK(ship.flyingWrites == std::vector<bool> { false });
}

TEST_CASE_METHOD(AppDataFixture, "AppData can set and get flying ships CSV", "[AppData]")
{
  data.setFlyingShipsCsv(L"galley,cloud ship");
  CHECK(data.getFlyingShipsCsv() == L"galley,cloud ship");
}

TEST_CASE_METHOD(AppDataFixture, "AppData can set and get magic skill triggers CSV", "[AppData]")
{
  data.setMagicSkillTriggersCsv(L"a mage with this skill");
  CHECK(data.getMagicSkillTriggersCsv() == L"a mage with this skill");
}
