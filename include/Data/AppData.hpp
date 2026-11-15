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
 * File: AppData.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/ItemRepository.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/ReportRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/UnitRepository.hpp"
#include "Data/UnitNewRepository.hpp"
#include "Data/OrderRepository.hpp"

#include <string>
#include <vector>

/**
* @brief Aggregates application data models outside the GUI layer.
*
* Add future domain objects here so the UI depends on a single data root
* instead of owning individual data models directly.
*/
class AppData
{
public:
  struct Repositories
  {
    ReportRepository& reports;
    FactionRepository& factions;
    ItemRepository& items;
    SkillRepository& skills;
    RegionRepository& regions;
    UnitRepository& units;
    UnitNewRepository& newUnits;
    BattleRepository& battles;
    EventRepository& events;
    StructureRepository& structures;
    StructInfoRepository& structureInfo;
    OrderRepository& orders;
  };

  // The supplied repositories must outlive this instance.
  explicit AppData(Repositories repositories);
  virtual ~AppData() = default;

  static AppData& getInstance();
  static AppData* getCurrent();

  AppData(const AppData&) = delete;
  AppData& operator=(const AppData&) = delete;
  AppData(AppData&&) = delete;
  AppData& operator=(AppData&&) = delete;

  virtual ReportRepository& reportRepository();
  virtual const ReportRepository& reportRepository() const;

  virtual FactionRepository& factionRepository();
  virtual const FactionRepository& factionRepository() const;

  virtual ItemRepository& itemRepository();
  virtual const ItemRepository& itemRepository() const;

  virtual SkillRepository& skillRepository();
  virtual const SkillRepository& skillRepository() const;

  virtual RegionRepository& regionRepository();
  virtual const RegionRepository& regionRepository() const;

  virtual UnitRepository& unitRepository();
  virtual const UnitRepository& unitRepository() const;

  virtual UnitNewRepository& unitNewRepository();
  virtual const UnitNewRepository& unitNewRepository() const;

  BattleRepository& battleRepository();
  const BattleRepository& battleRepository() const;

  virtual EventRepository& eventRepository();
  virtual const EventRepository& eventRepository() const;

  virtual StructureRepository& structureRepository();
  virtual const StructureRepository& structureRepository() const;

  virtual StructInfoRepository& structInfoRepository();
  virtual const StructInfoRepository& structInfoRepository() const;

  OrderRepository& orderRepository();
  virtual const OrderRepository& orderRepository() const;

  virtual int getShipStructureIdThreshold() const;
  virtual void setShipStructureIdThreshold(int threshold);

  virtual const std::wstring& getFlyingShipsCsv() const;
  virtual void setFlyingShipsCsv(std::wstring flyingShipsCsv);

  virtual const std::wstring& getMagicSkillTriggersCsv() const;
  virtual void setMagicSkillTriggersCsv(std::wstring magicSkillTriggersCsv);

  virtual bool getOnlyLeaderCanTeach() const;
  virtual void setOnlyLeaderCanTeach(bool onlyLeaderCanTeach);

  virtual bool getLeaderMages() const;
  virtual void setLeaderMages(bool leaderMages);

  std::vector<std::wstring> getFlyingShipTypeTokens() const;
  std::vector<std::wstring> getMagicSkillTriggerPhrases() const;
  virtual void refreshStructureDerivedFlags();

  virtual void clear();

private:
  ReportRepository& reportRepository_;
  FactionRepository& factionRepository_;
  ItemRepository& itemRepository_;
  SkillRepository& skillRepository_;
  RegionRepository& regionRepository_;
  UnitRepository& unitRepository_;
  UnitNewRepository& unitNewRepository_;
  BattleRepository& battleRepository_;
  EventRepository& eventRepository_;
  StructureRepository& structureRepository_;
  StructInfoRepository& structInfoRepository_;
  OrderRepository& orderRepository_;

  int shipStructureIdThreshold_ { 100 };
  std::wstring flyingShipsCsv_;
  std::wstring magicSkillTriggersCsv_;
  bool onlyLeaderCanTeach_ { false };
  bool leaderMages_ { true };
};
