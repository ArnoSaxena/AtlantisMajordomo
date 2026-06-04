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
 * File: StructInfo.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/StructInfo.hpp"
#include "Function/StringUtils.hpp"

#include <cwctype>


StructInfo::StructInfo(std::wstring structureType,
                    int needs,
                    int magesCapacity,
                    bool isShip,
                    bool isFlying)
  : structureType_(std::move(structureType))
  , needs_(needs)
  , magesCapacity_(magesCapacity)
  , isShip_(isShip)
  , isFlying_(isFlying)
{
}

const std::wstring& StructInfo::getStructureType() const
{
  return structureType_;
}

int StructInfo::getNeeds() const
{
  return needs_;
}

int StructInfo::getMagesCapacity() const
{
  return magesCapacity_;
}

bool StructInfo::isShip() const
{
  return isShip_;
}

bool StructInfo::protoIsFlying() const
{
  return isFlying_;
}

const std::wstring& StructInfo::getItemIdentifierToken() const
{
  return itemIdentifierToken_;
}

void StructInfo::setStructureType(std::wstring structureType)
{
  structureType_ = std::move(structureType);
}

void StructInfo::setNeeds(int needs)
{
  needs_ = needs;
}

void StructInfo::setMagesCapacity(int magesCapacity)
{
  magesCapacity_ = magesCapacity;
}

void StructInfo::setIsShip(bool isShip)
{
  isShip_ = isShip;
}

void StructInfo::setIsFlying(bool isFlying)
{
  isFlying_ = isFlying;
}

void StructInfo::setItemIdentifierToken(std::wstring itemIdentifierToken)
{
  itemIdentifierToken_ = std::move(itemIdentifierToken);
}


std::wstring StructInfo::extractRoadDirectionFromStructureType(const std::wstring& structureType)
{
  const std::wstring trimmedType = StringUtils::trimWhitespace(structureType);
  const std::wstring lowerType = StringUtils::toLower(trimmedType);
  const std::wstring roadPrefix = L"road";

  if (lowerType.size() < roadPrefix.size() || lowerType.rfind(roadPrefix, 0) != 0)
  {
    return L"";
  }

  std::wstring suffix = StringUtils::trimWhitespace(trimmedType.substr(roadPrefix.size()));
  while (!suffix.empty())
  {
    const wchar_t last = suffix.back();
    if (last == L'.' || last == L',' || last == L';' || last == L':')
    {
      suffix.pop_back();
      suffix = StringUtils::trimWhitespace(suffix);
      continue;
    }
    break;
  }

  // no need for "normalising" as the road type is ALWAYS "Road" with upper case direction abbrevation.
  return suffix;
}