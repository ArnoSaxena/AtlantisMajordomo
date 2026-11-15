/* 
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * File: CoordinateFormattingUtilsTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Function/CoordinateFormattingUtils.hpp"

TEST_CASE("CoordinateFormattingUtils formatCoordinates default z omitted with no spaces")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 1, true, false);
  CHECK(result == L"(10,20)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates default z omitted with spaces")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 1, true, true);
  CHECK(result == L"(10, 20)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates non-default z included with no spaces")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 2, true, false);
  CHECK(result == L"(10,20,2)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates non-default z included with spaces")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 2, true, true);
  CHECK(result == L"(10, 20, 2)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates always includes z when omitDefaultZ false")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 1, false, false);
  CHECK(result == L"(10,20,1)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates always includes z when omitDefaultZ false with spaces")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(10, 20, 1, false, true);
  CHECK(result == L"(10, 20, 1)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates zero coordinates")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(0, 0, 1, true, false);
  CHECK(result == L"(0,0)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates negative coordinates")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(-5, -10, 1, true, false);
  CHECK(result == L"(-5,-10)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates mixed positive and negative with z")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(-5, 10, -3, true, false);
  CHECK(result == L"(-5,10,-3)");
}

TEST_CASE("CoordinateFormattingUtils formatCoordinates large coordinates")
{
  std::wstring result = CoordinateFormattingUtils::formatCoordinates(1000, 2000, 5, true, false);
  CHECK(result == L"(1000,2000,5)");
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates valid format without z")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20)", x, y, z);
  CHECK(result);
  CHECK(x == 10);
  CHECK(y == 20);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates valid format with z")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20,2)", x, y, z);
  CHECK(result);
  CHECK(x == 10);
  CHECK(y == 20);
  CHECK(z == 2);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates with spaces around commas")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10, 20)", x, y, z);
  CHECK(result);
  CHECK(x == 10);
  CHECK(y == 20);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates with spaces around commas and z")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10, 20, 2)", x, y, z);
  CHECK(result);
  CHECK(x == 10);
  CHECK(y == 20);
  CHECK(z == 2);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates negative coordinates")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(-5,-10)", x, y, z);
  CHECK(result);
  CHECK(x == -5);
  CHECK(y == -10);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates negative z")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(-5,-10,-3)", x, y, z);
  CHECK(result);
  CHECK(x == -5);
  CHECK(y == -10);
  CHECK(z == -3);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates zero coordinates")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(0,0)", x, y, z);
  CHECK(result);
  CHECK(x == 0);
  CHECK(y == 0);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates large coordinates")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(1000,2000,5)", x, y, z);
  CHECK(result);
  CHECK(x == 1000);
  CHECK(y == 2000);
  CHECK(z == 5);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates missing opening paren")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"10,20)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates missing closing paren")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates missing first comma")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10 20)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates missing second comma when z present")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20 2)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates trailing content after closing paren")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20)extra", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates trailing spaces after closing paren accepted")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20)  ", x, y, z);
  CHECK(result);
  CHECK(x == 10);
  CHECK(y == 20);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates empty string")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates only parentheses")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"()", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates non-numeric x")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(abc,20)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates non-numeric y")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,xyz)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils tryParseCoordinates non-numeric z")
{
  int x = 0, y = 0, z = 0;
  bool result = CoordinateFormattingUtils::tryParseCoordinates(L"(10,20,abc)", x, y, z);
  CHECK(!result);
}

TEST_CASE("CoordinateFormattingUtils round-trip format and parse")
{
  std::wstring formatted = CoordinateFormattingUtils::formatCoordinates(15, 25, 1, true, false);
  int x = 0, y = 0, z = 0;
  bool parsed = CoordinateFormattingUtils::tryParseCoordinates(formatted, x, y, z);
  CHECK(parsed);
  CHECK(x == 15);
  CHECK(y == 25);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils round-trip format and parse with non-default z")
{
  std::wstring formatted = CoordinateFormattingUtils::formatCoordinates(15, 25, 3, true, false);
  int x = 0, y = 0, z = 0;
  bool parsed = CoordinateFormattingUtils::tryParseCoordinates(formatted, x, y, z);
  CHECK(parsed);
  CHECK(x == 15);
  CHECK(y == 25);
  CHECK(z == 3);
}

TEST_CASE("CoordinateFormattingUtils round-trip format with spaces and parse")
{
  std::wstring formatted = CoordinateFormattingUtils::formatCoordinates(15, 25, 1, true, true);
  int x = 0, y = 0, z = 0;
  bool parsed = CoordinateFormattingUtils::tryParseCoordinates(formatted, x, y, z);
  CHECK(parsed);
  CHECK(x == 15);
  CHECK(y == 25);
  CHECK(z == 1);
}

TEST_CASE("CoordinateFormattingUtils round-trip format with spaces and z and parse")
{
  std::wstring formatted = CoordinateFormattingUtils::formatCoordinates(15, 25, 3, true, true);
  int x = 0, y = 0, z = 0;
  bool parsed = CoordinateFormattingUtils::tryParseCoordinates(formatted, x, y, z);
  CHECK(parsed);
  CHECK(x == 15);
  CHECK(y == 25);
  CHECK(z == 3);
}

TEST_CASE("CoordinateFormattingUtils round-trip negative coordinates")
{
  std::wstring formatted = CoordinateFormattingUtils::formatCoordinates(-10, -20, 2, true, false);
  int x = 0, y = 0, z = 0;
  bool parsed = CoordinateFormattingUtils::tryParseCoordinates(formatted, x, y, z);
  CHECK(parsed);
  CHECK(x == -10);
  CHECK(y == -20);
  CHECK(z == 2);
}
