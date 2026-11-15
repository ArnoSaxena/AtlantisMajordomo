/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/RegionRepository.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
struct RegionRepositoryFixture
{
  RegionRepository repository;
};
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository starts empty", "[RegionRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can add a region with z-coordinate", "[RegionRepository]")
{
  const bool result = repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can add a region without z-coordinate", "[RegionRepository]")
{
  const bool result = repository.add(1, 2, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository prevents duplicate regions by x,y,z", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const bool result = repository.add(1, 2, 3, L"forest", L"Other", false, L"", L"", L"orc", 500, 2.0, 150, 600, false);
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository allows different z-coordinates at same x,y", "[RegionRepository]")
{
  repository.add(1, 2, 1, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const bool result = repository.add(1, 2, 2, L"underground", L"Caverns", false, L"", L"", L"dwarf", 800, 1.8, 120, 550, false);
  CHECK(result);
  CHECK(repository.size() == 2);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can store multiple regions", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  repository.add(2, 2, 3, L"mountain", L"Ironpeak", false, L"", L"", L"dwarf", 600, 2.0, 150, 700, false);
  CHECK(repository.size() == 3);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can find region by x,y,z", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"Aethermoor");
  CHECK(found->getRegionType() == L"plain");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository returns nullptr for non-existent x,y,z", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const Region* found = repository.findByCoordinates(5, 6, 7);
  CHECK(found == nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can find region by x,y without z", "[RegionRepository]")
{
  repository.add(1, 2, 1, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const Region* found = repository.findByCoordinates(1, 2);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"Aethermoor");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository returns nullptr for non-existent x,y", "[RegionRepository]")
{
  repository.add(1, 2, 1, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const Region* found = repository.findByCoordinates(5, 6);
  CHECK(found == nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can remove region by x,y,z", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  const bool result = repository.removeByCoordinates(1, 2, 3);
  CHECK(result);
  CHECK(repository.size() == 1);
  CHECK(repository.findByCoordinates(1, 2, 3) == nullptr);
  CHECK(repository.findByCoordinates(1, 3, 3) != nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository returns false when removing non-existent x,y,z", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const bool result = repository.removeByCoordinates(5, 6, 7);
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can remove region by x,y", "[RegionRepository]")
{
  repository.add(1, 2, 1, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 1, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  const bool result = repository.removeByCoordinates(1, 2);
  CHECK(result);
  CHECK(repository.size() == 1);
  CHECK(repository.findByCoordinates(1, 2) == nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can clear all regions", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  repository.add(2, 2, 3, L"mountain", L"Ironpeak", false, L"", L"", L"dwarf", 600, 2.0, 150, 700, false);
  repository.clear();
  CHECK(repository.size() == 0);
  CHECK(repository.findByCoordinates(1, 2, 3) == nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can access region by index", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  const Region& first = repository.at(0);
  const Region& second = repository.at(1);
  CHECK(first.getProvinceName() == L"Aethermoor");
  CHECK(second.getProvinceName() == L"Silvara");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository upsert inserts new region", "[RegionRepository]")
{
  const bool result = repository.upsertRegion(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  CHECK(result);
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository upsert updates existing with newer month", "[RegionRepository]")
{
  repository.upsertRegion(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  const bool result = repository.upsertRegion(1, 2, 3, L"forest", L"UpdatedName", false, L"", L"", L"orc", 500, 2.0, 150, 600, 7, 1, false);
  CHECK(result);
  CHECK(repository.size() == 1);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"UpdatedName");
  CHECK(found->getMonth() == 7);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository upsert rejects older month", "[RegionRepository]")
{
  repository.upsertRegion(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 7, 1, false);
  const bool result = repository.upsertRegion(1, 2, 3, L"forest", L"UpdatedName", false, L"", L"", L"orc", 500, 2.0, 150, 600, 6, 1, false);
  CHECK_FALSE(result);
  CHECK(repository.size() == 1);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"Aethermoor");
  CHECK(found->getMonth() == 7);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository upsert updates existing with same month", "[RegionRepository]")
{
  repository.upsertRegion(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  const bool result = repository.upsertRegion(1, 2, 3, L"forest", L"UpdatedName", false, L"", L"", L"orc", 500, 2.0, 150, 600, 6, 1, false);
  CHECK(result);
  CHECK(repository.size() == 1);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"UpdatedName");
  CHECK(found->getMonth() == 6);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores regions with settlements", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"ocean", L"Seahaven", true, L"Port Royal", L"port", L"mixed", 800, 2.0, 150, 1000, false);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getContainsSettlement());
  CHECK(found->getSettlementName() == L"Port Royal");
  CHECK(found->getSettlementType() == L"port");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores regions with zero peasants", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"ocean", L"Sea", false, L"", L"", L"none", 0, 0.0, 0, 0, false);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getPeasantNumber() == 0);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores negative coordinates", "[RegionRepository]")
{
  repository.add(-5, -10, -3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const Region* found = repository.findByCoordinates(-5, -10, -3);
  REQUIRE(found != nullptr);
  CHECK(found->getXCoordinate() == -5);
  CHECK(found->getYCoordinate() == -10);
  CHECK(found->getZCoordinate() == -3);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores large coordinate values", "[RegionRepository]")
{
  repository.add(1000, 2000, 3000, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  const Region* found = repository.findByCoordinates(1000, 2000, 3000);
  REQUIRE(found != nullptr);
  CHECK(found->getXCoordinate() == 1000);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository distinguishes between different coordinates", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 2, 4, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  repository.add(1, 3, 3, L"mountain", L"Ironpeak", false, L"", L"", L"dwarf", 600, 2.0, 150, 700, false);
  repository.add(2, 2, 3, L"ocean", L"Sea", false, L"", L"", L"none", 0, 0.0, 0, 0, false);
  CHECK(repository.size() == 4);
  CHECK(repository.findByCoordinates(1, 2, 3)->getProvinceName() == L"Aethermoor");
  CHECK(repository.findByCoordinates(1, 2, 4)->getProvinceName() == L"Silvara");
  CHECK(repository.findByCoordinates(1, 3, 3)->getProvinceName() == L"Ironpeak");
  CHECK(repository.findByCoordinates(2, 2, 3)->getProvinceName() == L"Sea");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository preserves unrelated regions when removing", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 3, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  repository.removeByCoordinates(1, 2, 3);
  CHECK(repository.size() == 1);
  CHECK(repository.findByCoordinates(1, 3, 3) != nullptr);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can clear and re-add regions", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.clear();
  CHECK(repository.size() == 0);
  repository.add(1, 2, 3, L"forest", L"Silvara", false, L"", L"", L"elf", 800, 1.8, 120, 600, false);
  CHECK(repository.size() == 1);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getProvinceName() == L"Silvara");
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores and finds regions with resources", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  std::map<std::wstring, int> resources;
  resources[L"iron"] = 100;
  resources[L"wood"] = 50;
  found->setResources(resources);
  const Region* refound = repository.findByCoordinates(1, 2, 3);
  REQUIRE(refound != nullptr);
  CHECK(refound->getResources().at(L"iron") == 100);
  CHECK(refound->getResources().at(L"wood") == 50);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores and finds regions with wanted items", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  found->setWantedItem(L"sword", 50, 100);
  const Region* refound = repository.findByCoordinates(1, 2, 3);
  REQUIRE(refound != nullptr);
  CHECK(refound->getWanted().at(L"sword").first == 50);
  CHECK(refound->getWanted().at(L"sword").second == 100);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository stores and finds regions with for-sale items", "[RegionRepository]")
{
  repository.add(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  found->setForSaleItem(L"fish", 200, 5);
  const Region* refound = repository.findByCoordinates(1, 2, 3);
  REQUIRE(refound != nullptr);
  CHECK(refound->getForSale().at(L"fish").first == 200);
  CHECK(refound->getForSale().at(L"fish").second == 5);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository upsert with different year", "[RegionRepository]")
{
  repository.upsertRegion(1, 2, 3, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, 6, 1, false);
  const bool result = repository.upsertRegion(1, 2, 3, L"forest", L"UpdatedName", false, L"", L"", L"orc", 500, 2.0, 150, 600, 7, 2, false);
  CHECK(result);
  const Region* found = repository.findByCoordinates(1, 2, 3);
  REQUIRE(found != nullptr);
  CHECK(found->getYear() == 2);
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository can find non-existent by x,y when z alternatives exist", "[RegionRepository]")
{
  repository.add(1, 2, 1, L"plain", L"Aethermoor", false, L"", L"", L"human", 1000, 1.5, 100, 500, false);
  repository.add(1, 2, 2, L"underground", L"Caverns", false, L"", L"", L"dwarf", 800, 1.8, 120, 550, false);
  const Region* found = repository.findByCoordinates(1, 2);
  REQUIRE(found != nullptr);
  const int z = found->getZCoordinate();
  CHECK((z == 1 || z == 2));
}

TEST_CASE_METHOD(RegionRepositoryFixture, "RegionRepository has empty error message when initialized", "[RegionRepository]")
{
  CHECK(repository.getLastError().empty());
}
