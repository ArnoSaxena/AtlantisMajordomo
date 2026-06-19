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
 * File: SkillFormattingUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/SkillFormattingUtils.hpp"

#include "Data/Skill.hpp"
#include "Function/StringUtils.hpp"

#include <regex>
#include <sstream>
#include <string>

namespace SkillFormattingUtils
{

std::wstring formatSkills(const std::map<std::wstring, int>& skills)
{
  std::wstring result;
  bool first = true;
  for (const auto& [skillToken, days] : skills)
  {
    if (!first)
    {
      result += L", ";
    }
    const int level = Skill::trainingDaysToLevel(days);
    result += skillToken + L" [" + skillToken + L"] " + std::to_wstring(level) +
              L" (" + std::to_wstring(days) + L")";
    first = false;
  }
  return result;
}

std::wstring formatPrerequisites(const std::vector<SkillPrerequisite>& prerequisites)
{
  std::wstring result;
  bool first = true;
  for (const auto& prerequisite : prerequisites)
  {
    if (!first)
    {
      result += L"\r\n";
    }
    result += prerequisite.token + L":" + std::to_wstring(prerequisite.requiredLevel);
    first = false;
  }
  return result;
}

std::vector<SkillPrerequisite> parsePrerequisites(const std::wstring& text)
{
  std::vector<SkillPrerequisite> prerequisites;

  std::wstringstream stream(text);
  std::wstring line;
  static const std::wregex linePattern(L"^([A-Za-z0-9]{3,})\\s*:\\s*(\\d+)\\s*$");
  while (std::getline(stream, line))
  {
    if (!line.empty() && line.back() == L'\r')
    {
      line.pop_back();
    }

    line = StringUtils::trimWhitespace(line);
    if (line.empty())
    {
      continue;
    }

    std::wsmatch match;
    if (!std::regex_match(line, match, linePattern))
    {
      continue;
    }

    SkillPrerequisite prerequisite;
    prerequisite.token = StringUtils::trimWhitespace(match[1].str());
    prerequisite.requiredLevel = StringUtils::parseIntSafe(match[2].str());
    if (!prerequisite.token.empty() && prerequisite.requiredLevel > 0)
    {
      prerequisites.push_back(std::move(prerequisite));
    }
  }

  return prerequisites;
}

} // namespace SkillFormattingUtils
