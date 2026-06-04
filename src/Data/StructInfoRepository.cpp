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
 * File: StructInfoRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/StructInfoRepository.hpp"

#include <algorithm>
#include <cwctype>

namespace
{
std::wstring normalizeStructureType(const std::wstring& value)
{
  std::wstring normalized = value;
  std::transform(normalized.begin(), normalized.end(), normalized.begin(), towlower);
  return normalized;
}
}

bool StructInfoRepository::add(std::wstring structureType,
                              int needs,
                              int magesCapacity,
                              bool isShip,
                              bool isFlying,
                              std::wstring itemIdentifierToken)
{
  // Check if a structure info with this type already exists.
  if (findByType(structureType) != nullptr)
  {
    lastError_ = L"A structure info with the same type already exists";
    return false;
  }

  structures_.emplace_back(std::move(structureType),
                          needs,
                          magesCapacity,
                          isShip,
                          isFlying);
  if (!structures_.empty())
  {
    structures_.back().setItemIdentifierToken(std::move(itemIdentifierToken));
  }
  lastError_.clear();
  return true;
}

bool StructInfoRepository::addOrUpdateByType(std::wstring structureType,
                                             int needs,
                                             int magesCapacity,
                                             bool isShip,
                                             bool isFlying,
                                             std::wstring itemIdentifierToken)
{
  StructInfo* existing = findByType(structureType);
  if (existing == nullptr)
  {
    return add(std::move(structureType),
              needs,
              magesCapacity,
              isShip,
              isFlying,
              std::move(itemIdentifierToken));
  }

  existing->setStructureType(std::move(structureType));
  existing->setNeeds(needs);
  existing->setMagesCapacity(magesCapacity);
  existing->setIsShip(isShip);
  existing->setIsFlying(isFlying);
  existing->setItemIdentifierToken(std::move(itemIdentifierToken));
  lastError_.clear();
  return true;
}

bool StructInfoRepository::removeByType(const std::wstring& structureType)
{
  const std::wstring normalized = normalizeStructureType(structureType);
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [&normalized](const StructInfo& s)
                        {
                          return normalizeStructureType(s.getStructureType()) == normalized;
                        });

  if (it != structures_.end())
  {
    structures_.erase(it);
    lastError_.clear();
    return true;
  }

  lastError_ = L"Structure Info not found";
  return false;
}

StructInfo* StructInfoRepository::findByType(const std::wstring& structureType)
{
  const std::wstring normalized = normalizeStructureType(structureType);
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [&normalized](const StructInfo& s)
                        {
                          return normalizeStructureType(s.getStructureType()) == normalized;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

const StructInfo* StructInfoRepository::findByType(const std::wstring& structureType) const
{
  const std::wstring normalized = normalizeStructureType(structureType);
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [&normalized](const StructInfo& s)
                        {
                          return normalizeStructureType(s.getStructureType()) == normalized;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

StructInfo* StructInfoRepository::findByItemIdentifierToken(const std::wstring& itemIdentifierToken)
{
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [&itemIdentifierToken](const StructInfo& s)
                        {
                          return s.getItemIdentifierToken() == itemIdentifierToken;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

const StructInfo* StructInfoRepository::findByItemIdentifierToken(const std::wstring& itemIdentifierToken) const
{
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [&itemIdentifierToken](const StructInfo& s)
                        {
                          return s.getItemIdentifierToken() == itemIdentifierToken;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

void StructInfoRepository::clear()
{
  structures_.clear();
  lastError_.clear();
}

std::size_t StructInfoRepository::size() const
{
  return structures_.size();
}

StructInfo& StructInfoRepository::at(std::size_t index)
{
  return structures_.at(index);
}

const StructInfo& StructInfoRepository::at(std::size_t index) const
{
  return structures_.at(index);
}

const std::wstring& StructInfoRepository::getLastError() const
{
  return lastError_;
}
