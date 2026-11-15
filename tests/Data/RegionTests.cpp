/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Region.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace
{
struct RegionFixture
{
  Region region { 1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, true };
};
}

TEST_CASE_METHOD(RegionFixture, "Region stores the x coordinate", "[Region]")
{
  CHECK(region.getXCoordinate() == 1);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the y coordinate", "[Region]")
{
  CHECK(region.getYCoordinate() == 2);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the z coordinate", "[Region]")
{
  CHECK(region.getZCoordinate() == 3);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the region type", "[Region]")
{
  CHECK(region.getRegionType() == L"plain");
}

TEST_CASE_METHOD(RegionFixture, "Region stores the province name", "[Region]")
{
  CHECK(region.getProvinceName() == L"Aethermoor");
}

TEST_CASE_METHOD(RegionFixture, "Region stores the contains settlement flag", "[Region]")
{
  CHECK_FALSE(region.getContainsSettlement());
}

TEST_CASE_METHOD(RegionFixture, "Region stores the settlement name", "[Region]")
{
  CHECK(region.getSettlementName() == L"");
}

TEST_CASE_METHOD(RegionFixture, "Region stores the settlement type", "[Region]")
{
  CHECK(region.getSettlementType() == L"");
}

TEST_CASE_METHOD(RegionFixture, "Region stores the peasant type", "[Region]")
{
  CHECK(region.getPeasantType() == L"human");
}

TEST_CASE_METHOD(RegionFixture, "Region stores the peasant number", "[Region]")
{
  CHECK(region.getPeasantNumber() == 1000);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the wages", "[Region]")
{
  CHECK(region.getWages() == 1.5);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the wages max", "[Region]")
{
  CHECK(region.getWagesMax() == 100);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the taxable income", "[Region]")
{
  CHECK(region.getTaxableIncome() == 500);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the month", "[Region]")
{
  CHECK(region.getMonth() == 6);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the year", "[Region]")
{
  CHECK(region.getYear() == 1);
}

TEST_CASE_METHOD(RegionFixture, "Region stores the visited flag", "[Region]")
{
  CHECK(region.getVisited());
}

TEST_CASE("Region construction with settlement creates settlement data", "[Region]")
{
  Region region(1, 2, 3, L"ocean", L"Seahaven", true, L"Port Royal", L"port", L"elf", 500, 2.0, 150, 1000, 6, 1, true);
  CHECK(region.getContainsSettlement());
  CHECK(region.getSettlementName() == L"Port Royal");
  CHECK(region.getSettlementType() == L"port");
}

TEST_CASE("Region can change coordinates", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setZCoordinate(5);
  CHECK(region.getZCoordinate() == 5);
}

TEST_CASE("Region can change month and year", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setMonth(7);
  region.setYear(2);
  CHECK(region.getMonth() == 7);
  CHECK(region.getYear() == 2);
}

TEST_CASE("Region can change region type", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setRegionType(L"forest");
  CHECK(region.getRegionType() == L"forest");
}

TEST_CASE("Region can change province name", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setProvinceName(L"Silvara");
  CHECK(region.getProvinceName() == L"Silvara");
}

TEST_CASE("Region can enable and set settlement", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setContainsSettlement(true);
  region.setSettlementName(L"Millhaven");
  region.setSettlementType(L"town");
  CHECK(region.getContainsSettlement());
  CHECK(region.getSettlementName() == L"Millhaven");
  CHECK(region.getSettlementType() == L"town");
}

TEST_CASE("Region can change peasant data", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setPeasantType(L"orc");
  region.setPeasantNumber(2000);
  CHECK(region.getPeasantType() == L"orc");
  CHECK(region.getPeasantNumber() == 2000);
}

TEST_CASE("Region can change wage data", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWages(2.5);
  region.setWagesMax(200);
  CHECK(region.getWages() == 2.5);
  CHECK(region.getWagesMax() == 200);
}

TEST_CASE("Region can change taxable income", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setTaxableIncome(750);
  CHECK(region.getTaxableIncome() == 750);
}

TEST_CASE("Region can toggle visited flag", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK_FALSE(region.getVisited());
  region.setVisited(true);
  CHECK(region.getVisited());
}

TEST_CASE("Region can set entertainment", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setEntertainment(300);
  CHECK(region.getEntertainment() == 300);
}

TEST_CASE("Region can add exit directions", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.addExitDirection(L"north");
  region.addExitDirection(L"south");
  region.addExitDirection(L"east");
  const auto& directions = region.getExitDirections();
  REQUIRE(directions.size() == 3);
  CHECK(directions[0] == L"north");
  CHECK(directions[1] == L"south");
  CHECK(directions[2] == L"east");
}

TEST_CASE("Region can clear exit directions", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.addExitDirection(L"north");
  region.addExitDirection(L"south");
  region.clearExitDirections();
  CHECK(region.getExitDirections().empty());
}

TEST_CASE("Region can set resources", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, int> resources;
  resources[L"iron"] = 100;
  resources[L"wood"] = 50;
  region.setResources(resources);
  const auto& retrieved = region.getResources();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"iron") == 100);
  CHECK(retrieved.at(L"wood") == 50);
}

TEST_CASE("Region can set individual resource", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setResource(L"silver", 250);
  CHECK(region.getResources().at(L"silver") == 250);
}

TEST_CASE("Region can set wanted items", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, std::pair<int, int>> wanted;
  wanted[L"sword"] = {50, 100};
  wanted[L"armor"] = {30, 150};
  region.setWanted(wanted);
  const auto& retrieved = region.getWanted();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"sword").first == 50);
  CHECK(retrieved.at(L"sword").second == 100);
  CHECK(retrieved.at(L"armor").first == 30);
  CHECK(retrieved.at(L"armor").second == 150);
}

TEST_CASE("Region can set individual wanted item", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWantedItem(L"horse", 40, 200);
  const auto& wanted = region.getWanted();
  REQUIRE(wanted.find(L"horse") != wanted.end());
  CHECK(wanted.at(L"horse").first == 40);
  CHECK(wanted.at(L"horse").second == 200);
}

TEST_CASE("Region can set for-sale items", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, std::pair<int, int>> forSale;
  forSale[L"fish"] = {200, 5};
  forSale[L"grain"] = {300, 3};
  region.setForSale(forSale);
  const auto& retrieved = region.getForSale();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"fish").first == 200);
  CHECK(retrieved.at(L"fish").second == 5);
  CHECK(retrieved.at(L"grain").first == 300);
  CHECK(retrieved.at(L"grain").second == 3);
}

TEST_CASE("Region can set individual for-sale item", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setForSaleItem(L"silver", 100, 10);
  const auto& forSale = region.getForSale();
  REQUIRE(forSale.find(L"silver") != forSale.end());
  CHECK(forSale.at(L"silver").first == 100);
  CHECK(forSale.at(L"silver").second == 10);
}

TEST_CASE("Region can set region report", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setRegionReport(L"This is a detailed region report.");
  CHECK(region.getRegionReport() == L"This is a detailed region report.");
}

TEST_CASE("Region without z-coordinate defaulting to 1", "[Region]")
{
  Region region(1, 2, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getZCoordinate() == 1);
}

TEST_CASE("Region stores zero coordinates", "[Region]")
{
  Region region(0, 0, 0, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getXCoordinate() == 0);
  CHECK(region.getYCoordinate() == 0);
  CHECK(region.getZCoordinate() == 0);
}

TEST_CASE("Region stores negative coordinates", "[Region]")
{
  Region region(-5, -10, -3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getXCoordinate() == -5);
  CHECK(region.getYCoordinate() == -10);
  CHECK(region.getZCoordinate() == -3);
}

TEST_CASE("Region stores large coordinate values", "[Region]")
{
  Region region(1000, 2000, 3000, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getXCoordinate() == 1000);
  CHECK(region.getYCoordinate() == 2000);
  CHECK(region.getZCoordinate() == 3000);
}

TEST_CASE("Region stores empty province name", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getProvinceName() == L"");
}

TEST_CASE("Region stores zero peasants", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 0, 1.5, 100, 500, 6, 1, false);
  CHECK(region.getPeasantNumber() == 0);
}

TEST_CASE("Region stores zero wages", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 0.0, 0, 0, 6, 1, false);
  CHECK(region.getWages() == 0.0);
  CHECK(region.getWagesMax() == 0);
}

TEST_CASE("Region identifies ocean terrain", "[Region]")
{
  Region ocean(1, 2, 3, L"ocean", L"Sea", false, L"", L"", L"none", 0, 0.0, 0, 0, 6, 1, false);
  Region plain(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(ocean.isOcean());
  CHECK_FALSE(plain.isOcean());
}

TEST_CASE("Region can set and get wages after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWagesAfterOrders(90);
  CHECK(region.getWagesAfterOrders() == 90);
}

TEST_CASE("Region stores zero wages after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWagesAfterOrders(0);
  CHECK(region.getWagesAfterOrders() == 0);
}

TEST_CASE("Region can set and get taxable income after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setTaxableIncomeAfterOrders(450);
  CHECK(region.getTaxableIncomeAfterOrders() == 450);
}

TEST_CASE("Region stores zero taxable income after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setTaxableIncomeAfterOrders(0);
  CHECK(region.getTaxableIncomeAfterOrders() == 0);
}

TEST_CASE("Region can set and get entertainment after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setEntertainmentAfterOrders(75);
  CHECK(region.getEntertainmentAfterOrders() == 75);
}

TEST_CASE("Region stores zero entertainment after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setEntertainmentAfterOrders(0);
  CHECK(region.getEntertainmentAfterOrders() == 0);
}

TEST_CASE("Region can set and get resources after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, int> resourcesAfter;
  resourcesAfter[L"silver"] = 150;
  resourcesAfter[L"iron"] = 30;
  region.setResourcesAfterOrders(resourcesAfter);
  const auto& retrieved = region.getResourcesAfterOrders();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"silver") == 150);
  CHECK(retrieved.at(L"iron") == 30);
}

TEST_CASE("Region can set single resource after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setResourceAfterOrders(L"gold", 50);
  const auto& resourcesAfter = region.getResourcesAfterOrders();
  REQUIRE(resourcesAfter.find(L"gold") != resourcesAfter.end());
  CHECK(resourcesAfter.at(L"gold") == 50);
}

TEST_CASE("Region can set and get wanted after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, std::pair<int, int>> wantedAfter;
  wantedAfter[L"grain"] = {100, 5};
  wantedAfter[L"meat"] = {50, 8};
  region.setWantedAfterOrders(wantedAfter);
  const auto& retrieved = region.getWantedAfterOrders();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"grain").first == 100);
  CHECK(retrieved.at(L"grain").second == 5);
  CHECK(retrieved.at(L"meat").first == 50);
  CHECK(retrieved.at(L"meat").second == 8);
}

TEST_CASE("Region can set single wanted item after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWantedItemAfterOrders(L"wheat", 200, 3);
  const auto& wantedAfter = region.getWantedAfterOrders();
  REQUIRE(wantedAfter.find(L"wheat") != wantedAfter.end());
  CHECK(wantedAfter.at(L"wheat").first == 200);
  CHECK(wantedAfter.at(L"wheat").second == 3);
}

TEST_CASE("Region can set and get for-sale after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  std::map<std::wstring, std::pair<int, int>> forSaleAfter;
  forSaleAfter[L"fish"] = {100, 10};
  forSaleAfter[L"stone"] = {250, 2};
  region.setForSaleAfterOrders(forSaleAfter);
  const auto& retrieved = region.getForSaleAfterOrders();
  REQUIRE(retrieved.size() == 2);
  CHECK(retrieved.at(L"fish").first == 100);
  CHECK(retrieved.at(L"fish").second == 10);
  CHECK(retrieved.at(L"stone").first == 250);
  CHECK(retrieved.at(L"stone").second == 2);
}

TEST_CASE("Region can set single for-sale item after orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setForSaleItemAfterOrders(L"wood", 150, 4);
  const auto& forSaleAfter = region.getForSaleAfterOrders();
  REQUIRE(forSaleAfter.find(L"wood") != forSaleAfter.end());
  CHECK(forSaleAfter.at(L"wood").first == 150);
  CHECK(forSaleAfter.at(L"wood").second == 4);
}

TEST_CASE("Region after-orders values are independent from before-orders", "[Region]")
{
  Region region(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  region.setWagesMax(100);
  region.setWagesAfterOrders(90);
  region.setTaxableIncome(500);
  region.setTaxableIncomeAfterOrders(450);
  region.setEntertainment(200);
  region.setEntertainmentAfterOrders(150);
  CHECK(region.getWagesMax() != region.getWagesAfterOrders());
  CHECK(region.getTaxableIncome() != region.getTaxableIncomeAfterOrders());
  CHECK(region.getEntertainment() != region.getEntertainmentAfterOrders());
}
