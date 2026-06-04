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
 * File: StructInfoRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/StructInfo.hpp"

#include <cstddef>
#include <string>
#include <vector>

/**
* @brief Repository for structInfo entities.
*
* Enforces uniqueness on structureType.
*/
class StructInfoRepository
{
public:
  StructInfoRepository() = default;
  ~StructInfoRepository() = default;

  StructInfoRepository(const StructInfoRepository&) = default;
  StructInfoRepository& operator=(const StructInfoRepository&) = default;
  StructInfoRepository(StructInfoRepository&&) = default;
  StructInfoRepository& operator=(StructInfoRepository&&) = default;

  /**
  * @brief Add a new structInfo to the repository.
  *
  * Returns false if a structInfo with the same structureType already exists.
  * Sets lastError_ in that case.
  */
  bool add(std::wstring structureType,
           int needs,
           int magesCapacity,
           bool isShip,
           bool isFlying,
           std::wstring itemIdentifierToken = L"");

  bool addOrUpdateByType(std::wstring structureType,
                         int needs,
                         int magesCapacity,
                         bool isShip,
                         bool isFlying,
                         std::wstring itemIdentifierToken = L"");

  /**
  * @brief Remove a structInfo by its structure type.
  */
  bool removeByType(const std::wstring& structureType);

  /**
  * @brief Find a structure info by structure type.
  */
  StructInfo* findByType(const std::wstring& structureType);

  /**
  * @brief Find a structure info by structure type (const version).
  */
  const StructInfo* findByType(const std::wstring& structureType) const;
  StructInfo* findByItemIdentifierToken(const std::wstring& itemIdentifierToken);
  const StructInfo* findByItemIdentifierToken(const std::wstring& itemIdentifierToken) const;

  void clear();

  std::size_t size() const;
  StructInfo& at(std::size_t index);
  const StructInfo& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<StructInfo> structures_;
  std::wstring           lastError_;
};
