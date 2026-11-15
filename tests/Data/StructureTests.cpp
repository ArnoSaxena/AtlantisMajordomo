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
 * File: StructureTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Data/Structure.hpp"

TEST_CASE("Structure construction stores all fields correctly")
{
  Structure structure(101, 5, 10, 2, L"tower", L"Guard Tower", false, 3, 7);
  
  CHECK(structure.getStructureId() == 101);
  CHECK(structure.getXCoordinate() == 5);
  CHECK(structure.getYCoordinate() == 10);
  CHECK(structure.getZCoordinate() == 2);
  CHECK(structure.getStructureType() == L"tower");
  CHECK(structure.getStructureName() == L"Guard Tower");
  CHECK(structure.isClosed() == false);
  CHECK(structure.getMonth() == 3);
  CHECK(structure.getYear() == 7);
}

TEST_CASE("Structure default-constructs with zero ownerUnitId")
{
  Structure structure(201, 0, 0, 0, L"fort", L"My Fort", true, 1, 1);
  
  CHECK(structure.getOwnerUnitId() == 0);
}

TEST_CASE("Structure default-constructs with empty fleet items")
{
  Structure structure(301, 1, 1, 1, L"fleet", L"My Fleet", false, 2, 2);
  
  CHECK(structure.getFleetItems().empty());
  CHECK(structure.getFleetItemsAfterOrders().empty());
}

TEST_CASE("Structure supports copy construction")
{
  Structure original(401, 2, 3, 4, L"mine", L"Silver Mine", false, 5, 6);
  original.setOwnerUnitId(999);
  original.setFleetItem(L"ship", 5);
  
  Structure copy = original;
  
  CHECK(copy.getStructureId() == 401);
  CHECK(copy.getXCoordinate() == 2);
  CHECK(copy.getYCoordinate() == 3);
  CHECK(copy.getZCoordinate() == 4);
  CHECK(copy.getStructureType() == L"mine");
  CHECK(copy.getStructureName() == L"Silver Mine");
  CHECK(copy.isClosed() == false);
  CHECK(copy.getOwnerUnitId() == 999);
  CHECK(copy.getMonth() == 5);
  CHECK(copy.getYear() == 6);
  CHECK(copy.getFleetItems().at(L"ship") == 5);
}

TEST_CASE("Structure supports copy assignment")
{
  Structure original(501, 7, 8, 9, L"temple", L"Holy Temple", true, 10, 11);
  Structure target(1, 0, 0, 0, L"town", L"Town", false, 1, 1);
  
  target = original;
  
  CHECK(target.getStructureId() == 501);
  CHECK(target.getStructureType() == L"temple");
  CHECK(target.getStructureName() == L"Holy Temple");
}

TEST_CASE("Structure supports move construction")
{
  Structure original(601, 11, 12, 13, L"shrine", L"Small Shrine", false, 12, 7);
  Structure moved = std::move(original);
  
  CHECK(moved.getStructureId() == 601);
  CHECK(moved.getXCoordinate() == 11);
  CHECK(moved.getStructureType() == L"shrine");
}

TEST_CASE("Structure supports move assignment")
{
  Structure original(701, 14, 15, 16, L"vault", L"Treasure Vault", true, 11, 8);
  Structure target(1, 0, 0, 0, L"town", L"Town", false, 1, 1);
  
  target = std::move(original);
  
  CHECK(target.getStructureId() == 701);
  CHECK(target.getStructureType() == L"vault");
}

TEST_CASE("Structure setStructureType changes type")
{
  Structure structure(801, 0, 0, 0, L"tower", L"Guard Tower", false, 1, 1);
  
  structure.setStructureType(L"fort");
  
  CHECK(structure.getStructureType() == L"fort");
}

TEST_CASE("Structure setStructureName changes name")
{
  Structure structure(901, 0, 0, 0, L"tower", L"Guard Tower", false, 1, 1);
  
  structure.setStructureName(L"New Name");
  
  CHECK(structure.getStructureName() == L"New Name");
}

TEST_CASE("Structure setIsClosed changes closed status")
{
  Structure structure(1001, 0, 0, 0, L"tower", L"Tower", false, 1, 1);
  
  CHECK(structure.isClosed() == false);
  
  structure.setIsClosed(true);
  
  CHECK(structure.isClosed() == true);
}

TEST_CASE("Structure setOwnerUnitId changes owner unit ID")
{
  Structure structure(1101, 0, 0, 0, L"tower", L"Tower", false, 1, 1);
  
  structure.setOwnerUnitId(555);
  
  CHECK(structure.getOwnerUnitId() == 555);
}

TEST_CASE("Structure setFleetItem adds item to fleet")
{
  Structure structure(1201, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItem(L"ship", 10);
  
  CHECK(structure.getFleetItems().at(L"ship") == 10);
}

TEST_CASE("Structure setFleetItem replaces existing item")
{
  Structure structure(1301, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItem(L"ship", 5);
  
  structure.setFleetItem(L"ship", 20);
  
  CHECK(structure.getFleetItems().at(L"ship") == 20);
}

TEST_CASE("Structure setFleetItem rejects empty token")
{
  Structure structure(1401, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItem(L"", 10);
  
  CHECK(structure.getFleetItems().empty());
}

TEST_CASE("Structure setFleetItem rejects zero amount")
{
  Structure structure(1501, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItem(L"ship", 0);
  
  CHECK(structure.getFleetItems().empty());
}

TEST_CASE("Structure setFleetItem rejects negative amount")
{
  Structure structure(1601, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItem(L"ship", -5);
  
  CHECK(structure.getFleetItems().empty());
}

TEST_CASE("Structure setFleetItems replaces entire collection")
{
  Structure structure(1701, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItem(L"ship", 5);
  
  std::map<std::wstring, int> newItems;
  newItems[L"horse"] = 10;
  newItems[L"sword"] = 20;
  structure.setFleetItems(newItems);
  
  CHECK(structure.getFleetItems().size() == 2);
  CHECK(structure.getFleetItems().at(L"horse") == 10);
  CHECK(structure.getFleetItems().at(L"sword") == 20);
}

TEST_CASE("Structure clearFleetItems empties fleet items")
{
  Structure structure(1801, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItem(L"ship", 5);
  
  structure.clearFleetItems();
  
  CHECK(structure.getFleetItems().empty());
}

TEST_CASE("Structure setFleetItemAfterOrders adds item to after-orders collection")
{
  Structure structure(1901, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItemAfterOrders(L"ship", 15);
  
  CHECK(structure.getFleetItemsAfterOrders().at(L"ship") == 15);
}

TEST_CASE("Structure setFleetItemAfterOrders replaces existing item")
{
  Structure structure(2001, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItemAfterOrders(L"ship", 10);
  
  structure.setFleetItemAfterOrders(L"ship", 25);
  
  CHECK(structure.getFleetItemsAfterOrders().at(L"ship") == 25);
}

TEST_CASE("Structure setFleetItemAfterOrders rejects empty token")
{
  Structure structure(2101, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItemAfterOrders(L"", 10);
  
  CHECK(structure.getFleetItemsAfterOrders().empty());
}

TEST_CASE("Structure setFleetItemAfterOrders rejects zero amount")
{
  Structure structure(2201, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItemAfterOrders(L"ship", 0);
  
  CHECK(structure.getFleetItemsAfterOrders().empty());
}

TEST_CASE("Structure setFleetItemAfterOrders rejects negative amount")
{
  Structure structure(2301, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItemAfterOrders(L"ship", -5);
  
  CHECK(structure.getFleetItemsAfterOrders().empty());
}

TEST_CASE("Structure setFleetItemsAfterOrders replaces entire after-orders collection")
{
  Structure structure(2401, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItemAfterOrders(L"ship", 5);
  
  std::map<std::wstring, int> newItems;
  newItems[L"horse"] = 15;
  newItems[L"sword"] = 25;
  structure.setFleetItemsAfterOrders(newItems);
  
  CHECK(structure.getFleetItemsAfterOrders().size() == 2);
  CHECK(structure.getFleetItemsAfterOrders().at(L"horse") == 15);
  CHECK(structure.getFleetItemsAfterOrders().at(L"sword") == 25);
}

TEST_CASE("Structure clearFleetItemsAfterOrders empties after-orders collection")
{
  Structure structure(2501, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItemAfterOrders(L"ship", 10);
  
  structure.clearFleetItemsAfterOrders();
  
  CHECK(structure.getFleetItemsAfterOrders().empty());
}

TEST_CASE("Structure setMonth changes month")
{
  Structure structure(2601, 0, 0, 0, L"tower", L"Tower", false, 3, 7);
  
  structure.setMonth(9);
  
  CHECK(structure.getMonth() == 9);
}

TEST_CASE("Structure setYear changes year")
{
  Structure structure(2701, 0, 0, 0, L"tower", L"Tower", false, 3, 7);
  
  structure.setYear(10);
  
  CHECK(structure.getYear() == 10);
}

TEST_CASE("Structure fleet and after-orders collections are independent")
{
  Structure structure(2801, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  structure.setFleetItem(L"ship", 10);
  structure.setFleetItemAfterOrders(L"ship", 8);
  
  CHECK(structure.getFleetItems().at(L"ship") == 10);
  CHECK(structure.getFleetItemsAfterOrders().at(L"ship") == 8);
}

TEST_CASE("Structure with negative coordinates")
{
  Structure structure(2901, -5, -10, -1, L"tower", L"Tower", false, 1, 1);
  
  CHECK(structure.getXCoordinate() == -5);
  CHECK(structure.getYCoordinate() == -10);
  CHECK(structure.getZCoordinate() == -1);
}

TEST_CASE("Structure with zero coordinates")
{
  Structure structure(3001, 0, 0, 0, L"tower", L"Tower", false, 1, 1);
  
  CHECK(structure.getXCoordinate() == 0);
  CHECK(structure.getYCoordinate() == 0);
  CHECK(structure.getZCoordinate() == 0);
}

TEST_CASE("Structure with large ID values")
{
  Structure structure(999999, 0, 0, 0, L"tower", L"Tower", false, 1, 1);
  
  CHECK(structure.getStructureId() == 999999);
}

TEST_CASE("Structure with empty structure type")
{
  Structure structure(3101, 0, 0, 0, L"", L"Tower", false, 1, 1);
  
  CHECK(structure.getStructureType() == L"");
}

TEST_CASE("Structure with empty structure name")
{
  Structure structure(3201, 0, 0, 0, L"tower", L"", false, 1, 1);
  
  CHECK(structure.getStructureName() == L"");
}

TEST_CASE("Structure with multiple fleet items")
{
  Structure structure(3301, 0, 0, 0, L"fleet", L"Fleet", false, 1, 1);
  
  structure.setFleetItem(L"ship1", 5);
  structure.setFleetItem(L"ship2", 10);
  structure.setFleetItem(L"ship3", 15);
  
  CHECK(structure.getFleetItems().size() == 3);
  CHECK(structure.getFleetItems().at(L"ship1") == 5);
  CHECK(structure.getFleetItems().at(L"ship2") == 10);
  CHECK(structure.getFleetItems().at(L"ship3") == 15);
}

TEST_CASE("Structure update modifies only the target field")
{
  Structure structure(3401, 1, 2, 3, L"tower", L"Guard Tower", false, 5, 6);
  
  structure.setStructureType(L"fort");
  
  CHECK(structure.getStructureId() == 3401);
  CHECK(structure.getXCoordinate() == 1);
  CHECK(structure.getYCoordinate() == 2);
  CHECK(structure.getZCoordinate() == 3);
  CHECK(structure.getStructureName() == L"Guard Tower");
  CHECK(structure.isClosed() == false);
  CHECK(structure.getMonth() == 5);
  CHECK(structure.getYear() == 6);
}
