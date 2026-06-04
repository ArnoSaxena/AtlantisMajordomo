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
 * File: SkillRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/SkillRepository.hpp"

#include <algorithm>
#include <cwctype>
#include <utility>

namespace
{
  std::wstring upperToken(std::wstring token)
  {
    std::transform(token.begin(), token.end(), token.begin(),
                   [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
    return token;
  }
}

bool SkillRepository::addLevel(const std::wstring& identifierToken,
                               std::wstring name,
                               int level,
                               std::map<std::wstring, int> productionItems,
                               std::wstring description)
{
  const std::wstring normalizedToken = upperToken(identifierToken);

  if (normalizedToken.length() < 3)
  {
    lastError_ = L"Skill identifier token must be at least 3 characters";
    return false;
  }

  if (level < Skill::kMinLevel || level > Skill::kMaxLevel)
  {
    lastError_ = L"Skill level must be between 1 and 5";
    return false;
  }

  Skill* existing = findByIdentifier(normalizedToken);
  if (existing)
  {
    if (existing->hasLevel(level))
    {
      lastError_ = L"Skill level already exists for this token";
      return false;
    }
    existing->setLevelData(level, std::move(productionItems), std::move(description));
    lastError_.clear();
    return true;
  }

  skills_.emplace_back(normalizedToken, std::move(name));
  skills_.back().setLevelData(level, std::move(productionItems), std::move(description));
  lastError_.clear();
  return true;
}

bool SkillRepository::removeByIdentifier(const std::wstring& identifierToken)
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    skills_.begin(),
    skills_.end(),
    [&normalizedToken](const Skill& skill)
    {
      return skill.getIdentifierToken() == normalizedToken;
    }
  );

  if (it == skills_.end())
  {
    lastError_ = L"Skill not found";
    return false;
  }

  skills_.erase(it);
  lastError_.clear();
  return true;
}

Skill* SkillRepository::findByIdentifier(const std::wstring& identifierToken)
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    skills_.begin(),
    skills_.end(),
    [&normalizedToken](const Skill& skill)
    {
      return skill.getIdentifierToken() == normalizedToken;
    }
  );

  return it == skills_.end() ? nullptr : &(*it);
}

const Skill* SkillRepository::findByIdentifier(const std::wstring& identifierToken) const
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    skills_.cbegin(),
    skills_.cend(),
    [&normalizedToken](const Skill& skill)
    {
      return skill.getIdentifierToken() == normalizedToken;
    }
  );

  return it == skills_.cend() ? nullptr : &(*it);
}

void SkillRepository::clear()
{
  skills_.clear();
  lastError_.clear();
}

std::size_t SkillRepository::size() const
{
  return skills_.size();
}

Skill& SkillRepository::at(std::size_t index)
{
  return skills_.at(index);
}

const Skill& SkillRepository::at(std::size_t index) const
{
  return skills_.at(index);
}

const std::wstring& SkillRepository::getLastError() const
{
  return lastError_;
}
