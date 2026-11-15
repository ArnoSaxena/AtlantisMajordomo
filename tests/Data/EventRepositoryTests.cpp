/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Data/EventRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct EventRepositoryFixture
{
  EventRepository repository;
};
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository accepts an event and stores it", "[EventRepository]")
{
  Event event(42, 1, L"Test message", 8, 7);
  CHECK(repository.add(event));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository stores events with auto-assigned IDs", "[EventRepository]")
{
  CHECK(repository.add(42, L"First message"));
  const auto& stored = repository.at(0);
  CHECK(stored.getUnitId() == 42);
  CHECK(stored.getMessage() == L"First message");
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository stores events with period information", "[EventRepository]")
{
  CHECK(repository.add(42, L"Message", 8, 7, false));
  const auto& stored = repository.at(0);
  CHECK(stored.getMonth() == 8);
  CHECK(stored.getYear() == 7);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository stores error event classification", "[EventRepository]")
{
  CHECK(repository.add(42, L"Error message", 8, 7, true));
  const auto& stored = repository.at(0);
  CHECK(stored.isErrorEvent());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository stores non-error event classification", "[EventRepository]")
{
  CHECK(repository.add(42, L"Normal message", 8, 7, false));
  const auto& stored = repository.at(0);
  CHECK_FALSE(stored.isErrorEvent());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository allows multiple events with different units", "[EventRepository]")
{
  CHECK(repository.add(42, L"First message"));
  CHECK(repository.add(43, L"Second message"));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports empty size", "[EventRepository]")
{
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports size after adding one event", "[EventRepository]")
{
  repository.add(42, L"Message");
  CHECK(repository.size() == 1);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports size after adding multiple events", "[EventRepository]")
{
  repository.add(42, L"First");
  repository.add(43, L"Second");
  repository.add(44, L"Third");
  CHECK(repository.size() == 3);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds an event by its unique identifier", "[EventRepository]")
{
  Event event(42, 1, L"Message", 8, 7);
  repository.add(event);
  const Event* found = repository.findByIdentifier(event.getIdentifier());
  CHECK(found != nullptr);
  CHECK(found->getUnitId() == 42);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository returns nullptr for a missing identifier", "[EventRepository]")
{
  CHECK(repository.findByIdentifier(L"missing") == nullptr);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds events by unit ID", "[EventRepository]")
{
  repository.add(42, L"First");
  repository.add(42, L"Second");
  repository.add(43, L"Third");
  const auto events = repository.findByUnitId(42);
  CHECK(events.size() == 2);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository returns empty for a unit with no events", "[EventRepository]")
{
  CHECK(repository.findByUnitId(999).empty());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds error events by unit ID", "[EventRepository]")
{
  repository.add(42, L"Error 1", 8, 7, true);
  repository.add(42, L"Normal", 8, 7, false);
  repository.add(42, L"Error 2", 8, 7, true);
  repository.add(43, L"Error 3", 8, 7, true);
  const auto errors = repository.findErrorsByUnitId(42);
  CHECK(errors.size() == 2);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository returns empty for a unit with no errors", "[EventRepository]")
{
  repository.add(42, L"Normal", 8, 7, false);
  CHECK(repository.findErrorsByUnitId(42).empty());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds events by period", "[EventRepository]")
{
  repository.add(42, L"August year seven", 8, 7, false);
  repository.add(43, L"September year seven", 9, 7, false);
  repository.add(44, L"August year eight", 8, 8, false);
  const auto events = repository.findByPeriod(8, 7);
  CHECK(events.size() == 1);
  CHECK(events.at(0)->getUnitId() == 42);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository returns empty for a period with no events", "[EventRepository]")
{
  repository.add(42, L"Message", 8, 7, false);
  CHECK(repository.findByPeriod(9, 7).empty());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports available periods in reverse chronological order", "[EventRepository]")
{
  repository.add(42, L"Older", 8, 6, false);
  repository.add(43, L"Newer", 1, 7, false);
  repository.add(44, L"Middle", 12, 6, false);
  const auto periods = repository.getAvailablePeriods();
  CHECK(periods == std::vector<std::pair<int, int>> {{ 1, 7 }, { 12, 6 }, { 8, 6 }});
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository lists each available period only once", "[EventRepository]")
{
  repository.add(42, L"First", 8, 7, false);
  repository.add(43, L"Second", 8, 7, false);
  const auto periods = repository.getAvailablePeriods();
  CHECK(periods.size() == 1);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports failure when finding latest period while empty", "[EventRepository]")
{
  int month = 5;
  int year = 6;
  CHECK_FALSE(repository.getLatestPeriod(month, year));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository preserves output when latest period fails", "[EventRepository]")
{
  int month = 5;
  int year = 6;
  REQUIRE_FALSE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(5, 6));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds the latest period by year then month", "[EventRepository]")
{
  repository.add(42, L"Older", 8, 6, false);
  repository.add(43, L"Newer", 1, 7, false);
  int month = 0;
  int year = 0;
  REQUIRE(repository.getLatestPeriod(month, year));
  CHECK(std::make_pair(month, year) == std::make_pair(1, 7));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository omits invalid months from available periods", "[EventRepository]")
{
  repository.add(42, L"Invalid month", 0, 7, false);
  repository.add(43, L"Valid", 8, 7, false);
  const auto periods = repository.getAvailablePeriods();
  CHECK(periods == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository omits invalid years from available periods", "[EventRepository]")
{
  repository.add(42, L"Invalid year", 8, 0, false);
  repository.add(43, L"Valid", 8, 7, false);
  const auto periods = repository.getAvailablePeriods();
  CHECK(periods == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository omits negative years from available periods", "[EventRepository]")
{
  repository.add(42, L"Negative year", 8, -1, false);
  repository.add(43, L"Valid", 8, 7, false);
  const auto periods = repository.getAvailablePeriods();
  CHECK(periods == std::vector<std::pair<int, int>> {{ 8, 7 }});
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository finds events for the latest period by unit", "[EventRepository]")
{
  repository.add(42, L"Older unit", 8, 6, false);
  repository.add(42, L"Newer unit", 1, 7, false);
  repository.add(43, L"Other unit", 1, 7, false);
  const auto latest = repository.findLatestEventsByUnitId(42);
  CHECK(latest.size() == 1);
  CHECK(latest.at(0)->getMessage() == L"Newer unit");
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository returns empty for latest events of absent unit", "[EventRepository]")
{
  repository.add(42, L"Message", 8, 7, false);
  CHECK(repository.findLatestEventsByUnitId(999).empty());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository removes an event by identifier", "[EventRepository]")
{
  Event event(42, 1, L"Message", 8, 7);
  repository.add(event);
  CHECK_FALSE(repository.findByIdentifier(event.getIdentifier()) == nullptr);
  REQUIRE(repository.removeByIdentifier(event.getIdentifier()));
  CHECK(repository.findByIdentifier(event.getIdentifier()) == nullptr);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository reports failure when removing a missing identifier", "[EventRepository]")
{
  CHECK_FALSE(repository.removeByIdentifier(L"missing"));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository decreases size after removing an event", "[EventRepository]")
{
  Event event(42, 1, L"Message", 8, 7);
  repository.add(event);
  REQUIRE(repository.size() == 1);
  REQUIRE(repository.removeByIdentifier(event.getIdentifier()));
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository can be cleared", "[EventRepository]")
{
  repository.add(42, L"First");
  repository.add(43, L"Second");
  repository.clear();
  CHECK(repository.size() == 0);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository clearing resets available periods", "[EventRepository]")
{
  repository.add(42, L"Message", 8, 7, false);
  repository.clear();
  CHECK(repository.getAvailablePeriods().empty());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository clearing allows a previously used identifier again", "[EventRepository]")
{
  Event event(42, 1, L"Message", 8, 7);
  repository.add(event);
  repository.clear();
  CHECK(repository.add(event));
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository assigns incrementing sequence numbers", "[EventRepository]")
{
  repository.add(42, L"First");
  repository.add(42, L"Second");
  repository.add(42, L"Third");
  CHECK(repository.at(0).getEventId() == 1);
  CHECK(repository.at(1).getEventId() == 2);
  CHECK(repository.at(2).getEventId() == 3);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository provides the next event ID", "[EventRepository]")
{
  repository.add(42, L"First");
  repository.add(42, L"Second");
  CHECK(repository.getNextEventId() == 3);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository provides event ID starting at one", "[EventRepository]")
{
  CHECK(repository.getNextEventId() == 1);
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository persists error classification through find", "[EventRepository]")
{
  repository.add(42, L"Error message", 8, 7, true);
  repository.add(42, L"Normal message", 8, 7, false);
  const auto events = repository.findByUnitId(42);
  CHECK(events.at(0)->isErrorEvent());
  CHECK_FALSE(events.at(1)->isErrorEvent());
}

TEST_CASE_METHOD(EventRepositoryFixture, "EventRepository has empty error message when initialized", "[EventRepository]")
{
  CHECK(repository.getLastError().empty());
}
