/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#include "Function/MonthUtils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <limits>

TEST_CASE("MonthUtils formats January", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(1) == L"January");
}

TEST_CASE("MonthUtils formats February", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(2) == L"February");
}

TEST_CASE("MonthUtils formats March", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(3) == L"March");
}

TEST_CASE("MonthUtils formats April", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(4) == L"April");
}

TEST_CASE("MonthUtils formats May", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(5) == L"May");
}

TEST_CASE("MonthUtils formats June", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(6) == L"June");
}

TEST_CASE("MonthUtils formats July", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(7) == L"July");
}

TEST_CASE("MonthUtils formats August", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(8) == L"August");
}

TEST_CASE("MonthUtils formats September", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(9) == L"September");
}

TEST_CASE("MonthUtils formats October", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(10) == L"October");
}

TEST_CASE("MonthUtils formats November", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(11) == L"November");
}

TEST_CASE("MonthUtils formats December", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(12) == L"December");
}

TEST_CASE("MonthUtils returns an empty name for month zero", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(0).empty());
}

TEST_CASE("MonthUtils returns an empty name for month thirteen", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(13).empty());
}

TEST_CASE("MonthUtils returns an empty name for a negative month", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(-1).empty());
}

TEST_CASE("MonthUtils handles the minimum integer month", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(std::numeric_limits<int>::min()).empty());
}

TEST_CASE("MonthUtils handles the maximum integer month", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToName(std::numeric_limits<int>::max()).empty());
}

TEST_CASE("MonthUtils uses the valid month name instead of the fallback", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToNameOr(6, L"Unknown month") == L"June");
}

TEST_CASE("MonthUtils uses the fallback below the valid month range", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToNameOr(0, L"Unknown month") == L"Unknown month");
}

TEST_CASE("MonthUtils uses the fallback above the valid month range", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToNameOr(13, L"Unknown month") == L"Unknown month");
}

TEST_CASE("MonthUtils permits an empty fallback", "[MonthUtils]")
{
  CHECK(MonthUtils::monthNumberToNameOr(0, L"").empty());
}

TEST_CASE("MonthUtils parses January as month one", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"January", month);
  CHECK(month == 1);
}

TEST_CASE("MonthUtils parses February as month two", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"February", month);
  CHECK(month == 2);
}

TEST_CASE("MonthUtils parses March as month three", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"March", month);
  CHECK(month == 3);
}

TEST_CASE("MonthUtils parses April as month four", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"April", month);
  CHECK(month == 4);
}

TEST_CASE("MonthUtils parses May as month five", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"May", month);
  CHECK(month == 5);
}

TEST_CASE("MonthUtils parses June as month six", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"June", month);
  CHECK(month == 6);
}

TEST_CASE("MonthUtils parses July as month seven", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"July", month);
  CHECK(month == 7);
}

TEST_CASE("MonthUtils parses August as month eight", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"August", month);
  CHECK(month == 8);
}

TEST_CASE("MonthUtils parses September as month nine", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"September", month);
  CHECK(month == 9);
}

TEST_CASE("MonthUtils parses October as month ten", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"October", month);
  CHECK(month == 10);
}

TEST_CASE("MonthUtils parses November as month eleven", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"November", month);
  CHECK(month == 11);
}

TEST_CASE("MonthUtils parses December as month twelve", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"December", month);
  CHECK(month == 12);
}

TEST_CASE("MonthUtils accepts a mixed case month name and returns its number", "[MonthUtils]")
{
  int month = 0;
  CHECK(MonthUtils::tryParseMonthName(L"febRUARY", month));
  CHECK(month == 2);
}

TEST_CASE("MonthUtils ignores surrounding whitespace when resolving a month number", "[MonthUtils]")
{
  int month = 0;
  MonthUtils::tryParseMonthName(L"  February \t", month);
  CHECK(month == 2);
}

TEST_CASE("MonthUtils rejects an invalid month name without changing the result", "[MonthUtils]")
{
  int month = 7;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L"Not a month", month));
  CHECK(month == 7);
}

TEST_CASE("MonthUtils rejects an empty month name without changing the result", "[MonthUtils]")
{
  int month = 7;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L"", month));
  CHECK(month == 7);
}

TEST_CASE("MonthUtils rejects a whitespace only month name without changing the result", "[MonthUtils]")
{
  int month = 7;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L" \t\r\n ", month));
  CHECK(month == 7);
}

TEST_CASE("MonthUtils rejects an abbreviated month name", "[MonthUtils]")
{
  int month = 0;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L"Jan", month));
}

TEST_CASE("MonthUtils rejects a numeric month name", "[MonthUtils]")
{
  int month = 0;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L"1", month));
}

TEST_CASE("MonthUtils rejects trailing text after a month name", "[MonthUtils]")
{
  int month = 0;
  CHECK_FALSE(MonthUtils::tryParseMonthName(L"January extra", month));
}
