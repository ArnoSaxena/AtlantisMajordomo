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
 * File: OrderWarningService.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/OrderWarningService.hpp"

#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/Faction.hpp"
#include "Data/Item.hpp"
#include "Data/Skill.hpp"
#include "Data/Structure.hpp"
#include "Data/Unit.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
using OrderParsingUtils::isMonthLongOrderLine;
using OrderParsingUtils::normalizeItemTokenForWarning;
using OrderParsingUtils::tokenizeOrderLine;
using OrderParsingUtils::tryExtractOrderKeywordUpper;
using OrderParsingUtils::tryParseGiveTakeOrder;
using OrderParsingUtils::tryParseIntStrict;
using OrderParsingUtils::tryResolveItemTokenForWarning;
using WarningGiveMode = OrderParsingUtils::WarningGiveMode;
using WarningGiveTakeOrder = OrderParsingUtils::WarningGiveTakeOrder;


const Item* findItemByTokenNormalizedForWarning(const AppData& appData, const std::wstring& itemToken)
{
  const std::wstring normalizedToken = normalizeItemTokenForWarning(itemToken);
  if (normalizedToken.empty())
  {
    return nullptr;
  }

  if (const Item* exact = appData.itemRepository().findByIdentifierToken(normalizedToken))
  {
    return exact;
  }

  for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
  {
    const Item& candidate = appData.itemRepository().at(index);
    if (normalizeItemTokenForWarning(candidate.getIdentifierToken()) == normalizedToken)
    {
      return &candidate;
    }
  }

  return nullptr;
}

const StructInfo* findStructInfoForStructure(const AppData& appData, const Structure& structure)
{
  return appData.structInfoRepository().findByType(structure.getStructureType());
}

bool tryExtractStructureIdFromOrder(const std::wstring& orderLine,
                                    const std::wstring& keyword,
                                    int& structureId)
{
  if (orderLine.length() <= keyword.length())
  {
    return false;
  }

  std::wstring afterKeyword = orderLine.substr(keyword.length());
  const auto start = afterKeyword.find_first_not_of(L" \t");
  if (start == std::wstring::npos)
  {
    return false;
  }

  afterKeyword = afterKeyword.substr(start);
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!tokenizeOrderLine(afterKeyword, tokens, tokenWasQuoted))
  {
    return false;
  }

  for (auto it = tokens.rbegin(); it != tokens.rend(); ++it)
  {
    const std::wstring upperToken = StringUtils::toUpper(*it);
    if (upperToken == L"IN")
    {
      continue;
    }

    try
    {
      const int parsedId = std::stoi(*it);
      if (parsedId > 0)
      {
        structureId = parsedId;
        return true;
      }
    }
    catch (...)
    {
    }
  }

  return false;
}

int getManItemSkillLimitForWarning(const Item& item, const std::wstring& skillToken)
{
  const std::wstring normalizedSkillToken = normalizeItemTokenForWarning(skillToken);
  for (const auto& [limitedSkillToken, levelLimit] : item.getSkillsMax())
  {
    if (normalizeItemTokenForWarning(limitedSkillToken) == normalizedSkillToken)
    {
      return levelLimit > 0 ? levelLimit : 2;
    }
  }

  const int defaultLimit = item.getDefaultSkillMax();
  return defaultLimit > 0 ? defaultLimit : 2;
}

std::optional<int> resolveUnitSkillLevelLimitForWarning(const AppData& appData,
                                                        const std::map<std::wstring, int>& itemCounts,
                                                        const std::wstring& skillToken)
{
  std::optional<int> lowestLimit;

  for (const auto& [itemToken, amount] : itemCounts)
  {
    if (amount <= 0)
    {
      continue;
    }

    const Item* item = findItemByTokenNormalizedForWarning(appData, itemToken);
    if (!item || !item->isMan())
    {
      continue;
    }

    const int itemLimit = getManItemSkillLimitForWarning(*item, skillToken);
    if (!lowestLimit.has_value())
    {
      lowestLimit = itemLimit;
    }
    else
    {
      lowestLimit = (std::min)(*lowestLimit, itemLimit);
    }
  }

  return lowestLimit;
}

int maxTrainingDaysForLevelLimitForWarning(int level)
{
  if (level <= 0)
  {
    return 0;
  }

  const int clampedLevel = (std::min)(level, Skill::kMaxLevel);
  int maxDaysAtLimit = 0;
  for (int rank = 1; rank <= clampedLevel; ++rank)
  {
    maxDaysAtLimit += rank * 30;
  }

  return maxDaysAtLimit;
}

int resolveStudyCostPerManMonth(const Skill& skill)
{
  return skill.getStudyCost();
}

bool checkStudyPrerequisitesForWarning(const Unit& unit,
                                      const Skill& studiedSkill,
                                      std::vector<SkillPrerequisite>& failedPrerequisites)
{
  failedPrerequisites.clear();

  for (const SkillPrerequisite& prerequisite : studiedSkill.getPrerequisites())
  {
    if (prerequisite.token.empty() || prerequisite.requiredLevel <= 0)
    {
      continue;
    }

    const int knownLevel = Skill::trainingDaysToLevel(unit.getSkillDays(prerequisite.token));
    if (knownLevel < prerequisite.requiredLevel)
    {
      failedPrerequisites.push_back(prerequisite);
    }
  }

  return failedPrerequisites.empty();
}

int countManItemsByTokenForWarning(const AppData& appData,
                                   const std::map<std::wstring, int>& itemCounts,
                                   const std::wstring& requiredToken,
                                   int& totalManCount)
{
  totalManCount = 0;
  int matchingTokenCount = 0;
  const std::wstring normalizedRequiredToken = normalizeItemTokenForWarning(requiredToken);

  for (const auto& [itemToken, count] : itemCounts)
  {
    if (count <= 0)
    {
      continue;
    }

    const Item* item = findItemByTokenNormalizedForWarning(appData, itemToken);
    if (!item || !item->isMan())
    {
      continue;
    }

    totalManCount += count;
    if (normalizeItemTokenForWarning(item->getIdentifierToken()) == normalizedRequiredToken)
    {
      matchingTokenCount += count;
    }
  }

  return matchingTokenCount;
}

int resolveMageStudyAboveLevel2CapacityForWarning(const AppData& appData, const Unit& unit)
{
  if (unit.getStructureId() == 0)
  {
    return 0;
  }

  const Structure* locationStructure = appData.structureRepository().findByIdAndCoordinates(
    unit.getStructureId(),
    unit.getXCoordinate(),
    unit.getYCoordinate(),
    unit.getZCoordinate());

  if (locationStructure)
  {
    if (const StructInfo* info = findStructInfoForStructure(appData, *locationStructure))
    {
      return info->getMagesCapacity();
    }
  }

  if (!locationStructure)
  {
    return 0;
  }

  const std::wstring normalizedType = normalizeItemTokenForWarning(locationStructure->getStructureType());
  int maxCapacity = 0;
  for (std::size_t index = 0; index < appData.structInfoRepository().size(); ++index)
  {
    const StructInfo& candidate = appData.structInfoRepository().at(index);
    if (normalizeItemTokenForWarning(candidate.getStructureType()) != normalizedType)
    {
      continue;
    }

    maxCapacity = std::max(maxCapacity, candidate.getMagesCapacity());
  }

  return maxCapacity;
}

bool tryParseStudyOrder(const std::wstring& orderLine, std::wstring& skillOperand)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return false;
  }

  std::size_t tokenIndex = 0;
  if (!tokens.front().empty() && tokens.front().front() == L'@')
  {
    if (tokens.front().size() == 1)
    {
      tokenIndex = 1;
    }
    else
    {
      tokens.front().erase(tokens.front().begin());
    }
  }

  if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"STUDY")
  {
    return false;
  }

  ++tokenIndex;
  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  skillOperand = StringUtils::trimWhitespace(tokens[tokenIndex]);
  return !skillOperand.empty();
}

const Skill* tryResolveStudySkill(const AppData& appData,
                                  const std::wstring& skillOperand,
                                  std::wstring& resolvedSkillToken)
{
  const std::wstring trimmedOperand = StringUtils::trimWhitespace(skillOperand);
  const std::wstring normalizedOperand = StringUtils::toUpper(trimmedOperand);
  const std::wstring normalizedTokenOperand = normalizeItemTokenForWarning(trimmedOperand);

  if (normalizedOperand.empty() && normalizedTokenOperand.empty())
  {
    return nullptr;
  }

  if (!normalizedTokenOperand.empty())
  {
    if (const Skill* byIdentifier = appData.skillRepository().findByIdentifier(normalizedTokenOperand))
    {
      resolvedSkillToken = byIdentifier->getIdentifierToken();
      return byIdentifier;
    }
  }

  if (const Skill* byIdentifier = appData.skillRepository().findByIdentifier(normalizedOperand))
  {
    resolvedSkillToken = byIdentifier->getIdentifierToken();
    return byIdentifier;
  }

  for (std::size_t index = 0; index < appData.skillRepository().size(); ++index)
  {
    const Skill& skill = appData.skillRepository().at(index);
    const std::wstring normalizedIdentifier = normalizeItemTokenForWarning(skill.getIdentifierToken());
    if ((!normalizedTokenOperand.empty() && normalizedIdentifier == normalizedTokenOperand) ||
      (!normalizedOperand.empty() && normalizedIdentifier == normalizedOperand))
    {
      resolvedSkillToken = skill.getIdentifierToken();
      return &skill;
    }
  }

  for (std::size_t index = 0; index < appData.skillRepository().size(); ++index)
  {
    const Skill& skill = appData.skillRepository().at(index);
    if (StringUtils::toUpper(StringUtils::trimWhitespace(skill.getName())) == normalizedOperand)
    {
      resolvedSkillToken = skill.getIdentifierToken();
      return &skill;
    }
  }

  return nullptr;
}

void checkGiveTakeWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  std::map<int, Unit*> mainUnits;
  std::map<int, std::map<std::wstring, int>> availableByUnit;
  std::set<int> unitsWithGiveTakeOrders;

  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    mainUnits[unit->getUnitNumber()] = unit;

    std::map<std::wstring, int> normalizedCounts;
    for (const auto& [token, amount] : unit->getItems())
    {
      if (amount <= 0)
      {
        continue;
      }

      const std::wstring normalizedToken = normalizeItemTokenForWarning(token);
      if (!normalizedToken.empty())
      {
        normalizedCounts[normalizedToken] += amount;
      }
    }

    availableByUnit[unit->getUnitNumber()] = std::move(normalizedCounts);
  }

  for (const auto& [unitNumber, unit] : mainUnits)
  {
    if (!unit)
    {
      continue;
    }

    for (const std::wstring& orderLine : unit->getOrders())
    {
      WarningGiveTakeOrder parsedOrder;
      if (!tryParseGiveTakeOrder(orderLine, parsedOrder))
      {
        continue;
      }

      unitsWithGiveTakeOrders.insert(unitNumber);

      std::wstring resolvedToken;
      if (!tryResolveItemTokenForWarning(appData,
                                         parsedOrder.itemOperand,
                                         parsedOrder.itemOperandWasQuoted,
                                         resolvedToken))
      {
        continue;
      }

      int sourceUnitNumber = unitNumber;
      int destinationUnitNumber = parsedOrder.otherUnitNumber;
      if (parsedOrder.isTake)
      {
        sourceUnitNumber = parsedOrder.otherUnitNumber;
        destinationUnitNumber = unitNumber;
      }

      auto sourceIt = availableByUnit.find(sourceUnitNumber);
      if (sourceIt == availableByUnit.end())
      {
        unit->addWarning(orderLine);
        continue;
      }

      std::map<std::wstring, int>& sourceCounts = sourceIt->second;
      const int availableAmount = sourceCounts.count(resolvedToken) > 0 ? sourceCounts[resolvedToken] : 0;

      int transferAmount = 0;
      bool invalidAmount = false;
      switch (parsedOrder.mode)
      {
        case WarningGiveMode::Quantity:
          transferAmount = parsedOrder.quantity;
          invalidAmount = availableAmount < transferAmount;
          break;
        case WarningGiveMode::All:
          transferAmount = availableAmount;
          break;
        case WarningGiveMode::AllExcept:
          transferAmount = (std::max)(0, availableAmount - parsedOrder.exceptQuantity);
          break;
      }

      if (invalidAmount)
      {
        unit->addWarning(orderLine);
        continue;
      }

      if (transferAmount <= 0)
      {
        continue;
      }

      const int remainingAmount = availableAmount - transferAmount;
      if (remainingAmount > 0)
      {
        sourceCounts[resolvedToken] = remainingAmount;
      }
      else
      {
        sourceCounts.erase(resolvedToken);
      }

      auto destinationIt = availableByUnit.find(destinationUnitNumber);
      if (destinationIt != availableByUnit.end())
      {
        destinationIt->second[resolvedToken] += transferAmount;
      }
    }
  }

  for (const auto& [unitNumber, finalCounts] : availableByUnit)
  {
    if (unitsWithGiveTakeOrders.find(unitNumber) == unitsWithGiveTakeOrders.end())
    {
      continue;
    }

    const int finalManCount = appData.itemRepository().calculateManItemCount(finalCounts);
    if (finalManCount > 0)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(unitNumber);
    if (unit)
    {
      unit->addWarning(L"GIVE/TAKE leaves unit with no men");
    }
  }
}

void checkStudyWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    std::map<std::wstring, int> estimatedCounts =
      Commands::calculateAfterCommandItemCountsForUnit(appData, *unit);

    int estimatedSilver = 0;
    const auto estimatedSilverIt = estimatedCounts.find(L"SILV");
    if (estimatedSilverIt != estimatedCounts.end())
    {
      estimatedSilver = estimatedSilverIt->second;
    }

    int manCount = appData.itemRepository().calculateManItemCount(estimatedCounts);
    if (manCount <= 0)
    {
      // Keep warning behavior aligned with simulation when man metadata is incomplete.
      if (!estimatedCounts.empty())
      {
        manCount = 1;
      }
      else
      {
        continue;
      }
    }

    std::map<std::wstring, int> estimatedSkillDays = unit->getSkills();

    for (const std::wstring& orderLine : unit->getOrders())
    {
      std::wstring skillOperand;
      if (!tryParseStudyOrder(orderLine, skillOperand))
      {
        continue;
      }

      std::wstring resolvedSkillToken;
      const Skill* skillObj = tryResolveStudySkill(appData, skillOperand, resolvedSkillToken);
      if (!skillObj)
      {
        continue;
      }

      std::vector<SkillPrerequisite> failedPrerequisites;
      if (!checkStudyPrerequisitesForWarning(*unit,
                                            *skillObj,
                                            failedPrerequisites))
      {
        std::wstring failedPrerequisitesText;
        for (std::size_t prerequisiteIndex = 0; prerequisiteIndex < failedPrerequisites.size(); ++prerequisiteIndex)
        {
          if (prerequisiteIndex > 0)
          {
            failedPrerequisitesText += L", ";
          }

          failedPrerequisitesText += failedPrerequisites[prerequisiteIndex].token +
                                     L" " +
                                     std::to_wstring(failedPrerequisites[prerequisiteIndex].requiredLevel);
        }

        unit->addWarning(L"prerequisites not met for STUDY " + resolvedSkillToken + L": " + failedPrerequisitesText);
        continue;
      }

      const int currentDays = estimatedSkillDays.count(resolvedSkillToken) > 0 ? estimatedSkillDays[resolvedSkillToken] : 0;
      const int proposedDays = currentDays + 30;
      const int proposedLevel = Skill::trainingDaysToLevel(proposedDays);

      if (skillObj->isMagic())
      {
        int totalManCount = 0;
        const int leadManCount = countManItemsByTokenForWarning(appData, estimatedCounts, L"LEAD", totalManCount);

        if (appData.getLeaderMages())
        {
          if (!(totalManCount == 1 && leadManCount == 1))
          {
            unit->addWarning(L"magic STUDY requires a single LEAD mage");
          }
        }
        else
        {
          if (totalManCount > 1)
          {
            unit->addWarning(L"magic STUDY requires a single mage item");
          }
        }

        if (proposedLevel > 2)
        {
          const int buildingMageCapacity = resolveMageStudyAboveLevel2CapacityForWarning(appData, *unit);
          if (buildingMageCapacity <= 0)
          {
            unit->addWarning(L"magic STUDY above level 2 requires a structure with mage-study capacity");
          }
        }
      }

      const int studyCost = resolveStudyCostPerManMonth(*skillObj);
      if (studyCost <= 0)
      {
        continue;
      }

      const int totalStudyCost = manCount * studyCost;
      const int silverAfterStudy = estimatedSilver - totalStudyCost;
      if (silverAfterStudy < 0)
      {
        unit->addWarning(L"not enough silver for STUDY " + resolvedSkillToken);
        estimatedSilver = 0;
      }
      else
      {
        estimatedSilver = silverAfterStudy;
      }

      const std::optional<int> unitLevelLimit = resolveUnitSkillLevelLimitForWarning(appData,
                                                                                      estimatedCounts,
                                                                                      resolvedSkillToken);

      if (unitLevelLimit.has_value())
      {
        const int cappedDays = maxTrainingDaysForLevelLimitForWarning(*unitLevelLimit);
        if (proposedDays > cappedDays)
        {
          unit->addWarning(L"skill limit exceeded for STUDY " + resolvedSkillToken);
          estimatedSkillDays[resolvedSkillToken] = (std::max)(currentDays, (std::min)(proposedDays, cappedDays));
        }
        else
        {
          estimatedSkillDays[resolvedSkillToken] = proposedDays;
        }
      }
      else
      {
        estimatedSkillDays[resolvedSkillToken] = proposedDays;
      }
    }
  }
}

void checkTeachWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    // Check for TEACH commands
    for (const std::wstring& orderLine : unit->getOrders())
    {
      std::vector<std::wstring> tokens;
      std::vector<bool> tokenWasQuoted;
      if (!tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
      {
        continue;
      }

      // Skip order tag if present
      std::size_t tokenIdx = 0;
      if (!tokens[0].empty() && tokens[0][0] == L'@')
      {
        if (tokens[0].size() == 1)
        {
          ++tokenIdx;
        }
      }

      if (tokenIdx >= tokens.size())
      {
        continue;
      }

      std::wstring keyword = StringUtils::toUpper(tokens[tokenIdx]);
      if (keyword != L"TEACH")
      {
        continue;
      }

      // This is a TEACH command
      ++tokenIdx; // Skip TEACH keyword
      
      if (tokenIdx >= tokens.size())
      {
        unit->addWarning(L"TEACH: no student units specified");
        continue;
      }

      // Get teacher's items
      std::map<std::wstring, int> teacherEstimatedCounts =
        Commands::calculateAfterCommandItemCountsForUnit(appData, *unit);

      int teacherManCount = appData.itemRepository().calculateManItemCount(teacherEstimatedCounts);
      if (teacherManCount <= 0)
      {
        if (!teacherEstimatedCounts.empty())
        {
          teacherManCount = 1;
        }
        else
        {
          unit->addWarning(L"TEACH: unit has no items to teach with");
          continue;
        }
      }

      // Check onlyLeaderCanTeach setting
      if (appData.getOnlyLeaderCanTeach())
      {
        bool hasLeadItem = false;
        for (const auto& [itemToken, amount] : teacherEstimatedCounts)
        {
          if (amount > 0)
          {
            const std::wstring normalizedToken = normalizeItemTokenForWarning(itemToken);
            const Item* itemDef = appData.itemRepository().findByIdentifierToken(normalizedToken);
            if (!itemDef)
            {
              // Search by normalized name
              for (std::size_t i = 0; i < appData.itemRepository().size(); ++i)
              {
                const Item& candidate = appData.itemRepository().at(i);
                if (normalizeItemTokenForWarning(candidate.getIdentifierToken()) == normalizedToken)
                {
                  itemDef = &candidate;
                  break;
                }
              }
            }
            if (itemDef && itemDef->isMan() && normalizedToken == L"LEAD")
            {
              hasLeadItem = true;
              break;
            }
          }
        }
        if (!hasLeadItem)
        {
          unit->addWarning(L"TEACH: unit does not have LEAD items, cannot teach");
          continue;
        }
      }

      // Count total student man units to check teaching capacity
      int totalStudentManCount = 0;
      std::vector<int> studentUnitNumbers;
      
      while (tokenIdx < tokens.size())
      {
        int studentNumber = 0;
        if (swscanf_s(tokens[tokenIdx].c_str(), L"%d", &studentNumber) == 1 && studentNumber > 0)
        {
          studentUnitNumbers.push_back(studentNumber);
        }
        ++tokenIdx;
      }

      if (studentUnitNumbers.empty())
      {
        unit->addWarning(L"TEACH: no valid student unit numbers specified");
        continue;
      }

      // Check each student
      for (int studentNumber : studentUnitNumbers)
      {
        Unit* studentUnit = unitRepository.findByNumber(studentNumber);
        if (!studentUnit)
        {
          unit->addWarning(L"TEACH: student unit " + std::to_wstring(studentNumber) + L" not found");
          continue;
        }

        // Check if student is studying
        bool studentIsStudying = false;
        for (const std::wstring& studentOrder : studentUnit->getOrders())
        {
          std::vector<std::wstring> studentTokens;
          std::vector<bool> studentTokenWasQuoted;
          if (!tokenizeOrderLine(studentOrder, studentTokens, studentTokenWasQuoted))
          {
            continue;
          }
          if (!studentTokens.empty())
          {
            std::wstring firstToken = studentTokens[0];
            if (!firstToken.empty() && firstToken[0] == L'@')
            {
              if (firstToken.size() > 1)
              {
                firstToken = firstToken.substr(1);
              }
              else if (studentTokens.size() > 1)
              {
                firstToken = studentTokens[1];
              }
            }
            if (StringUtils::toUpper(firstToken) == L"STUDY")
            {
              studentIsStudying = true;
              break;
            }
          }
        }

        if (!studentIsStudying)
        {
          unit->addWarning(L"TEACH: student unit " + std::to_wstring(studentNumber) + L" is not studying");
        }
        else
        {
          // Count student's man for teaching capacity
          std::map<std::wstring, int> studentEstimatedCounts =
            Commands::calculateAfterCommandItemCountsForUnit(appData, *studentUnit);
          int studentManCount = appData.itemRepository().calculateManItemCount(studentEstimatedCounts);
          if (studentManCount <= 0 && !studentEstimatedCounts.empty())
          {
            studentManCount = 1;
          }
          totalStudentManCount += studentManCount;
        }
      }

      // Check teaching capacity
      const int teachingCapacity = teacherManCount * 10;
      if (totalStudentManCount > teachingCapacity)
      {
        unit->addWarning(L"TEACH: capacity exceeded for unit " + std::to_wstring(unit->getUnitNumber()));
      }
    }
  }
}

void checkMoveCapacityWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    bool hasMoveOrder = false;
    bool hasSailOrder = false;
    for (const std::wstring& orderLine : unit->getOrders())
    {
      std::wstring keyword;
      if (!tryExtractOrderKeywordUpper(orderLine, keyword))
      {
        continue;
      }

      if (keyword == L"MOVE" || keyword == L"ADVANCE")
      {
        hasMoveOrder = true;
      }
      else if (keyword == L"SAIL")
      {
        hasSailOrder = true;
      }
    }

    if (!hasMoveOrder && !hasSailOrder)
    {
      continue;
    }

    const int totalWeight = appData.itemRepository().calculateTotalWeight(unit->getItems());
    int walkCapacity = 0;
    for (const auto& [itemToken, count] : unit->getItems())
    {
      if (count <= 0)
      {
        continue;
      }

      const Item* item = appData.itemRepository().findByIdentifierToken(itemToken);
      if (!item)
      {
        continue;
      }

      const int itemWeight = item->getWeight();
      if (item->isMan())
      {
        walkCapacity += (item->getWalkCapacity() + itemWeight) * count;
      }
      else if (item->isMount())
      {
        walkCapacity += itemWeight * count;
      }
    }
    walkCapacity -= totalWeight;

    bool overloaded = hasMoveOrder && walkCapacity < 0;

    if (!overloaded && hasSailOrder && unit->getStructureId() > 0)
    {
      const Structure* structure = appData.structureRepository().findByIdAndCoordinates(
        unit->getStructureId(), unit->getXCoordinate(), unit->getYCoordinate(), unit->getZCoordinate());

      const StructInfo* structInfo = structure ? findStructInfoForStructure(appData, *structure) : nullptr;
      if (structure && structInfo && structInfo->isShip())
      {
        const std::wstring& shipToken = structInfo->getItemIdentifierToken();
        const Item* shipItem = shipToken.empty()
          ? appData.itemRepository().findByItemName(structure->getStructureType())
          : appData.itemRepository().findByIdentifierToken(shipToken);
        if (!shipItem)
        {
          shipItem = appData.itemRepository().findByItemName(structure->getStructureName());
        }

        if (shipItem)
        {
          int combinedWeight = 0;
          for (std::size_t innerIndex = 0; innerIndex < unitRepository.size(); ++innerIndex)
          {
            const Unit& candidate = unitRepository.at(innerIndex);
            if (candidate.getStructureId() == unit->getStructureId() &&
                candidate.getXCoordinate() == unit->getXCoordinate() &&
                candidate.getYCoordinate() == unit->getYCoordinate() &&
                candidate.getZCoordinate() == unit->getZCoordinate())
            {
              combinedWeight += appData.itemRepository().calculateTotalWeight(candidate.getItems());
            }
          }

          overloaded = (shipItem->getSwimCapacity() - combinedWeight) < 0;
        }
      }
    }

    if (overloaded)
    {
      unit->addWarning(L"overloaded");
    }
  }
}

void updateFutureStructureIdsForMainFaction(AppData& appData, int mainFactionNumber)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    unit->setFutureStructureId(unit->getStructureId());

    for (const std::wstring& orderLine : unit->getOrders())
    {
      std::wstring keyword;
      if (!tryExtractOrderKeywordUpper(orderLine, keyword))
      {
        continue;
      }

      if (keyword == L"MOVE" || keyword == L"ADVANCE")
      {
        int structureId = 0;
        if (tryExtractStructureIdFromOrder(orderLine, keyword, structureId))
        {
          unit->setFutureStructureId(structureId);
          break;
        }
      }
      else if (keyword == L"LEAVE")
      {
        unit->setFutureStructureId(0);
        break;
      }
      else if (keyword == L"ENTER")
      {
        int structureId = 0;
        if (tryExtractStructureIdFromOrder(orderLine, keyword, structureId))
        {
          unit->setFutureStructureId(structureId);
          break;
        }
      }
    }
  }
}

void checkShipCapacityWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  const auto& structureRepository = appData.structureRepository();
  auto& unitRepository = appData.unitRepository();

  std::map<std::tuple<int, int, int>, std::vector<const Unit*>> unitsByRegion;
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    const Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    const auto regionKey = std::make_tuple(unit->getXCoordinate(), unit->getYCoordinate(), unit->getZCoordinate());
    unitsByRegion[regionKey].push_back(unit);
  }

  for (const auto& [regionKey, unitsInRegion] : unitsByRegion)
  {
    const auto [regionX, regionY, regionZ] = regionKey;

    for (std::size_t strIndex = 0; strIndex < structureRepository.size(); ++strIndex)
    {
      const Structure& view = structureRepository.at(strIndex);
      if (view.getXCoordinate() != regionX ||
          view.getYCoordinate() != regionY ||
          view.getZCoordinate() != regionZ)
      {
        continue;
      }

      const Structure* structure = structureRepository.findByIdAndCoordinates(view.getStructureId(), regionX, regionY, regionZ);
      if (!structure)
      {
        continue;
      }

      const StructInfo* structInfo = findStructInfoForStructure(appData, *structure);
      if (!structInfo || !structInfo->isShip())
      {
        continue;
      }

      int totalManCount = 0;
      for (const Unit* unit : unitsInRegion)
      {
        const int unitFutureStructureId = unit->getFutureStructureId();

        bool willBeInStructure = false;
        if (unitFutureStructureId == structure->getStructureId())
        {
          willBeInStructure = true;
        }
        else if (unitFutureStructureId == 0 && unit->getStructureId() == structure->getStructureId())
        {
          willBeInStructure = false;
        }
        else if (unitFutureStructureId == -1 && unit->getStructureId() == structure->getStructureId())
        {
          willBeInStructure = true;
        }

        if (unit->getFutureStructureId() == -2 && unit->getStructureId() == structure->getStructureId())
        {
          willBeInStructure = true;
        }

        if (willBeInStructure)
        {
          const ItemRepository& itemRepo = appData.itemRepository();
          for (const auto& [itemToken, count] : unit->getItems())
          {
            if (count <= 0)
            {
              continue;
            }

            const Item* item = itemRepo.findByIdentifierToken(itemToken);
            if (item && item->isMan())
            {
              totalManCount += count;
            }
          }
        }
      }

      const int shipCapacity = structInfo->getNeeds();
      if (totalManCount > shipCapacity)
      {
        Unit* ownerUnit = nullptr;
        const int ownerUnitNumber = structure->getOwnerUnitId();
        if (ownerUnitNumber > 0)
        {
          ownerUnit = const_cast<Unit*>(unitRepository.findByNumber(ownerUnitNumber));
        }

        if (ownerUnit && ownerUnit->getFactionNumber() == mainFactionNumber)
        {
          const std::wstring warning =
            L"Ship '" + structure->getStructureName() + L"' exceeds capacity.";
          ownerUnit->addWarning(warning);
        }
      }
    }
  }
}

void checkMonthLongOrderWarningsForMainFaction(AppData& appData, int mainFactionNumber)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    const Unit& view = unitRepository.at(index);
    if (view.getFactionNumber() != mainFactionNumber)
    {
      continue;
    }

    Unit* unit = unitRepository.findByNumber(view.getUnitNumber());
    if (!unit)
    {
      continue;
    }

    int monthLongOrderCount = 0;
    for (const std::wstring& orderLine : unit->getOrders())
    {
      if (isMonthLongOrderLine(orderLine))
      {
        ++monthLongOrderCount;
      }
    }

    if (monthLongOrderCount == 0)
    {
      unit->addWarning(L"month long order missing");
      continue;
    }

    if (monthLongOrderCount >= 2)
    {
      unit->addWarning(L"multiple month long orders");
    }
  }
}
}

void OrderWarningService::runForMainFaction(AppData& appData)
{
  auto& unitRepository = appData.unitRepository();
  for (std::size_t index = 0; index < unitRepository.size(); ++index)
  {
    Unit* unit = unitRepository.findByNumber(unitRepository.at(index).getUnitNumber());
    if (unit)
    {
      unit->clearWarnings();
    }
  }

  auto& unitNewRepository = appData.unitNewRepository();
  for (std::size_t index = 0; index < unitNewRepository.size(); ++index)
  {
    const UnitNew& view = unitNewRepository.at(index);
    UnitNew* unitNew = unitNewRepository.findByNumberAndCoordinates(
      view.getUnitNumber(),
      view.getXCoordinate(),
      view.getYCoordinate(),
      view.getZCoordinate());
    if (unitNew)
    {
      unitNew->clearWarnings();
    }
  }

  const auto& factionRepository = appData.factionRepository();
  int mainFactionCount = 0;
  int mainFactionNumber = 0;
  for (std::size_t index = 0; index < factionRepository.size(); ++index)
  {
    const Faction& faction = factionRepository.at(index);
    if (faction.isMainFaction())
    {
      ++mainFactionCount;
      mainFactionNumber = faction.getFactionNumber();
    }
  }

  if (mainFactionCount != 1)
  {
    return;
  }

  checkMonthLongOrderWarningsForMainFaction(appData, mainFactionNumber);

  checkGiveTakeWarningsForMainFaction(appData, mainFactionNumber);
  CommandSimulationService::processMainFactionClaimEffects(appData);
  checkStudyWarningsForMainFaction(appData, mainFactionNumber);
  checkTeachWarningsForMainFaction(appData, mainFactionNumber);
  checkMoveCapacityWarningsForMainFaction(appData, mainFactionNumber);
  updateFutureStructureIdsForMainFaction(appData, mainFactionNumber);
  checkShipCapacityWarningsForMainFaction(appData, mainFactionNumber);
}
