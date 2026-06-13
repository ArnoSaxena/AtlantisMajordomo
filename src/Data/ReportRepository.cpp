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
 * File: ReportRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/ReportRepository.hpp"
#include "Data/AppData.hpp"
#include "Data/BattleRepository.hpp"
#include "Data/FactionRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/SkillRepository.hpp"
#include "Data/StructureRepository.hpp"
#include "Data/StructInfoRepository.hpp"
#include "Data/OrderRepository.hpp"
#include "Data/UnitRepository.hpp"
#include "DebugLog.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/FactionAttitudeUtils.hpp"
#include "Function/StringUtils.hpp"

#include <future>
#include <map>
#include <set>
#include <utility>

namespace
{
  bool isLaterPeriod(int candidateYear, int candidateMonth, int referenceYear, int referenceMonth)
  {
    return (candidateYear > referenceYear) ||
           (candidateYear == referenceYear && candidateMonth > referenceMonth);
  }

  bool isSameOrLaterPeriod(int candidateYear, int candidateMonth, int referenceYear, int referenceMonth)
  {
    return (candidateYear == referenceYear && candidateMonth == referenceMonth) ||
           isLaterPeriod(candidateYear, candidateMonth, referenceYear, referenceMonth);
  }

  std::wstring extractOrderToken(const std::wstring& orderText)
  {
    std::wstring trimmed = StringUtils::trimWhitespace(orderText);
    if (trimmed.empty())
    {
      return std::wstring();
    }

    if (trimmed.front() == L'@')
    {
      trimmed.erase(trimmed.begin());
      trimmed = StringUtils::trimWhitespace(trimmed);
      if (trimmed.empty())
      {
        return std::wstring();
      }
    }

    if (trimmed.front() == L';')
    {
      return std::wstring();
    }

    const std::size_t tokenEnd = trimmed.find_first_of(L" \t");
    const std::wstring token = tokenEnd == std::wstring::npos ? trimmed : trimmed.substr(0, tokenEnd);
    return StringUtils::toUpper(token);
  }

  // move this to OrderParsingUtils.
  bool isTrackedOrderToken(const std::wstring& orderToken)
  {
    static const std::set<std::wstring> trackedOrderTokens = {
      L"BUY",
      L"CLAIM",
      L"DISTRIBUTE",
      L"ENTERTAIN",
      L"GIVE",
      L"PRODUCE",
      L"NAME",
      L"SELL",
      L"STUDY",
      L"TAKE",
      L"TAX",
      L"TEACH",
      L"TRANSPORT",
      L"WORK"
    };

    return trackedOrderTokens.find(orderToken) != trackedOrderTokens.end();
  }
}

bool ReportRepository::addFromFile(const std::wstring& filePath,
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
                                  bool syncFactionFromHeader)
{
  DebugLog(L"ReportRepository::addFromFile() - begin: " + filePath);
  Report report;
  if (!report.loadFromFile(filePath))
  {
    lastError_ = report.getLastError();
    DebugLog(L"ReportRepository::addFromFile() - ERROR loading file: " + lastError_);
    return false;
  }

  // Synchronise faction repository from parsed header fields
  const int    factionNum  = report.getFactionNumber();
  const auto&  factionName = report.getFactionName();
  const int    month       = report.getMonth();
  const int    year        = report.getYear();
  const int    taxedOrTradedRegionsCurrent = report.getFactionTaxedOrTradedRegionsCurrent();
  const int    taxedOrTradedRegionsMax = report.getFactionTaxedOrTradedRegionsMax();
  const int    quartermastersCurrent = report.getFactionQuartermastersCurrent();
  const int    quartermastersMax = report.getFactionQuartermastersMax();
  const int    magesCurrent = report.getFactionMagesCurrent();
  const int    magesMax = report.getFactionMagesMax();
  const int    apprenticesCurrent = report.getFactionApprenticesCurrent();
  const int    apprenticesMax = report.getFactionApprenticesMax();
  const int    unclaimedSilver = report.getFactionUnclaimedSilver();
  const Faction::Attitude defaultAttitude = FactionAttitudeUtils::textToAttitude(report.getFactionDefaultAttitudeText());
  const auto& declaredAttitudesText = report.getFactionDeclaredAttitudesText();
  const auto& declaredFactionNames = report.getFactionDeclaredFactionNames();

  bool shouldRebuildTrackedOrders = reports_.empty();
  if (!reports_.empty())
  {
    int latestYear = reports_.front().getYear();
    int latestMonth = reports_.front().getMonth();
    for (const Report& existingReport : reports_)
    {
      if (isLaterPeriod(existingReport.getYear(), existingReport.getMonth(), latestYear, latestMonth))
      {
        latestYear = existingReport.getYear();
        latestMonth = existingReport.getMonth();
      }
    }

    shouldRebuildTrackedOrders = isSameOrLaterPeriod(year, month, latestYear, latestMonth);
  }

  if (syncFactionFromHeader && factionNum != 0 && !factionName.empty())
  {
    Faction* existing = factionRepository.findByNumber(factionNum);
    if (existing)
    {
      // Update month/year if this report is more recent
      const bool isLater = (year > existing->getYear()) ||
                          (year == existing->getYear() && month > existing->getMonth());
      const bool isSamePeriod = (year == existing->getYear() && month == existing->getMonth());
      if (isLater)
      {
        existing->setYear(year);
        existing->setMonth(month);
        if (factionName != existing->getName())
        {
          existing->setName(factionName);
        }
      }

      if (isLater || isSamePeriod)
      {
        existing->setTaxedOrTradedRegionsCurrent(taxedOrTradedRegionsCurrent);
        existing->setTaxedOrTradedRegionsMax(taxedOrTradedRegionsMax);
        existing->setQuartermastersCurrent(quartermastersCurrent);
        existing->setQuartermastersMax(quartermastersMax);
        existing->setMagesCurrent(magesCurrent);
        existing->setMagesMax(magesMax);
        existing->setApprenticesCurrent(apprenticesCurrent);
        existing->setApprenticesMax(apprenticesMax);
        existing->setUnclaimedSilver(unclaimedSilver);
        existing->setDefaultAttitude(defaultAttitude);
        existing->clearDeclaredAttitudes();
        for (const auto& [targetFactionNumber, attitudeText] : declaredAttitudesText)
        {
          existing->setDeclaredAttitude(targetFactionNumber, FactionAttitudeUtils::textToAttitude(attitudeText));
        }
      }
    }
    else
    {
      const bool isFirstFaction = factionRepository.size() == 0;
      std::map<int, Faction::Attitude> declaredAttitudes;
      for (const auto& [targetFactionNumber, attitudeText] : declaredAttitudesText)
      {
        declaredAttitudes[targetFactionNumber] = FactionAttitudeUtils::textToAttitude(attitudeText);
      }

      factionRepository.add(factionNum,
                            factionName,
                            isFirstFaction,
                            month,
                            year,
                            L"",
                            taxedOrTradedRegionsCurrent,
                            taxedOrTradedRegionsMax,
                            quartermastersCurrent,
                            quartermastersMax,
                            magesCurrent,
                            magesMax,
                            apprenticesCurrent,
                            apprenticesMax,
                            0,
                            defaultAttitude,
                            std::move(declaredAttitudes),
                            unclaimedSilver);
    }

    // Resolve and store a stable pointer to the faction entry.
    // FactionRepository uses std::deque so this pointer remains valid
    // as long as the repository is not cleared or the entry is not removed.
    for (const auto& [targetFactionNumber, _attitudeText] : declaredAttitudesText)
    {
      (void)_attitudeText;

      if (targetFactionNumber == factionNum)
      {
        continue;
      }

      auto nameIt = declaredFactionNames.find(targetFactionNumber);
      const std::wstring targetFactionName =
        (nameIt != declaredFactionNames.end()) ? nameIt->second : L"";

      Faction* declaredFaction = factionRepository.findByNumber(targetFactionNumber);
      if (!declaredFaction)
      {
        factionRepository.add(targetFactionNumber, targetFactionName, false, month, year);
        continue;
      }

      if (declaredFaction->getName().empty() && !targetFactionName.empty())
      {
        declaredFaction->setName(targetFactionName);
      }
    }

    report.setFaction(factionRepository.findByNumber(factionNum));
  }

  DebugLog(L"ReportRepository::addFromFile() - faction synced: " + factionName
           + L" (" + std::to_wstring(factionNum) + L")"
           + L", month=" + std::to_wstring(month)
           + L", year=" + std::to_wstring(year));

  // Parse regions and units from the report and add/update them in repositories.
  // Stage 1: Parse regions first (blocking) - other parse methods depend on populated repositories
  DebugLog(L"ReportRepository::addFromFile() - Stage 1: calling parseRegions");
  report.parseRegions(regionRepository,
                      unitRepository,
                      structureRepository,
                      itemRepository,
                      factionRepository,
                      shipStructureIdThreshold,
                      flyingShipTypeTokens);
  DebugLog(L"ReportRepository::addFromFile() - Stage 1 complete");

  // Parallel parse pipeline:
  //
  //   Stage 1:  parseRegions()
  //             |
  //   Stage 2:  parseBattles() --+
  //             parseEvents()  --|  (concurrent)
  //             parseOrders()  --|
  //             parseItems()   --|
  //                   |          | (Stage 3 starts immediately when parseItems finishes,
  //                   v          |  without waiting for the Stage 2 siblings)
  //   Stage 3:  parseStructures() --+
  //             parseSkills()       +  (concurrent with each other)
  //                   |
  //             All futures joined
  //
  // parseItems chains directly into Stage 3 (parseStructures + parseSkills) as a continuation,
  // so Stage 3 starts as soon as items are ready without waiting for other Stage 2 tasks.
  DebugLog(L"ReportRepository::addFromFile() - Stage 2: launching parallel parse tasks");
  auto battlesFuture = std::async(std::launch::async,
    [&report, &battleRepository, &regionRepository, &unitRepository]()
    {
      DebugLog(L"ReportRepository::addFromFile() - thread: calling parseBattles");
      report.parseBattles(battleRepository, regionRepository, unitRepository);
      DebugLog(L"ReportRepository::addFromFile() - thread: parseBattles complete");
    });

  auto eventsFuture = std::async(std::launch::async,
    [&report, &eventRepository]()
    {
      DebugLog(L"ReportRepository::addFromFile() - thread: calling parseEvents");
      report.parseEvents(eventRepository);
      DebugLog(L"ReportRepository::addFromFile() - thread: parseEvents complete");
    });

  auto ordersFuture = std::async(std::launch::async,
    [&report, &factionRepository, &unitRepository]()
    {
      DebugLog(L"ReportRepository::addFromFile() - thread: calling parseOrders");
      report.parseOrders(factionRepository, unitRepository);
      DebugLog(L"ReportRepository::addFromFile() - thread: parseOrders complete");
    });

  // parseItems chains directly into parseStructures and parseSkills (Stage 3).
  // Both Stage 3 tasks are independent of each other so they run in parallel.
  auto itemsAndDependentsFuture = std::async(std::launch::async,
    [&report, &itemRepository, &structInfoRepository, &skillRepository, &magicSkillTriggerPhrases]()
    {
      DebugLog(L"ReportRepository::addFromFile() - thread: calling parseItems");
      report.parseItems(itemRepository);
      DebugLog(L"ReportRepository::addFromFile() - thread: parseItems complete, launching Stage 3");

      auto structuresFuture = std::async(std::launch::async,
        [&report, &structInfoRepository, &itemRepository]()
        {
          DebugLog(L"ReportRepository::addFromFile() - thread: calling parseStructures");
          report.parseStructures(structInfoRepository, itemRepository);
          DebugLog(L"ReportRepository::addFromFile() - thread: parseStructures complete");
        });

      auto skillsFuture = std::async(std::launch::async,
        [&report, &skillRepository, &itemRepository, &magicSkillTriggerPhrases]()
        {
          DebugLog(L"ReportRepository::addFromFile() - thread: calling parseSkills");
          report.parseSkills(skillRepository, itemRepository, magicSkillTriggerPhrases);
          DebugLog(L"ReportRepository::addFromFile() - thread: parseSkills complete");
        });

      structuresFuture.get();
      skillsFuture.get();
      DebugLog(L"ReportRepository::addFromFile() - thread: Stage 3 complete");
    });

  // Wait for all tasks (Stage 3 completes inside itemsAndDependentsFuture)
  battlesFuture.get();
  eventsFuture.get();
  ordersFuture.get();
  itemsAndDependentsFuture.get();
  DebugLog(L"ReportRepository::addFromFile() - all parse stages done, adding report to collection");

  reports_.push_back(std::move(report));

  if (shouldRebuildTrackedOrders)
  {
    orderRepository.clear();
    for (std::size_t unitIndex = 0; unitIndex < unitRepository.size(); ++unitIndex)
    {
      const Unit& unit = unitRepository.at(unitIndex);
      for (const std::wstring& orderText : unit.getOrders())
      {
        const std::wstring orderToken = extractOrderToken(orderText);
        if (!isTrackedOrderToken(orderToken))
        {
          continue;
        }

        orderRepository.addOrder(
          unit.getUnitNumber(),
          false,
          unit.getXCoordinate(),
          unit.getYCoordinate(),
          unit.getZCoordinate(),
          orderToken,
          orderText
        );
      }
    }

    CommandSimulationService::recalculateAfterOrdersValues(AppData::getInstance());
  }

  lastError_.clear();
  DebugLog(L"ReportRepository::addFromFile() - completed successfully");
  return true;
}

bool ReportRepository::removeAt(std::size_t index)
{
  if (index >= reports_.size())
  {
    lastError_ = L"Invalid report index";
    return false;
  }

  reports_.erase(reports_.begin() + static_cast<std::ptrdiff_t>(index));
  lastError_.clear();
  return true;
}

void ReportRepository::clear()
{
  reports_.clear();
  lastError_.clear();
}

std::size_t ReportRepository::size() const
{
  return reports_.size();
}

const Report& ReportRepository::at(std::size_t index) const
{
  return reports_.at(index);
}

const std::wstring& ReportRepository::getLastError() const
{
  return lastError_;
}
