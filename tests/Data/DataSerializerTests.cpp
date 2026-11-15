/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/AppData.hpp"
#include "Data/DataSerializer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Counts = std::map<std::wstring, int>;
using Market = std::map<std::wstring, std::pair<int, int>>;
using Strings = std::vector<std::wstring>;

template<typename Repository, typename Entity>
struct RepositoryMock : Repository
{
  std::size_t size() const override
  {
    if (throwOnSize) throw std::runtime_error("repository unavailable");
    return entries.size();
  }
  const Entity& at(std::size_t index) const override { return *entries.at(index); }
  void clear() override { ++clearCalls; }

  std::vector<Entity*> entries;
  bool throwOnSize { false };
  int clearCalls { 0 };
};

struct FactionMock : Faction
{
  FactionMock() : Faction(0, L"", false) {}

  int getFactionNumber() const override { return number; }
  const std::wstring& getName() const override { return name; }
  bool isMainFaction() const override { return main; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  const std::wstring& getPassword() const override { return password; }
  int getTaxedOrTradedRegionsCurrent() const override { return taxedCurrent; }
  int getTaxedOrTradedRegionsMax() const override { return taxedMax; }
  int getQuartermastersCurrent() const override { return quartermastersCurrent; }
  int getQuartermastersMax() const override { return quartermastersMax; }
  int getMagesCurrent() const override { return magesCurrent; }
  int getMagesMax() const override { return magesMax; }
  int getApprenticesCurrent() const override { return apprenticesCurrent; }
  int getApprenticesMax() const override { return apprenticesMax; }
  int getUnclaimedSilver() const override { return unclaimed; }
  int getUnclaimedSilverAfterOrders() const override { return unclaimedAfter; }
  int getCommandUnitNumber() const override { return command; }
  Attitude getDefaultAttitude() const override { return attitude; }
  const std::map<int, Attitude>& getDeclaredAttitudes() const override { return attitudes; }
  void setUnclaimedSilver(int value) override { unclaimed = value; }
  void setUnclaimedSilverAfterOrders(int value) override { unclaimedAfter = value; }
  void setCommandUnitNumber(int value) override { command = value; }
  void setDefaultAttitude(Attitude value) override { attitude = value; }
  void setDeclaredAttitude(int numberValue, Attitude value) override { attitudes[numberValue] = value; }

  int number { 12 }, month { 8 }, year { 7 };
  std::wstring name { L"North" }, password { L"secret" };
  bool main { true };
  int taxedCurrent { 1 }, taxedMax { 2 }, quartermastersCurrent { 3 }, quartermastersMax { 4 };
  int magesCurrent { 5 }, magesMax { 6 }, apprenticesCurrent { 7 }, apprenticesMax { 8 };
  int unclaimed { 90 }, unclaimedAfter { 70 }, command { 123 };
  Attitude attitude { Attitude::Neutral };
  std::map<int, Attitude> attitudes;
};

struct RegionMock : Region
{
  RegionMock() : Region(0, 0, 1, L"", L"", false, L"", L"", L"", 0) {}

  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  const std::wstring& getRegionType() const override { return type; }
  const std::wstring& getProvinceName() const override { return province; }
  bool getContainsSettlement() const override { return settlement; }
  const std::wstring& getSettlementName() const override { return settlementName; }
  const std::wstring& getSettlementType() const override { return settlementType; }
  const std::wstring& getPeasantType() const override { return peasants; }
  int getPeasantNumber() const override { return population; }
  double getWages() const override { return wages; }
  int getWagesMax() const override { return wagesMax; }
  int getTaxableIncome() const override { return tax; }
  int getEntertainment() const override { return entertainment; }
  bool getVisited() const override { return visited; }
  const Strings& getExitDirections() const override { return exits; }
  const Counts& getResources() const override { return resources; }
  const Market& getWanted() const override { return wanted; }
  const Market& getForSale() const override { return sale; }
  const std::wstring& getRegionReport() const override { return report; }
  void clearExitDirections() override { ++clearExitCalls; exits.clear(); }
  void addExitDirection(std::wstring value) override { exits.push_back(std::move(value)); }
  void setEntertainment(int value) override { entertainment = value; }
  void setResources(Counts value) override { resources = std::move(value); }
  void setWanted(Market value) override { wanted = std::move(value); }
  void setForSale(Market value) override { sale = std::move(value); }
  void setRegionReport(std::wstring value) override { report = std::move(value); }

  int x { -2 }, y { 4 }, z { 1 }, month { 8 }, year { 7 };
  std::wstring type { L"plain" }, province { L"Valley" }, settlementName { L"Town" };
  std::wstring settlementType { L"town" }, peasants { L"HUMA" }, report { L"Region report" };
  bool settlement { true }, visited { true };
  int population { 200 }, wagesMax { 300 }, tax { 400 }, entertainment { 500 };
  double wages { 12.75 };
  Strings exits { L"N", L"SE" };
  Counts resources {{ L"WOOD", 30 }};
  Market wanted {{ L"IRON", { 10, 5 } }}, sale {{ L"WOOD", { 20, 3 } }};
  int clearExitCalls { 0 };
};

struct UnitFields
{
  int number { 42 };
  std::wstring name { L"Guard" };
  int faction { 12 }, structure { 5 }, x { -2 }, y { 4 }, z { 1 };
  Strings flags { L"holding", L"avoiding" };
  Counts items {{ L"SILV", 100 }, { L"WOOD", 3 }};
  int weight { 9 }, walk { 10 }, ride { 20 }, fly { 30 }, swim { 40 };
  Counts skills {{ L"COMB", 90 }};
  int month { 8 }, year { 7 };
  bool guard { false };
};

struct UnitMock : Unit, UnitFields
{
  UnitMock() : Unit(0, L"", 0, 0, 0, 0, 1, {}, {}, 0, 0, 0, 0, 0, {}, 0, 0) {}

  int getUnitNumber() const override { return number; }
  const std::wstring& getUnitName() const override { return name; }
  int getFactionNumber() const override { return faction; }
  int getStructureId() const override { return structure; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  const Strings& getFlags() const override { return flags; }
  const Counts& getItems() const override { return items; }
  int getWeight() const override { return weight; }
  int getCapacityWalk() const override { return walk; }
  int getCapacityRide() const override { return ride; }
  int getCapacityFly() const override { return fly; }
  int getCapacitySwim() const override { return swim; }
  const Counts& getSkills() const override { return skills; }
  const Strings& getCanStudySkillTokens() const override { return canStudy; }
  const Strings& getOrders() const override { return orders; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  bool isOnGuard() const override { return guard; }
  void setOrders(Strings value) override { orders = std::move(value); }
  void setCanStudySkillTokens(Strings value) override { canStudy = std::move(value); }
  void setOnGuard(bool value) override { guard = value; }

  Strings orders, canStudy;
};

struct UnitNewMock : UnitNew, UnitFields
{
  UnitNewMock() : UnitNew(0, L"", 0, 0, 0, 1, {}, {}, 0, 0, 0, 0, 0, {}, 0, 0, 0) {}

  int getUnitNumber() const override { return number; }
  const std::wstring& getUnitName() const override { return name; }
  int getStructureId() const override { return structure; }
  int getFutureStructureId() const override { return futureStructure; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  const Strings& getFlags() const override { return flags; }
  const Counts& getItems() const override { return items; }
  int getWeight() const override { return weight; }
  int getCapacityWalk() const override { return walk; }
  int getCapacityRide() const override { return ride; }
  int getCapacityFly() const override { return fly; }
  int getCapacitySwim() const override { return swim; }
  const Counts& getSkills() const override { return skills; }
  const Strings& getCanStudySkillTokens() const override { return canStudy; }
  const Strings& getWarnings() const override { return warnings; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  int getOriginUnit() const override { return origin; }
  void setFutureStructureId(int value) override { futureStructure = value; }
  void setCanStudySkillTokens(Strings value) override { canStudy = std::move(value); }
  void addWarning(std::wstring value) override { warnings.push_back(std::move(value)); }

  int origin { 42 }, futureStructure { 6 };
  Strings warnings, canStudy;
};

struct EventMock : Event
{
  EventMock() : Event(0, 0, L"") {}
  int getUnitId() const override { return unit; }
  const std::wstring& getMessage() const override { return message; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  bool isErrorEvent() const override { return error; }

  int unit { 42 }, month { 8 }, year { 7 };
  std::wstring message { L"Failed order" };
  bool error { true };
};

struct StructureMock : Structure
{
  StructureMock() : Structure(0, 0, 0, 1, L"", L"", false, 0, 0) {}
  int getStructureId() const override { return id; }
  int getXCoordinate() const override { return x; }
  int getYCoordinate() const override { return y; }
  int getZCoordinate() const override { return z; }
  const std::wstring& getStructureType() const override { return type; }
  const std::wstring& getStructureName() const override { return name; }
  bool isClosed() const override { return closed; }
  int getOwnerUnitId() const override { return owner; }
  const Counts& getFleetItems() const override { return fleet; }
  int getMonth() const override { return month; }
  int getYear() const override { return year; }
  void setOwnerUnitId(int value) override { owner = value; }
  void setFleetItems(Counts value) override { fleet = std::move(value); }

  int id { 5 }, x { -2 }, y { 4 }, z { 1 }, month { 8 }, year { 7 }, owner { 42 };
  std::wstring type { L"fleet" }, name { L"Voyager" };
  bool closed { true };
  Counts fleet {{ L"LONG", 2 }};
};

struct StructInfoMock : StructInfo
{
  StructInfoMock() : StructInfo(L"", 0, 0, false, false) {}
  const std::wstring& getStructureType() const override { return type; }
  int getNeeds() const override { return needs; }
  int getMagesCapacity() const override { return mages; }
  bool isShip() const override { return ship; }
  bool protoIsFlying() const override { return flying; }
  const std::wstring& getItemIdentifierToken() const override { return token; }

  std::wstring type { L"airship" }, token { L"AIRS" };
  int needs { 100 }, mages { 2 };
  bool ship { true }, flying { true };
};

struct ItemMock : Item
{
  ItemMock() : Item(L"", L"", 0, false, false, false, false, false, 0, 0, 0, 0, 0, false) {}
  const std::wstring& getIdentifierToken() const override { return token; }
  const std::wstring& getItemName() const override { return name; }
  const std::wstring& getItemNamePlural() const override { return plural; }
  int getWeight() const override { return weight; }
  bool isMeeleWeapon() const override { return melee; }
  bool isRangedWeapon() const override { return ranged; }
  bool isArmour() const override { return armour; }
  bool isResource() const override { return resource; }
  bool isMount() const override { return mount; }
  bool isProvisional() const override { return provisional; }
  int getMoves() const override { return moves; }
  int getWalkCapacity() const override { return walk; }
  int getRideCapacity() const override { return ride; }
  int getSwimCapacity() const override { return swim; }
  int getFlyCapacity() const override { return fly; }
  int getShipSpeedHexesPerMonth() const override { return speed; }
  int getShipSailingSkillRequired() const override { return sailing; }
  int getMagesStudy() const override { return mages; }
  bool isMan() const override { return man; }
  int getDefaultSkillMax() const override { return skillMax; }
  const std::wstring& getFullText() const override { return text; }
  const Counts& getResources() const override { return resources; }
  const Counts& getSkillsMax() const override { return limits; }
  const Counts& getProductionSkill() const override { return production; }
  const Counts& getProductionHelp() const override { return help; }
  void setItemName(std::wstring value) override { name = std::move(value); }
  void setItemNamePlural(std::wstring value) override { plural = std::move(value); }
  void setWeight(int value) override { weight = value; }
  void setMeeleWeapon(bool value) override { melee = value; }
  void setRangedWeapon(bool value) override { ranged = value; }
  void setArmour(bool value) override { armour = value; }
  void setResource(bool value) override { resource = value; }
  void setMount(bool value) override { mount = value; }
  void setProvisional(bool value) override { provisional = value; }
  void setMoves(int value) override { moves = value; }
  void setWalkCapacity(int value) override { walk = value; }
  void setRideCapacity(int value) override { ride = value; }
  void setSwimCapacity(int value) override { swim = value; }
  void setFlyCapacity(int value) override { fly = value; }
  void setShipSpeedHexesPerMonth(int value) override { speed = value; }
  void setShipSailingSkillRequired(int value) override { sailing = value; }
  void setMagesStudy(int value) override { mages = value; }
  void setMan(bool value) override { man = value; }
  void setDefaultSkillMax(int value) override { skillMax = value; }
  void setFullText(std::wstring value) override { text = std::move(value); }
  void setResources(Counts value) override { resources = std::move(value); }
  void setSkillsMax(Counts value) override { limits = std::move(value); }
  void setProductionSkill(Counts value) override { production = std::move(value); }
  void setProductionHelp(Counts value) override { help = std::move(value); }
  void setProductionSkillLevel(std::wstring skill, int level) override { production[skill] = level; }

  std::wstring token { L"WOOD" }, name { L"wood" }, plural { L"wooden logs" }, text { L"Item report" };
  int weight { 1 }, moves { 2 }, walk { 3 }, ride { 4 }, swim { 5 }, fly { 6 };
  int speed { 7 }, sailing { 8 }, mages { 9 }, skillMax { 5 };
  bool melee { false }, ranged { false }, armour { false }, resource { true };
  bool mount { false }, provisional { false }, man { false };
  Counts resources {{ L"IRON", 2 }}, limits {{ L"COMB", 4 }};
  Counts production {{ L"LUMB", 1 }}, help {{ L"AXE", 2 }};
};

struct SkillMock : Skill
{
  SkillMock() : Skill(L"", L"") {}
  const std::wstring& getIdentifierToken() const override { return token; }
  const std::wstring& getName() const override { return name; }
  const SkillLevelData* getLevelData(int level) const override
  {
    const auto it = levels.find(level);
    return it == levels.end() ? nullptr : &it->second;
  }
  bool isMagic() const override { return magic; }
  bool isMagicFoundation() const override { return foundation; }
  int getStudyCost() const override { return cost; }
  const std::vector<SkillPrerequisite>& getPrerequisites() const override { return prerequisites; }
  void setMagic(bool value) override { magicWrites.push_back(value); }
  void setMagicFoundation(bool value) override { foundationWrites.push_back(value); }
  void setStudyCost(int value) override { costWrites.push_back(value); }
  void setPrerequisites(std::vector<SkillPrerequisite> value) override { prerequisites = std::move(value); }

  std::wstring token { L"LUMB" }, name { L"Lumberjack" };
  bool magic { false }, foundation { false };
  int cost { 10 };
  std::map<int, SkillLevelData> levels;
  std::vector<SkillPrerequisite> prerequisites;
  std::vector<bool> magicWrites, foundationWrites;
  std::vector<int> costWrites;
};

struct FactionInput
{
  int number;
  std::wstring name;
  bool main;
  int month, year;
  std::wstring password;
  int taxedCurrent, taxedMax, quartermastersCurrent, quartermastersMax;
  int magesCurrent, magesMax, apprenticesCurrent, apprenticesMax;
};

struct FactionRepositoryMock : RepositoryMock<FactionRepository, Faction>
{
  bool add(int number, std::wstring name, bool main, int month, int year, std::wstring password,
           int taxedCurrent, int taxedMax, int quartermastersCurrent, int quartermastersMax,
           int magesCurrent, int magesMax, int apprenticesCurrent, int apprenticesMax,
           int, Faction::Attitude, std::map<int, Faction::Attitude>, int) override
  {
    added.push_back({ number, std::move(name), main, month, year, std::move(password),
      taxedCurrent, taxedMax, quartermastersCurrent, quartermastersMax,
      magesCurrent, magesMax, apprenticesCurrent, apprenticesMax });
    return true;
  }
  Faction* findByNumber(int) override { return found; }

  Faction* found { nullptr };
  std::vector<FactionInput> added;
};

struct RegionInput
{
  int x, y, z;
  std::wstring type, province;
  bool settlement;
  std::wstring settlementName, settlementType, peasants;
  int population;
  double wages;
  int wagesMax, tax, month, year;
  bool visited;
};

struct RegionRepositoryMock : RepositoryMock<RegionRepository, Region>
{
  bool upsertRegion(int x, int y, int z, std::wstring type, std::wstring province,
                    bool settlement, std::wstring settlementName, std::wstring settlementType,
                    std::wstring peasants, int population, double wages, int wagesMax,
                    int tax, int month, int year, bool visited) override
  {
    added.push_back({ x, y, z, std::move(type), std::move(province), settlement,
      std::move(settlementName), std::move(settlementType), std::move(peasants),
      population, wages, wagesMax, tax, month, year, visited });
    return true;
  }
  Region* findByCoordinates(int, int, int) override { return found; }

  Region* found { nullptr };
  std::vector<RegionInput> added;
};

struct UnitRepositoryMock : RepositoryMock<UnitRepository, Unit>
{
  bool add(int number, std::wstring name, int faction, int structure, int x, int y, int z,
           Strings flags, Counts items, int weight, int walk, int ride, int fly, int swim,
           Counts skills, int month, int year, bool guard) override
  {
    added.push_back({ number, std::move(name), faction, structure, x, y, z,
      std::move(flags), std::move(items), weight, walk, ride, fly, swim,
      std::move(skills), month, year, guard });
    return true;
  }
  Unit* findByNumber(int) override { return found; }

  Unit* found { nullptr };
  std::vector<UnitFields> added;
};

struct NewUnitInput
{
  int number;
  std::wstring name;
  int structure, x, y, z;
  Strings flags;
  Counts items;
  int weight, walk, ride, fly, swim;
  Counts skills;
  int month, year, origin;
};

struct UnitNewRepositoryMock : RepositoryMock<UnitNewRepository, UnitNew>
{
  bool add(int number, std::wstring name, int structure, int x, int y, int z,
           Strings flags, Counts items, int weight, int walk, int ride, int fly, int swim,
           Counts skills, int month, int year, int origin) override
  {
    added.push_back({ number, std::move(name), structure, x, y, z, std::move(flags),
      std::move(items), weight, walk, ride, fly, swim, std::move(skills), month, year, origin });
    return true;
  }
  UnitNew* findByNumberAndCoordinates(int number, int x, int y, int z) override
  {
    lookups.push_back({ number, x, y, z });
    return found;
  }

  UnitNew* found { nullptr };
  std::vector<NewUnitInput> added;
  std::vector<std::vector<int>> lookups;
};

struct EventInput
{
  int unit;
  std::wstring message;
  int month, year;
  bool error;
};

struct EventRepositoryMock : RepositoryMock<EventRepository, Event>
{
  bool add(int unit, const std::wstring& message, int month, int year, bool error) override
  {
    added.push_back({ unit, message, month, year, error });
    return true;
  }
  std::vector<EventInput> added;
};

struct StructureInput
{
  int id, x, y, z;
  std::wstring type, name;
  bool closed;
  int month, year;
};

struct StructureRepositoryMock : RepositoryMock<StructureRepository, Structure>
{
  bool add(int id, int x, int y, int z, std::wstring type, std::wstring name,
           bool closed, int month, int year) override
  {
    added.push_back({ id, x, y, z, std::move(type), std::move(name), closed, month, year });
    return true;
  }
  Structure* findByIdAndCoordinates(int, int, int, int) override { return found; }

  Structure* found { nullptr };
  std::vector<StructureInput> added;
};

struct StructInfoInput
{
  std::wstring type;
  int needs, mages;
  bool ship, flying;
  std::wstring token;
};

struct StructInfoRepositoryMock : RepositoryMock<StructInfoRepository, StructInfo>
{
  bool addOrUpdateByType(std::wstring type, int needs, int mages, bool ship, bool flying,
                         std::wstring token) override
  {
    added.push_back({ std::move(type), needs, mages, ship, flying, std::move(token) });
    return true;
  }
  std::vector<StructInfoInput> added;
};

struct ItemInput
{
  std::wstring token, name;
  int weight;
  bool melee, ranged, armour, resource, mount;
  int moves, walk, ride, swim, fly;
  bool man;
};

struct ItemRepositoryMock : RepositoryMock<ItemRepository, Item>
{
  bool add(std::wstring token, std::wstring name, int weight, bool melee, bool ranged,
           bool armour, bool resource, bool mount, int moves, int walk, int ride,
           int swim, int fly, bool man) override
  {
    added.push_back({ token, std::move(name), weight, melee, ranged, armour,
      resource, mount, moves, walk, ride, swim, fly, man });
    byToken[token] = addedItem;
    return true;
  }
  Item* findByIdentifierToken(const std::wstring& token) override
  {
    const auto it = byToken.find(token);
    return it == byToken.end() ? nullptr : it->second;
  }
  std::wstring resolveRaceNameToToken(const std::wstring& value) const override
  {
    raceQueries.push_back(value);
    return raceToken;
  }

  Item* addedItem { nullptr };
  std::map<std::wstring, Item*> byToken;
  std::vector<ItemInput> added;
  mutable Strings raceQueries;
  std::wstring raceToken { L"HUMA" };
};

struct SkillInput
{
  std::wstring token, name;
  int level;
  Counts production;
  std::wstring description;
};

struct SkillRepositoryMock : RepositoryMock<SkillRepository, Skill>
{
  bool addLevel(const std::wstring& token, std::wstring name, int level,
                Counts production, std::wstring description) override
  {
    added.push_back({ token, std::move(name), level, std::move(production), std::move(description) });
    return acceptLevel;
  }
  Skill* findByIdentifier(const std::wstring&) override { return found; }

  Skill* found { nullptr };
  bool acceptLevel { true };
  std::vector<SkillInput> added;
};

struct ReportRepositoryMock : ReportRepository
{
  bool getLatestPeriod(int& monthValue, int& yearValue) const override
  {
    if (!hasPeriod) return false;
    monthValue = month;
    yearValue = year;
    return true;
  }
  void setLatestPeriod(int monthValue, int yearValue) override
  {
    writtenPeriods.emplace_back(monthValue, yearValue);
  }
  void clear() override {}

  int month { 8 }, year { 7 };
  bool hasPeriod { true };
  std::vector<std::pair<int, int>> writtenPeriods;
};

template<typename Repository>
struct UnusedRepositoryMock : Repository
{
  void clear() override {}
};

struct AppDataMock : AppData
{
  explicit AppDataMock(Repositories repositories) : AppData(repositories), repos(repositories) {}
  ReportRepository& reportRepository() override { return repos.reports; }
  const ReportRepository& reportRepository() const override { return repos.reports; }
  FactionRepository& factionRepository() override { return repos.factions; }
  const FactionRepository& factionRepository() const override { return repos.factions; }
  ItemRepository& itemRepository() override { return repos.items; }
  const ItemRepository& itemRepository() const override { return repos.items; }
  SkillRepository& skillRepository() override { return repos.skills; }
  const SkillRepository& skillRepository() const override { return repos.skills; }
  RegionRepository& regionRepository() override { return repos.regions; }
  const RegionRepository& regionRepository() const override { return repos.regions; }
  UnitRepository& unitRepository() override { return repos.units; }
  const UnitRepository& unitRepository() const override { return repos.units; }
  UnitNewRepository& unitNewRepository() override { return repos.newUnits; }
  const UnitNewRepository& unitNewRepository() const override { return repos.newUnits; }
  EventRepository& eventRepository() override { return repos.events; }
  const EventRepository& eventRepository() const override { return repos.events; }
  StructureRepository& structureRepository() override { return repos.structures; }
  const StructureRepository& structureRepository() const override { return repos.structures; }
  StructInfoRepository& structInfoRepository() override { return repos.structureInfo; }
  const StructInfoRepository& structInfoRepository() const override { return repos.structureInfo; }
  int getShipStructureIdThreshold() const override { return shipThreshold; }
  const std::wstring& getFlyingShipsCsv() const override { return flying; }
  const std::wstring& getMagicSkillTriggersCsv() const override { return magic; }
  bool getOnlyLeaderCanTeach() const override { return leaderTeach; }
  bool getLeaderMages() const override { return leaderMages; }
  void setShipStructureIdThreshold(int value) override { shipThreshold = value; }
  void setFlyingShipsCsv(std::wstring value) override { flying = std::move(value); }
  void setMagicSkillTriggersCsv(std::wstring value) override { magic = std::move(value); }
  void setOnlyLeaderCanTeach(bool value) override { leaderTeach = value; }
  void setLeaderMages(bool value) override { leaderMages = value; }
  void refreshStructureDerivedFlags() override { ++refreshCalls; }
  void clear() override { ++clearCalls; }

  Repositories repos;
  int shipThreshold { 100 }, clearCalls { 0 }, refreshCalls { 0 };
  std::wstring flying { L"airship" }, magic { L"a mage with this skill" };
  bool leaderTeach { false }, leaderMages { true };
};

struct FailingOutputBuffer : std::wstreambuf
{
  std::streamsize xsputn(const wchar_t*, std::streamsize) override { return 0; }
  int_type overflow(int_type) override { return traits_type::eof(); }
};

struct ThrowingInputBuffer : std::wstreambuf
{
  int_type underflow() override { throw std::runtime_error("read interrupted"); }
};

struct ServicesMock : DataSerializer::Services
{
  std::unique_ptr<std::wistream> openForReading(const std::wstring& path) override
  {
    readPaths.push_back(path);
    if (throwRead) throw std::runtime_error("read unavailable");
    if (failReadOpen) return nullptr;
    auto stream = std::make_unique<std::wistream>(interruptRead
      ? static_cast<std::wstreambuf*>(&throwingInput) : &input);
    if (badRead) stream->setstate(std::ios::badbit);
    return stream;
  }
  std::unique_ptr<std::wostream> openForWriting(const std::wstring& path) override
  {
    writePaths.push_back(path);
    if (throwWrite) throw std::runtime_error("write unavailable");
    if (failWriteOpen) return nullptr;
    return std::make_unique<std::wostream>(failWrite
      ? static_cast<std::wstreambuf*>(&failingOutput) : &output);
  }
  void finishLoad(AppData& appData) override
  {
    finished.push_back(&appData);
    if (throwFinish) throw std::runtime_error("recalculation failed");
  }

  std::wstringbuf input, output;
  FailingOutputBuffer failingOutput;
  ThrowingInputBuffer throwingInput;
  Strings readPaths, writePaths;
  std::vector<AppData*> finished;
  bool failReadOpen { false }, failWriteOpen { false }, failWrite { false }, badRead { false };
  bool throwRead { false }, throwWrite { false }, interruptRead { false }, throwFinish { false };
};

const wchar_t* const factionJson = LR"({
  "factions": [{"number":99,"name":"Recovered faction","mainFaction":true,
    "password":"new-password","taxedOrTradedRegionsCurrent":11,"taxedOrTradedRegionsMax":12,
    "quartermastersCurrent":13,"quartermastersMax":14,"magesCurrent":15,"magesMax":16,
    "apprenticesCurrent":17,"apprenticesMax":18,"month":9,"year":10,
    "unclaimedSilver":777,"unclaimedSilverAfterOrders":666,"commandUnitNumber":900,
    "defaultAttitude":"4","declaredAttitudes":{"13":3,"14":4}}]
})";

const wchar_t* const regionJson = LR"({
  "regions": [{"x":-6,"y":8,"z":2,"type":"forest","province":"Recovered province",
    "hasSettlement":true,"settlementName":"New town","settlementType":"city",
    "peasantType":"humans","peasantNumber":600,"wages":4.25,"wagesMax":700,
    "taxableIncome":800,"entertainment":911,"resources":{"IRON":17},
    "wanted":{"WOOD":[18,19]},"forSale":{"IRON":[20,21]},
    "exitDirections":["NW","S"],"visited":true,"month":9,"year":10,
    "regionReport":"Recovered {region}\nreport"}]
})";

const wchar_t* const unitJson = LR"({
  "units": [{"number":99,"name":"Recovered unit","factionNumber":13,"structureId":17,
    "x":-6,"y":8,"z":2,"month":9,"year":10,"weight":81,
    "capacityWalk":82,"capacityRide":83,"capacityFly":84,"capacitySwim":85,
    "flags":["guarding","revealing"],"items":{"silv":71,"WOOD":0,"IRON":-4},
    "skills":["Combat [comb] 180","Invalid skill","Empty [NONE] 0"],
    "canStudySkills":["TACT"],"orders":["@STUDY COMB","NAME UNIT \"North\""],
    "onGuard":true}]
})";

const wchar_t* const newUnitJson = LR"({
  "unitNews": [{"number":99,"name":"Recovered new unit","structureId":17,"futureStructureId":80,
    "x":-6,"y":8,"z":2,"month":9,"year":10,"originUnit":500,
    "weight":81,"capacityWalk":82,"capacityRide":83,"capacityFly":84,"capacitySwim":85,
    "flags":["holding"],"items":{"silv":71,"WOOD":0},
    "skills":["Combat [comb] 180"],"canStudySkills":["TACT"],
    "warnings":["First warning","Second \"warning\""]}]
})";

const wchar_t* const itemJson = LR"({
  "itemRepository": [{"token":"TOOL","name":"new tool","namePlural":"new tools",
    "provisional":true,"weight":81,"meeleWeapon":true,"rangedWeapon":true,
    "armour":true,"resource":false,"mount":true,"moves":82,
    "walkCapacity":83,"rideCapacity":84,"swimCapacity":85,"flyCapacity":86,
    "shipSpeedHexesPerMonth":70,"shipSailingSkillRequired":71,"magesStudy":72,
    "man":true,"defaultSkillMax":73,"fullText":"Recovered \"item\"\nreport",
    "resources":{"STONE":30},"skillsMax":{"MAGI":3},
    "productionSkill":{"FORG":2},"productionHelp":{"HAMM":4}}]
})";

const wchar_t* const skillJson = LR"({
  "skillRepository": [{"token":"FORG","level":3,"name":"Forging",
    "productionItems":{"TOOL":12},"magic":true,"magicFoundation":true,
    "description":"Recovered \"skill\"\nreport",
    "prerequisites":[{"token":"TACT","requiredLevel":2}],"studyCost":70}]
})";

struct SerializerFixture
{
  SerializerFixture()
  {
    factions.found = &faction;
    regions.found = &region;
    units.found = &unit;
    newUnits.found = &newUnit;
    structures.found = &structure;
    items.addedItem = &item;
    skills.found = &skill;
  }

  bool save() { return DataSerializer::saveToFile(data, L"state.json", services); }
  bool load(const std::wstring& json)
  {
    services.input.str(json);
    return DataSerializer::loadFromFile(data, L"state.json", services);
  }
  std::wstring savedArray(const std::wstring& key)
  {
    REQUIRE(save());
    const std::wstring json = services.output.str();
    const auto start = json.find(L"  \"" + key + L"\": [");
    REQUIRE(start != std::wstring::npos);
    auto end = json.find(L"\n  ],", start);
    if (end == std::wstring::npos) end = json.find(L"\n  ]\n", start);
    REQUIRE(end != std::wstring::npos);
    return json.substr(start, end - start);
  }
  void loadFaction() { REQUIRE(load(factionJson)); REQUIRE(factions.added.size() == 1); }
  void loadRegion() { REQUIRE(load(regionJson)); REQUIRE(regions.added.size() == 1); }
  void loadUnit() { REQUIRE(load(unitJson)); REQUIRE(units.added.size() == 1); }
  void loadNewUnit() { REQUIRE(load(newUnitJson)); REQUIRE(newUnits.added.size() == 1); }
  void loadItem() { REQUIRE(load(itemJson)); REQUIRE(items.added.size() == 1); }
  void loadSkill() { REQUIRE(load(skillJson)); REQUIRE(skills.added.size() == 1); }

  FactionMock faction;
  RegionMock region;
  UnitMock unit;
  UnitNewMock newUnit;
  EventMock event;
  StructureMock structure;
  StructInfoMock info;
  ItemMock item;
  SkillMock skill;
  ReportRepositoryMock reports;
  FactionRepositoryMock factions;
  ItemRepositoryMock items;
  SkillRepositoryMock skills;
  RegionRepositoryMock regions;
  UnitRepositoryMock units;
  UnitNewRepositoryMock newUnits;
  UnusedRepositoryMock<BattleRepository> battles;
  EventRepositoryMock events;
  StructureRepositoryMock structures;
  StructInfoRepositoryMock structureInfo;
  UnusedRepositoryMock<OrderRepository> orders;
  AppDataMock data {{
    reports, factions, items, skills, regions, units, newUnits,
    battles, events, structures, structureInfo, orders
  }};
  ServicesMock services;
};
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves through the supplied file services without changing AppData", "[DataSerializer]")
{
  REQUIRE(save());
  CHECK(services.writePaths == Strings { L"state.json" });
  CHECK(data.clearCalls == 0);
  CHECK(services.finished.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a save-open failure", "[DataSerializer]")
{
  services.failWriteOpen = true;
  REQUIRE_FALSE(save());
  CHECK(DataSerializer::getLastError() == L"Failed to open file for writing: state.json");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a failed output stream", "[DataSerializer]")
{
  services.failWrite = true;
  REQUIRE_FALSE(save());
  CHECK(DataSerializer::getLastError() == L"Failed to write file: state.json");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports an exception opening output", "[DataSerializer]")
{
  services.throwWrite = true;
  REQUIRE_FALSE(save());
  CHECK(DataSerializer::getLastError() == L"Exception during save: write unavailable");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a repository exception during save", "[DataSerializer]")
{
  factions.throwOnSize = true;
  REQUIRE_FALSE(save());
  CHECK(DataSerializer::getLastError() == L"Exception during save: repository unavailable");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer clears a previous error after saving successfully", "[DataSerializer]")
{
  services.failWriteOpen = true;
  REQUIRE_FALSE(save());
  services.failWriteOpen = false;
  REQUIRE(save());
  CHECK(DataSerializer::getLastError().empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer writes the latest report period", "[DataSerializer]")
{
  reports.month = 11;
  reports.year = 20;
  REQUIRE(save());
  CHECK(services.output.str().find(L"\"latestReportMonth\": 11,\n  \"latestReportYear\": 20,") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer writes zero report coordinates when no period exists", "[DataSerializer]")
{
  reports.hasPeriod = false;
  REQUIRE(save());
  CHECK(services.output.str().find(L"\"latestReportMonth\": 0,\n  \"latestReportYear\": 0,") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer writes an empty faction array without objects", "[DataSerializer]")
{
  CHECK(savedArray(L"factions") == L"  \"factions\": [\n");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer escapes faction names and passwords", "[DataSerializer]")
{
  faction.name = L"North \"Clan\"\n";
  faction.password = L"a\\b\"c";
  factions.entries = { &faction };
  const std::wstring saved = savedArray(L"factions");
  CHECK(saved.find(L"\"name\": \"North \\\"Clan\\\"\\n\"") != std::wstring::npos);
  CHECK(saved.find(L"\"password\": \"a\\\\b\\\"c\"") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer separates multiple faction records", "[DataSerializer]")
{
  factions.entries = { &faction, &faction };
  CHECK(savedArray(L"factions").find(L"    },\n    {") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer writes declared faction attitudes", "[DataSerializer]")
{
  faction.attitudes = {{ 13, Faction::Attitude::Friendly }, { 14, Faction::Attitude::Ally }};
  factions.entries = { &faction };
  CHECK(savedArray(L"factions").find(L"\"declaredAttitudes\": {\"13\": 3, \"14\": 4}") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves region economy, exits, and escaped report text", "[DataSerializer]")
{
  region.report = L"First\n\"Second\"";
  regions.entries = { &region };
  const std::wstring saved = savedArray(L"regions");
  CHECK(saved.find(L"\"wages\": 12.75") != std::wstring::npos);
  CHECK(saved.find(L"\"wanted\": {\"IRON\": [10, 5]}") != std::wstring::npos);
  CHECK(saved.find(L"\"exitDirections\": [\"N\", \"SE\"]") != std::wstring::npos);
  CHECK(saved.find(L"\"regionReport\": \"First\\n\\\"Second\\\"\"") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves unit inventory, training, and guard status", "[DataSerializer]")
{
  unit.guard = true;
  units.entries = { &unit };
  const std::wstring saved = savedArray(L"units");
  CHECK(saved.find(L"\"items\": {\"SILV\": 100, \"WOOD\": 3}") != std::wstring::npos);
  CHECK(saved.find(L"\"skills\": [\"COMB [COMB] 90\"]") != std::wstring::npos);
  CHECK(saved.find(L"\"onGuard\": true") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer preserves repeat markers in saved orders", "[DataSerializer]")
{
  unit.orders = { L"@STUDY COMB" };
  units.entries = { &unit };
  CHECK(savedArray(L"units").find(L"\"orders\": [\"@STUDY COMB\"]") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer escapes quotation marks in saved orders", "[DataSerializer]")
{
  unit.orders = { L"NAME UNIT \"North\"" };
  units.entries = { &unit };
  CHECK(savedArray(L"units").find(L"\"orders\": [\"NAME UNIT \\\"North\\\"\"]") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves new-unit origin, placement, and escaped warnings", "[DataSerializer]")
{
  newUnit.warnings = { L"Bad \"order\"\n" };
  newUnits.entries = { &newUnit };
  const std::wstring saved = savedArray(L"unitNews");
  CHECK(saved.find(L"\"futureStructureId\": 6") != std::wstring::npos);
  CHECK(saved.find(L"\"originUnit\": 42") != std::wstring::npos);
  CHECK(saved.find(L"\"warnings\": [\"Bad \\\"order\\\"\\n\"]") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves escaped event messages and error classification", "[DataSerializer]")
{
  event.message = L"Bad \"order\"\n";
  events.entries = { &event };
  const std::wstring saved = savedArray(L"events");
  CHECK(saved.find(L"\"message\": \"Bad \\\"order\\\"\\n\"") != std::wstring::npos);
  CHECK(saved.find(L"\"errorEvent\": true") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves configured teaching, mage, and magic-trigger settings", "[DataSerializer]")
{
  data.leaderTeach = true;
  data.leaderMages = false;
  data.magic = L"with \"magic\"\n";
  REQUIRE(save());
  CHECK(services.output.str().find(L"\"onlyLeaderCanTeach\": true") != std::wstring::npos);
  CHECK(services.output.str().find(L"\"leaderMages\": false") != std::wstring::npos);
  CHECK(services.output.str().find(L"\"magicSkillTriggersCsv\": \"with \\\"magic\\\"\\n\"") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves structure ownership and fleet composition", "[DataSerializer]")
{
  structures.entries = { &structure };
  const std::wstring saved = savedArray(L"structures");
  CHECK(saved.find(L"\"ownerUnitId\": 42") != std::wstring::npos);
  CHECK(saved.find(L"\"fleetItems\": {\"LONG\": 2}") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves flying structure metadata and item associations", "[DataSerializer]")
{
  structureInfo.entries = { &info };
  const std::wstring saved = savedArray(L"structureInfo");
  CHECK(saved.find(L"\"itemIdentifierToken\": \"AIRS\"") != std::wstring::npos);
  CHECK(saved.find(L"\"isFlying\": true") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves item names, provisional status, and production metadata", "[DataSerializer]")
{
  item.provisional = true;
  items.entries = { &item };
  const std::wstring saved = savedArray(L"itemRepository");
  CHECK(saved.find(L"\"namePlural\": \"wooden logs\"") != std::wstring::npos);
  CHECK(saved.find(L"\"provisional\": true") != std::wstring::npos);
  CHECK(saved.find(L"\"productionHelp\": {\"AXE\": 2}") != std::wstring::npos);
  CHECK(saved.find(L"\"skillsMax\": {\"COMB\": 4}") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer writes only populated skill levels", "[DataSerializer]")
{
  skill.levels[3] = { {}, L"Level three" };
  skills.entries = { &skill };
  const std::wstring saved = savedArray(L"skillRepository");
  CHECK(saved.find(L"\"level\": 3") != std::wstring::npos);
  CHECK(saved.find(L"\"level\": 1") == std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer separates multiple skill levels", "[DataSerializer]")
{
  skill.levels[1] = { {}, L"First" };
  skill.levels[3] = { {}, L"Third" };
  skills.entries = { &skill };
  CHECK(savedArray(L"skillRepository").find(L"    },\n    {") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer saves skill prerequisites and escaped level descriptions", "[DataSerializer]")
{
  skill.levels[1] = { {}, L"First\n\"Second\"" };
  skill.prerequisites = {{ L"TACT", 2 }};
  skills.entries = { &skill };
  const std::wstring saved = savedArray(L"skillRepository");
  CHECK(saved.find(L"\"prerequisites\": [{\"token\": \"TACT\", \"requiredLevel\": 2}]") != std::wstring::npos);
  CHECK(saved.find(L"\"description\": \"First\\n\\\"Second\\\"\"") != std::wstring::npos);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads an empty document through file services and completes the supplied AppData", "[DataSerializer]")
{
  REQUIRE(load(L"{}"));
  CHECK(services.readPaths == Strings { L"state.json" });
  CHECK(data.clearCalls == 1);
  CHECK(services.finished == std::vector<AppData*> { &data });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a load-open failure without clearing or completing AppData", "[DataSerializer]")
{
  services.failReadOpen = true;
  REQUIRE_FALSE(load(L"{}"));
  CHECK(DataSerializer::getLastError() == L"Failed to open file for reading: state.json");
  CHECK(data.clearCalls == 0);
  CHECK(services.finished.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a bad input stream without clearing AppData", "[DataSerializer]")
{
  services.badRead = true;
  REQUIRE_FALSE(load(L"{}"));
  CHECK(DataSerializer::getLastError() == L"Failed to read file: state.json");
  CHECK(data.clearCalls == 0);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports an exception opening input", "[DataSerializer]")
{
  services.throwRead = true;
  REQUIRE_FALSE(load(L"{}"));
  CHECK(DataSerializer::getLastError() == L"Exception during load: read unavailable");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports an interrupted read", "[DataSerializer]")
{
  services.interruptRead = true;
  REQUIRE_FALSE(load(L"{}"));
  CHECK(DataSerializer::getLastError() == L"Exception during load: read interrupted");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer reports a load-completion exception", "[DataSerializer]")
{
  services.throwFinish = true;
  REQUIRE_FALSE(load(L"{}"));
  CHECK(DataSerializer::getLastError() == L"Exception during load: recalculation failed");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer clears partially loaded state after an exception", "[DataSerializer]")
{
  services.throwFinish = true;
  REQUIRE_FALSE(load(factionJson));
  CHECK(data.clearCalls == 2);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer clears a previous error after loading successfully", "[DataSerializer]")
{
  services.failReadOpen = true;
  REQUIRE_FALSE(load(L"{}"));
  services.failReadOpen = false;
  REQUIRE(load(L"{}"));
  CHECK(DataSerializer::getLastError().empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores the report period", "[DataSerializer]")
{
  REQUIRE(load(LR"({"latestReportMonth":9,"latestReportYear":10})"));
  CHECK(reports.writtenPeriods == std::vector<std::pair<int, int>> {{ 9, 10 }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer does not set a period without a report month", "[DataSerializer]")
{
  REQUIRE(load(LR"({"latestReportYear":10})"));
  CHECK(reports.writtenPeriods.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer does not set a period without a report year", "[DataSerializer]")
{
  REQUIRE(load(LR"({"latestReportMonth":9})"));
  CHECK(reports.writtenPeriods.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores the ship ID threshold", "[DataSerializer]")
{
  REQUIRE(load(LR"({"settings":{"shipStructureIdThreshold":400}})"));
  CHECK(data.shipThreshold == 400);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores the flying-ship configuration", "[DataSerializer]")
{
  REQUIRE(load(LR"({"settings":{"flyingShipsCsv":"cloud boat, airship"}})"));
  CHECK(data.flying == L"cloud boat, airship");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores escaped magic trigger phrases", "[DataSerializer]")
{
  const std::wstring json = LR"({"settings":{"magicSkillTriggersCsv":"with \"magic\"\n"}})";
  REQUIRE(load(json));
  CHECK(data.magic == L"with \"magic\"\n");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores the leader-only teaching setting", "[DataSerializer]")
{
  REQUIRE(load(LR"({"settings":{"onlyLeaderCanTeach":true}})"));
  CHECK(data.leaderTeach);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores a disabled leader-mage setting", "[DataSerializer]")
{
  REQUIRE(load(LR"({"settings":{"leaderMages":false}})"));
  CHECK_FALSE(data.leaderMages);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer preserves absent numeric and magic-trigger settings", "[DataSerializer]")
{
  data.shipThreshold = 777;
  data.magic = L"custom phrase";
  REQUIRE(load(LR"({"settings":{}})"));
  CHECK(data.shipThreshold == 777);
  CHECK(data.magic == L"custom phrase");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer preserves settings when the settings object is absent", "[DataSerializer]")
{
  data.flying = L"custom ship";
  REQUIRE(load(L"{}"));
  CHECK(data.flying == L"custom ship");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads faction identity, credentials, and command unit", "[DataSerializer]")
{
  loadFaction();
  CHECK(factions.added.at(0).number == 99);
  CHECK(factions.added.at(0).name == L"Recovered faction");
  CHECK(factions.added.at(0).password == L"new-password");
  CHECK(faction.command == 900);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads faction limit counters", "[DataSerializer]")
{
  loadFaction();
  CHECK(factions.added.at(0).quartermastersMax == 14);
  CHECK(factions.added.at(0).apprenticesCurrent == 17);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores current and projected faction silver", "[DataSerializer]")
{
  loadFaction();
  CHECK(faction.unclaimed == 777);
  CHECK(faction.unclaimedAfter == 666);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores saved default and declared faction attitudes", "[DataSerializer]")
{
  loadFaction();
  CHECK(faction.attitude == Faction::Attitude::Ally);
  CHECK(faction.attitudes == std::map<int, Faction::Attitude> {
    { 13, Faction::Attitude::Friendly }, { 14, Faction::Attitude::Ally }
  });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer accepts numeric default attitudes", "[DataSerializer]")
{
  REQUIRE(load(LR"({"factions":[{"number":99,"defaultAttitude":3}]})"));
  CHECK(faction.attitude == Faction::Attitude::Friendly);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads multiple faction records", "[DataSerializer]")
{
  REQUIRE(load(LR"({"factions":[{"number":99,"name":"First"},{"number":100,"name":"Second"}]})"));
  REQUIRE(factions.added.size() == 2);
  CHECK(factions.added.at(1).number == 100);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer preserves braces inside faction names", "[DataSerializer]")
{
  REQUIRE(load(LR"({"factions":[{"number":99,"name":"North } { clan","month":9}]})"));
  REQUIRE(factions.added.size() == 1);
  CHECK(factions.added.at(0).name == L"North } { clan");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads region location, settlement, exits, and report text", "[DataSerializer]")
{
  loadRegion();
  CHECK(regions.added.at(0).x == -6);
  CHECK(regions.added.at(0).z == 2);
  CHECK(regions.added.at(0).settlementType == L"city");
  CHECK(region.exits == Strings { L"NW", L"S" });
  CHECK(region.report == L"Recovered {region}\nreport");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer resolves and stores peasant tokens through the item repository", "[DataSerializer]")
{
  items.raceToken = L"RACE";
  loadRegion();
  CHECK(items.raceQueries == Strings { L"humans" });
  CHECK(regions.added.at(0).peasants == L"RACE");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores region economy, resources, and markets", "[DataSerializer]")
{
  loadRegion();
  CHECK(regions.added.at(0).wages == 4.25);
  CHECK(region.entertainment == 911);
  CHECK(region.resources == Counts {{ L"IRON", 17 }});
  CHECK(region.wanted == Market {{ L"WOOD", { 18, 19 } }});
  CHECK(region.sale == Market {{ L"IRON", { 20, 21 } }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads unit ownership, capacity, flags, and guard status", "[DataSerializer]")
{
  loadUnit();
  CHECK(units.added.at(0).faction == 13);
  CHECK(units.added.at(0).structure == 17);
  CHECK(units.added.at(0).walk == 82);
  CHECK(units.added.at(0).flags == Strings { L"guarding", L"revealing" });
  CHECK(unit.guard);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer normalizes loaded inventory while excluding nonpositive counts", "[DataSerializer]")
{
  loadUnit();
  CHECK(units.added.at(0).items.at(L"SILV") == 71);
  CHECK(units.added.at(0).items.count(L"WOOD") == 0);
  CHECK(units.added.at(0).items.count(L"IRON") == 0);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores unit training and studyable skills", "[DataSerializer]")
{
  loadUnit();
  CHECK(units.added.at(0).skills == Counts {{ L"COMB", 180 }});
  CHECK(unit.canStudy == Strings { L"TACT" });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores repeat markers and escaped text in unit orders", "[DataSerializer]")
{
  loadUnit();
  CHECK(unit.orders.at(0) == L"@STUDY COMB");
  CHECK(unit.orders.at(1) == L"NAME UNIT \"North\"");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores new-unit origin and structure placement at the correct coordinates", "[DataSerializer]")
{
  loadNewUnit();
  CHECK(newUnits.added.at(0).origin == 500);
  CHECK(newUnits.lookups == std::vector<std::vector<int>> {{ 99, -6, 8, 2 }});
  CHECK(newUnit.futureStructure == 80);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores new-unit inventory and training", "[DataSerializer]")
{
  loadNewUnit();
  CHECK(newUnits.added.at(0).items == Counts {{ L"SILV", 71 }});
  CHECK(newUnits.added.at(0).skills == Counts {{ L"COMB", 180 }});
  CHECK(newUnit.canStudy == Strings { L"TACT" });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores the order of new-unit warnings", "[DataSerializer]")
{
  loadNewUnit();
  CHECK(newUnit.warnings == Strings { L"First warning", L"Second \"warning\"" });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads event messages without constructing domain events", "[DataSerializer]")
{
  const std::wstring json = LR"({"events":[{"unitId":99,"message":"Failed \"order\"","month":9,"year":10,"errorEvent":true}]})";
  REQUIRE(load(json));
  REQUIRE(events.added.size() == 1);
  CHECK(events.added.at(0).message == L"Failed \"order\"");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores error-event classification", "[DataSerializer]")
{
  REQUIRE(load(LR"({"events":[{"unitId":99,"message":"Failure","errorEvent":true}]})"));
  REQUIRE(events.added.size() == 1);
  CHECK(events.added.at(0).error);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer preserves braces inside event messages", "[DataSerializer]")
{
  REQUIRE(load(LR"({"events":[{"unitId":99,"message":"Failed } { order","month":9}]})"));
  REQUIRE(events.added.size() == 1);
  CHECK(events.added.at(0).message == L"Failed } { order");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer skips events without a unit ID", "[DataSerializer]")
{
  REQUIRE(load(LR"({"events":[{"unitId":0,"message":"Failure"}]})"));
  CHECK(events.added.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer skips events without a message", "[DataSerializer]")
{
  REQUIRE(load(LR"({"events":[{"unitId":99,"message":""}]})"));
  CHECK(events.added.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores event report years", "[DataSerializer]")
{
  REQUIRE(load(LR"({"events":[{"unitId":99,"message":"Failure","year":10}]})"));
  REQUIRE(events.added.size() == 1);
  CHECK(events.added.at(0).year == 10);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads structure names", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structures":[{"id":99,"name":"Recovered fleet","x":-6,"y":8,"z":2}]})"));
  REQUIRE(structures.added.size() == 1);
  CHECK(structures.added.at(0).name == L"Recovered fleet");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores structure closed status", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structures":[{"id":99,"isClosed":true}]})"));
  REQUIRE(structures.added.size() == 1);
  CHECK(structures.added.at(0).closed);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores structure ownership", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structures":[{"id":99,"ownerUnitId":900}]})"));
  CHECK(structure.owner == 900);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores nested fleet composition", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structures":[{"id":99,"fleetItems":{"AIRS":3}}]})"));
  CHECK(structure.fleet == Counts {{ L"AIRS", 3 }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer refreshes derived structure flags after loading structures", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structures":[]})"));
  CHECK(data.refreshCalls == 1);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads structure-info construction requirements", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structureInfo":[{"type":"fort","needs":300}]})"));
  REQUIRE(structureInfo.added.size() == 1);
  CHECK(structureInfo.added.at(0).needs == 300);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads structure-info item associations", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structureInfo":[{"type":"fort","itemIdentifierToken":"FORT"}]})"));
  REQUIRE(structureInfo.added.size() == 1);
  CHECK(structureInfo.added.at(0).token == L"FORT");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer accepts the legacy structInfo key", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structInfo":[{"type":"legacy fort","needs":300}]})"));
  REQUIRE(structureInfo.added.size() == 1);
  CHECK(structureInfo.added.at(0).type == L"legacy fort");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer prefers structureInfo over the legacy key", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structureInfo":[{"type":"new fort"}],"structInfo":[{"type":"old fort"}]})"));
  REQUIRE(structureInfo.added.size() == 1);
  CHECK(structureInfo.added.at(0).type == L"new fort");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer skips structure metadata without a type", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structureInfo":[{"type":"","needs":300}]})"));
  CHECK(structureInfo.added.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer refreshes derived flags after loading structure metadata", "[DataSerializer]")
{
  REQUIRE(load(LR"({"structureInfo":[{"type":"fort"}]})"));
  CHECK(data.refreshCalls == 1);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer creates item identifiers with restored names, status, and description", "[DataSerializer]")
{
  loadItem();
  CHECK(items.added.at(0).token == L"TOOL");
  CHECK(item.plural == L"new tools");
  CHECK(item.provisional);
  CHECK(item.text == L"Recovered \"item\"\nreport");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores item movement capacity and sailing requirements", "[DataSerializer]")
{
  loadItem();
  CHECK(items.added.at(0).fly == 86);
  CHECK(item.speed == 70);
  CHECK(item.sailing == 71);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores item mage-study capacity and skill limits", "[DataSerializer]")
{
  loadItem();
  CHECK(item.mages == 72);
  CHECK(item.limits == Counts {{ L"MAGI", 3 }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores item production requirements and assistance", "[DataSerializer]")
{
  loadItem();
  CHECK(item.resources == Counts {{ L"STONE", 30 }});
  CHECK(item.production == Counts {{ L"FORG", 2 }});
  CHECK(item.help == Counts {{ L"HAMM", 4 }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer updates existing item metadata without adding a duplicate", "[DataSerializer]")
{
  items.byToken[L"TOOL"] = &item;
  REQUIRE(load(itemJson));
  CHECK(items.added.empty());
  CHECK(item.name == L"new tool");
  CHECK(item.weight == 81);
  CHECK_FALSE(item.resource);
  CHECK(item.man);
  CHECK(item.help == Counts {{ L"HAMM", 4 }});
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer loads skill-level numbers, production, and escaped descriptions", "[DataSerializer]")
{
  loadSkill();
  CHECK(skills.added.at(0).level == 3);
  CHECK(skills.added.at(0).production == Counts {{ L"TOOL", 12 }});
  CHECK(skills.added.at(0).description == L"Recovered \"skill\"\nreport");
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores skill classifications and study costs", "[DataSerializer]")
{
  loadSkill();
  CHECK(skill.magicWrites == std::vector<bool> { true });
  CHECK(skill.foundationWrites == std::vector<bool> { true });
  CHECK(skill.costWrites == std::vector<int> { 70 });
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer does not apply negative skill study costs", "[DataSerializer]")
{
  REQUIRE(load(LR"({"skillRepository":[{"token":"FORG","studyCost":-20}]})"));
  CHECK(skill.costWrites.empty());
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer restores skill prerequisite identifiers and required levels", "[DataSerializer]")
{
  loadSkill();
  REQUIRE(skill.prerequisites.size() == 1);
  CHECK(skill.prerequisites.at(0).token == L"TACT");
  CHECK(skill.prerequisites.at(0).requiredLevel == 2);
}

TEST_CASE_METHOD(SerializerFixture, "DataSerializer links produced items to their loaded skill level", "[DataSerializer]")
{
  items.byToken[L"TOOL"] = &item;
  loadSkill();
  CHECK(item.production.at(L"FORG") == 3);
}
