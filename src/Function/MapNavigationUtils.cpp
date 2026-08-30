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
 * File: MapNavigationUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/MapNavigationUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Region.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/StringUtils.hpp"

#include <cstddef>
#include <string>

namespace MapNavigationUtils
{

bool tryParsePositiveUnitNumber(const std::wstring& text, int& unitNumber)
{
  unitNumber = 0;
  const std::wstring trimmed = StringUtils::trimWhitespace(text);
  if (trimmed.empty())
  {
    return false;
  }

  std::size_t parsedLength = 0;
  int parsedValue = 0;
  try
  {
    parsedValue = std::stoi(trimmed, &parsedLength);
  }
  catch (...)
  {
    return false;
  }

  if (parsedLength != trimmed.size() || parsedValue <= 0)
  {
    return false;
  }

  unitNumber = parsedValue;
  return true;
}

std::wstring buildUnitNotFoundMessage(int unitNumber)
{
  return L"Unit " + std::to_wstring(unitNumber) + L" was not found in the database.";
}

UnitSearchResult resolveUnitSearch(const AppData& appData, const std::wstring& unitIdText)
{
  UnitSearchResult result {};

  if (StringUtils::trimWhitespace(unitIdText).empty())
  {
    result.status = UnitSearchStatus::EmptyInput;
    return result;
  }

  int unitNumber = 0;
  if (!tryParsePositiveUnitNumber(unitIdText, unitNumber))
  {
    result.status = UnitSearchStatus::InvalidInput;
    return result;
  }

  if (appData.unitRepository().findByNumber(unitNumber) == nullptr)
  {
    result.status = UnitSearchStatus::NotFound;
    result.unitNumber = unitNumber;
    return result;
  }

  result.status = UnitSearchStatus::Found;
  result.unitNumber = unitNumber;
  return result;
}

bool tryBuildUnitSelectionContext(const AppData& appData, int unitNumber, UnitSelectionContext& context)
{
  if (unitNumber < 0)
  {
    const int unitNewNumber = -unitNumber;
    for (std::size_t index = 0; index < appData.unitNewRepository().size(); ++index)
    {
      const UnitNew& unitNew = appData.unitNewRepository().at(index);
      if (unitNew.getUnitNumber() != unitNewNumber || unitNew.getWarnings().empty())
      {
        continue;
      }

      context.unitNumber = unitNewNumber;
      context.xCoordinate = unitNew.getXCoordinate();
      context.yCoordinate = unitNew.getYCoordinate();
      context.zCoordinate = unitNew.getZCoordinate();
      context.region = appData.regionRepository().findByCoordinates(
        context.xCoordinate, context.yCoordinate, context.zCoordinate);
      return true;
    }
    return false;
  }

  const Unit* unit = appData.unitRepository().findByNumber(unitNumber);
  if (unit == nullptr)
  {
    return false;
  }

  context.unitNumber = unitNumber;
  context.xCoordinate = unit->getXCoordinate();
  context.yCoordinate = unit->getYCoordinate();
  context.zCoordinate = unit->getZCoordinate();

  const Region* region = appData.regionRepository().findByCoordinates(
    context.xCoordinate,
    context.yCoordinate,
    context.zCoordinate);
  if (region == nullptr)
  {
    region = appData.regionRepository().findByCoordinates(
      context.xCoordinate,
      context.yCoordinate);
  }

  context.region = region;
  return true;
}

} // namespace MapNavigationUtils
