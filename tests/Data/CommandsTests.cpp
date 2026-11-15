/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
using Counts = std::map<std::wstring, int>;
using Market = std::map<std::wstring, std::pair<int, int>>;
using OrderReferences = std::vector<std::reference_wrapper<const Order>>;

struct UnitMock : Unit
{
  explicit UnitMock(int numberValue = 1)
    : Unit(0, L"", 0, 0, 0, 0, 0, {}, {}, 0, 0, 0, 0, 0, {}, 0, 0),
      number(numberValue)
  {
  }

  int getUnitNumber() const override { return number; }
  const std::wstring& getUnitName() const override { return name; }
  int getFactionNumber() const override { return faction; }
  int getStructureId() const override { return structureId; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  const std::vector<std::wstring>& getFlags() const override { return flags; }
  const Counts& getItems() const override { return items; }
  const Counts& getSkills() const override { return skills; }
  int getSkillDays(const std::wstring& token) const override { return skillDays.at(token); }

  int number;
  std::wstring name { L"Original" };
  int faction { 10 };
  int structureId { 0 };
  int x { 0 };
  int y { 0 };
  int z { 1 };
  int month { 8 };
  int year { 7 };
  std::vector<std::wstring> flags;
  Counts items {{ L"MAN", 2 }, { L"SILV", 100 }};
  Counts skills;
  Counts skillDays {{ L"QUAM", 0 }};
};

struct UnitNewMock : UnitNew
{
  explicit UnitNewMock(int numberValue = 1)
    : UnitNew(0, L"", 0, 0, 0, 0, {}, {}, 0, 0, 0, 0, 0, {}, 0, 0, 0),
      number(numberValue)
  {
  }

  int getUnitNumber() const override { return number; }
  const std::wstring& getUnitName() const override { return name; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  const Counts& getItems() const override { return items; }
  const Counts& getSkills() const override { return skills; }
  int getOriginUnit() const override { return parent; }

  int number;
  std::wstring name { L"New original" };
  int x { 0 };
  int y { 0 };
  int z { 1 };
  int month { 8 };
  int year { 7 };
  int parent { 1 };
  Counts items {{ L"MAN", 2 }, { L"SILV", 100 }};
  Counts skills;
};

struct RegionMock : Region
{
  RegionMock() : Region(0, 0, 1, L"", L"", false, L"", L"", L"", 0) {}

  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  int getTaxableIncome() const override { return tax; }
  int getEntertainment() const override { return entertainment; }
  int getWagesMax() const override { return wagesMax; }
  double getWages() const override { return wages; }
  const Counts& getResources() const override { return resources; }
  const Market& getForSale() const override { return forSale; }
  const Market& getWanted() const override { return wanted; }

  int x { 0 };
  int y { 0 };
  int z { 1 };
  int month { 8 };
  int year { 7 };
  int tax { 0 };
  int entertainment { 0 };
  int wagesMax { 0 };
  double wages { 0.0 };
  Counts resources;
  Market forSale;
  Market wanted;
};

struct ItemMock : Item
{
  ItemMock(std::wstring tokenValue, std::wstring nameValue)
    : Item(L"", L"", 0, false, false, false, false, false, 0, 0, 0, 0, 0, false),
      token(std::move(tokenValue)), name(std::move(nameValue))
  {
  }

  const std::wstring& getIdentifierToken() const override { return token; }
  const std::wstring& getItemName() const override { return name; }
  bool isMan() const override { return man; }
  bool isResource() const override { return resource; }
  int getDefaultSkillMax() const override { return maxSkill; }
  const Counts& getSkillsMax() const override { return skillLimits; }
  const Counts& getResources() const override { return ingredients; }
  const Counts& getProductionSkill() const override { return productionSkills; }
  const Counts& getProductionHelp() const override { return productionHelp; }

  std::wstring token;
  std::wstring name;
  bool man { false };
  bool resource { false };
  int maxSkill { 5 };
  Counts skillLimits;
  Counts ingredients;
  Counts productionSkills;
  Counts productionHelp;
};

struct SkillMock : Skill
{
  SkillMock(std::wstring tokenValue = L"COMB", std::wstring nameValue = L"Combat")
    : Skill(L"", L""), token(std::move(tokenValue)), name(std::move(nameValue))
  {
  }

  const std::wstring& getIdentifierToken() const override { return token; }
  const std::wstring& getName() const override { return name; }
  int getStudyCost() const override { return cost; }
  const std::vector<SkillPrerequisite>& getPrerequisites() const override { return prerequisites; }

  std::wstring token;
  std::wstring name;
  int cost { 10 };
  std::vector<SkillPrerequisite> prerequisites;
};

struct OrderMock : Order
{
  explicit OrderMock(std::wstring textValue)
    : Order(0, false, L"", 0, 0, 0, L""), text(std::move(textValue))
  {
  }

  const std::wstring& getFullOrderText() const override { return text; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }

  std::wstring text;
  int x { 0 };
  int y { 0 };
  int z { 1 };
};

struct StructureMock : Structure
{
  StructureMock() : Structure(0, 0, 0, 0, L"", L"", false, 0, 0) {}

  int getStructureId() const override { return number; }
  const std::wstring& getStructureType() const override { return type; }
  const std::wstring& getStructureName() const override { return name; }
  int getOwnerUnitId() const override { return owner; }

  int number { 9 };
  int owner { 1 };
  std::wstring type { L"Caravanserai" };
  std::wstring name;
};

struct StructInfoMock : StructInfo
{
  StructInfoMock() : StructInfo(L"", 0, 0, false, false) {}
  bool isShip() const override { return ship; }
  bool ship { true };
};

template<typename Repository>
struct RepositoryMock : Repository
{
  void clear() override { throw std::logic_error("Unexpected repository clear"); }
};

template<typename Repository, typename Entity>
struct EntityRepositoryMock : RepositoryMock<Repository>
{
  std::size_t size() const override { return entries.size(); }
  const Entity& at(std::size_t index) const override { return *entries.at(index); }
  std::vector<const Entity*> entries;
};

struct UnitRepositoryMock : EntityRepositoryMock<UnitRepository, Unit>
{
  const Unit* findByNumber(int number) const override
  {
    const auto it = byNumber.find(number);
    return it == byNumber.end() ? nullptr : it->second;
  }
  std::map<int, const Unit*> byNumber;
};

struct ItemRepositoryMock : EntityRepositoryMock<ItemRepository, Item>
{
  const Item* findByIdentifierToken(const std::wstring& token) const override
  {
    const auto it = byToken.find(token);
    return it == byToken.end() ? nullptr : it->second;
  }
  int calculateManItemCount(const Counts&) const override { return manCount; }

  int manCount { 2 };
  std::map<std::wstring, const Item*> byToken;
};

struct SkillRepositoryMock : EntityRepositoryMock<SkillRepository, Skill>
{
  const Skill* findByIdentifier(const std::wstring& token) const override
  {
    const auto it = byToken.find(token);
    return it == byToken.end() ? nullptr : it->second;
  }
  int trainingDaysToLevel(int days) const override { return levels.at(days); }

  std::map<std::wstring, const Skill*> byToken;
  std::map<int, int> levels {{ 0, 0 }, { 29, 0 }, { 30, 1 }, { 60, 1 },
                            { 90, 2 }, { 180, 3 }, { 300, 4 }, { 450, 5 }};
};

struct ReportRepositoryMock : RepositoryMock<ReportRepository>
{
  bool getLatestPeriod(int& month, int& year) const override
  {
    if (hasLatest)
    {
      month = latestMonth;
      year = latestYear;
    }
    return hasLatest;
  }

  bool hasLatest { false };
  int latestMonth { 8 };
  int latestYear { 7 };
};

struct RegionRepositoryMock : RepositoryMock<RegionRepository>
{
  const Region* findByCoordinates(int, int, int) const override { return exact; }
  const Region* findByCoordinates(int, int) const override { return fallback; }
  const Region* exact { nullptr };
  const Region* fallback { nullptr };
};

struct StructureRepositoryMock : EntityRepositoryMock<StructureRepository, Structure>
{
  const Structure* findByIdAndCoordinates(int, int, int, int) const override { return found; }
  const Structure* found { nullptr };
};

struct StructInfoRepositoryMock : RepositoryMock<StructInfoRepository>
{
  const StructInfo* findByType(const std::wstring&) const override { return found; }
  const StructInfo* found { nullptr };
};

struct OrderRepositoryMock : RepositoryMock<OrderRepository>
{
  OrderReferences getOrderReferencesForUnit(int number, bool isNew) const override
  {
    const auto it = byUnit.find({ number, isNew });
    return it == byUnit.end() ? OrderReferences {} : it->second;
  }

  OrderMock& add(int number, std::wstring text, bool isNew = false)
  {
    auto order = std::make_unique<OrderMock>(std::move(text));
    OrderMock& result = *order;
    byUnit[{ number, isNew }].push_back(std::cref(result));
    owned.push_back(std::move(order));
    return result;
  }

  std::map<std::pair<int, bool>, OrderReferences> byUnit;
  std::vector<std::unique_ptr<OrderMock>> owned;
};

struct AppDataMock : AppData
{
  explicit AppDataMock(Repositories dependencies) : AppData(dependencies), repositories(dependencies) {}

  const ReportRepository& reportRepository() const override { return repositories.reports; }
  const UnitRepository& unitRepository() const override { return repositories.units; }
  const UnitNewRepository& unitNewRepository() const override { return repositories.newUnits; }
  const RegionRepository& regionRepository() const override { return repositories.regions; }
  const ItemRepository& itemRepository() const override { return repositories.items; }
  const SkillRepository& skillRepository() const override { return repositories.skills; }
  const StructureRepository& structureRepository() const override { return repositories.structures; }
  const StructInfoRepository& structInfoRepository() const override { return repositories.structureInfo; }
  const OrderRepository& orderRepository() const override { return repositories.orders; }
  bool getOnlyLeaderCanTeach() const override { return leadersOnly; }

  Repositories repositories;
  bool leadersOnly { false };
};

struct CommandsFixture
{
  CommandsFixture()
  {
    units.entries = { &unit };
    units.byNumber = {{ 1, &unit }, { 2, &other }};
    regions.exact = &region;
    items.entries = { &man, &wood };
    items.byToken = {{ L"MAN", &man }, { L"WOOD", &wood }};
    man.man = true;
    wood.resource = true;
    skills.entries = { &combat };
    skills.byToken = {{ L"COMB", &combat }};
  }

  void useOther() { units.entries.push_back(&other); }
  void useNew() { newUnits.entries.push_back(&newUnit); }
  void qualifyTransport()
  {
    unit.skillDays[L"QUAM"] = 30;
    structures.entries = { &structure };
  }

  UnitMock unit;
  UnitMock other { 2 };
  UnitNewMock newUnit;
  RegionMock region;
  ItemMock man { L"MAN", L"Man" };
  ItemMock wood { L"WOOD", L"Wood" };
  SkillMock combat;
  StructureMock structure;
  StructInfoMock shipInfo;
  ReportRepositoryMock reports;
  RepositoryMock<FactionRepository> factions;
  ItemRepositoryMock items;
  SkillRepositoryMock skills;
  RegionRepositoryMock regions;
  UnitRepositoryMock units;
  EntityRepositoryMock<UnitNewRepository, UnitNew> newUnits;
  RepositoryMock<BattleRepository> battles;
  RepositoryMock<EventRepository> events;
  StructureRepositoryMock structures;
  StructInfoRepositoryMock structureInfo;
  OrderRepositoryMock orders;
  AppDataMock data {{
    reports, factions, items, skills, regions, units, newUnits,
    battles, events, structures, structureInfo, orders
  }};
};

struct KeywordsFixture
{
  KeywordsFixture() : original(Commands::getFullMonthOrderKeywordsCsv())
  {
    Commands::setFullMonthOrderKeywordsCsv(L"");
  }
  ~KeywordsFixture() { Commands::setFullMonthOrderKeywordsCsv(original); }
  std::wstring original;
};
}

TEST_CASE_METHOD(KeywordsFixture, "Commands uppercases configured month-long keywords", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L"move, work");
  CHECK(Commands::getFullMonthOrderKeywords() == std::vector<std::wstring> { L"MOVE", L"WORK" });
}

TEST_CASE_METHOD(KeywordsFixture, "Commands trims configured month-long keywords", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L" \tMOVE \n,  WORK ");
  CHECK(Commands::getFullMonthOrderKeywords() == std::vector<std::wstring> { L"MOVE", L"WORK" });
}

TEST_CASE_METHOD(KeywordsFixture, "Commands ignores empty month-long CSV entries", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L",MOVE,, ,WORK,");
  CHECK(Commands::getFullMonthOrderKeywords() == std::vector<std::wstring> { L"MOVE", L"WORK" });
}

TEST_CASE_METHOD(KeywordsFixture, "Commands serializes month-long keywords with comma space separators", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L"MOVE,WORK");
  CHECK(Commands::getFullMonthOrderKeywordsCsv() == L"MOVE, WORK");
}

TEST_CASE_METHOD(KeywordsFixture, "Commands replaces rather than appends month-long keyword configuration", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L"MOVE");
  Commands::setFullMonthOrderKeywordsCsv(L"WORK");
  CHECK(Commands::getFullMonthOrderKeywords() == std::vector<std::wstring> { L"WORK" });
}

TEST_CASE_METHOD(KeywordsFixture, "Commands restores default month-long keywords for empty configuration", "[Commands]")
{
  Commands::setFullMonthOrderKeywordsCsv(L"CUSTOM");
  Commands::setFullMonthOrderKeywordsCsv(L"");
  CHECK(Commands::getFullMonthOrderKeywordsCsv() ==
        L"ADVANCE, BUILD, ENTERTAIN, MOVE, PILLAGE, PRODUCE, SAIL, STUDY, TAX, TEACH, WORK");
}

TEST_CASE_METHOD(KeywordsFixture, "Commands restores default month-long keywords for blank CSV configuration", "[Commands]")
{
  const auto defaults = Commands::getFullMonthOrderKeywords();
  Commands::setFullMonthOrderKeywordsCsv(L"CUSTOM");
  Commands::setFullMonthOrderKeywordsCsv(L" , \t, ");
  CHECK(Commands::getFullMonthOrderKeywords() == defaults);
}

TEST_CASE_METHOD(CommandsFixture, "Commands normalizes simulated inventory tokens", "[Commands]")
{
  unit.items = {{ L" [silv] ", 20 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == Counts {{ L"SILV", 20 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands merges inventory entries that normalize to the same token", "[Commands]")
{
  unit.items = {{ L"silv", 20 }, { L"SILV", 30 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == Counts {{ L"SILV", 50 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands drops nonpositive inventory entries from simulation", "[Commands]")
{
  unit.items = {{ L"SILV", 20 }, { L"WOOD", 0 }, { L"MAN", -1 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == Counts {{ L"SILV", 20 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands drops inventory keys without a token", "[Commands]")
{
  unit.items = {{ L"[]", 20 }, { L"SILV", 30 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == Counts {{ L"SILV", 30 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands preserves the original state when the requested unit is absent", "[Commands]")
{
  units.entries.clear();
  unit.items = {{ L"silv", -2 }};
  unit.skills = {{ L"COMB", 90 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit) == unit.skills);
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"Original");
}

TEST_CASE_METHOD(CommandsFixture, "Commands preserves the original state when the requested new unit is absent", "[Commands]")
{
  newUnit.items = {{ L"silv", -2 }};
  newUnit.skills = {{ L"COMB", 90 }};
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit) == newUnit.items);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnitNew(data, newUnit) == newUnit.skills);
  CHECK(Commands::calculateAfterCommandUnitNameForUnitNew(data, newUnit) == L"New original");
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores orders from snapshots with an older month", "[Commands]")
{
  reports.hasLatest = true;
  useOther();
  other.month = 7;
  orders.add(2, L"GIVE 1 10 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores orders from snapshots with an older year", "[Commands]")
{
  reports.hasLatest = true;
  useOther();
  other.year = 6;
  orders.add(2, L"GIVE 1 10 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores incoming GIVE from a neighboring region", "[Commands]")
{
  useOther();
  other.x = 2;
  orders.add(2, L"GIVE 1 10 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores units on a different z level", "[Commands]")
{
  useOther();
  other.z = 2;
  orders.add(2, L"GIVE 1 10 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands does not apply orders against an outdated region snapshot", "[Commands]")
{
  reports.hasLatest = true;
  region.month = 7;
  orders.add(1, L"CLAIM 10");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands falls back to two-dimensional region lookup", "[Commands]")
{
  regions.exact = nullptr;
  regions.fallback = &region;
  region.resources = {{ L"WOOD", 10 }};
  CHECK(Commands::calculateAfterCommandRegionResources(data, region) == Counts {{ L"WOOD", 10 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands can simulate claims without a known region", "[Commands]")
{
  regions.exact = nullptr;
  orders.add(1, L"CLAIM 10");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 110);
}

TEST_CASE_METHOD(CommandsFixture, "Commands does not mutate the source unit inventory", "[Commands]")
{
  const Counts original = unit.items;
  orders.add(1, L"CLAIM 10");
  static_cast<void>(Commands::calculateAfterCommandItemCountsForUnit(data, unit));
  CHECK(unit.items == original);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores unsupported order verbs", "[Commands]")
{
  orders.add(1, L"UNKNOWN 10");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands accepts a leading repeat marker", "[Commands]")
{
  orders.add(1, L"@cLaIm 10");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 110);
}

TEST_CASE_METHOD(CommandsFixture, "Commands accepts a semicolon inline comment", "[Commands]")
{
  orders.add(1, L"CLAIM 10 ; another 100");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 110);
}

TEST_CASE_METHOD(CommandsFixture, "Commands rejects an inline at-sign instead of treating it as a comment", "[Commands]")
{
  orders.add(1, L"CLAIM 10 @ another 100");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores an order with an unterminated quote", "[Commands]")
{
  orders.add(1, L"CLAIM \"10");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE transfers the requested quantity between units", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE ALL removes the transferred token from the source", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 ALL SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).count(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE ALL EXCEPT retains the requested reserve", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE rejects the EXCEPT-first spelling", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 EXCEPT 15 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE rejects item before quantity", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 SILV 20");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE rejects a quantity exceeding the available inventory", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 101 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE leaves inventory unchanged when the reserve exceeds availability", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 ALL SILV EXCEPT 101");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE ignores an unknown destination", "[Commands]")
{
  orders.add(1, L"GIVE 999 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE ignores an unknown item", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 20 UNKNOWN");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE resolves a quoted item name through mocked metadata", "[Commands]")
{
  useOther();
  wood.name = L"Hard wood";
  unit.items[L"WOOD"] = 10;
  orders.add(1, L"GIVE 2 3 \"hard wood\"");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 7);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE treats silver as currency without imported item metadata", "[Commands]")
{
  useOther();
  items.entries.clear();
  items.byToken.clear();
  orders.add(1, L"GIVE 2 20 silver");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands recognizes GIVE UNIT without simulating an item transfer", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 UNIT");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands executes a leading repeat GIVE order", "[Commands]")
{
  useOther();
  orders.add(1, L"@GIVE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE rejects a trailing at-sign directive", "[Commands]")
{
  useOther();
  orders.add(1, L"GIVE 2 20 SILV @repeat");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects GIVE EXCEPT before the item", "[Commands]")
{
  orders.add(1, L"GIVE 2 EXCEPT 15 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects GIVE item before quantity", "[Commands]")
{
  orders.add(1, L"GIVE 2 SILV 20");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview rejects GIVE EXCEPT before the item", "[Commands]")
{
  orders.add(1, L"GIVE 2 EXCEPT 15 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview rejects GIVE item before quantity", "[Commands]")
{
  orders.add(1, L"GIVE 2 SILV 20", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands retains at-signs within a quoted name", "[Commands]")
{
  orders.add(1, L"NAME UNIT \"Guard @ Home\"");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"Guard @ Home");
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE FROM transfers the requested quantity to the issuer", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE FROM 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE FROM rejects a source in another faction", "[Commands]")
{
  useOther();
  other.faction = 11;
  orders.add(1, L"TAKE FROM 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands rejects TAKE syntax without FROM", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE FROM ALL credits all available items", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE FROM 2 ALL SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 200);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE FROM ALL EXCEPT leaves the reserve with the source", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE FROM 2 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE rejects EXCEPT before the item", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE FROM 2 EXCEPT 15 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE rejects item before quantity", "[Commands]")
{
  useOther();
  orders.add(1, L"TAKE FROM 2 SILV 20");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit TAKE rejects item before quantity", "[Commands]")
{
  useOther();
  useNew();
  orders.add(1, L"TAKE FROM 2 SILV 20", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands GIVE can target a new unit with the same numeric identifier", "[Commands]")
{
  useNew();
  orders.add(1, L"GIVE NEW 1 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new units can GIVE using their parent faction", "[Commands]")
{
  useOther();
  useNew();
  orders.add(1, L"GIVE 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAKE FROM NEW checks the parent faction", "[Commands]")
{
  useOther();
  useNew();
  newUnit.parent = 2;
  other.faction = 11;
  orders.add(1, L"TAKE FROM NEW 1 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit orders must match the snapshot x coordinate", "[Commands]")
{
  useNew();
  orders.add(1, L"CLAIM 20", true).x = 2;
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit orders must match the snapshot y coordinate", "[Commands]")
{
  useNew();
  orders.add(1, L"CLAIM 20", true).y = 2;
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit orders must match the snapshot z coordinate", "[Commands]")
{
  useNew();
  orders.add(1, L"CLAIM 20", true).z = 2;
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands retains the local new-unit snapshot when a neighboring number collides", "[Commands]")
{
  useNew();
  UnitNewMock neighbor;
  neighbor.x = 2;
  neighbor.items[L"SILV"] = 999;
  newUnits.entries.push_back(&neighbor);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands replaces a neighboring new-unit snapshot with the local collision", "[Commands]")
{
  UnitNewMock neighbor;
  neighbor.x = 2;
  neighbor.items[L"SILV"] = 999;
  newUnits.entries = { &neighbor, &newUnit };
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview subtracts explicit GIVE quantities", "[Commands]")
{
  orders.add(1, L"GIVE 999 20 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview retains negative balances for warnings", "[Commands]")
{
  orders.add(1, L"GIVE 999 120 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == -20);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview assigns zero for GIVE ALL", "[Commands]")
{
  orders.add(1, L"GIVE 999 ALL SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview uses the GIVE EXCEPT reserve", "[Commands]")
{
  orders.add(1, L"GIVE 999 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview ignores incoming TAKE orders", "[Commands]")
{
  orders.add(1, L"TAKE FROM 2 20 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview subtracts GIVE quantities", "[Commands]")
{
  orders.add(1, L"GIVE 999 20 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview handles GIVE ALL", "[Commands]")
{
  orders.add(1, L"GIVE 999 ALL SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview handles GIVE EXCEPT reserves", "[Commands]")
{
  orders.add(1, L"GIVE 999 ALL SILV EXCEPT 15", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview ignores orders at other coordinates", "[Commands]")
{
  orders.add(1, L"GIVE 999 20 SILV", true).x = 2;
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY charges silver for training while preserving the pre-study preview", "[Commands]")
{
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).at(L"COMB") == 30);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitExcludingStudy(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY removes an exactly exhausted silver balance", "[Commands]")
{
  unit.items[L"SILV"] = 20;
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).count(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY records insufficient silver without advancing skills", "[Commands]")
{
  unit.items[L"SILV"] = 5;
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == -15);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY ignores an unknown skill", "[Commands]")
{
  orders.add(1, L"STUDY UNKNOWN");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects a skill display name", "[Commands]")
{
  orders.add(1, L"STUDY \"Combat\"");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects a unique skill prefix without charging silver", "[Commands]")
{
  orders.add(1, L"STUDY com");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY matches exact skill tokens case-insensitively", "[Commands]")
{
  orders.add(1, L"STUDY comb");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).at(L"COMB") == 30);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects punctuation around a skill token", "[Commands]")
{
  orders.add(1, L"STUDY [COMB]");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects an ambiguous skill prefix", "[Commands]")
{
  SkillMock competing(L"COMM", L"Commerce");
  skills.entries.push_back(&competing);
  orders.add(1, L"STUDY com");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects a missing prerequisite", "[Commands]")
{
  combat.prerequisites = {{ L"TACT", 1 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY rejects a prerequisite below its required level", "[Commands]")
{
  combat.prerequisites = {{ L"TACT", 2 }};
  unit.skills = {{ L"TACT", 30 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).count(L"COMB") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY checks prerequisites after the first satisfied prerequisite", "[Commands]")
{
  combat.prerequisites = {{ L"TACT", 1 }, { L"OBSE", 1 }};
  unit.skills = {{ L"TACT", 30 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).count(L"COMB") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY advances when all prerequisites are satisfied", "[Commands]")
{
  combat.prerequisites = {{ L"TACT", 2 }, { L"OBSE", 1 }};
  unit.skills = {{ L"TACT", 90 }, { L"OBSE", 30 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).at(L"COMB") == 30);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY respects a man item's per-skill limit", "[Commands]")
{
  man.skillLimits = {{ L"COMB", 1 }};
  unit.skills = {{ L"COMB", 30 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).at(L"COMB") == 30);
}

TEST_CASE_METHOD(CommandsFixture, "Commands STUDY preserves existing skill-token spelling", "[Commands]")
{
  unit.skills = {{ L"comb", 30 }};
  orders.add(1, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit) == Counts {{ L"comb", 60 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands study-excluding inventory still applies other supported commands", "[Commands]")
{
  orders.add(1, L"STUDY COMB");
  orders.add(1, L"CLAIM 15");
  CHECK(Commands::calculateAfterCommandItemCountsForUnitExcludingStudy(data, unit).at(L"SILV") == 115);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit STUDY charges silver for training while preserving the pre-study preview", "[Commands]")
{
  useNew();
  orders.add(1, L"STUDY COMB", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnitNew(data, newUnit).at(L"COMB") == 30);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNewExcludingStudy(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit STUDY rejects a unique skill prefix without charging silver", "[Commands]")
{
  useNew();
  orders.add(1, L"STUDY com", true);
  CHECK(Commands::calculateAfterCommandSkillDaysForUnitNew(data, newUnit).empty());
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit) == newUnit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TEACH adds a bonus before the student's study phase", "[Commands]")
{
  useOther();
  unit.skills = {{ L"COMB", 90 }};
  orders.add(1, L"TEACH 2");
  orders.add(2, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, other).at(L"COMB") == 60);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TEACH rejects a student's abbreviated STUDY skill", "[Commands]")
{
  useOther();
  unit.skills = {{ L"COMB", 90 }};
  other.skills = {{ L"COMB", 30 }};
  orders.add(1, L"TEACH 2");
  orders.add(2, L"STUDY com");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, other) == other.skills);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TEACH requires a higher skill level than the student", "[Commands]")
{
  useOther();
  unit.skills = {{ L"COMB", 30 }};
  other.skills = {{ L"COMB", 30 }};
  orders.add(1, L"TEACH 2");
  orders.add(2, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, other).at(L"COMB") == 60);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TEACH honors the leader-only setting", "[Commands]")
{
  useOther();
  data.leadersOnly = true;
  unit.skills = {{ L"COMB", 90 }};
  orders.add(1, L"TEACH 2");
  orders.add(2, L"STUDY COMB");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, other).at(L"COMB") == 30);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAX transfers up to fifty silver per man from the regional pool", "[Commands]")
{
  region.tax = 200;
  unit.skills = {{ L"COMB", 30 }};
  orders.add(1, L"TAX");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 200);
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingTaxableIncome == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAX caps income at the remaining tax pool", "[Commands]")
{
  region.tax = 25;
  unit.skills = {{ L"COMB", 30 }};
  orders.add(1, L"TAX");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 125);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TAX requires combat level one", "[Commands]")
{
  region.tax = 200;
  unit.skills = {{ L"COMB", 29 }};
  orders.add(1, L"TAX");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ENTERTAIN transfers income from the regional pool using men and skill", "[Commands]")
{
  region.entertainment = 200;
  unit.skills = {{ L"ENTE", 90 }};
  orders.add(1, L"ENTERTAIN");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 180);
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingEntertainment == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ENTERTAIN requires entertainment skill", "[Commands]")
{
  region.entertainment = 200;
  orders.add(1, L"ENTERTAIN");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands WORK transfers rounded-down wages from the regional pool", "[Commands]")
{
  region.wagesMax = 100;
  region.wages = 3.75;
  orders.add(1, L"WORK");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 107);
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingWorkWages == 93);
}

TEST_CASE_METHOD(CommandsFixture, "Commands WORK caps wages at the available pool", "[Commands]")
{
  region.wagesMax = 5;
  region.wages = 3.75;
  orders.add(1, L"WORK");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 105);
}

TEST_CASE_METHOD(CommandsFixture, "Commands WORK produces no income without men", "[Commands]")
{
  region.wagesMax = 100;
  region.wages = 3.75;
  items.manCount = 0;
  orders.add(1, L"WORK");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands clamps negative regional tax income to zero", "[Commands]")
{
  region.tax = -5;
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingTaxableIncome == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands clamps negative regional entertainment to zero", "[Commands]")
{
  region.entertainment = -5;
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingEntertainment == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands clamps negative regional wage pools to zero", "[Commands]")
{
  region.wagesMax = -5;
  CHECK(Commands::calculateAfterCommandRegionEconomy(data, region).remainingWorkWages == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands BUY exchanges silver for goods and reduces market stock", "[Commands]")
{
  region.forSale = {{ L"WOOD", { 10, 5 } }};
  orders.add(1, L"BUY 3 WOOD");
  const Counts inventory = Commands::calculateAfterCommandItemCountsForUnit(data, unit);
  CHECK(inventory.at(L"WOOD") == 3);
  CHECK(inventory.at(L"SILV") == 85);
  CHECK(Commands::calculateAfterCommandRegionForSale(data, region) == Market {{ L"WOOD", { 7, 5 } }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands BUY limits received goods to the affordable amount", "[Commands]")
{
  region.forSale = {{ L"WOOD", { 10, 30 } }};
  orders.add(1, L"BUY 5 WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 3);
}

TEST_CASE_METHOD(CommandsFixture, "Commands BUY ALL caps purchases at market stock", "[Commands]")
{
  region.forSale = {{ L"WOOD", { 4, 5 } }};
  orders.add(1, L"BUY ALL WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 4);
}

TEST_CASE_METHOD(CommandsFixture, "Commands BUY can acquire a free market item", "[Commands]")
{
  region.forSale = {{ L"WOOD", { 4, 0 } }};
  orders.add(1, L"BUY 3 WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 3);
}

TEST_CASE_METHOD(CommandsFixture, "Commands BUY blends existing skill days when hiring untrained men", "[Commands]")
{
  region.forSale = {{ L"MAN", { 4, 5 } }};
  unit.skills = {{ L"COMB", 90 }};
  orders.add(1, L"BUY 2 MAN");
  CHECK(Commands::calculateAfterCommandSkillDaysForUnit(data, unit).at(L"COMB") == 45);
}

TEST_CASE_METHOD(CommandsFixture, "Commands SELL exchanges goods for silver and reduces market demand", "[Commands]")
{
  unit.items[L"WOOD"] = 10;
  region.wanted = {{ L"WOOD", { 8, 5 } }};
  orders.add(1, L"SELL 3 WOOD");
  const Counts inventory = Commands::calculateAfterCommandItemCountsForUnit(data, unit);
  CHECK(inventory.at(L"SILV") == 115);
  CHECK(inventory.at(L"WOOD") == 7);
  CHECK(Commands::calculateAfterCommandRegionWanted(data, region) == Market {{ L"WOOD", { 5, 5 } }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands SELL ALL caps sales at market demand", "[Commands]")
{
  unit.items[L"WOOD"] = 10;
  region.wanted = {{ L"WOOD", { 4, 5 } }};
  orders.add(1, L"SELL ALL WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 6);
}

TEST_CASE_METHOD(CommandsFixture, "Commands resolves SELL before BUY regardless of order text sequence", "[Commands]")
{
  unit.items[L"SILV"] = 5;
  unit.items[L"WOOD"] = 2;
  region.wanted = {{ L"WOOD", { 2, 10 } }};
  region.forSale = {{ L"MAN", { 3, 10 } }};
  orders.add(1, L"BUY 2 MAN");
  orders.add(1, L"SELL 2 WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"MAN") == 4);
}

TEST_CASE_METHOD(CommandsFixture, "Commands normalizes regional resource tokens", "[Commands]")
{
  region.resources = {{ L"wood", 4 }, { L"[WOOD]", 6 }};
  CHECK(Commands::calculateAfterCommandRegionResources(data, region) == Counts {{ L"WOOD", 10 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands excludes nonpositive regional resources", "[Commands]")
{
  region.resources = {{ L"WOOD", 10 }, { L"IRON", -1 }, { L"STONE", 0 }};
  CHECK(Commands::calculateAfterCommandRegionResources(data, region) == Counts {{ L"WOOD", 10 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE transfers regional resources to the unit using available men", "[Commands]")
{
  region.resources = {{ L"WOOD", 10 }};
  orders.add(1, L"PRODUCE WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 2);
  CHECK(Commands::calculateAfterCommandRegionResources(data, region) == Counts {{ L"WOOD", 8 }});
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE honors an explicit amount cap", "[Commands]")
{
  region.resources = {{ L"WOOD", 10 }};
  orders.add(1, L"PRODUCE 1 WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 1);
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE caps output at available regional resources", "[Commands]")
{
  region.resources = {{ L"WOOD", 1 }};
  orders.add(1, L"PRODUCE WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 1);
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE requires the configured production skill level", "[Commands]")
{
  region.resources = {{ L"WOOD", 10 }};
  wood.productionSkills = {{ L"COMB", 2 }};
  unit.skills = {{ L"COMB", 30 }};
  orders.add(1, L"PRODUCE WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).count(L"WOOD") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE consumes inventory ingredients for manufactured items", "[Commands]")
{
  wood.resource = false;
  wood.productionSkills = {{ L"COMB", 1 }};
  wood.ingredients = {{ L"IRON", 2 }};
  unit.skills = {{ L"COMB", 30 }};
  unit.items[L"IRON"] = 5;
  orders.add(1, L"PRODUCE WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"IRON") == 1);
}

TEST_CASE_METHOD(CommandsFixture, "Commands PRODUCE applies helper tools with a one-per-man limit", "[Commands]")
{
  ItemMock tool(L"TOOL", L"Tool");
  tool.productionHelp = {{ L"WOOD", 2 }};
  items.byToken[L"TOOL"] = &tool;
  unit.items[L"TOOL"] = 5;
  region.resources = {{ L"WOOD", 20 }};
  orders.add(1, L"PRODUCE WOOD");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"WOOD") == 6);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes TRANSPORT quantities", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 20 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes DISTRIBUTE quantities", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 20 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes TRANSPORT ALL", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 ALL SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes DISTRIBUTE ALL", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 ALL SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes TRANSPORT EXCEPT reserves", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands outgoing transfer preview includes DISTRIBUTE EXCEPT reserves", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes TRANSPORT quantities", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 20 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes DISTRIBUTE quantities", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 20 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 80);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes TRANSPORT ALL", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 ALL SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes DISTRIBUTE ALL", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 ALL SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes TRANSPORT EXCEPT reserves", "[Commands]")
{
  orders.add(1, L"TRANSPORT 999 ALL SILV EXCEPT 15", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview includes DISTRIBUTE EXCEPT reserves", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 999 ALL SILV EXCEPT 15", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT can reach a neighboring region", "[Commands]")
{
  useOther();
  other.x = 2;
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE can reach a neighboring region", "[Commands]")
{
  useOther();
  other.x = 2;
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT rejects destinations beyond its range", "[Commands]")
{
  useOther();
  other.x = 4;
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE rejects destinations beyond its range", "[Commands]")
{
  useOther();
  other.x = 4;
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT requires a qualified caravanserai owner", "[Commands]")
{
  useOther();
  orders.add(1, L"TRANSPORT 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE requires a qualified caravanserai owner", "[Commands]")
{
  useOther();
  orders.add(1, L"DISTRIBUTE 2 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT ALL EXCEPT retains the reserve", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE ALL EXCEPT retains the reserve", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 ALL SILV EXCEPT 15");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 15);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT ALL transfers the entire source balance", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 ALL SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).count(L"SILV") == 0);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 200);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE ALL transfers the entire source balance", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 ALL SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).count(L"SILV") == 0);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 200);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT rejects item before quantity", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 SILV 20");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE rejects item before quantity", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 SILV 20");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects TRANSPORT EXCEPT-first spelling", "[Commands]")
{
  orders.add(1, L"TRANSPORT 2 EXCEPT 15 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects DISTRIBUTE EXCEPT-first spelling", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 2 EXCEPT 15 SILV");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects TRANSPORT EXCEPT after explicit quantity", "[Commands]")
{
  orders.add(1, L"TRANSPORT 2 20 SILV EXCEPT 5");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands transfer preview rejects DISTRIBUTE EXCEPT after explicit quantity", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 2 20 SILV EXCEPT 5");
  CHECK(Commands::calculateAfterGiveTransfersForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview rejects TRANSPORT EXCEPT-first spelling", "[Commands]")
{
  orders.add(1, L"TRANSPORT 2 EXCEPT 15 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit transfer preview rejects DISTRIBUTE EXCEPT-first spelling", "[Commands]")
{
  orders.add(1, L"DISTRIBUTE 2 EXCEPT 15 SILV", true);
  CHECK(Commands::calculateAfterGiveTransfersForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT rejects insufficient inventory", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 101 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE rejects insufficient inventory", "[Commands]")
{
  useOther();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 101 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands TRANSPORT can deliver to a new unit", "[Commands]")
{
  useNew();
  qualifyTransport();
  orders.add(1, L"TRANSPORT NEW 1 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands DISTRIBUTE can deliver to a new unit", "[Commands]")
{
  useNew();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE NEW 1 20 SILV");
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 120);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit TRANSPORT does not dereference a missing regular-unit origin", "[Commands]")
{
  useOther();
  useNew();
  orders.add(1, L"TRANSPORT 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit) == newUnit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit DISTRIBUTE does not dereference a missing regular-unit origin", "[Commands]")
{
  useOther();
  useNew();
  orders.add(1, L"DISTRIBUTE 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit) == newUnit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit TRANSPORT transfers only new-unit inventory to the qualified destination", "[Commands]")
{
  useOther();
  useNew();
  other.skillDays[L"QUAM"] = 30;
  structure.owner = 2;
  structures.entries = { &structure };
  orders.add(1, L"TRANSPORT 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit DISTRIBUTE transfers only new-unit inventory to the qualified destination", "[Commands]")
{
  useOther();
  useNew();
  other.skillDays[L"QUAM"] = 30;
  structure.owner = 2;
  structures.entries = { &structure };
  orders.add(1, L"DISTRIBUTE 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 80);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, other).at(L"SILV") == 120);
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit TRANSPORT cannot borrow ownership from a colliding existing unit", "[Commands]")
{
  useOther();
  useNew();
  qualifyTransport();
  orders.add(1, L"TRANSPORT 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit DISTRIBUTE cannot borrow ownership from a colliding existing unit", "[Commands]")
{
  useOther();
  useNew();
  qualifyTransport();
  orders.add(1, L"DISTRIBUTE 2 20 SILV", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit).at(L"SILV") == 100);
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT applies a quoted display name", "[Commands]")
{
  orders.add(1, L"NAME UNIT \"New name\"");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"New name");
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT joins unquoted name words", "[Commands]")
{
  orders.add(1, L"NAME UNIT New name");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"New name");
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT preserves semicolons inside quotes", "[Commands]")
{
  orders.add(1, L"NAME UNIT \"Name; retained\"");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"Name; retained");
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT accepts escaped quotation marks", "[Commands]")
{
  orders.add(1, L"NAME UNIT \"The \\\"Guard\\\"\"");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"The \"Guard\"");
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT uses the last supported rename", "[Commands]")
{
  orders.add(1, L"NAME UNIT First");
  orders.add(1, L"NAME UNIT Second");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"Second");
}

TEST_CASE_METHOD(CommandsFixture, "Commands ignores NAME variants other than UNIT", "[Commands]")
{
  orders.add(1, L"NAME FACTION Other");
  CHECK(Commands::calculateAfterCommandUnitNameForUnit(data, unit) == L"Original");
}

TEST_CASE_METHOD(CommandsFixture, "Commands NAME UNIT applies to new units", "[Commands]")
{
  useNew();
  orders.add(1, L"NAME UNIT Newborn", true);
  CHECK(Commands::calculateAfterCommandUnitNameForUnitNew(data, newUnit) == L"Newborn");
}

TEST_CASE_METHOD(CommandsFixture, "Commands MOVE is display-only and leaves inventory unchanged", "[Commands]")
{
  orders.add(1, L"MOVE N NE");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ADVANCE is display-only and leaves inventory unchanged", "[Commands]")
{
  orders.add(1, L"ADVANCE SW");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands LEAVE is display-only and leaves inventory unchanged", "[Commands]")
{
  orders.add(1, L"LEAVE");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands ENTER is display-only and leaves inventory unchanged", "[Commands]")
{
  orders.add(1, L"ENTER 9");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands SAIL is display-only for a mocked ship owner", "[Commands]")
{
  unit.structureId = 9;
  structures.found = &structure;
  structureInfo.found = &shipInfo;
  orders.add(1, L"SAIL N");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands new-unit SAIL does not dereference a missing regular-unit origin", "[Commands]")
{
  useNew();
  orders.add(1, L"SAIL N", true);
  CHECK(Commands::calculateAfterCommandItemCountsForUnitNew(data, newUnit) == newUnit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands PROMOTE does not change item counts", "[Commands]")
{
  useOther();
  unit.structureId = 9;
  structures.found = &structure;
  orders.add(1, L"PROMOTE 2");
  CHECK(Commands::calculateAfterCommandItemCountsForUnit(data, unit) == unit.items);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary totals claims from regular and new units", "[Commands]")
{
  useNew();
  orders.add(1, L"CLAIM 10");
  orders.add(1, L"CLAIM 20", true);
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 30);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary totals multiple claims while listing their issuer once", "[Commands]")
{
  orders.add(1, L"CLAIM 10");
  orders.add(1, L"CLAIM 20");
  const auto summary = Commands::summarizeClaims(data);
  CHECK(summary.totalClaimAmount == 30);
  CHECK(summary.issuers.size() == 1);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary records regular issuer identity and coordinates", "[Commands]")
{
  unit.x = -4;
  unit.y = 6;
  unit.z = 2;
  orders.add(1, L"CLAIM 10");
  const auto summary = Commands::summarizeClaims(data);
  REQUIRE(summary.issuers.size() == 1);
  const auto& issuer = summary.issuers.front();
  CHECK(std::make_tuple(issuer.unitNumber, issuer.isNewUnit, issuer.xCoordinate, issuer.yCoordinate, issuer.zCoordinate) ==
        std::make_tuple(1, false, -4, 6, 2));
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary records new issuer identity and coordinates", "[Commands]")
{
  useNew();
  newUnit.x = -4;
  newUnit.y = 6;
  newUnit.z = 2;
  orders.add(1, L"CLAIM 10", true);
  const auto summary = Commands::summarizeClaims(data);
  REQUIRE(summary.issuers.size() == 1);
  const auto& issuer = summary.issuers.front();
  CHECK(std::make_tuple(issuer.unitNumber, issuer.isNewUnit, issuer.xCoordinate, issuer.yCoordinate, issuer.zCoordinate) ==
        std::make_tuple(1, true, -4, 6, 2));
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary ignores units without orders", "[Commands]")
{
  CHECK(Commands::summarizeClaims(data).issuers.empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary ignores nonclaim commands", "[Commands]")
{
  orders.add(1, L"WORK");
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary rejects a zero claim", "[Commands]")
{
  orders.add(1, L"CLAIM 0");
  CHECK(Commands::summarizeClaims(data).issuers.empty());
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary rejects a negative claim", "[Commands]")
{
  orders.add(1, L"CLAIM -1");
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary rejects an overflowing integer", "[Commands]")
{
  orders.add(1, L"CLAIM 999999999999999999999");
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary requires the complete numeric token", "[Commands]")
{
  orders.add(1, L"CLAIM 10junk");
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 0);
}

TEST_CASE_METHOD(CommandsFixture, "Commands claim summary rejects extra operands", "[Commands]")
{
  orders.add(1, L"CLAIM 10 extra");
  CHECK(Commands::summarizeClaims(data).totalClaimAmount == 0);
}
