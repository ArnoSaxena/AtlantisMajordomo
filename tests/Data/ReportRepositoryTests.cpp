/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/ReportRepository.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/OrderRepository.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/UnitRepository.hpp"
#include "Data/UnitNewRepository.hpp"
#include "TestUtils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace
{
struct ReportRepositoryFixture
{
  ReportRepository repository;
};
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository reports zero size when empty", "[ReportRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository throws when accessing empty repository", "[ReportRepository]")
{
  CHECK_THROWS(repository.at(0));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository clears repository", "[ReportRepository]")
{
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository clears latest period on clear", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  repository.clear();
  int month = 5;
  int year = 5;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository reports failure getting latest period when empty", "[ReportRepository]")
{
  int month = 5;
  int year = 6;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository returns zero values for latest period when empty", "[ReportRepository]")
{
  int month = 5;
  int year = 6;
  repository.getLatestPeriod(month, year);
  CHECK(month == 0);
  CHECK(year == 0);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository retrieves latest period when one report set", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(month == 8);
  CHECK(year == 7);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository selects latest period by year first", "[ReportRepository]")
{
  repository.setLatestPeriod(12, 6);
  repository.setLatestPeriod(1, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(month == 1);
  CHECK(year == 7);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository selects latest period by month within same year", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  repository.setLatestPeriod(12, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(month == 12);
  CHECK(year == 7);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores invalid month during setLatestPeriod", "[ReportRepository]")
{
  repository.setLatestPeriod(0, 7);
  int month = 0;
  int year = 0;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores invalid high month during setLatestPeriod", "[ReportRepository]")
{
  repository.setLatestPeriod(13, 7);
  int month = 0;
  int year = 0;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores zero year during setLatestPeriod", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 0);
  int month = 0;
  int year = 0;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores negative year during setLatestPeriod", "[ReportRepository]")
{
  repository.setLatestPeriod(8, -1);
  int month = 0;
  int year = 0;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository updates latest period to more recent valid period", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  repository.setLatestPeriod(12, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(month == 12);
  CHECK(year == 7);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores older period when updating latest", "[ReportRepository]")
{
  repository.setLatestPeriod(12, 7);
  repository.setLatestPeriod(8, 7);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(month == 12);
  CHECK(year == 7);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository reports failure removing from empty repository", "[ReportRepository]")
{
  CHECK_FALSE(repository.removeAt(0));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository sets error message when removal fails", "[ReportRepository]")
{
  repository.removeAt(0);
  CHECK_FALSE(repository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository clears latest period when all reports are removed", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  repository.clear();
  int month = 0;
  int year = 0;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository has empty error message on initialization", "[ReportRepository]")
{
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository clears error message when clearing repository", "[ReportRepository]")
{
  repository.removeAt(0);
  REQUIRE_FALSE(repository.getLastError().empty());
  repository.clear();
  CHECK(repository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository accepts valid single month in range", "[ReportRepository]")
{
  for (int m = 1; (m <= 12); ++m)
  {
    ReportRepository r;
    r.setLatestPeriod(m, 7);
    int month = 0, year = 0;
    REQUIRE(r.getLatestPeriod(month, year));
    CHECK(month == m);
  }
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository accepts valid years", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 1);
  int month = 0, year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(year == 1);

  repository.setLatestPeriod(8, 100);
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(year == 100);
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository ignores duplicate same period set", "[ReportRepository]")
{
  repository.setLatestPeriod(8, 7);
  repository.setLatestPeriod(8, 7);
  int month = 0, year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(8, 7));
}

TEST_CASE_METHOD(ReportRepositoryFixture, "ReportRepository chooses year 8 month 1 over year 7 month 12", "[ReportRepository]")
{
  repository.setLatestPeriod(12, 7);
  repository.setLatestPeriod(1, 8);
  int month = 0, year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(year == 8);
  CHECK(month == 1);
}

namespace
{
struct ReportRepositoryFullFixture
{
  ReportRepository reportRepository;
  FactionRepository factionRepository;
  RegionRepository regionRepository;
  UnitRepository unitRepository;
  UnitNewRepository unitNewRepository;
  BattleRepository battleRepository;
  EventRepository eventRepository;
  ItemRepository itemRepository;
  SkillRepository skillRepository;
  StructureRepository structureRepository;
  StructInfoRepository structInfoRepository;
  OrderRepository orderRepository;

  std::vector<std::wstring> emptyShipTokens;
  std::vector<std::wstring> emptyMagicTokens;
};
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile fails for non-existent file", "[ReportRepository]")
{
  bool result = reportRepository.addFromFile(
    L"D:\\nonexistent\\file.rep",
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens);

  CHECK_FALSE(result);
  CHECK_FALSE(reportRepository.getLastError().empty());
  CHECK(reportRepository.size() == 0);
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile succeeds for valid report", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  bool result = reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens);

  CHECK(result);
  CHECK(reportRepository.size() == 1);
  CHECK(reportRepository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile parses report metadata", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  const Report& report = reportRepository.at(0);
  CHECK_FALSE(report.getFactionName().empty());
  CHECK(report.getFactionNumber() > 0);
  CHECK(report.getMonth() > 0);
  CHECK((report.getMonth() <= 12));
  CHECK(report.getYear() > 0);
  CHECK(report.hasContent());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile updates latest period", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  int month = 0, year = 0;
  REQUIRE(reportRepository.getLatestPeriod(month, year));
  CHECK(month > 0);
  CHECK((month <= 12));
  CHECK(year > 0);

  const Report& report = reportRepository.at(0);
  CHECK(month == report.getMonth());
  CHECK(year == report.getYear());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile stores faction in repository", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  const Report& report = reportRepository.at(0);
  int factionNumber = report.getFactionNumber();

  Faction* faction = factionRepository.findByNumber(factionNumber);
  REQUIRE(faction != nullptr);
  CHECK(faction->getName() == report.getFactionName());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile synchronizes faction data", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  const Report& report = reportRepository.at(0);
  Faction* faction = factionRepository.findByNumber(report.getFactionNumber());
  REQUIRE(faction != nullptr);

  CHECK(faction->getYear() == report.getYear());
  CHECK(faction->getMonth() == report.getMonth());
  CHECK(faction->getTaxedOrTradedRegionsCurrent() == report.getFactionTaxedOrTradedRegionsCurrent());
  CHECK(faction->getTaxedOrTradedRegionsMax() == report.getFactionTaxedOrTradedRegionsMax());
  CHECK(faction->getQuartermastersCurrent() == report.getFactionQuartermastersCurrent());
  CHECK(faction->getQuartermastersMax() == report.getFactionQuartermastersMax());
  CHECK(faction->getMagesCurrent() == report.getFactionMagesCurrent());
  CHECK(faction->getMagesMax() == report.getFactionMagesMax());
  CHECK(faction->getApprenticesCurrent() == report.getFactionApprenticesCurrent());
  CHECK(faction->getApprenticesMax() == report.getFactionApprenticesMax());
  CHECK(faction->getUnclaimedSilver() == report.getFactionUnclaimedSilver());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile does not skip syncFactionFromHeader false", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens,
    false));

  CHECK(reportRepository.size() == 1);
  CHECK(reportRepository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile clears error on success", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  reportRepository.addFromFile(
    L"D:\\nonexistent\\file.rep",
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens);
  REQUIRE_FALSE(reportRepository.getLastError().empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  bool result = reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens);

  CHECK(result);
  CHECK(reportRepository.getLastError().empty());
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile preserves error message on failure", "[ReportRepository]")
{
  bool result = reportRepository.addFromFile(
    L"D:\\nonexistent\\file.rep",
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens);

  CHECK_FALSE(result);
  std::wstring error = reportRepository.getLastError();
  CHECK_FALSE(error.empty());
  CHECK(((error.find(L"file") != std::wstring::npos) || (error.find(L"File") != std::wstring::npos)));
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile populates regions", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  CHECK(regionRepository.size() > 0);
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile populates units", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());

  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());
  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  CHECK(unitRepository.size() > 0);
}

TEST_CASE_METHOD(ReportRepositoryFullFixture, "ReportRepository addFromFile with multiple reports tracks latest period", "[ReportRepository]")
{
  std::string testFilePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(testFilePath.empty());
  std::wstring wFilePath(testFilePath.begin(), testFilePath.end());

  REQUIRE(reportRepository.addFromFile(
    wFilePath,
    factionRepository,
    regionRepository,
    unitRepository,
    unitNewRepository,
    battleRepository,
    eventRepository,
    itemRepository,
    skillRepository,
    structureRepository,
    structInfoRepository,
    orderRepository,
    0,
    emptyShipTokens,
    emptyMagicTokens));

  int month1 = 0, year1 = 0;
  REQUIRE(reportRepository.getLatestPeriod(month1, year1));

  std::string testFilePath2 = TestUtils::getTestDataPath("report_17_004.rep");
  if (!testFilePath2.empty())
  {
    std::wstring wFilePath2(testFilePath2.begin(), testFilePath2.end());
    REQUIRE(reportRepository.addFromFile(
      wFilePath2,
      factionRepository,
      regionRepository,
      unitRepository,
      unitNewRepository,
      battleRepository,
      eventRepository,
      itemRepository,
      skillRepository,
      structureRepository,
      structInfoRepository,
      orderRepository,
      0,
      emptyShipTokens,
      emptyMagicTokens));

    int month2 = 0, year2 = 0;
    REQUIRE(reportRepository.getLatestPeriod(month2, year2));

    CHECK(reportRepository.size() == 2);
    CHECK((std::make_pair(year2, month2) >= std::make_pair(year1, month1)));
  }
}
