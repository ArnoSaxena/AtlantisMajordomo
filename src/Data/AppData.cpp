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
 * File: AppData.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/AppData.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>
#include <utility>

AppData::AppData() = default;

AppData& AppData::getInstance()
{
  static AppData instance;
  return instance;
}

AppData* AppData::getCurrent()
{
  return &getInstance();
}

ReportRepository& AppData::reportRepository()
{
  return reportRepository_;
}

const ReportRepository& AppData::reportRepository() const
{
  return reportRepository_;
}

FactionRepository& AppData::factionRepository()
{
  return factionRepository_;
}

const FactionRepository& AppData::factionRepository() const
{
  return factionRepository_;
}

ItemRepository& AppData::itemRepository()
{
  return itemRepository_;
}

const ItemRepository& AppData::itemRepository() const
{
  return itemRepository_;
}

SkillRepository& AppData::skillRepository()
{
  return skillRepository_;
}

const SkillRepository& AppData::skillRepository() const
{
  return skillRepository_;
}

RegionRepository& AppData::regionRepository()
{
  return regionRepository_;
}

const RegionRepository& AppData::regionRepository() const
{
  return regionRepository_;
}

UnitRepository& AppData::unitRepository()
{
  return unitRepository_;
}

const UnitRepository& AppData::unitRepository() const
{
  return unitRepository_;
}

UnitNewRepository& AppData::unitNewRepository()
{
  return unitNewRepository_;
}

const UnitNewRepository& AppData::unitNewRepository() const
{
  return unitNewRepository_;
}

BattleRepository& AppData::battleRepository()
{
  return battleRepository_;
}

const BattleRepository& AppData::battleRepository() const
{
  return battleRepository_;
}

EventRepository& AppData::eventRepository()
{
  return eventRepository_;
}

const EventRepository& AppData::eventRepository() const
{
  return eventRepository_;
}

StructureRepository& AppData::structureRepository()
{
  return structureRepository_;
}

const StructureRepository& AppData::structureRepository() const
{
  return structureRepository_;
}

StructInfoRepository& AppData::structInfoRepository()
{
  return structInfoRepository_;
}

const StructInfoRepository& AppData::structInfoRepository() const
{
  return structInfoRepository_;
}

OrderRepository& AppData::orderRepository()
{
  return orderRepository_;
}

const OrderRepository& AppData::orderRepository() const
{
  return orderRepository_;
}

int AppData::getShipStructureIdThreshold() const
{
  return shipStructureIdThreshold_;
}

void AppData::setShipStructureIdThreshold(int threshold)
{
  shipStructureIdThreshold_ = threshold;
  refreshStructureDerivedFlags();
}

const std::wstring& AppData::getFlyingShipsCsv() const
{
  return flyingShipsCsv_;
}

void AppData::setFlyingShipsCsv(std::wstring flyingShipsCsv)
{
  flyingShipsCsv_ = std::move(flyingShipsCsv);
  refreshStructureDerivedFlags();
}

const std::wstring& AppData::getMagicSkillTriggersCsv() const
{
  return magicSkillTriggersCsv_;
}

void AppData::setMagicSkillTriggersCsv(std::wstring magicSkillTriggersCsv)
{
  magicSkillTriggersCsv_ = std::move(magicSkillTriggersCsv);
}

bool AppData::getOnlyLeaderCanTeach() const
{
  return onlyLeaderCanTeach_;
}

void AppData::setOnlyLeaderCanTeach(bool onlyLeaderCanTeach)
{
  onlyLeaderCanTeach_ = onlyLeaderCanTeach;
}

bool AppData::getLeaderMages() const
{
  return leaderMages_;
}

void AppData::setLeaderMages(bool leaderMages)
{
  leaderMages_ = leaderMages;
}

std::vector<std::wstring> AppData::getFlyingShipTypeTokens() const
{
  std::vector<std::wstring> tokens;
  std::wstring current;

  for (wchar_t ch : flyingShipsCsv_)
  {
    if (ch == L',')
    {
      std::wstring token = StringUtils::trimWhitespace(current);
      if (!token.empty())
      {
        std::transform(token.begin(), token.end(), token.begin(), towlower);
        tokens.push_back(std::move(token));
      }
      current.clear();
    }
    else
    {
      current.push_back(ch);
    }
  }

  std::wstring token = StringUtils::trimWhitespace(current);
  if (!token.empty())
  {
    std::transform(token.begin(), token.end(), token.begin(), towlower);
    tokens.push_back(std::move(token));
  }

  return tokens;
}

std::vector<std::wstring> AppData::getMagicSkillTriggerPhrases() const
{
  std::vector<std::wstring> phrases;
  std::wstring current;

  auto trim = [](std::wstring value)
  {
    while (!value.empty() && iswspace(value.front()))
    {
      value.erase(value.begin());
    }
    while (!value.empty() && iswspace(value.back()))
    {
      value.pop_back();
    }
    return value;
  };

  for (wchar_t ch : magicSkillTriggersCsv_)
  {
    if (ch == L',')
    {
      std::wstring phrase = trim(current);
      if (!phrase.empty())
      {
        std::transform(phrase.begin(), phrase.end(), phrase.begin(), towlower);
        phrases.push_back(std::move(phrase));
      }
      current.clear();
    }
    else
    {
      current.push_back(ch);
    }
  }

  std::wstring phrase = trim(current);
  if (!phrase.empty())
  {
    std::transform(phrase.begin(), phrase.end(), phrase.begin(), towlower);
    phrases.push_back(std::move(phrase));
  }

  return phrases;
}

void AppData::refreshStructureDerivedFlags()
{
  const std::vector<std::wstring> flyingTokens = getFlyingShipTypeTokens();

  for (std::size_t index = 0; index < structInfoRepository_.size(); ++index)
  {
    StructInfo& structInfo = structInfoRepository_.at(index);

    std::wstring lowerType = structInfo.getStructureType();
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), towlower);

    bool isFlying = false;
    for (const std::wstring& tokenValue : flyingTokens)
    {
      if (!tokenValue.empty() && lowerType.find(tokenValue) != std::wstring::npos)
      {
        isFlying = true;
        break;
      }
    }

    structInfo.setIsFlying(structInfo.isShip() && isFlying);
  }
}

void AppData::clear()
{
  reportRepository_.clear();
  factionRepository_.clear();
  itemRepository_.clear();
  skillRepository_.clear();
  regionRepository_.clear();
  unitRepository_.clear();
  unitNewRepository_.clear();
  battleRepository_.clear();
  eventRepository_.clear();
  structureRepository_.clear();
  structInfoRepository_.clear();
  orderRepository_.clear();
}
