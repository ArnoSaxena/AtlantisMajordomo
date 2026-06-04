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
 * File: Skill.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Per-level data for one level of a skill.
 */
struct SkillLevelData
{
  std::map<std::wstring, int> productionItems;
  std::wstring description;
};

/**
 * @brief Prerequisite requirement needed to study a skill.
 *
 * Token-only by design: names must be resolved from SkillRepository on demand
 * and are not persisted outside the Skill object.
 */
struct SkillPrerequisite
{
  std::wstring token;
  int requiredLevel { 0 };
};

/**
 * @brief Data model for a skill entity covering all levels 1-5.
 *
 * All level variants of the same identifier token are stored in one object.
 */
class Skill
{
public:
  static constexpr int kMinLevel = 1;
  static constexpr int kMaxLevel = 5;

  Skill(std::wstring identifierToken, std::wstring name);
  ~Skill() = default;

  Skill(const Skill&) = default;
  Skill& operator=(const Skill&) = default;
  Skill(Skill&&) = default;
  Skill& operator=(Skill&&) = default;

  const std::wstring& getIdentifierToken() const;
  const std::wstring& getName() const;
  void setName(std::wstring name);

  // Set all data for a specific level. Returns false if level is out of range.
  bool setLevelData(int level,
                    std::map<std::wstring, int> productionItems,
                    std::wstring description);

  bool hasLevel(int level) const;
  std::vector<int> getLevels() const;
  const SkillLevelData* getLevelData(int level) const;
  SkillLevelData* getLevelData(int level);

  // Level-specific accessors (return defaults when level data is absent)
  bool isProduction(int level) const;
  const std::map<std::wstring, int>& getProductionItems(int level) const;
  bool isMagic() const;
  bool isMagicFoundation() const;
  const std::wstring& getDescription(int level) const;
  int getStudyCost() const;
  const std::vector<SkillPrerequisite>& getPrerequisites() const;

  // Level-specific setters (silently ignored for invalid or absent levels)
  void setProductionItems(int level, std::map<std::wstring, int> items);
  void setMagic(bool magic);
  void setMagicFoundation(bool magicFoundation);
  void setDescription(int level, std::wstring description);
  void setStudyCost(int studyCost);
  void setPrerequisites(std::vector<SkillPrerequisite> prerequisites);

  // Training utility function
  static int trainingDaysToLevel(int trainingDays);

private:
  std::wstring identifierToken_;
  std::wstring name_;
  bool magic_ { false };
  bool magicFoundation_ { false };
  int studyCost_ { 0 };
  std::array<std::optional<SkillLevelData>, kMaxLevel> levels_;
  std::vector<SkillPrerequisite> prerequisites_;

  static bool isValidLevel(int level);
  static std::size_t levelIndex(int level);
  void refreshDerivedFlags();
};
