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
 * File: Report.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Faction.hpp"

#include <map>
#include <string>
#include <vector>

class Faction;
class FactionRepository;
class BattleRepository;
class EventRepository;
class ItemRepository;
class RegionRepository;
class StructureRepository;
class StructInfoRepository;
class UnitRepository;

/**
* @brief Data model for storing report content.
*
* This class represents the business logic layer, separate from GUI concerns.
* It stores report data loaded from files.
*/
class Report
{
public:
  Report() = default;
  ~Report() = default;

  Report(const Report&) = delete;
  Report& operator=(const Report&) = delete;
  Report(Report&&) = default;
  Report& operator=(Report&&) = default;

  /**
  * @brief Loads report content from a file.
  * @param[in] filePath  Full path to the file.
  * @return true if successful, false on error.
  */
  bool loadFromFile(const std::wstring& filePath);

  /**
  * @brief Gets the report content as a single string.
  * @return Report content.
  */
  const std::wstring& getContent() const;

  /**
  * @brief Gets the report content as lines.
  * @return Vector of lines in the report.
  */
  const std::vector<std::wstring>& getLines() const;

  /**
  * @brief Gets the file path of the loaded report.
  * @return File path.
  */
  const std::wstring& getFilePath() const;

  /**
  * @brief Gets the report month metadata.
  * @return Month value.
  */
  int getMonth() const;

  /**
  * @brief Gets the report year metadata.
  * @return Year value.
  */
  int getYear() const;

  /**
  * @brief Checks if a report is loaded.
  * @return true if report has content.
  */
  bool hasContent() const;

  /**
  * @brief Gets the last error message.
  * @return Error message.
  */
  const std::wstring& getLastError() const;

  int getFactionNumber() const;
  const std::wstring& getFactionName() const;
  Faction* getFaction() const;
  int getFactionTaxedOrTradedRegionsCurrent() const;
  int getFactionTaxedOrTradedRegionsMax() const;
  int getFactionQuartermastersCurrent() const;
  int getFactionQuartermastersMax() const;
  int getFactionMagesCurrent() const;
  int getFactionMagesMax() const;
  int getFactionApprenticesCurrent() const;
  int getFactionApprenticesMax() const;
  int getFactionUnclaimedSilver() const;
  const std::wstring& getFactionDefaultAttitudeText() const;
  const std::map<int, std::wstring>& getFactionDeclaredAttitudesText() const;
  const std::map<int, std::wstring>& getFactionDeclaredFactionNames() const;

  void setMonth(int month);
  void setYear(int year);
  void setFactionNumber(int number);
  void setFactionName(std::wstring name);
  void setFaction(Faction* faction);
  void setFactionTaxedOrTradedRegionsCurrent(int value);
  void setFactionTaxedOrTradedRegionsMax(int value);
  void setFactionQuartermastersCurrent(int value);
  void setFactionQuartermastersMax(int value);
  void setFactionMagesCurrent(int value);
  void setFactionMagesMax(int value);
  void setFactionApprenticesCurrent(int value);
  void setFactionApprenticesMax(int value);
  void setFactionUnclaimedSilver(int value);

  /**
  * @brief Gets the number of regions found in this report.
  * @return Number of regions.
  */
  int getFoundRegions() const;
  int getVisitedRegions() const;

  /**
  * @brief Clears the report data.
  */
  void clear();

  /**
  * @brief Parses regions and units from report content and upserts them to repositories.
  * @param[in] regionRepository Repository to store/update found regions.
  * @param[in] unitRepository Repository to store parsed units.
  * @param[in] factionRepository Repository to store/update factions found in unit entries.
  */
  void parseRegions(RegionRepository& regionRepository,
                    UnitRepository& unitRepository,
                    StructureRepository& structureRepository,
                    ItemRepository& itemRepository,
                    FactionRepository& factionRepository,
                    int shipStructureIdThreshold,
                    const std::vector<std::wstring>& flyingShipTypeTokens);
  void parseBattles(BattleRepository& battleRepository,
                    RegionRepository& regionRepository,
                    UnitRepository& unitRepository);
  void parseEvents(EventRepository& eventRepository);
  void parseOrders(FactionRepository& factionRepository, UnitRepository& unitRepository);
  void parseItems(ItemRepository& itemRepository);
  void parseStructures(StructInfoRepository& structInfoRepository, ItemRepository& itemRepository);
  void parseSkills(class SkillRepository& skillRepository,
                   ItemRepository& itemRepository,
                   const std::vector<std::wstring>& magicSkillTriggerPhrases);
private:
  std::wstring              content_;
  std::vector<std::wstring> lines_;
  std::wstring              filePath_;
  std::wstring              lastError_;
  int                       month_ { 0 };
  int                       year_ { 0 };
  int                       factionNumber_ { 0 };
  std::wstring              factionName_;
  Faction*                  faction_ { nullptr };
  int                       factionTaxedOrTradedRegionsCurrent_ { 0 };
  int                       factionTaxedOrTradedRegionsMax_ { 0 };
  int                       factionQuartermastersCurrent_ { 0 };
  int                       factionQuartermastersMax_ { 0 };
  int                       factionMagesCurrent_ { 0 };
  int                       factionMagesMax_ { 0 };
  int                       factionApprenticesCurrent_ { 0 };
  int                       factionApprenticesMax_ { 0 };
  int                       factionUnclaimedSilver_ { 0 };
  std::wstring              factionDefaultAttitudeText_ { L"Neutral" };
  std::map<int, std::wstring> factionDeclaredAttitudesText_;
  std::map<int, std::wstring> factionDeclaredFactionNames_;
  int                       foundRegions_ { 0 };
  int                       visitedRegions_ { 0 };

  void parseAtlantisHeader();
  /**
  * @brief Splits content into lines.
  */
  void splitIntoLines();
};
