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
 * File: ReportRepository.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Report.hpp"

#include <cstddef>
#include <string>
#include <vector>

class FactionRepository;
class BattleRepository;
class EventRepository;
class ItemRepository;
class RegionRepository;
class SkillRepository;
class StructureRepository;
class StructInfoRepository;
class OrderRepository;
class UnitRepository;

/**
* @brief Repository for loaded reports.
*
* Maintains an ordered collection of reports loaded from files.
*/
class ReportRepository
{
public:
  ReportRepository() = default;
  ~ReportRepository() = default;

  ReportRepository(const ReportRepository&) = delete;
  ReportRepository& operator=(const ReportRepository&) = delete;
  ReportRepository(ReportRepository&&) = default;
  ReportRepository& operator=(ReportRepository&&) = default;

  bool addFromFile(const std::wstring& filePath,
                  FactionRepository& factionRepository,
                  RegionRepository& regionRepository,
                  UnitRepository& unitRepository,
                  BattleRepository& battleRepository,
                  EventRepository& eventRepository,
                  ItemRepository& itemRepository,
                  SkillRepository& skillRepository,
                  StructureRepository& structureRepository,
                  StructInfoRepository& structInfoRepository,
                  OrderRepository& orderRepository,
                  int shipStructureIdThreshold,
                  const std::vector<std::wstring>& flyingShipTypeTokens,
                  const std::vector<std::wstring>& magicSkillTriggerPhrases,
                  bool syncFactionFromHeader = true);
  bool removeAt(std::size_t index);

  void clear();

  std::size_t size() const;
  const Report& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<Report> reports_;
  std::wstring        lastError_;
};
