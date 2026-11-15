/*
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo - Unit Tests
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
 * File: StructInfoTests.cpp
 */

#include <catch2/catch_all.hpp>
#include "Data/StructInfo.hpp"

struct StructInfoFixture
{
  StructInfo structInfo { L"Tower", 50, 2, false, false };
};

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can be constructed with required fields", "[StructInfo]")
{
  CHECK(structInfo.getStructureType() == L"Tower");
  CHECK(structInfo.getNeeds() == 50);
  CHECK(structInfo.getMagesCapacity() == 2);
  CHECK_FALSE(structInfo.isShip());
  CHECK_FALSE(structInfo.protoIsFlying());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo item identifier token defaults to empty", "[StructInfo]")
{
  CHECK(structInfo.getItemIdentifierToken().empty());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo structure type can be set", "[StructInfo]")
{
  structInfo.setStructureType(L"Castle");
  CHECK(structInfo.getStructureType() == L"Castle");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo needs can be set", "[StructInfo]")
{
  structInfo.setNeeds(100);
  CHECK(structInfo.getNeeds() == 100);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo mages capacity can be set", "[StructInfo]")
{
  structInfo.setMagesCapacity(5);
  CHECK(structInfo.getMagesCapacity() == 5);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo ship flag can be set to true", "[StructInfo]")
{
  structInfo.setIsShip(true);
  CHECK(structInfo.isShip());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo ship flag can be set to false", "[StructInfo]")
{
  StructInfo ship(L"Longship", 0, 0, true, false);
  ship.setIsShip(false);
  CHECK_FALSE(ship.isShip());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo flying flag can be set to true", "[StructInfo]")
{
  structInfo.setIsFlying(true);
  CHECK(structInfo.protoIsFlying());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo flying flag can be set to false", "[StructInfo]")
{
  StructInfo flying(L"Airship", 0, 0, true, true);
  flying.setIsFlying(false);
  CHECK_FALSE(flying.protoIsFlying());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo item identifier token can be set", "[StructInfo]")
{
  structInfo.setItemIdentifierToken(L"TOWE");
  CHECK(structInfo.getItemIdentifierToken() == L"TOWE");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo ship with zero needs", "[StructInfo]")
{
  StructInfo ship(L"Longship", 0, 1, true, false);
  CHECK(ship.getNeeds() == 0);
  CHECK(ship.isShip());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo flying structure", "[StructInfo]")
{
  StructInfo flying(L"Airship", 100, 3, true, true);
  CHECK(flying.isShip());
  CHECK(flying.protoIsFlying());
  CHECK(flying.getNeeds() == 100);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can handle high needs value", "[StructInfo]")
{
  structInfo.setNeeds(10000);
  CHECK(structInfo.getNeeds() == 10000);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can handle high mages capacity", "[StructInfo]")
{
  structInfo.setMagesCapacity(1000);
  CHECK(structInfo.getMagesCapacity() == 1000);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can set negative needs value", "[StructInfo]")
{
  structInfo.setNeeds(-50);
  CHECK(structInfo.getNeeds() == -50);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can set negative mages capacity", "[StructInfo]")
{
  structInfo.setMagesCapacity(-5);
  CHECK(structInfo.getMagesCapacity() == -5);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can set zero needs", "[StructInfo]")
{
  structInfo.setNeeds(0);
  CHECK(structInfo.getNeeds() == 0);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo can set zero mages capacity", "[StructInfo]")
{
  structInfo.setMagesCapacity(0);
  CHECK(structInfo.getMagesCapacity() == 0);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType for non-road type", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Tower");
  CHECK(result.empty());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType for road with direction", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Road N");
  CHECK(result == L"N");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType for road NE", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Road NE");
  CHECK(result == L"NE");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType case insensitive", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"road nw");
  CHECK(result == L"nw");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType removes trailing punctuation", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Road S.");
  CHECK(result == L"S");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType removes multiple punctuation", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Road E.;");
  CHECK(result == L"E");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType with whitespace", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"  Road  SW  ");
  CHECK(result == L"SW");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo extractRoadDirectionFromStructureType road alone", "[StructInfo]")
{
  std::wstring result = StructInfo::extractRoadDirectionFromStructureType(L"Road");
  CHECK(result.empty());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo supports copy construction", "[StructInfo]")
{
  structInfo.setItemIdentifierToken(L"TOWE");
  structInfo.setNeeds(100);
  
  StructInfo copy(structInfo);
  CHECK(copy.getStructureType() == L"Tower");
  CHECK(copy.getNeeds() == 100);
  CHECK(copy.getItemIdentifierToken() == L"TOWE");
  CHECK(copy.getMagesCapacity() == 2);
  CHECK_FALSE(copy.isShip());
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo supports move construction", "[StructInfo]")
{
  structInfo.setItemIdentifierToken(L"TOWE");
  
  StructInfo moved(std::move(structInfo));
  CHECK(moved.getStructureType() == L"Tower");
  CHECK(moved.getItemIdentifierToken() == L"TOWE");
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo supports copy assignment", "[StructInfo]")
{
  structInfo.setNeeds(75);
  
  StructInfo other(L"Castle", 50, 3, false, false);
  other = structInfo;
  
  CHECK(other.getStructureType() == L"Tower");
  CHECK(other.getNeeds() == 75);
}

TEST_CASE_METHOD(StructInfoFixture, "StructInfo supports move assignment", "[StructInfo]")
{
  structInfo.setNeeds(150);
  
  StructInfo other(L"Castle", 50, 3, false, false);
  other = std::move(structInfo);
  
  CHECK(other.getStructureType() == L"Tower");
  CHECK(other.getNeeds() == 150);
}
