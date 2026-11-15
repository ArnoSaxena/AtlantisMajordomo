/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/Event.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace
{
struct EventFixture
{
  Event event { 42, 1, L"Test message", 8, 7 };
};
}

TEST_CASE_METHOD(EventFixture, "Event stores unit identifier", "[Event]")
{
  CHECK(event.getUnitId() == 42);
}

TEST_CASE_METHOD(EventFixture, "Event stores event sequence number", "[Event]")
{
  CHECK(event.getEventId() == 1);
}

TEST_CASE_METHOD(EventFixture, "Event stores the message text", "[Event]")
{
  CHECK(event.getMessage() == L"Test message");
}

TEST_CASE_METHOD(EventFixture, "Event stores the report month", "[Event]")
{
  CHECK(event.getMonth() == 8);
}

TEST_CASE_METHOD(EventFixture, "Event stores the report year", "[Event]")
{
  CHECK(event.getYear() == 7);
}

TEST_CASE("Event calculates a deterministic hexadecimal identifier from unit, message, and period", "[Event]")
{
  const std::wstring id1 = Event::calculateIdentifier(42, L"Test message", 8, 7);
  const std::wstring id2 = Event::calculateIdentifier(42, L"Test message", 8, 7);
  CHECK(id1 == id2);
}

TEST_CASE("Event produces different identifiers for different units", "[Event]")
{
  const std::wstring id42 = Event::calculateIdentifier(42, L"Test message", 8, 7);
  const std::wstring id43 = Event::calculateIdentifier(43, L"Test message", 8, 7);
  CHECK(id42 != id43);
}

TEST_CASE("Event produces different identifiers for different messages", "[Event]")
{
  const std::wstring id1 = Event::calculateIdentifier(42, L"First message", 8, 7);
  const std::wstring id2 = Event::calculateIdentifier(42, L"Second message", 8, 7);
  CHECK(id1 != id2);
}

TEST_CASE("Event produces different identifiers for different months", "[Event]")
{
  const std::wstring id8 = Event::calculateIdentifier(42, L"Test message", 8, 7);
  const std::wstring id9 = Event::calculateIdentifier(42, L"Test message", 9, 7);
  CHECK(id8 != id9);
}

TEST_CASE("Event produces different identifiers for different years", "[Event]")
{
  const std::wstring id7 = Event::calculateIdentifier(42, L"Test message", 8, 7);
  const std::wstring id8 = Event::calculateIdentifier(42, L"Test message", 8, 8);
  CHECK(id7 != id8);
}

TEST_CASE_METHOD(EventFixture, "Event uses its stored identifier", "[Event]")
{
  CHECK(event.getIdentifier() == Event::calculateIdentifier(42, L"Test message", 8, 7));
}

TEST_CASE_METHOD(EventFixture, "Event is not an error event by default", "[Event]")
{
  CHECK_FALSE(event.isErrorEvent());
}

TEST_CASE_METHOD(EventFixture, "Event can be marked as an error event", "[Event]")
{
  event.setErrorEvent(true);
  CHECK(event.isErrorEvent());
}

TEST_CASE_METHOD(EventFixture, "Event can have its error status cleared", "[Event]")
{
  event.setErrorEvent(true);
  event.setErrorEvent(false);
  CHECK_FALSE(event.isErrorEvent());
}

TEST_CASE("Event construction accepts month default of zero", "[Event]")
{
  Event event(42, 1, L"Message", 0, 7);
  CHECK(event.getMonth() == 0);
}

TEST_CASE("Event construction accepts year default of zero", "[Event]")
{
  Event event(42, 1, L"Message", 8, 0);
  CHECK(event.getYear() == 0);
}

TEST_CASE("Event construction accepts both period defaults", "[Event]")
{
  Event event(42, 1, L"Message");
  CHECK(event.getMonth() == 0);
  CHECK(event.getYear() == 0);
}

TEST_CASE("Event handles escaped characters in messages", "[Event]")
{
  Event event(42, 1, L"Message with \"quotes\" and \\backslash");
  CHECK(event.getMessage() == L"Message with \"quotes\" and \\backslash");
}

TEST_CASE("Event handles special characters in identifiers", "[Event]")
{
  const std::wstring id = Event::calculateIdentifier(42, L"Message with ñ und éèê", 8, 7);
  CHECK_FALSE(id.empty());
  CHECK(id.length() == 16);
}
