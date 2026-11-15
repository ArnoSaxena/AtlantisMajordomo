/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/OrderRepository.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
struct OrderRepositoryFixture
{
  OrderRepository repository;
};
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores a single order for a unit", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  const auto* orders = repository.getOrdersForUnit(42, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->size() == 1);
  CHECK(orders->at(0).getIssuingUnitNumber() == 42);
  CHECK(orders->at(0).getFullOrderText() == L"GIVE 50 100 silver");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository returns nullptr for non-existent unit", "[OrderRepository]")
{
  const auto* orders = repository.getOrdersForUnit(42, false);
  CHECK(orders == nullptr);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository filters orders by from-new-unit flag", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, true, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  const auto regularRefs = repository.getOrderReferencesForUnit(42, false);
  const auto newUnitRefs = repository.getOrderReferencesForUnit(42, true);
  REQUIRE(regularRefs.size() == 1);
  REQUIRE(newUnitRefs.size() == 1);
  CHECK(regularRefs.at(0).get().getFullOrderText() == L"GIVE 50 100 silver");
  CHECK(newUnitRefs.at(0).get().getFullOrderText() == L"STUDY MELEE");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores multiple orders for one unit with same flag", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, false, 2, 3, 4, L"TAKE", L"TAKE FROM 51 50 gold");
  const auto* orders = repository.getOrdersForUnit(42, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->size() == 2);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores orders for different units independently", "[OrderRepository]")
{
  repository.addOrder(1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, false, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  repository.addOrder(3, false, 1, 2, 3, L"TEACH", L"TEACH 2");
  const auto refs1 = repository.getOrderReferencesForUnit(1, false);
  const auto refs2 = repository.getOrderReferencesForUnit(2, false);
  const auto refs3 = repository.getOrderReferencesForUnit(3, false);
  REQUIRE(refs1.size() == 1);
  REQUIRE(refs2.size() == 1);
  REQUIRE(refs3.size() == 1);
  CHECK(refs1.at(0).get().getOrderToken() == L"GIVE");
  CHECK(refs2.at(0).get().getOrderToken() == L"STUDY");
  CHECK(refs3.at(0).get().getOrderToken() == L"TEACH");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores orders with coordinates in region", "[OrderRepository]")
{
  repository.addOrder(1, false, 5, 6, 7, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, false, 5, 6, 7, L"STUDY", L"STUDY MELEE");
  repository.addOrder(3, false, 8, 9, 10, L"TEACH", L"TEACH 2");
  const auto& regionOrders = repository.getOrdersForRegion(5, 6, 7);
  CHECK(regionOrders.size() == 2);
  CHECK(regionOrders.at(0).getIssuingUnitNumber() == 1);
  CHECK(regionOrders.at(1).getIssuingUnitNumber() == 2);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository returns empty vector for region with no orders", "[OrderRepository]")
{
  repository.addOrder(1, false, 5, 6, 7, L"GIVE", L"GIVE 50 100 silver");
  const auto& regionOrders = repository.getOrdersForRegion(10, 11, 12);
  CHECK(regionOrders.empty());
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository includes orders from both new and regular units in region", "[OrderRepository]")
{
  repository.addOrder(1, false, 5, 6, 7, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, true, 5, 6, 7, L"STUDY", L"STUDY MELEE");
  const auto& regionOrders = repository.getOrdersForRegion(5, 6, 7);
  CHECK(regionOrders.size() == 2);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository getOrderReferencesForUnit returns no references for non-existent unit", "[OrderRepository]")
{
  const auto refs = repository.getOrderReferencesForUnit(42, false);
  CHECK(refs.empty());
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository getOrderReferencesForUnit filters by from-new-unit flag", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, true, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  const auto regularRefs = repository.getOrderReferencesForUnit(42, false);
  const auto newUnitRefs = repository.getOrderReferencesForUnit(42, true);
  CHECK(regularRefs.size() == 1);
  CHECK(newUnitRefs.size() == 1);
  CHECK(regularRefs[0].get().getOrderToken() == L"GIVE");
  CHECK(newUnitRefs[0].get().getOrderToken() == L"STUDY");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository getOrderReferencesForUnit returns references for multiple orders", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, false, 2, 3, 4, L"TAKE", L"TAKE FROM 51 50 gold");
  repository.addOrder(42, false, 3, 4, 5, L"TEACH", L"TEACH 100");
  const auto refs = repository.getOrderReferencesForUnit(42, false);
  CHECK(refs.size() == 3);
  CHECK(refs[0].get().getOrderToken() == L"GIVE");
  CHECK(refs[1].get().getOrderToken() == L"TAKE");
  CHECK(refs[2].get().getOrderToken() == L"TEACH");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository removeOrdersForUnit removes all matching orders", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, true, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  const bool removed = repository.removeOrdersForUnit(42, false);
  CHECK(removed);
  const auto* remainingOrders = repository.getOrdersForUnit(42, false);
  CHECK(remainingOrders == nullptr);
  const auto* newUnitOrders = repository.getOrdersForUnit(42, true);
  REQUIRE(newUnitOrders != nullptr);
  CHECK(newUnitOrders->size() == 1);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository removeOrdersForUnit returns false for non-existent unit", "[OrderRepository]")
{
  const bool removed = repository.removeOrdersForUnit(42, false);
  CHECK_FALSE(removed);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository removeOrdersForUnit removes unit entirely when all orders are removed", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.removeOrdersForUnit(42, false);
  const auto* orders = repository.getOrdersForUnit(42, false);
  CHECK(orders == nullptr);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository removeOrdersForUnit returns false when no matching orders exist", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 100 silver");
  const bool removed = repository.removeOrdersForUnit(42, true);
  CHECK_FALSE(removed);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository clear removes all orders", "[OrderRepository]")
{
  repository.addOrder(1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, false, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  repository.addOrder(2, true, 1, 2, 3, L"TEACH", L"TEACH 100");
  repository.clear();
  CHECK(repository.getOrdersForUnit(1, false) == nullptr);
  CHECK(repository.getOrdersForUnit(2, false) == nullptr);
  CHECK(repository.getOrdersForUnit(2, true) == nullptr);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores empty order token", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"", L"GIVE 50 100 silver");
  const auto* orders = repository.getOrdersForUnit(42, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->at(0).getOrderToken() == L"");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores empty full order text", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"");
  const auto* orders = repository.getOrdersForUnit(42, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->at(0).getFullOrderText() == L"");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores negative unit number", "[OrderRepository]")
{
  repository.addOrder(-1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  const auto* orders = repository.getOrdersForUnit(-1, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->at(0).getIssuingUnitNumber() == -1);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores zero unit number", "[OrderRepository]")
{
  repository.addOrder(0, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  const auto* orders = repository.getOrdersForUnit(0, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->at(0).getIssuingUnitNumber() == 0);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores negative coordinates", "[OrderRepository]")
{
  repository.addOrder(42, false, -1, -2, -3, L"GIVE", L"GIVE 50 100 silver");
  const auto& regionOrders = repository.getOrdersForRegion(-1, -2, -3);
  CHECK(regionOrders.size() == 1);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores large coordinate values", "[OrderRepository]")
{
  repository.addOrder(42, false, 1000, 2000, 3000, L"GIVE", L"GIVE 50 100 silver");
  const auto& regionOrders = repository.getOrdersForRegion(1000, 2000, 3000);
  CHECK(regionOrders.size() == 1);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository distinguishes between coordinate differences", "[OrderRepository]")
{
  repository.addOrder(1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, false, 1, 2, 4, L"STUDY", L"STUDY MELEE");
  repository.addOrder(3, false, 1, 3, 3, L"TEACH", L"TEACH 100");
  repository.addOrder(4, false, 2, 2, 3, L"BUILD", L"BUILD FORT");
  const auto& orders123 = repository.getOrdersForRegion(1, 2, 3);
  const auto& orders124 = repository.getOrdersForRegion(1, 2, 4);
  const auto& orders133 = repository.getOrdersForRegion(1, 3, 3);
  const auto& orders223 = repository.getOrdersForRegion(2, 2, 3);
  CHECK(orders123.size() == 1);
  CHECK(orders124.size() == 1);
  CHECK(orders133.size() == 1);
  CHECK(orders223.size() == 1);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository add and remove preserves unrelated orders", "[OrderRepository]")
{
  repository.addOrder(1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(2, false, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  repository.removeOrdersForUnit(1, false);
  const auto* orders2 = repository.getOrdersForUnit(2, false);
  REQUIRE(orders2 != nullptr);
  CHECK(orders2->size() == 1);
  CHECK(orders2->at(0).getIssuingUnitNumber() == 2);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository can clear and then add new orders", "[OrderRepository]")
{
  repository.addOrder(1, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.clear();
  repository.addOrder(2, false, 1, 2, 3, L"STUDY", L"STUDY MELEE");
  const auto* orders = repository.getOrdersForUnit(2, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->size() == 1);
  CHECK(orders->at(0).getIssuingUnitNumber() == 2);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository handles multiple removes for same unit flag", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 100 silver");
  repository.addOrder(42, false, 2, 3, 4, L"TAKE", L"TAKE FROM 51 50 gold");
  const bool firstRemove = repository.removeOrdersForUnit(42, false);
  CHECK(firstRemove);
  const bool secondRemove = repository.removeOrdersForUnit(42, false);
  CHECK_FALSE(secondRemove);
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository stores orders with valid GIVE syntax", "[OrderRepository]")
{
  repository.addOrder(42, false, 1, 2, 3, L"GIVE", L"GIVE 50 ALL silver");
  const auto* orders = repository.getOrdersForUnit(42, false);
  REQUIRE(orders != nullptr);
  CHECK(orders->at(0).getFullOrderText() == L"GIVE 50 ALL silver");
}

TEST_CASE_METHOD(OrderRepositoryFixture, "OrderRepository region query includes all units at coordinate", "[OrderRepository]")
{
  for (int i = 1; i <= 10; ++i) {
    repository.addOrder(i, false, 5, 5, 5, L"GIVE", L"GIVE 50 100 silver");
  }
  const auto& regionOrders = repository.getOrdersForRegion(5, 5, 5);
  CHECK(regionOrders.size() == 10);
}
