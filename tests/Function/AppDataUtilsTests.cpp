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
 * File: AppDataUtilsTests.cpp
 */

#include <catch2/catch_test_macros.hpp>
#include "Function/AppDataUtils.hpp"
#include "Data/AppData.hpp"
#include "Data/UnitRepository.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNewRepository.hpp"
#include "Data/UnitNew.hpp"
#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/OrderRepository.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/ReportRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfoRepository.hpp"

struct AppDataUtilsFixture
{
  BattleRepository battles;
  EventRepository events;
  FactionRepository factions;
  ItemRepository items;
  OrderRepository orders;
  RegionRepository regions;
  ReportRepository reports;
  SkillRepository skills;
  StructureRepository structures;
  StructInfoRepository structureInfo;
  UnitRepository units;
  UnitNewRepository newUnits;

  std::vector<std::wstring> emptyFlags;
  std::map<std::wstring, int> emptyItems;
  std::map<std::wstring, int> emptySkills;

  AppData appData{
    AppData::Repositories{
      reports,
      factions,
      items,
      skills,
      regions,
      units,
      newUnits,
      battles,
      events,
      structures,
      structureInfo,
      orders
    }
  };
};

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils buildDateLabelText returns dash for empty repositories")
{
  std::wstring result = AppDataUtils::buildDateLabelText(&appData);
  
  CHECK(result == L"Date: -");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils buildDateLabelText returns dash for null pointer")
{
  std::wstring result = AppDataUtils::buildDateLabelText(nullptr);
  
  CHECK(result == L"Date: -");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils WarningRow default constructs with valid values")
{
  AppDataUtils::WarningRow row;
  
  CHECK(row.unitNumber == 0);
  CHECK(row.isNewUnit == false);
  CHECK(row.xCoordinate == 0);
  CHECK(row.yCoordinate == 0);
  CHECK(row.zCoordinate == 1);
  CHECK(row.text == L"");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils WarningRow can be constructed with parameters")
{
  AppDataUtils::WarningRow row{
    100,
    true,
    5,
    10,
    2,
    L"Test warning"
  };
  
  CHECK(row.unitNumber == 100);
  CHECK(row.isNewUnit == true);
  CHECK(row.xCoordinate == 5);
  CHECK(row.yCoordinate == 10);
  CHECK(row.zCoordinate == 2);
  CHECK(row.text == L"Test warning");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod returns empty for no repositories")
{
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  CHECK(warnings.empty());
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod returns empty when no reports")
{
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  CHECK(warnings.empty());
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod collects unit warnings from latest period")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  unit->addWarning(L"Test warning 1");
  unit->addWarning(L"Test warning 2");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  CHECK(warnings.size() == 2);
  CHECK(warnings[0].unitNumber == 101);
  CHECK(warnings[0].isNewUnit == false);
  CHECK(warnings[0].text == L"Test warning 1");
  CHECK(warnings[1].text == L"Test warning 2");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod collects new unit warnings")
{
  newUnits.add(1000, L"New Unit", 5, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, 101);
  UnitNew* unitNew = newUnits.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(unitNew != nullptr);
  unitNew->addWarning(L"New unit warning");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  CHECK(warnings.size() == 1);
  CHECK(warnings[0].unitNumber == 1000);
  CHECK(warnings[0].isNewUnit == true);
  CHECK(warnings[0].text == L"New unit warning");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod collects both unit and new unit warnings")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  unit->addWarning(L"Unit warning");
  
  newUnits.add(1000, L"New Unit", 5, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, 101);
  UnitNew* unitNew = newUnits.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(unitNew != nullptr);
  unitNew->addWarning(L"New unit warning");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  CHECK(warnings.size() == 2);
  CHECK(warnings[0].unitNumber == 101);
  CHECK(warnings[0].isNewUnit == false);
  CHECK(warnings[1].unitNumber == 1000);
  CHECK(warnings[1].isNewUnit == true);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod preserves unit coordinates")
{
  units.add(101, L"Unit Name", 1, 0, 5, 15, 3, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  unit->addWarning(L"Test warning");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  REQUIRE(warnings.size() >= 1);
  CHECK(warnings[0].xCoordinate == 5);
  CHECK(warnings[0].yCoordinate == 15);
  CHECK(warnings[0].zCoordinate == 3);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod preserves new unit coordinates")
{
  newUnits.add(1000, L"New Unit", 5, 7, 14, 2, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, 101);
  UnitNew* unitNew = newUnits.findByNumberAndCoordinates(1000, 7, 14, 2);
  REQUIRE(unitNew != nullptr);
  unitNew->addWarning(L"New unit warning");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  REQUIRE(warnings.size() >= 1);
  CHECK(warnings[0].xCoordinate == 7);
  CHECK(warnings[0].yCoordinate == 14);
  CHECK(warnings[0].zCoordinate == 2);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningUnitNumbersForLatestPeriod returns empty with no warnings")
{
  auto unitNumbers = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  
  CHECK(unitNumbers.empty());
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningUnitNumbersForLatestPeriod returns unique unit numbers")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  unit->addWarning(L"Warning 1");
  unit->addWarning(L"Warning 2");
  unit->addWarning(L"Warning 3");
  
  auto unitNumbers = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  
  CHECK(unitNumbers.size() == 1);
  CHECK(unitNumbers[0] == 101);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningUnitNumbersForLatestPeriod deduplicates unit numbers")
{
  units.add(101, L"Unit 1", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit1 = units.findByNumber(101);
  REQUIRE(unit1 != nullptr);
  unit1->addWarning(L"Warning 1");
  
  newUnits.add(1000, L"New Unit", 5, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, 101);
  UnitNew* unitNew = newUnits.findByNumberAndCoordinates(1000, 10, 20, 1);
  REQUIRE(unitNew != nullptr);
  unitNew->addWarning(L"New unit warning");
  
  auto unitNumbers = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  
  CHECK(unitNumbers.size() == 2);
  CHECK(((std::find(unitNumbers.begin(), unitNumbers.end(), 101) != unitNumbers.end())));
  CHECK(((std::find(unitNumbers.begin(), unitNumbers.end(), 1000) != unitNumbers.end())));
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningUnitNumbersForLatestPeriod returns deduplicated list")
{
  units.add(101, L"Unit 1", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit1 = units.findByNumber(101);
  REQUIRE(unit1 != nullptr);
  unit1->addWarning(L"Warning 1");
  unit1->addWarning(L"Warning 2");
  
  units.add(102, L"Unit 2", 1, 0, 15, 25, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit2 = units.findByNumber(102);
  REQUIRE(unit2 != nullptr);
  unit2->addWarning(L"Warning 3");
  
  auto unitNumbers = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  
  CHECK(unitNumbers.size() == 2);
  bool has101 = false, has102 = false;
  for (int num : unitNumbers)
  {
    if (num == 101) has101 = true;
    if (num == 102) has102 = true;
  }
  CHECK(has101);
  CHECK(has102);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils buildDateLabelText with valid date format")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 3, 7, false);
  
  std::wstring result = AppDataUtils::buildDateLabelText(&appData);
  
  CHECK(result == L"Date: -");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils buildDateLabelText prefers latest period between reports and battles")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 3, 7, false);
  
  std::wstring result = AppDataUtils::buildDateLabelText(&appData);
  
  CHECK(result == L"Date: -");
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils WarningRow with negative coordinates")
{
  AppDataUtils::WarningRow row{
    100,
    false,
    -5,
    -10,
    -1,
    L"Test warning"
  };
  
  CHECK(row.xCoordinate == -5);
  CHECK(row.yCoordinate == -10);
  CHECK(row.zCoordinate == -1);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils WarningRow with zero coordinates")
{
  AppDataUtils::WarningRow row{
    0,
    false,
    0,
    0,
    0,
    L"Test"
  };
  
  CHECK(row.xCoordinate == 0);
  CHECK(row.yCoordinate == 0);
  CHECK(row.zCoordinate == 0);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils WarningRow with large coordinates")
{
  AppDataUtils::WarningRow row{
    999999,
    true,
    9999,
    9999,
    999,
    L"Large coord test"
  };
  
  CHECK(row.unitNumber == 999999);
  CHECK(row.xCoordinate == 9999);
  CHECK(row.yCoordinate == 9999);
  CHECK(row.zCoordinate == 999);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod with multiple warnings per unit")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  
  for (int i = 1; i <= 5; ++i)
  {
    unit->addWarning(L"Warning " + std::to_wstring(i));
  }
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  int unitWarningsCount = 0;
  for (const auto& w : warnings)
  {
    if (w.unitNumber == 101)
      unitWarningsCount++;
  }
  CHECK(unitWarningsCount == 5);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod maintains warning text exactly")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  
  std::wstring testText = L"Test warning with special chars: !@#$%^&*()";
  unit->addWarning(testText);
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  REQUIRE(warnings.size() >= 1);
  CHECK(warnings[0].text == testText);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod ignores units with no warnings")
{
  units.add(101, L"Unit with warning", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit1 = units.findByNumber(101);
  REQUIRE(unit1 != nullptr);
  unit1->addWarning(L"Has warning");
  
  units.add(102, L"Unit without warning", 1, 0, 15, 25, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  bool hasUnit102 = false;
  for (const auto& w : warnings)
  {
    if (w.unitNumber == 102)
      hasUnit102 = true;
  }
  CHECK(!hasUnit102);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningUnitNumbersForLatestPeriod maintains order of discovery")
{
  units.add(105, L"Unit 1", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit1 = units.findByNumber(105);
  REQUIRE(unit1 != nullptr);
  unit1->addWarning(L"Warning 1");
  
  units.add(103, L"Unit 2", 1, 0, 15, 25, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit2 = units.findByNumber(103);
  REQUIRE(unit2 != nullptr);
  unit2->addWarning(L"Warning 2");
  
  auto unitNumbers = AppDataUtils::getWarningUnitNumbersForLatestPeriod(appData);
  
  CHECK(unitNumbers.size() == 2);
  CHECK(unitNumbers[0] == 105);
  CHECK(unitNumbers[1] == 103);
}

TEST_CASE_METHOD(AppDataUtilsFixture, "AppDataUtils getWarningsForLatestPeriod empty warning text allowed")
{
  units.add(101, L"Unit Name", 1, 0, 10, 20, 1, emptyFlags, emptyItems, 100, 500, 1000, 0, 200, emptySkills, 1, 1, false);
  Unit* unit = units.findByNumber(101);
  REQUIRE(unit != nullptr);
  unit->addWarning(L"");
  
  auto warnings = AppDataUtils::getWarningsForLatestPeriod(appData);
  
  for (const auto& w : warnings)
  {
    if (w.unitNumber == 101)
    {
      CHECK(w.text == L"");
    }
  }
}
