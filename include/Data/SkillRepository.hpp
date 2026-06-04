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
 * File: SkillRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Skill.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Repository for skill entities.
 *
 * Each unique identifier token maps to one Skill object that holds data for
 * all levels (1-5).
 */
class SkillRepository
{
public:
  SkillRepository() = default;
  ~SkillRepository() = default;

  SkillRepository(const SkillRepository&) = default;
  SkillRepository& operator=(const SkillRepository&) = default;
  SkillRepository(SkillRepository&&) = default;
  SkillRepository& operator=(SkillRepository&&) = default;

  // Add level data to a skill, creating the Skill entry if it does not exist.
  // Returns true when the level was added; false when that level already exists
  // for the given token (existing data is preserved).
  bool addLevel(const std::wstring& identifierToken,
                std::wstring name,
                int level,
                std::map<std::wstring, int> productionItems = {},
                std::wstring description = L"");

  bool removeByIdentifier(const std::wstring& identifierToken);

  Skill* findByIdentifier(const std::wstring& identifierToken);
  const Skill* findByIdentifier(const std::wstring& identifierToken) const;

  void clear();

  std::size_t size() const;
  Skill& at(std::size_t index);
  const Skill& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<Skill> skills_;
  std::wstring       lastError_;
};
