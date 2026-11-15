/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Function/OrderParsingUtils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
struct GiveParserFixture
{
  OrderParsingUtils::WarningGiveTakeOrder parsed;
};

struct TokenizerFixture
{
  std::vector<std::wstring> tokens;
  std::vector<bool> quoted;
};

struct TransportParserFixture
{
  OrderParsingUtils::TransportOrder parsed;
};
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE parses unit quantity item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20 SILV", parsed));
  CHECK(std::make_tuple(parsed.otherUnitNumber, parsed.mode, parsed.quantity, parsed.itemOperand) ==
        std::make_tuple(2, OrderParsingUtils::WarningGiveMode::Quantity, 20, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE parses ALL item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE parses ALL item EXCEPT quantity", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV EXCEPT 15", parsed));
  CHECK(std::make_tuple(parsed.mode, parsed.itemOperand, parsed.exceptQuantity) ==
        std::make_tuple(OrderParsingUtils::WarningGiveMode::AllExcept, std::wstring(L"SILV"), 15));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE retains an ALL operand for later item-class resolution", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL WEAPONS", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"WEAPONS")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE recognizes UNIT without an additional unit number", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 UNIT", parsed));
  CHECK(parsed.isUnitTransfer);
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects an additional number after UNIT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 UNIT 1", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects EXCEPT before the item", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 EXCEPT 15 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects item before quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 SILV 20", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a missing destination", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a missing amount or UNIT keyword", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a missing item after quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a missing ALL operand", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a missing EXCEPT quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV EXCEPT", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a negative EXCEPT quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV EXCEPT -1", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE permits a zero EXCEPT reserve", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV EXCEPT 0", parsed));
  CHECK(parsed.exceptQuantity == 0);
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects zero explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 0 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects negative explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 -1 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects an overflowing explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 999999999999999999999 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects numeric tokens with trailing characters", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20junk SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects extra operands after an item", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20 SILV extra", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects extra operands after an EXCEPT reserve", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV EXCEPT 15 extra", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects a trailing number without EXCEPT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 ALL SILV 15", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE accepts a leading repeat marker", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"@GIVE 2 20 SILV", parsed));
  CHECK(parsed.quantity == 20);
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE rejects an inline repeat marker", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20 SILV @repeat", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE accepts NEW unit references", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE NEW 2 20 SILV", parsed));
  CHECK(std::make_pair(parsed.otherUnitIsNew, parsed.otherUnitNumber) == std::make_pair(true, 2));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE preserves quoted item operands", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20 \"Hard wood\"", parsed));
  CHECK(std::make_pair(parsed.itemOperandWasQuoted, parsed.itemOperand) ==
        std::make_pair(true, std::wstring(L"Hard wood")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils GIVE resets UNIT classification when parsing another order", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 UNIT", parsed));
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"GIVE 2 20 SILV", parsed));
  CHECK_FALSE(parsed.isUnitTransfer);
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects EXCEPT before the item", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 EXCEPT 15 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects item before quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 SILV 20", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE parses FROM unit quantity item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 20 SILV", parsed));
  CHECK(std::make_tuple(parsed.isTake, parsed.otherUnitNumber, parsed.mode, parsed.quantity, parsed.itemOperand) ==
        std::make_tuple(true, 2, OrderParsingUtils::WarningGiveMode::Quantity, 20, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE parses FROM unit ALL item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL SILV", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE parses ALL item EXCEPT quantity", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL SILV EXCEPT 15", parsed));
  CHECK(std::make_tuple(parsed.mode, parsed.itemOperand, parsed.exceptQuantity) ==
        std::make_tuple(OrderParsingUtils::WarningGiveMode::AllExcept, std::wstring(L"SILV"), 15));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE retains ALL item-class operands without expanding them", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL WEAPONS", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"WEAPONS")));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE requires FROM", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE 2 20 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects UNIT transfers", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 UNIT", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a missing source after FROM", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a missing quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a missing item after quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 20", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a missing ALL operand", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a missing EXCEPT reserve", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL SILV EXCEPT", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects a negative EXCEPT reserve", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL SILV EXCEPT -1", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects an explicit quantity followed by EXCEPT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 20 SILV EXCEPT 5", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects extra operands after ALL EXCEPT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 ALL SILV EXCEPT 5 extra", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects zero explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 0 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects an overflowing explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 999999999999999999999 SILV", parsed));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE accepts a leading repeat marker", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseGiveTakeOrder(L"@TAKE FROM 2 20 SILV", parsed));
  CHECK(std::make_pair(parsed.isTake, parsed.quantity) == std::make_pair(true, 20));
}

TEST_CASE_METHOD(GiveParserFixture, "OrderParsingUtils TAKE rejects inline at-signs", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseGiveTakeOrder(L"TAKE FROM 2 20 SILV @repeat", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT parses unit quantity item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20 SILV", parsed));
  CHECK(std::make_tuple(parsed.targetUnitNumber, parsed.mode, parsed.quantity, parsed.itemOperand) ==
        std::make_tuple(2, OrderParsingUtils::WarningGiveMode::Quantity, 20, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE parses unit quantity item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20 SILV", parsed));
  CHECK(std::make_tuple(parsed.targetUnitNumber, parsed.mode, parsed.quantity, parsed.itemOperand) ==
        std::make_tuple(2, OrderParsingUtils::WarningGiveMode::Quantity, 20, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT parses ALL item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE parses ALL item", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV", parsed));
  CHECK(std::make_pair(parsed.mode, parsed.itemOperand) ==
        std::make_pair(OrderParsingUtils::WarningGiveMode::All, std::wstring(L"SILV")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT parses ALL item EXCEPT amount", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT 15", parsed));
  CHECK(std::make_tuple(parsed.mode, parsed.itemOperand, parsed.exceptQuantity) ==
        std::make_tuple(OrderParsingUtils::WarningGiveMode::AllExcept, std::wstring(L"SILV"), 15));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE parses ALL item EXCEPT amount", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT 15", parsed));
  CHECK(std::make_tuple(parsed.mode, parsed.itemOperand, parsed.exceptQuantity) ==
        std::make_tuple(OrderParsingUtils::WarningGiveMode::AllExcept, std::wstring(L"SILV"), 15));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects FROM", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT FROM 2 20 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects FROM", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE FROM 2 20 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects EXCEPT-first spelling", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 EXCEPT 15 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects EXCEPT-first spelling", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 EXCEPT 15 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects item before quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 SILV 20", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects item before quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 SILV 20", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects UNIT transfers", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 UNIT", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects UNIT transfers", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 UNIT", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a missing destination", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a missing destination", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a missing quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a missing quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a missing item", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a missing item", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a missing ALL operand", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a missing ALL operand", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a missing EXCEPT amount", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a missing EXCEPT amount", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects a negative EXCEPT amount", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT -1", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects a negative EXCEPT amount", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT -1", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT permits a zero EXCEPT reserve", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT 0", parsed));
  CHECK(parsed.exceptQuantity == 0);
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE permits a zero EXCEPT reserve", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT 0", parsed));
  CHECK(parsed.exceptQuantity == 0);
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects zero explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 0 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects zero explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 0 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects negative explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 -1 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects negative explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 -1 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects overflowing explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 999999999999999999999 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects overflowing explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 999999999999999999999 SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects numeric tokens with trailing characters", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20junk SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects numeric tokens with trailing characters", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20junk SILV", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects EXCEPT after explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20 SILV EXCEPT 5", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects EXCEPT after explicit quantity", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20 SILV EXCEPT 5", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects trailing operands after ALL EXCEPT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT 5 extra", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects trailing operands after ALL EXCEPT", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT 5 extra", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT accepts leading repeat markers", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"@TRANSPORT 2 20 SILV", parsed));
  CHECK(parsed.quantity == 20);
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE accepts leading repeat markers", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"@DISTRIBUTE 2 20 SILV", parsed));
  CHECK(parsed.quantity == 20);
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT rejects inline at-signs", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20 SILV @repeat", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE rejects inline at-signs", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20 SILV @repeat", parsed));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT preserves quoted item operands", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20 \"Hard wood\"", parsed));
  CHECK(std::make_pair(parsed.itemOperandWasQuoted, parsed.itemOperand) ==
        std::make_pair(true, std::wstring(L"Hard wood")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE preserves quoted item operands", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20 \"Hard wood\"", parsed));
  CHECK(std::make_pair(parsed.itemOperandWasQuoted, parsed.itemOperand) ==
        std::make_pair(true, std::wstring(L"Hard wood")));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT accepts NEW unit references", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT NEW 2 20 SILV", parsed));
  CHECK(std::make_pair(parsed.targetIsNewUnit, parsed.targetUnitNumber) == std::make_pair(true, 2));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE accepts NEW unit references", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE NEW 2 20 SILV", parsed));
  CHECK(std::make_pair(parsed.targetIsNewUnit, parsed.targetUnitNumber) == std::make_pair(true, 2));
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils TRANSPORT clears a previous reserve when parsing quantity", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 ALL SILV EXCEPT 15", parsed));
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"TRANSPORT 2 20 SILV", parsed));
  CHECK(parsed.exceptQuantity == 0);
}

TEST_CASE_METHOD(TransportParserFixture, "OrderParsingUtils DISTRIBUTE clears a previous reserve when parsing quantity", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 ALL SILV EXCEPT 15", parsed));
  REQUIRE(OrderParsingUtils::tryParseTransportOrder(L"DISTRIBUTE 2 20 SILV", parsed));
  CHECK(parsed.exceptQuantity == 0);
}

TEST_CASE_METHOD(TokenizerFixture, "OrderParsingUtils preserves the leading repeat marker token", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tokenizeOrderLine(L" \t@GIVE 2 20 SILV", tokens, quoted));
  CHECK(tokens.front() == L"@GIVE");
}

TEST_CASE_METHOD(TokenizerFixture, "OrderParsingUtils rejects at-signs embedded in an unquoted token", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tokenizeOrderLine(L"CLAIM 10@repeat", tokens, quoted));
}

TEST_CASE_METHOD(TokenizerFixture, "OrderParsingUtils rejects duplicate repeat prefixes", "[OrderParsingUtils]")
{
  CHECK_FALSE(OrderParsingUtils::tokenizeOrderLine(L"@@GIVE 2 20 SILV", tokens, quoted));
}

TEST_CASE_METHOD(TokenizerFixture, "OrderParsingUtils preserves at-signs inside quoted text", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tokenizeOrderLine(L"NAME UNIT \"Guard @ Home\"", tokens, quoted));
  CHECK(tokens == std::vector<std::wstring> { L"NAME", L"UNIT", L"Guard @ Home" });
}

TEST_CASE_METHOD(TokenizerFixture, "OrderParsingUtils terminates an order at a semicolon comment", "[OrderParsingUtils]")
{
  REQUIRE(OrderParsingUtils::tokenizeOrderLine(L"GIVE 2 20 SILV ; comment", tokens, quoted));
  CHECK(tokens == std::vector<std::wstring> { L"GIVE", L"2", L"20", L"SILV" });
}
