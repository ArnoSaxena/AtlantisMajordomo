/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Order.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace
{
struct OrderFixture
{
  Order order { 42, false, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver" };
};
}

TEST_CASE_METHOD(OrderFixture, "Order stores the issuing unit number", "[Order]")
{
  CHECK(order.getIssuingUnitNumber() == 42);
}

TEST_CASE_METHOD(OrderFixture, "Order stores the from-new-unit flag", "[Order]")
{
  CHECK_FALSE(order.getFromNewUnit());
}

TEST_CASE_METHOD(OrderFixture, "Order stores the x coordinate", "[Order]")
{
  CHECK(order.getXCoordinate() == 1);
}

TEST_CASE_METHOD(OrderFixture, "Order stores the y coordinate", "[Order]")
{
  CHECK(order.getYCoordinate() == 2);
}

TEST_CASE_METHOD(OrderFixture, "Order stores the z coordinate", "[Order]")
{
  CHECK(order.getZCoordinate() == 3);
}

TEST_CASE_METHOD(OrderFixture, "Order stores the order token", "[Order]")
{
  CHECK(order.getOrderToken() == L"GIVE");
}

TEST_CASE_METHOD(OrderFixture, "Order stores the full order text", "[Order]")
{
  CHECK(order.getFullOrderText() == L"GIVE 50 100 silver");
}

TEST_CASE("Order construction accepts from-new-unit flag true", "[Order]")
{
  Order order(42, true, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getFromNewUnit());
}

TEST_CASE("Order construction stores zero coordinates", "[Order]")
{
  Order order(42, false, L"GIVE", 0, 0, 0, L"GIVE 50 100 silver");
  CHECK(order.getXCoordinate() == 0);
  CHECK(order.getYCoordinate() == 0);
  CHECK(order.getZCoordinate() == 0);
}

TEST_CASE_METHOD(OrderFixture, "Order can change its issuing unit number", "[Order]")
{
  order.setIssuingUnitNumber(100);
  CHECK(order.getIssuingUnitNumber() == 100);
}

TEST_CASE_METHOD(OrderFixture, "Order can change from-new-unit flag", "[Order]")
{
  order.setFromNewUnit(true);
  CHECK(order.getFromNewUnit());
}

TEST_CASE_METHOD(OrderFixture, "Order can change its x coordinate", "[Order]")
{
  order.setXCoordinate(5);
  CHECK(order.getXCoordinate() == 5);
}

TEST_CASE_METHOD(OrderFixture, "Order can change its y coordinate", "[Order]")
{
  order.setYCoordinate(6);
  CHECK(order.getYCoordinate() == 6);
}

TEST_CASE_METHOD(OrderFixture, "Order can change its z coordinate", "[Order]")
{
  order.setZCoordinate(7);
  CHECK(order.getZCoordinate() == 7);
}

TEST_CASE_METHOD(OrderFixture, "Order can change its order token", "[Order]")
{
  order.setOrderToken(L"STUDY");
  CHECK(order.getOrderToken() == L"STUDY");
}

TEST_CASE_METHOD(OrderFixture, "Order can change its full order text", "[Order]")
{
  order.setFullOrderText(L"STUDY MELEE");
  CHECK(order.getFullOrderText() == L"STUDY MELEE");
}

TEST_CASE("Order stores empty order token", "[Order]")
{
  Order order(42, false, L"", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getOrderToken() == L"");
}

TEST_CASE("Order stores empty full order text", "[Order]")
{
  Order order(42, false, L"GIVE", 1, 2, 3, L"");
  CHECK(order.getFullOrderText() == L"");
}

TEST_CASE("Order stores negative unit number", "[Order]")
{
  Order order(-1, false, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getIssuingUnitNumber() == -1);
}

TEST_CASE("Order stores zero unit number", "[Order]")
{
  Order order(0, false, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getIssuingUnitNumber() == 0);
}

TEST_CASE("Order stores negative coordinates", "[Order]")
{
  Order order(42, false, L"GIVE", -1, -2, -3, L"GIVE 50 100 silver");
  CHECK(order.getXCoordinate() == -1);
  CHECK(order.getYCoordinate() == -2);
  CHECK(order.getZCoordinate() == -3);
}

TEST_CASE("Order stores large coordinate values", "[Order]")
{
  Order order(42, false, L"GIVE", 1000, 2000, 3000, L"GIVE 50 100 silver");
  CHECK(order.getXCoordinate() == 1000);
  CHECK(order.getYCoordinate() == 2000);
  CHECK(order.getZCoordinate() == 3000);
}

TEST_CASE("Order stores order token with spaces", "[Order]")
{
  Order order(42, false, L"GIVE WITH SPACES", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getOrderToken() == L"GIVE WITH SPACES");
}

TEST_CASE("Order stores full order text with valid GIVE syntax", "[Order]")
{
  Order order(42, false, L"GIVE", 1, 2, 3, L"GIVE 50 ALL silver");
  CHECK(order.getFullOrderText() == L"GIVE 50 ALL silver");
}

TEST_CASE("Order stores full order text with GIVE ALL EXCEPT syntax", "[Order]")
{
  Order order(42, false, L"GIVE", 1, 2, 3, L"GIVE 50 ALL silver EXCEPT 10");
  CHECK(order.getFullOrderText() == L"GIVE 50 ALL silver EXCEPT 10");
}

TEST_CASE("Order stores full order text with GIVE UNIT syntax", "[Order]")
{
  Order order(42, false, L"GIVE", 1, 2, 3, L"GIVE 50 UNIT");
  CHECK(order.getFullOrderText() == L"GIVE 50 UNIT");
}

TEST_CASE("Order stores full order text with TAKE syntax", "[Order]")
{
  Order order(42, false, L"TAKE", 1, 2, 3, L"TAKE FROM 50 100 gold");
  CHECK(order.getFullOrderText() == L"TAKE FROM 50 100 gold");
}

TEST_CASE("Order stores full order text with TRANSPORT syntax", "[Order]")
{
  Order order(42, false, L"TRANSPORT", 1, 2, 3, L"TRANSPORT 50 100 sword");
  CHECK(order.getFullOrderText() == L"TRANSPORT 50 100 sword");
}

TEST_CASE("Order can transition from new unit to regular unit", "[Order]")
{
  Order order(42, true, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK(order.getFromNewUnit());
  order.setFromNewUnit(false);
  CHECK_FALSE(order.getFromNewUnit());
}

TEST_CASE("Order can transition from regular unit to new unit", "[Order]")
{
  Order order(42, false, L"GIVE", 1, 2, 3, L"GIVE 50 100 silver");
  CHECK_FALSE(order.getFromNewUnit());
  order.setFromNewUnit(true);
  CHECK(order.getFromNewUnit());
}

TEST_CASE("Order preserves all properties after coordinate change", "[Order]")
{
  Order order(42, true, L"STUDY", 1, 2, 3, L"STUDY MELEE");
  order.setXCoordinate(10);
  order.setYCoordinate(20);
  order.setZCoordinate(30);
  CHECK(order.getIssuingUnitNumber() == 42);
  CHECK(order.getFromNewUnit());
  CHECK(order.getOrderToken() == L"STUDY");
  CHECK(order.getFullOrderText() == L"STUDY MELEE");
}

