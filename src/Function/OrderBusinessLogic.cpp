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
 * File: OrderBusinessLogic.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/OrderBusinessLogic.hpp"
#include "Function/FactionAttitudeUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/OrderRepository.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Data/UnitRepository.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"
#include "AppConfig.hpp"

#include <algorithm>
#include <map>

namespace {
// Helper to check if an order is a DECLARE DEFAULT order
bool isDeclareDefaultOrder(const std::wstring& order)
{
    std::wstring normalized = StringUtils::toUpper(StringUtils::trimWhitespace(order));
    return normalized.rfind(L"DECLARE DEFAULT", 0) == 0;
}

bool isDeclareOrder(const std::wstring& order)
{
    const std::wstring normalized = StringUtils::toUpper(StringUtils::trimWhitespace(order));
    return normalized.rfind(L"DECLARE", 0) == 0;
}

}

namespace OrderBusinessLogic {

bool updateDeclareDefaultOrder(AppData& appData, int commandUnitNumber, const std::wstring& attitudeText, bool addOrder)
{
    bool changed = false;
    auto& unitRepo = appData.unitRepository();
    // Remove all DECLARE DEFAULT orders from all units
    for (std::size_t i = 0; i < unitRepo.size(); ++i) {
        Unit* unit = unitRepo.findByNumber(unitRepo.at(i).getUnitNumber());
        if (!unit) continue;
        const auto& orders = unit->getOrders();
        std::vector<std::wstring> filtered;
        filtered.reserve(orders.size());
        for (const auto& order : orders) {
            if (!isDeclareDefaultOrder(order)) {
                filtered.push_back(order);
            } else {
                changed = true;
            }
        }
        if (filtered.size() != orders.size()) {
            unit->setOrders(std::move(filtered));
        }
    }
    // Add new DECLARE DEFAULT order to command unit if requested
    if (addOrder && commandUnitNumber > 0) {
        Unit* commandUnit = unitRepo.findByNumber(commandUnitNumber);
        if (commandUnit) {
            std::wstring order = L"DECLARE DEFAULT ";
            order += attitudeText;
            commandUnit->addOrder(std::move(order));
            changed = true;
        }
    }
    return changed;
}

CommandUnitResolution resolveFactionCommandUnit(AppData& appData,
                                                int sourceFactionNumber,
                                                int requestedCommandUnitNumber)
{
    CommandUnitResolution resolution;
    if (sourceFactionNumber <= 0) {
        return resolution;
    }

    const Unit* requestedUnit = nullptr;
    if (requestedCommandUnitNumber > 0) {
        requestedUnit = appData.unitRepository().findByNumber(requestedCommandUnitNumber);
    }

    if (requestedUnit && requestedUnit->getFactionNumber() == sourceFactionNumber) {
        resolution.commandUnitNumber = requestedCommandUnitNumber;
        return resolution;
    }

    resolution.commandUnitNumber = appData.unitRepository().findSmallestUnitNumberForFaction(sourceFactionNumber);
    resolution.usedFallback = (requestedCommandUnitNumber > 0);
    return resolution;
}

bool applyDeclaredAttitudeChanges(AppData& appData,
                                  int sourceFactionNumber,
                                  const std::vector<DeclaredAttitudeChange>& changes)
{
    if (changes.empty() || sourceFactionNumber <= 0) {
        return false;
    }

    Faction* sourceFaction = appData.factionRepository().findByNumber(sourceFactionNumber);
    if (!sourceFaction) {
        return false;
    }

    std::map<int, DeclaredAttitudeChange> collapsed;
    for (const DeclaredAttitudeChange& change : changes) {
        if (change.targetFactionNumber > 0 && change.targetFactionNumber != sourceFactionNumber) {
            collapsed[change.targetFactionNumber] = change;
        }
    }

    if (collapsed.empty()) {
        return false;
    }

    bool changed = false;

    for (const auto& [targetFactionNumber, change] : collapsed) {
        if (change.useDefault) {
            const auto& declared = sourceFaction->getDeclaredAttitudes();
            if (declared.find(targetFactionNumber) != declared.end()) {
                sourceFaction->removeDeclaredAttitude(targetFactionNumber);
                changed = true;
            }
        } else {
            const std::wstring normalizedAttitude = FactionAttitudeUtils::normalizeAttitudeText(change.attitudeText);
            const Faction::Attitude parsed = FactionAttitudeUtils::textToAttitude(normalizedAttitude);
            const auto& declared = sourceFaction->getDeclaredAttitudes();
            const auto existing = declared.find(targetFactionNumber);
            if (existing == declared.end() || existing->second != parsed) {
                sourceFaction->setDeclaredAttitude(targetFactionNumber, parsed);
                changed = true;
            }
        }
    }

    return changed;
}

bool rewriteFactionDeclareOrders(AppData& appData,
                                 int sourceFactionNumber,
                                 int commandUnitNumber,
                                 const std::wstring& originalDefaultAttitudeText,
                                 const std::map<int, std::wstring>& originalDeclaredAttitudesText)
{
    if (sourceFactionNumber <= 0 || commandUnitNumber <= 0) {
        return false;
    }

    Faction* sourceFaction = appData.factionRepository().findByNumber(sourceFactionNumber);
    Unit* commandUnit = appData.unitRepository().findByNumber(commandUnitNumber);
    if (!sourceFaction || !commandUnit || commandUnit->getFactionNumber() != sourceFactionNumber) {
        return false;
    }

    bool changed = false;

    auto& unitRepo = appData.unitRepository();
    for (std::size_t index = 0; index < unitRepo.size(); ++index) {
        const int unitNumber = unitRepo.at(index).getUnitNumber();
        Unit* unit = unitRepo.findByNumber(unitNumber);
        if (!unit || unit->getFactionNumber() != sourceFactionNumber) {
            continue;
        }

        std::vector<std::wstring> filteredOrders;
        filteredOrders.reserve(unit->getOrders().size());
        for (const std::wstring& order : unit->getOrders()) {
            if (isDeclareOrder(order)) {
                changed = true;
                continue;
            }

            filteredOrders.push_back(order);
        }

        if (filteredOrders.size() != unit->getOrders().size()) {
            unit->setOrders(std::move(filteredOrders));
        }
    }

    const Faction::Attitude originalDefault = FactionAttitudeUtils::textToAttitude(originalDefaultAttitudeText);
    if (sourceFaction->getDefaultAttitude() != originalDefault) {
        std::wstring defaultOrder = L"DECLARE DEFAULT ";
        defaultOrder += FactionAttitudeUtils::attitudeToText(sourceFaction->getDefaultAttitude());
        commandUnit->addOrder(std::move(defaultOrder));
        changed = true;
    }

    std::map<int, Faction::Attitude> originalDeclared;
    for (const auto& [factionNumber, attitudeText] : originalDeclaredAttitudesText) {
        if (factionNumber > 0 && factionNumber != sourceFactionNumber) {
            originalDeclared[factionNumber] = FactionAttitudeUtils::textToAttitude(attitudeText);
        }
    }

    std::map<int, Faction::Attitude> currentDeclared;
    for (const auto& [factionNumber, attitude] : sourceFaction->getDeclaredAttitudes()) {
        if (factionNumber > 0 && factionNumber != sourceFactionNumber) {
            currentDeclared[factionNumber] = attitude;
        }
    }

    std::map<int, bool> allTargets;
    for (const auto& [factionNumber, _] : originalDeclared) {
        allTargets[factionNumber] = true;
    }
    for (const auto& [factionNumber, _] : currentDeclared) {
        allTargets[factionNumber] = true;
    }

    for (const auto& [targetFactionNumber, _] : allTargets) {
        const auto oldIt = originalDeclared.find(targetFactionNumber);
        const auto newIt = currentDeclared.find(targetFactionNumber);
        const bool oldExplicit = oldIt != originalDeclared.end();
        const bool newExplicit = newIt != currentDeclared.end();

        if (oldExplicit == newExplicit) {
            if (!oldExplicit) {
                continue;
            }
            if (oldIt->second == newIt->second) {
                continue;
            }
        }

        std::wstring order = L"DECLARE " + std::to_wstring(targetFactionNumber);
        if (newExplicit) {
            order += L" ";
            order += FactionAttitudeUtils::attitudeToText(newIt->second);
        }

        commandUnit->addOrder(std::move(order));
        changed = true;
    }

    return changed;
}

void syncOrderRepositoryForSavedUnit(AppData& appData,
                                     int originUnitNumber,
                                     bool recalculateAfterOrdersValues)
{
    if (originUnitNumber <= 0)
    {
        return;
    }

    Unit* originUnit = appData.unitRepository().findByNumber(originUnitNumber);
    if (!originUnit)
    {
        return;
    }

    auto& orderRepository = appData.orderRepository();

    std::vector<int> staleNewUnitNumbers;
    for (const UnitNew* unitNew : appData.unitNewRepository().findByOriginUnit(originUnitNumber))
    {
        if (unitNew)
        {
            staleNewUnitNumbers.push_back(unitNew->getUnitNumber());
        }
    }

    orderRepository.removeOrdersForUnit(originUnitNumber, false);
    orderRepository.removeOrdersForUnit(originUnitNumber, true);
    for (int staleNewUnitNumber : staleNewUnitNumbers)
    {
        orderRepository.removeOrdersForUnit(staleNewUnitNumber, false);
        orderRepository.removeOrdersForUnit(staleNewUnitNumber, true);
    }

    bool insideFormBlock = false;
    int currentFormUnitNumber = 0;

    for (const std::wstring& orderLine : originUnit->getOrders())
    {
        const std::wstring trimmedLine = StringUtils::trimWhitespace(orderLine);
        if (trimmedLine.empty())
        {
            continue;
        }

        if (trimmedLine.rfind(L";", 0) == 0 || trimmedLine.rfind(L"@;", 0) == 0)
        {
            continue;
        }

        int parsedFormUnitNumber = 0;
        if (!insideFormBlock && OrderParsingUtils::tryParseFormNewUnitLine(trimmedLine, parsedFormUnitNumber))
        {
            insideFormBlock = true;
            currentFormUnitNumber = parsedFormUnitNumber;
        }

        const int issuingUnitNumber = insideFormBlock ? currentFormUnitNumber : originUnitNumber;
        const bool fromNewUnit = insideFormBlock;

        std::wstring orderToken;
        if (!OrderParsingUtils::tryExtractOrderKeywordUpper(trimmedLine, orderToken))
        {
            orderToken.clear();
        }

        orderRepository.addOrder(
            issuingUnitNumber,
            fromNewUnit,
            originUnit->getXCoordinate(),
            originUnit->getYCoordinate(),
            originUnit->getZCoordinate(),
            std::move(orderToken),
            trimmedLine
        );

        if (insideFormBlock && StringUtils::toUpper(trimmedLine) == L"END")
        {
            insideFormBlock = false;
            currentFormUnitNumber = 0;
        }
    }

    if (recalculateAfterOrdersValues)
    {
        CommandSimulationService::recalculateAfterOrdersValues(appData);
    }
}

} // namespace OrderBusinessLogic
