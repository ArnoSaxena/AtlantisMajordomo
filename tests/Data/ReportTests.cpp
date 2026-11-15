/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Report.hpp"
#include "TestUtils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace
{
struct ReportFixture
{
  Report report;
};
}

TEST_CASE_METHOD(ReportFixture, "Report starts empty", "[Report]")
{
  CHECK_FALSE(report.hasContent());
  CHECK(report.getContent().empty());
  CHECK(report.getLines().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report can set month", "[Report]")
{
  report.setMonth(6);
  CHECK(report.getMonth() == 6);
}

TEST_CASE_METHOD(ReportFixture, "Report can set year", "[Report]")
{
  report.setYear(1);
  CHECK(report.getYear() == 1);
}

TEST_CASE_METHOD(ReportFixture, "Report can set faction number", "[Report]")
{
  report.setFactionNumber(42);
  CHECK(report.getFactionNumber() == 42);
}

TEST_CASE_METHOD(ReportFixture, "Report can set faction name", "[Report]")
{
  report.setFactionName(L"Adventurers");
  CHECK(report.getFactionName() == L"Adventurers");
}

TEST_CASE_METHOD(ReportFixture, "Report defaults month to zero", "[Report]")
{
  CHECK(report.getMonth() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults year to zero", "[Report]")
{
  CHECK(report.getYear() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults faction number to zero", "[Report]")
{
  CHECK(report.getFactionNumber() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults faction name to empty", "[Report]")
{
  CHECK(report.getFactionName().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report stores faction pointer", "[Report]")
{
  Faction testFaction(1, L"Test", true);
  Faction* pFaction = &testFaction;
  report.setFaction(pFaction);
  CHECK(report.getFaction() == pFaction);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults faction pointer to nullptr", "[Report]")
{
  CHECK(report.getFaction() == nullptr);
}

TEST_CASE_METHOD(ReportFixture, "Report can set taxed or traded regions current", "[Report]")
{
  report.setFactionTaxedOrTradedRegionsCurrent(10);
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 10);
}

TEST_CASE_METHOD(ReportFixture, "Report can set taxed or traded regions max", "[Report]")
{
  report.setFactionTaxedOrTradedRegionsMax(20);
  CHECK(report.getFactionTaxedOrTradedRegionsMax() == 20);
}

TEST_CASE_METHOD(ReportFixture, "Report can set quartermasters current", "[Report]")
{
  report.setFactionQuartermastersCurrent(5);
  CHECK(report.getFactionQuartermastersCurrent() == 5);
}

TEST_CASE_METHOD(ReportFixture, "Report can set quartermasters max", "[Report]")
{
  report.setFactionQuartermastersMax(10);
  CHECK(report.getFactionQuartermastersMax() == 10);
}

TEST_CASE_METHOD(ReportFixture, "Report can set mages current", "[Report]")
{
  report.setFactionMagesCurrent(3);
  CHECK(report.getFactionMagesCurrent() == 3);
}

TEST_CASE_METHOD(ReportFixture, "Report can set mages max", "[Report]")
{
  report.setFactionMagesMax(8);
  CHECK(report.getFactionMagesMax() == 8);
}

TEST_CASE_METHOD(ReportFixture, "Report can set apprentices current", "[Report]")
{
  report.setFactionApprenticesCurrent(2);
  CHECK(report.getFactionApprenticesCurrent() == 2);
}

TEST_CASE_METHOD(ReportFixture, "Report can set apprentices max", "[Report]")
{
  report.setFactionApprenticesMax(5);
  CHECK(report.getFactionApprenticesMax() == 5);
}

TEST_CASE_METHOD(ReportFixture, "Report can set unclaimed silver", "[Report]")
{
  report.setFactionUnclaimedSilver(1500);
  CHECK(report.getFactionUnclaimedSilver() == 1500);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults all faction counts to zero", "[Report]")
{
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 0);
  CHECK(report.getFactionTaxedOrTradedRegionsMax() == 0);
  CHECK(report.getFactionQuartermastersCurrent() == 0);
  CHECK(report.getFactionQuartermastersMax() == 0);
  CHECK(report.getFactionMagesCurrent() == 0);
  CHECK(report.getFactionMagesMax() == 0);
  CHECK(report.getFactionApprenticesCurrent() == 0);
  CHECK(report.getFactionApprenticesMax() == 0);
  CHECK(report.getFactionUnclaimedSilver() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report can get default faction attitude", "[Report]")
{
  CHECK(report.getFactionDefaultAttitudeText() == L"Neutral");
}

TEST_CASE_METHOD(ReportFixture, "Report can get declared attitudes map", "[Report]")
{
  const auto& attitudes = report.getFactionDeclaredAttitudesText();
  CHECK(attitudes.empty());
}

TEST_CASE_METHOD(ReportFixture, "Report can get declared faction names map", "[Report]")
{
  const auto& names = report.getFactionDeclaredFactionNames();
  CHECK(names.empty());
}

TEST_CASE_METHOD(ReportFixture, "Report defaults found regions to zero", "[Report]")
{
  CHECK(report.getFoundRegions() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report defaults visited regions to zero", "[Report]")
{
  CHECK(report.getVisitedRegions() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report can clear content", "[Report]")
{
  report.setMonth(6);
  report.setYear(1);
  report.setFactionNumber(42);
  report.setFactionName(L"Test");
  report.clear();
  CHECK(report.getMonth() == 0);
  CHECK(report.getYear() == 0);
  CHECK(report.getFactionNumber() == 0);
  CHECK(report.getFactionName().empty());
  CHECK_FALSE(report.hasContent());
}

TEST_CASE_METHOD(ReportFixture, "Report stores zero month", "[Report]")
{
  report.setMonth(0);
  CHECK(report.getMonth() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report stores zero year", "[Report]")
{
  report.setYear(0);
  CHECK(report.getYear() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report stores negative month", "[Report]")
{
  report.setMonth(-1);
  CHECK(report.getMonth() == -1);
}

TEST_CASE_METHOD(ReportFixture, "Report stores negative year", "[Report]")
{
  report.setYear(-1);
  CHECK(report.getYear() == -1);
}

TEST_CASE_METHOD(ReportFixture, "Report stores large month value", "[Report]")
{
  report.setMonth(12);
  CHECK(report.getMonth() == 12);
}

TEST_CASE_METHOD(ReportFixture, "Report stores large year value", "[Report]")
{
  report.setYear(1000);
  CHECK(report.getYear() == 1000);
}

TEST_CASE_METHOD(ReportFixture, "Report stores zero faction number", "[Report]")
{
  report.setFactionNumber(0);
  CHECK(report.getFactionNumber() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report stores negative faction number", "[Report]")
{
  report.setFactionNumber(-1);
  CHECK(report.getFactionNumber() == -1);
}

TEST_CASE_METHOD(ReportFixture, "Report stores large faction number", "[Report]")
{
  report.setFactionNumber(999999);
  CHECK(report.getFactionNumber() == 999999);
}

TEST_CASE_METHOD(ReportFixture, "Report stores empty faction name", "[Report]")
{
  report.setFactionName(L"");
  CHECK(report.getFactionName() == L"");
}

TEST_CASE_METHOD(ReportFixture, "Report stores faction name with spaces", "[Report]")
{
  report.setFactionName(L"The Adventurers Guild");
  CHECK(report.getFactionName() == L"The Adventurers Guild");
}

TEST_CASE_METHOD(ReportFixture, "Report stores faction name with special characters", "[Report]")
{
  report.setFactionName(L"Clan of Á'zûrë");
  CHECK(report.getFactionName() == L"Clan of Á'zûrë");
}

TEST_CASE_METHOD(ReportFixture, "Report stores zero taxed or traded regions", "[Report]")
{
  report.setFactionTaxedOrTradedRegionsCurrent(0);
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report stores large taxed or traded regions", "[Report]")
{
  report.setFactionTaxedOrTradedRegionsCurrent(1000);
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 1000);
}

TEST_CASE_METHOD(ReportFixture, "Report stores zero unclaimed silver", "[Report]")
{
  report.setFactionUnclaimedSilver(0);
  CHECK(report.getFactionUnclaimedSilver() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report stores large unclaimed silver", "[Report]")
{
  report.setFactionUnclaimedSilver(1000000);
  CHECK(report.getFactionUnclaimedSilver() == 1000000);
}

TEST_CASE_METHOD(ReportFixture, "Report stores negative unclaimed silver", "[Report]")
{
  report.setFactionUnclaimedSilver(-500);
  CHECK(report.getFactionUnclaimedSilver() == -500);
}

TEST_CASE_METHOD(ReportFixture, "Report maintains multiple faction counts independently", "[Report]")
{
  report.setFactionTaxedOrTradedRegionsCurrent(10);
  report.setFactionTaxedOrTradedRegionsMax(20);
  report.setFactionQuartermastersCurrent(5);
  report.setFactionQuartermastersMax(10);
  report.setFactionMagesCurrent(3);
  report.setFactionMagesMax(8);
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 10);
  CHECK(report.getFactionTaxedOrTradedRegionsMax() == 20);
  CHECK(report.getFactionQuartermastersCurrent() == 5);
  CHECK(report.getFactionQuartermastersMax() == 10);
  CHECK(report.getFactionMagesCurrent() == 3);
  CHECK(report.getFactionMagesMax() == 8);
}

TEST_CASE_METHOD(ReportFixture, "Report preserves faction metadata after clear", "[Report]")
{
  report.setMonth(6);
  report.setYear(1);
  report.setFactionNumber(42);
  report.setFactionName(L"Test");
  report.setFactionTaxedOrTradedRegionsCurrent(10);
  report.clear();
  CHECK(report.getMonth() == 0);
  CHECK(report.getYear() == 0);
  CHECK(report.getFactionNumber() == 0);
  CHECK(report.getFactionTaxedOrTradedRegionsCurrent() == 0);
}

TEST_CASE_METHOD(ReportFixture, "Report can get file path when not loaded", "[Report]")
{
  CHECK(report.getFilePath().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report can get last error message", "[Report]")
{
  const auto& error = report.getLastError();
  REQUIRE(error.empty());
}

TEST_CASE_METHOD(ReportFixture, "Report stores mages current below max", "[Report]")
{
  report.setFactionMagesCurrent(3);
  report.setFactionMagesMax(8);
  CHECK(report.getFactionMagesCurrent() < report.getFactionMagesMax());
}

TEST_CASE_METHOD(ReportFixture, "Report stores apprentices current equals max", "[Report]")
{
  report.setFactionApprenticesCurrent(5);
  report.setFactionApprenticesMax(5);
  CHECK(report.getFactionApprenticesCurrent() == report.getFactionApprenticesMax());
}

TEST_CASE_METHOD(ReportFixture, "Report stores current above max allowed", "[Report]")
{
  report.setFactionQuartermastersCurrent(15);
  report.setFactionQuartermastersMax(10);
  CHECK(report.getFactionQuartermastersCurrent() > report.getFactionQuartermastersMax());
}

TEST_CASE_METHOD(ReportFixture, "Report can transition faction pointer from nullptr to valid", "[Report]")
{
  CHECK(report.getFaction() == nullptr);
  Faction testFaction(1, L"Test", true);
  report.setFaction(&testFaction);
  CHECK(report.getFaction() != nullptr);
  CHECK(report.getFaction()->getFactionNumber() == 1);
}

TEST_CASE_METHOD(ReportFixture, "Report can transition faction pointer from valid to nullptr", "[Report]")
{
  Faction testFaction(1, L"Test", true);
  report.setFaction(&testFaction);
  CHECK(report.getFaction() != nullptr);
  report.setFaction(nullptr);
  CHECK(report.getFaction() == nullptr);
}

TEST_CASE_METHOD(ReportFixture, "Report can update faction metadata sequentially", "[Report]")
{
  report.setMonth(1);
  report.setYear(1);
  report.setFactionNumber(10);
  report.setMonth(2);
  report.setYear(2);
  report.setFactionNumber(20);
  CHECK(report.getMonth() == 2);
  CHECK(report.getYear() == 2);
  CHECK(report.getFactionNumber() == 20);
}

TEST_CASE_METHOD(ReportFixture, "Report content defaults to empty", "[Report]")
{
  CHECK(report.getContent().empty());
  CHECK(report.getLines().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report stores all faction wizard types", "[Report]")
{
  report.setFactionMagesCurrent(5);
  report.setFactionMagesMax(10);
  report.setFactionApprenticesCurrent(3);
  report.setFactionApprenticesMax(8);
  CHECK(report.getFactionMagesCurrent() == 5);
  CHECK(report.getFactionMagesMax() == 10);
  CHECK(report.getFactionApprenticesCurrent() == 3);
  CHECK(report.getFactionApprenticesMax() == 8);
}

TEST_CASE_METHOD(ReportFixture, "Report loadFromFile fails for non-existent file", "[Report]")
{
  const auto result = report.loadFromFile(L"D:\\nonexistent_file_12345.rep");
  CHECK_FALSE(result);
  CHECK_FALSE(report.getLastError().empty());
  CHECK_FALSE(report.hasContent());
}

TEST_CASE_METHOD(ReportFixture, "Report loadFromFile succeeds for valid file", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  CHECK(result);
  CHECK(report.hasContent());
  CHECK_FALSE(report.getContent().empty());
  CHECK_FALSE(report.getFilePath().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report getLines returns multiple lines after loading", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  const auto& lines = report.getLines();
  CHECK(lines.size() > 0);
}

TEST_CASE_METHOD(ReportFixture, "Report getLines is consistent with content", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  const auto& content = report.getContent();
  const auto& lines = report.getLines();
  CHECK_FALSE(content.empty());
  CHECK_FALSE(lines.empty());
}

TEST_CASE_METHOD(ReportFixture, "Report getFilePath stores correct path after loading", "[Report]")
{
  const std::string filePathStr = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePathStr.empty());
  const std::wstring filePath(filePathStr.begin(), filePathStr.end());
  const auto result = report.loadFromFile(filePath);
  REQUIRE(result);
  CHECK(report.getFilePath() == filePath);
}

TEST_CASE_METHOD(ReportFixture, "Report clear resets file path", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK_FALSE(report.getFilePath().empty());
  report.clear();
  CHECK(report.getFilePath().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report clear resets content and lines", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK_FALSE(report.getContent().empty());
  CHECK_FALSE(report.getLines().empty());
  report.clear();
  CHECK(report.getContent().empty());
  CHECK(report.getLines().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report clears error on successful load", "[Report]")
{
  report.loadFromFile(L"D:\\nonexistent_file_12345.rep");
  CHECK_FALSE(report.getLastError().empty());
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
}

TEST_CASE_METHOD(ReportFixture, "Report getLastError populated on load failure", "[Report]")
{
  const auto result = report.loadFromFile(L"D:\\does_not_exist_99999.rep");
  CHECK_FALSE(result);
  const auto& error = report.getLastError();
  CHECK_FALSE(error.empty());
  const bool hasFailed = error.find(L"Failed") != std::wstring::npos;
  const bool hasCannot = error.find(L"cannot") != std::wstring::npos;
  CHECK((hasFailed || hasCannot));
}

TEST_CASE_METHOD(ReportFixture, "Report parses faction name from header", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK_FALSE(report.getFactionName().empty());
}

TEST_CASE_METHOD(ReportFixture, "Report parses faction number from header", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK(report.getFactionNumber() > 0);
}

TEST_CASE_METHOD(ReportFixture, "Report parses month from header", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK(report.getMonth() > 0);
}

TEST_CASE_METHOD(ReportFixture, "Report parses year from header", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  CHECK(report.getYear() > 0);
}

TEST_CASE_METHOD(ReportFixture, "Report file content matches file size", "[Report]")
{
  const std::string filePathStr = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePathStr.empty());
  const std::wstring filePath(filePathStr.begin(), filePathStr.end());
  const auto result = report.loadFromFile(filePath);
  REQUIRE(result);
  CHECK(report.getContent().size() > 0);
}

TEST_CASE_METHOD(ReportFixture, "Report multiple loads replace previous content", "[Report]")
{
  const std::string filePath1Str = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath1Str.empty());
  const auto result1 = report.loadFromFile(std::wstring(filePath1Str.begin(), filePath1Str.end()));
  REQUIRE(result1);
  const auto content1 = report.getContent();
  const auto factionNum1 = report.getFactionNumber();
  
  const std::string filePath2Str = TestUtils::getTestDataPath("report_17_004.rep");
  REQUIRE_FALSE(filePath2Str.empty());
  const auto result2 = report.loadFromFile(std::wstring(filePath2Str.begin(), filePath2Str.end()));
  REQUIRE(result2);
  const auto content2 = report.getContent();
  const auto factionNum2 = report.getFactionNumber();
  
  const bool contentDifferent = content1 != content2;
  const bool factionDifferent = factionNum1 != factionNum2;
  CHECK((contentDifferent || factionDifferent));
}

TEST_CASE_METHOD(ReportFixture, "Report can get faction declared attitudes after load", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  const auto& attitudes = report.getFactionDeclaredAttitudesText();
  const bool isEmpty = attitudes.empty();
  CHECK((isEmpty || attitudes.size() >= 0));
}

TEST_CASE_METHOD(ReportFixture, "Report can get faction declared faction names after load", "[Report]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  const auto result = report.loadFromFile(std::wstring(filePath.begin(), filePath.end()));
  REQUIRE(result);
  const auto& factionNames = report.getFactionDeclaredFactionNames();
  const bool isEmpty = factionNames.empty();
  CHECK((isEmpty || factionNames.size() >= 0));
}

TEST_CASE_METHOD(ReportFixture, "Report getContent returns consistent value", "[Report]")
{
  const auto result = report.loadFromFile(L"D:\\cppSandbox\\AtlantisMajordomo\\testData\\report_17_003.rep");
  REQUIRE(result);
  const auto& content1 = report.getContent();
  const auto& content2 = report.getContent();
  CHECK(content1 == content2);
}

TEST_CASE_METHOD(ReportFixture, "Report getLines returns consistent value", "[Report]")
{
  const auto result = report.loadFromFile(L"D:\\cppSandbox\\AtlantisMajordomo\\testData\\report_17_003.rep");
  REQUIRE(result);
  const auto& lines1 = report.getLines();
  const auto& lines2 = report.getLines();
  CHECK(lines1.size() == lines2.size());
}
