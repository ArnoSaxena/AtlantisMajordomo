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
 * File: Skill.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Skill.hpp"

#include <utility>

Skill::Skill(std::wstring identifierToken, std::wstring name)
  : identifierToken_(std::move(identifierToken))
  , name_(std::move(name))
{
}

bool Skill::isValidLevel(int level)
{
  return level >= kMinLevel && level <= kMaxLevel;
}

std::size_t Skill::levelIndex(int level)
{
  return static_cast<std::size_t>(level - 1);
}

const std::wstring& Skill::getIdentifierToken() const
{
  return identifierToken_;
}

const std::wstring& Skill::getName() const
{
  return name_;
}

void Skill::setName(std::wstring name)
{
  name_ = std::move(name);
}

bool Skill::setLevelData(int level,
                         std::map<std::wstring, int> productionItems,
                         std::wstring description)
{
  if (!isValidLevel(level))
    return false;
  SkillLevelData data;
  data.productionItems = std::move(productionItems);
  data.description    = std::move(description);
  levels_[levelIndex(level)] = std::move(data);
  refreshDerivedFlags();
  return true;
}

bool Skill::hasLevel(int level) const
{
  if (!isValidLevel(level))
    return false;
  return levels_[levelIndex(level)].has_value();
}

std::vector<int> Skill::getLevels() const
{
  std::vector<int> result;
  for (int l = kMinLevel; l <= kMaxLevel; ++l)
    if (levels_[levelIndex(l)].has_value())
      result.push_back(l);
  return result;
}

const SkillLevelData* Skill::getLevelData(int level) const
{
  if (!isValidLevel(level))
    return nullptr;
  const auto& opt = levels_[levelIndex(level)];
  return opt.has_value() ? &opt.value() : nullptr;
}

SkillLevelData* Skill::getLevelData(int level)
{
  if (!isValidLevel(level))
    return nullptr;
  auto& opt = levels_[levelIndex(level)];
  return opt.has_value() ? &opt.value() : nullptr;
}

bool Skill::isProduction(int level) const
{
  const auto* d = getLevelData(level);
  return d ? !d->productionItems.empty() : false;
}

const std::map<std::wstring, int>& Skill::getProductionItems(int level) const
{
  const auto* d = getLevelData(level);
  static const std::map<std::wstring, int> empty;
  return d ? d->productionItems : empty;
}

bool Skill::isMagic() const
{
  return magic_;
}

bool Skill::isMagicFoundation() const
{
  return magicFoundation_;
}

const std::wstring& Skill::getDescription(int level) const
{
  const auto* d = getLevelData(level);
  static const std::wstring empty;
  return d ? d->description : empty;
}

int Skill::getStudyCost() const
{
  return studyCost_;
}

const std::vector<SkillPrerequisite>& Skill::getPrerequisites() const
{
  return prerequisites_;
}

void Skill::setProductionItems(int level, std::map<std::wstring, int> items)
{
  auto* d = getLevelData(level);
  if (d) d->productionItems = std::move(items);
}

void Skill::setMagic(bool magic)
{
  magic_ = magic;
  refreshDerivedFlags();
}

void Skill::setMagicFoundation(bool magicFoundation)
{
  magicFoundation_ = magicFoundation;
  refreshDerivedFlags();
}

void Skill::setDescription(int level, std::wstring description)
{
  auto* d = getLevelData(level);
  if (d)
  {
    d->description = std::move(description);
    refreshDerivedFlags();
  }
}

void Skill::setStudyCost(int studyCost)
{
  studyCost_ = studyCost < 0 ? 0 : studyCost;
}

void Skill::setPrerequisites(std::vector<SkillPrerequisite> prerequisites)
{
  prerequisites_ = std::move(prerequisites);
}

void Skill::refreshDerivedFlags()
{
  if (magicFoundation_)
  {
    magic_ = true;
  }
}

int Skill::trainingDaysToLevel(int trainingDays)
{
  if (trainingDays < 30)
  {
    return 0;
  }

  int cumulative = 0;
  for (int i = 1; i <= 5; ++i)
  {
    cumulative += i * 30;
    if (trainingDays < cumulative)
    {
      return i - 1;
    }
  }
  return 5;
}
