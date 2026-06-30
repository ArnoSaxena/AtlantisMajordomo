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
 * File: MapTabContentQt_UnitDetails.cpp
 *
 * Step 7.3 - unit list + detail panel.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/BattleRepository.hpp"
#include "Data/EventRepository.hpp"
#include "Data/Faction.hpp"
#include "Data/Item.hpp"
#include "Data/Report.hpp"
#include "Data/Skill.hpp"
#include "Data/Structure.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/HexDirectionUtils.hpp"
#include "Function/SkillFormattingUtils.hpp"
#include "Function/StringUtils.hpp"
#include "Function/UnitCapacityUtils.hpp"

#include <QLabel>
#include <QListWidget>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <algorithm>
#include <cwctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr int kUnitNumberRole = Qt::UserRole;

QString toQString(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

QTableWidgetItem* makeReadOnlyItem(const QString& text)
{
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

} // namespace

void MapTabContentQt::refresh()
{
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedZ(selectedZ_);
        mapCanvas_->setSelectedRegion(hasSelectedRegion_, selectedRegionX_, selectedRegionY_);
        mapCanvas_->recalculateVisibleMap();
        selectedZ_ = mapCanvas_->selectedZ();
    }

    if (!appData_)
    {
        clearUnitsList();
        return;
    }

    if (hasSelectedRegion_)
    {
        populateUnitsForSelectedRegion();
    }
    else
    {
        clearUnitsList();
    }
}

void MapTabContentQt::refreshItemsForCurrentUnit()
{
    if (!appData_ || selectedUnitNumber_ <= 0)
    {
        return;
    }

    if (selectedUnitIsNew_)
    {
        const UnitNew* unitNew = appData_->unitNewRepository().findByNumberAndCoordinates(
            selectedUnitNumber_, selectedRegionX_, selectedRegionY_, selectedZ_);
        populateItemsForSelectedUnit(unitNew);
        return;
    }

    const Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    populateItemsForSelectedUnit(unit);
}

void MapTabContentQt::onUnitsSelectionChanged()
{
    updateSelectedUnitFromList();
}

void MapTabContentQt::onUnitDetailsTabChanged(int index)
{
    if (index >= 0)
    {
        updateUnitDetailsTabVisibility();
    }
}

void MapTabContentQt::populateUnitsForSelectedRegion()
{
    if (!unitsList_)
    {
        return;
    }

    const int previousSelectedUnitNumber = selectedUnitNumber_;
    const bool previousSelectedUnitIsNew = selectedUnitIsNew_;

    {
        const QSignalBlocker blocker(unitsList_);
        unitsList_->setRowCount(0);
    }

    if (!appData_ || !hasSelectedRegion_)
    {
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        clearSelectedUnitDetails();
        if (mapCanvas_)
        {
            mapCanvas_->setSelectedRegion(false, 0, 0);
        }
        return;
    }

    const auto& unitRepository = appData_->unitRepository();
    const auto& unitNewRepository = appData_->unitNewRepository();

    int latestMonth = 0;
    int latestYear = 0;
    {
        const auto& reportRepository = appData_->reportRepository();
        for (std::size_t i = 0; i < reportRepository.size(); ++i)
        {
            const Report& report = reportRepository.at(i);
            const int rm = report.getMonth();
            const int ry = report.getYear();
            if (rm >= 1 && rm <= 12 && ry > 0)
            {
                if (ry > latestYear || (ry == latestYear && rm > latestMonth))
                {
                    latestMonth = rm;
                    latestYear = ry;
                }
            }
        }
    }
    const bool hasLatestPeriod = (latestMonth >= 1 && latestMonth <= 12 && latestYear > 0);

    int latestBattleMonth = 0;
    int latestBattleYear = 0;
    const bool hasLatestBattlePeriod = appData_->battleRepository().getLatestPeriod(latestBattleMonth, latestBattleYear);

    int selectedRow = -1;

    auto appendUnitRow = [&](int lParamValue,
                             const std::wstring& numberText,
                             const std::wstring& nameText,
                             const std::wstring& factionNumberText,
                             const std::wstring& factionNameText,
                             const std::wstring& structureText,
                             const std::wstring& menText,
                             const std::wstring& silverText,
                             const std::wstring& flagsText,
                             const std::wstring& skillsText,
                             const std::wstring& warningText,
                             const std::wstring& damagedText)
    {
        const int row = unitsList_->rowCount();
        unitsList_->insertRow(row);

        auto* numberItem = makeReadOnlyItem(toQString(numberText));
        numberItem->setData(kUnitNumberRole, lParamValue);
        unitsList_->setItem(row, 0, numberItem);

        unitsList_->setItem(row, 1, makeReadOnlyItem(toQString(nameText)));
        unitsList_->setItem(row, 2, makeReadOnlyItem(toQString(factionNumberText)));
        unitsList_->setItem(row, 3, makeReadOnlyItem(toQString(factionNameText)));
        unitsList_->setItem(row, 4, makeReadOnlyItem(toQString(structureText)));
        unitsList_->setItem(row, 5, makeReadOnlyItem(toQString(menText)));
        unitsList_->setItem(row, 6, makeReadOnlyItem(toQString(silverText)));
        unitsList_->setItem(row, 7, makeReadOnlyItem(toQString(flagsText)));
        unitsList_->setItem(row, 8, makeReadOnlyItem(toQString(skillsText)));
        unitsList_->setItem(row, 9, makeReadOnlyItem(toQString(warningText)));
        unitsList_->setItem(row, 10, makeReadOnlyItem(toQString(damagedText)));

        if (previousSelectedUnitNumber != 0)
        {
            const bool rowIsNew = lParamValue < 0;
            const int rowUnitNumber = rowIsNew ? -lParamValue : lParamValue;
            if (rowUnitNumber == previousSelectedUnitNumber && rowIsNew == previousSelectedUnitIsNew)
            {
                selectedRow = row;
            }
        }
    };

    for (std::size_t index = 0; index < unitRepository.size(); ++index)
    {
        const Unit& unit = unitRepository.at(index);
        if (unit.getXCoordinate() != selectedRegionX_ ||
            unit.getYCoordinate() != selectedRegionY_ ||
            unit.getZCoordinate() != selectedZ_)
        {
            continue;
        }

        if (hasLatestPeriod && (unit.getMonth() != latestMonth || unit.getYear() != latestYear))
        {
            continue;
        }

        const std::wstring unitNumberText = std::to_wstring(unit.getUnitNumber());

        std::wstring factionNumberText;
        if (unit.getFactionNumber() > 0)
        {
            factionNumberText = std::to_wstring(unit.getFactionNumber());
        }

        std::wstring factionNameText;
        if (unit.getFactionNumber() > 0)
        {
            if (const Faction* faction = appData_->factionRepository().findByNumber(unit.getFactionNumber()))
            {
                factionNameText = faction->getName();
            }
        }

        std::wstring structureText;
        const int displayStructureId = unit.getFutureStructureId();
        if (displayStructureId != 0)
        {
            const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
                displayStructureId,
                unit.getXCoordinate(),
                unit.getYCoordinate(),
                unit.getZCoordinate());
            if (structure)
            {
                structureText = structure->getStructureType() + L" [" + std::to_wstring(displayStructureId) + L"]";
                if (!structure->getStructureName().empty())
                {
                    structureText += L" - " + structure->getStructureName();
                }
            }
        }

        const std::map<std::wstring, int>& afterCommandCounts = unit.getItemsAfterOrders();

        std::vector<std::wstring> menEntries;
        for (const auto& [itemToken, amount] : afterCommandCounts)
        {
            if (amount <= 0)
            {
                continue;
            }
            const Item* itemDefinition = appData_->itemRepository().findByIdentifierToken(itemToken);
            if (itemDefinition && itemDefinition->isMan())
            {
                menEntries.push_back(itemToken + L" (" + std::to_wstring(amount) + L")");
            }
        }
        const std::wstring menText = StringUtils::joinLines(menEntries, L", ");

        const auto silverCurrentIt = unit.getItems().find(L"SILV");
        const int silverCurrent = silverCurrentIt != unit.getItems().end() ? silverCurrentIt->second : 0;
        const auto silverAfterIt = afterCommandCounts.find(L"SILV");
        const int silverAfter = silverAfterIt != afterCommandCounts.end() ? silverAfterIt->second : 0;
        const std::wstring silverText = std::to_wstring(silverCurrent) + L" (" + std::to_wstring(silverAfter) + L")";

        const std::wstring flagsText = StringUtils::joinLines(unit.getFlags(), L", ");
        const std::wstring skillsText = SkillFormattingUtils::formatSkills(unit.getSkills());
        const std::wstring warningIndicator = unit.getWarnings().empty() ? L"" : L"!";

        const bool isDamagedInLatestBattle = hasLatestBattlePeriod &&
            appData_->battleRepository().isUnitDamagedInAnyBattleForPeriod(
                unit.getUnitNumber(), latestBattleMonth, latestBattleYear);
        const std::wstring damagedIndicator = isDamagedInLatestBattle ? L"x" : L"";

        appendUnitRow(unit.getUnitNumber(),
                      unitNumberText,
                      unit.getUnitNameAfterOrders(),
                      factionNumberText,
                      factionNameText,
                      structureText,
                      menText,
                      silverText,
                      flagsText,
                      skillsText,
                      warningIndicator,
                      damagedIndicator);

        for (std::size_t newIndex = 0; newIndex < unitNewRepository.size(); ++newIndex)
        {
            const UnitNew& unitNew = unitNewRepository.at(newIndex);
            if (unitNew.getOriginUnit() != unit.getUnitNumber() ||
                unitNew.getXCoordinate() != selectedRegionX_ ||
                unitNew.getYCoordinate() != selectedRegionY_ ||
                unitNew.getZCoordinate() != selectedZ_)
            {
                continue;
            }

            if (hasLatestPeriod && (unitNew.getMonth() != latestMonth || unitNew.getYear() != latestYear))
            {
                continue;
            }

            int newUnitFactionNumber = unitNew.getFactionNumber();
            if (newUnitFactionNumber <= 0)
            {
                if (const Unit* originUnit = appData_->unitRepository().findByNumber(unitNew.getOriginUnit()))
                {
                    newUnitFactionNumber = originUnit->getFactionNumber();
                }
            }

            std::wstring newFactionNumberText;
            if (newUnitFactionNumber > 0)
            {
                newFactionNumberText = std::to_wstring(newUnitFactionNumber);
            }

            std::wstring newFactionNameText;
            if (newUnitFactionNumber > 0)
            {
                if (const Faction* faction = appData_->factionRepository().findByNumber(newUnitFactionNumber))
                {
                    newFactionNameText = faction->getName();
                }
            }

            std::wstring newStructureText;
            const int newDisplayStructureId = unitNew.getFutureStructureId();
            if (newDisplayStructureId != 0)
            {
                const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
                    newDisplayStructureId,
                    unitNew.getXCoordinate(),
                    unitNew.getYCoordinate(),
                    unitNew.getZCoordinate());
                if (structure)
                {
                    newStructureText = structure->getStructureType() + L" [" + std::to_wstring(newDisplayStructureId) + L"]";
                    if (!structure->getStructureName().empty())
                    {
                        newStructureText += L" - " + structure->getStructureName();
                    }
                }
            }

            const auto& newAfterCommandCounts = unitNew.getItemsAfterOrders();

            std::vector<std::wstring> newMenEntries;
            for (const auto& [itemToken, amount] : newAfterCommandCounts)
            {
                if (amount <= 0)
                {
                    continue;
                }
                const Item* itemDefinition = appData_->itemRepository().findByIdentifierToken(itemToken);
                if (itemDefinition && itemDefinition->isMan())
                {
                    newMenEntries.push_back(itemToken + L" (" + std::to_wstring(amount) + L")");
                }
            }
            const std::wstring newMenText = StringUtils::joinLines(newMenEntries, L", ");

            const auto newSilverCurrentIt = unitNew.getItems().find(L"SILV");
            const int newSilverCurrent = newSilverCurrentIt != unitNew.getItems().end() ? newSilverCurrentIt->second : 0;
            const auto newSilverAfterIt = newAfterCommandCounts.find(L"SILV");
            const int newSilverAfter = newSilverAfterIt != newAfterCommandCounts.end() ? newSilverAfterIt->second : 0;
            const std::wstring newSilverText = std::to_wstring(newSilverCurrent) + L" (" + std::to_wstring(newSilverAfter) + L")";

            const std::wstring newFlagsText = StringUtils::joinLines(unitNew.getFlags(), L", ");
            const std::wstring newSkillsText = SkillFormattingUtils::formatSkills(unitNew.getSkills());
            const std::wstring newWarningIndicator = unitNew.getWarnings().empty() ? L"" : L"!";

            appendUnitRow(-unitNew.getUnitNumber(),
                          L"New " + std::to_wstring(unitNew.getUnitNumber()),
                          unitNew.getUnitNameAfterOrders(),
                          newFactionNumberText,
                          newFactionNameText,
                          newStructureText,
                          newMenText,
                          newSilverText,
                          newFlagsText,
                          newSkillsText,
                          newWarningIndicator,
                          L"");
        }
    }

    const auto regionStructures = appData_->structureRepository().findByCoordinates(
        selectedRegionX_, selectedRegionY_, selectedZ_);
    for (const Structure* structure : regionStructures)
    {
        if (!structure)
        {
            continue;
        }

        if (hasLatestPeriod && (structure->getMonth() != latestMonth || structure->getYear() != latestYear))
        {
            continue;
        }

        if (appData_->unitRepository().hasUnitInStructureAtCoordinates(
            structure->getStructureId(),
            structure->getXCoordinate(),
            structure->getYCoordinate(),
            structure->getZCoordinate()))
        {
            continue;
        }

        std::wstring structureText = structure->getStructureType() + L" [" + std::to_wstring(structure->getStructureId()) + L"]";
        if (!structure->getStructureName().empty())
        {
            structureText += L" - " + structure->getStructureName();
        }

        appendUnitRow(0,
                      L"",
                      L"",
                      L"",
                      L"",
                      structureText,
                      L"",
                      L"",
                      L"",
                      L"",
                      L"",
                      L"");
    }

    if (selectedRow >= 0)
    {
        unitsList_->selectRow(selectedRow);
    }

    updateSelectedUnitFromList();
}

void MapTabContentQt::clearUnitsList()
{
    if (unitsList_)
    {
        const QSignalBlocker blocker(unitsList_);
        unitsList_->setRowCount(0);
    }

    selectedUnitNumber_ = 0;
    selectedUnitIsNew_ = false;
    clearSelectedUnitDetails();
}

void MapTabContentQt::updateSelectedUnitFromList()
{
    if (!unitsList_)
    {
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        clearSelectedUnitDetails();
        return;
    }

    const auto selectedItems = unitsList_->selectedItems();
    if (selectedItems.empty())
    {
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        clearSelectedUnitDetails();
        return;
    }

    QTableWidgetItem* numberItem = unitsList_->item(selectedItems.front()->row(), 0);
    if (!numberItem)
    {
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        clearSelectedUnitDetails();
        return;
    }

    const int itemValue = numberItem->data(kUnitNumberRole).toInt();
    if (itemValue == 0)
    {
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        clearSelectedUnitDetails();
        return;
    }

    selectedUnitIsNew_ = itemValue < 0;
    selectedUnitNumber_ = selectedUnitIsNew_ ? -itemValue : itemValue;
    updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
}

void MapTabContentQt::populateItemsForSelectedUnit(const Unit* unit)
{
    if (!unitItemsList_)
    {
        return;
    }

    unitItemsList_->setRowCount(0);
    if (!appData_ || !unit)
    {
        return;
    }

    const std::map<std::wstring, int>& afterCommandCounts = unit->getItemsAfterOrders();

    std::set<std::wstring> itemTokens;
    for (const auto& [itemToken, _] : unit->getItems())
    {
        itemTokens.insert(itemToken);
    }
    for (const auto& [itemToken, _] : afterCommandCounts)
    {
        itemTokens.insert(itemToken);
    }

    std::vector<std::wstring> sortedItemTokens(itemTokens.begin(), itemTokens.end());
    std::sort(sortedItemTokens.begin(), sortedItemTokens.end(),
              [this](const std::wstring& leftToken, const std::wstring& rightToken)
              {
                  const Item* leftItem = appData_ ? appData_->itemRepository().findByIdentifierToken(leftToken) : nullptr;
                  const Item* rightItem = appData_ ? appData_->itemRepository().findByIdentifierToken(rightToken) : nullptr;

                  const bool leftIsMan = leftItem && leftItem->isMan();
                  const bool rightIsMan = rightItem && rightItem->isMan();
                  if (leftIsMan != rightIsMan)
                  {
                      return leftIsMan > rightIsMan;
                  }

                  return leftToken < rightToken;
              });

    for (const std::wstring& itemToken : sortedItemTokens)
    {
        const auto currentIt = unit->getItems().find(itemToken);
        const int amount = currentIt != unit->getItems().end() ? currentIt->second : 0;

        const auto afterIt = afterCommandCounts.find(itemToken);
        const int amountAfterCommands = afterIt != afterCommandCounts.end() ? afterIt->second : 0;

        std::wstring itemName;
        if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
        {
            itemName = item->getItemName();
        }

        const int row = unitItemsList_->rowCount();
        unitItemsList_->insertRow(row);
        unitItemsList_->setItem(row, 0, makeReadOnlyItem(toQString(itemToken)));
        unitItemsList_->setItem(row, 1, makeReadOnlyItem(toQString(itemName)));
        unitItemsList_->setItem(row, 2, makeReadOnlyItem(QString::number(amount)));
        unitItemsList_->setItem(row, 3, makeReadOnlyItem(QString::number(amountAfterCommands)));
    }
}

void MapTabContentQt::populateItemsForSelectedUnit(const UnitNew* unitNew)
{
    if (!unitItemsList_)
    {
        return;
    }

    unitItemsList_->setRowCount(0);
    if (!appData_ || !unitNew)
    {
        return;
    }

    const std::map<std::wstring, int>& afterCommandCounts = unitNew->getItemsAfterOrders();

    std::set<std::wstring> itemTokens;
    for (const auto& [itemToken, _] : unitNew->getItems())
    {
        itemTokens.insert(itemToken);
    }
    for (const auto& [itemToken, _] : afterCommandCounts)
    {
        itemTokens.insert(itemToken);
    }

    std::vector<std::wstring> sortedItemTokens(itemTokens.begin(), itemTokens.end());
    std::sort(sortedItemTokens.begin(), sortedItemTokens.end(),
              [this](const std::wstring& leftToken, const std::wstring& rightToken)
              {
                  const Item* leftItem = appData_ ? appData_->itemRepository().findByIdentifierToken(leftToken) : nullptr;
                  const Item* rightItem = appData_ ? appData_->itemRepository().findByIdentifierToken(rightToken) : nullptr;

                  const bool leftIsMan = leftItem && leftItem->isMan();
                  const bool rightIsMan = rightItem && rightItem->isMan();
                  if (leftIsMan != rightIsMan)
                  {
                      return leftIsMan > rightIsMan;
                  }

                  return leftToken < rightToken;
              });

    for (const std::wstring& itemToken : sortedItemTokens)
    {
        const auto currentIt = unitNew->getItems().find(itemToken);
        const int amount = currentIt != unitNew->getItems().end() ? currentIt->second : 0;

        const auto afterIt = afterCommandCounts.find(itemToken);
        const int amountAfterCommands = afterIt != afterCommandCounts.end() ? afterIt->second : 0;

        std::wstring itemName;
        if (const Item* item = appData_->itemRepository().findByIdentifierToken(itemToken))
        {
            itemName = item->getItemName();
        }

        const int row = unitItemsList_->rowCount();
        unitItemsList_->insertRow(row);
        unitItemsList_->setItem(row, 0, makeReadOnlyItem(toQString(itemToken)));
        unitItemsList_->setItem(row, 1, makeReadOnlyItem(toQString(itemName)));
        unitItemsList_->setItem(row, 2, makeReadOnlyItem(QString::number(amount)));
        unitItemsList_->setItem(row, 3, makeReadOnlyItem(QString::number(amountAfterCommands)));
    }
}

void MapTabContentQt::populateSkillsList(const Unit* unit)
{
    if (!unitSkillsList_)
    {
        return;
    }

    unitSkillsList_->clear();
    if (!unit)
    {
        return;
    }

    const auto& skills = unit->getSkills();
    const auto& afterCommandSkills = unit->getSkillsAfterOrders();

    for (const auto& [skillToken, days] : skills)
    {
        const int level = Skill::trainingDaysToLevel(days);
        const auto afterIt = afterCommandSkills.find(skillToken);
        const int afterDays = afterIt != afterCommandSkills.end() ? afterIt->second : days;
        const int afterLevel = Skill::trainingDaysToLevel(afterDays);

        const std::wstring line = skillToken + L": " +
                                  std::to_wstring(level) + L" [" + std::to_wstring(days) + L"] -> " +
                                  std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
        unitSkillsList_->addItem(toQString(line));
    }

    for (const auto& [skillToken, afterDays] : afterCommandSkills)
    {
        if (skills.find(skillToken) != skills.end())
        {
            continue;
        }

        const int afterLevel = Skill::trainingDaysToLevel(afterDays);
        const std::wstring line = skillToken + L": 0 [0] -> " +
                                  std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
        unitSkillsList_->addItem(toQString(line));
    }

    for (const std::wstring& canStudyToken : unit->getCanStudySkillTokens())
    {
        if (canStudyToken.empty())
        {
            continue;
        }
        QListWidgetItem* item = new QListWidgetItem(toQString(canStudyToken + L": can study"), unitSkillsList_);
        item->setForeground(QBrush(Qt::gray));
    }
}

void MapTabContentQt::populateSkillsList(const UnitNew* unitNew)
{
    if (!unitSkillsList_)
    {
        return;
    }

    unitSkillsList_->clear();
    if (!unitNew)
    {
        return;
    }

    const auto& skills = unitNew->getSkills();
    const auto& afterCommandSkills = unitNew->getSkillsAfterOrders();

    for (const auto& [skillToken, days] : skills)
    {
        const int level = Skill::trainingDaysToLevel(days);
        const auto afterIt = afterCommandSkills.find(skillToken);
        const int afterDays = afterIt != afterCommandSkills.end() ? afterIt->second : days;
        const int afterLevel = Skill::trainingDaysToLevel(afterDays);

        const std::wstring line = skillToken + L": " +
                                  std::to_wstring(level) + L" [" + std::to_wstring(days) + L"] -> " +
                                  std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
        unitSkillsList_->addItem(toQString(line));
    }

    for (const auto& [skillToken, afterDays] : afterCommandSkills)
    {
        if (skills.find(skillToken) != skills.end())
        {
            continue;
        }

        const int afterLevel = Skill::trainingDaysToLevel(afterDays);
        const std::wstring line = skillToken + L": 0 [0] -> " +
                                  std::to_wstring(afterLevel) + L" [" + std::to_wstring(afterDays) + L"]";
        unitSkillsList_->addItem(toQString(line));
    }

    for (const std::wstring& canStudyToken : unitNew->getCanStudySkillTokens())
    {
        if (canStudyToken.empty())
        {
            continue;
        }
        QListWidgetItem* item = new QListWidgetItem(toQString(canStudyToken + L": can study"), unitSkillsList_);
        item->setForeground(QBrush(Qt::gray));
    }
}

int MapTabContentQt::populateErrorsList(const Unit* unit)
{
    if (!unitErrorsList_)
    {
        return 0;
    }

    unitErrorsList_->clear();
    if (!unit || !appData_)
    {
        return 0;
    }

    const std::vector<const Event*> unitErrors = appData_->eventRepository().findErrorsByUnitId(unit->getUnitNumber());
    int count = 0;
    for (const Event* eventValue : unitErrors)
    {
        if (!eventValue)
        {
            continue;
        }
        unitErrorsList_->addItem(toQString(eventValue->getMessage()));
        ++count;
    }

    return count;
}

int MapTabContentQt::populateWarningsList(const Unit* unit)
{
    if (!unitWarningsList_)
    {
        return 0;
    }

    unitWarningsList_->clear();
    if (!unit)
    {
        return 0;
    }

    int count = 0;
    for (const std::wstring& warning : unit->getWarnings())
    {
        if (warning.empty())
        {
            continue;
        }
        unitWarningsList_->addItem(toQString(warning));
        ++count;
    }
    return count;
}

int MapTabContentQt::populateWarningsList(const UnitNew* unitNew)
{
    if (!unitWarningsList_)
    {
        return 0;
    }

    unitWarningsList_->clear();
    if (!unitNew)
    {
        return 0;
    }

    int count = 0;
    for (const std::wstring& warning : unitNew->getWarnings())
    {
        if (warning.empty())
        {
            continue;
        }
        unitWarningsList_->addItem(toQString(warning));
        ++count;
    }
    return count;
}

int MapTabContentQt::populateUnitEventsList(const Unit* unit)
{
    if (!unitEventsList_)
    {
        return 0;
    }

    unitEventsList_->clear();
    if (!unit || !appData_)
    {
        return 0;
    }

    std::vector<const Event*> unitEvents = appData_->eventRepository().findLatestEventsByUnitId(unit->getUnitNumber());
    std::sort(unitEvents.begin(), unitEvents.end(),
              [](const Event* left, const Event* right)
              {
                  return left && right ? left->getEventId() < right->getEventId() : left != nullptr;
              });

    int count = 0;
    for (const Event* eventValue : unitEvents)
    {
        if (!eventValue)
        {
            continue;
        }
        unitEventsList_->addItem(toQString(eventValue->getMessage()));
        ++count;
    }

    return count;
}

void MapTabContentQt::updateUnitWeightAndCapacities(const Unit* unit)
{
    if (!unit || !appData_ || !unitWeightLabel_ || !unitCapacitiesLabel_)
    {
        return;
    }

    const UnitCapacityUtils::UnitCapacities caps = UnitCapacityUtils::getUnitCapacities(*unit, *appData_);
    const UnitCapacityUtils::ShipCapacities ship = UnitCapacityUtils::getShipCapacities(*unit, *appData_);

    capacityWalkDisplay_ = caps.walkCapacity;
    capacityRideDisplay_ = caps.rideCapacity;
    capacityFlyDisplay_ = caps.flyCapacity;
    capacitySwimDisplay_ = caps.swimCapacity;
    showRideCapacity_ = caps.hasRideSource;
    showFlyCapacity_ = caps.hasFlySource;
    showSwimCapacity_ = caps.hasSwimSource;
    hasCapacityValues_ = true;
    shipCapacityDisplay_ = ship.shipCapacity;
    shipFreeCapacityDisplay_ = ship.shipFreeCapacity;
    shipSkillNeedDisplay_ = ship.shipSkillNeed;
    shipOwnerSailingDisplay_ = ship.ownerSailContrib;
    hasShipCapacityValues_ = ship.hasCapacityValues;
    hasShipOwnerSkillValues_ = ship.hasOwnerSkillValues;
    shipIsFlying_ = ship.isFlying;

    unitWeightLabel_->setText(QString("Weight: %1").arg(caps.totalWeight));

    QString capacities = QString("Walk: %1").arg(caps.walkCapacity);
    if (showRideCapacity_)
    {
        capacities += QString(" Ride: %1").arg(caps.rideCapacity);
    }
    if (showFlyCapacity_)
    {
        capacities += QString(" Fly: %1").arg(caps.flyCapacity);
    }
    if (showSwimCapacity_)
    {
        capacities += QString(" Swim: %1").arg(caps.swimCapacity);
    }
    unitCapacitiesLabel_->setText(capacities);
}

void MapTabContentQt::updateUnitWeightAndCapacities(const UnitNew* unitNew)
{
    if (!unitNew || !appData_ || !unitWeightLabel_ || !unitCapacitiesLabel_)
    {
        return;
    }

    const UnitCapacityUtils::UnitCapacities caps = UnitCapacityUtils::getUnitCapacities(*unitNew, *appData_);
    const UnitCapacityUtils::ShipCapacities ship = UnitCapacityUtils::getShipCapacities(*unitNew, *appData_);

    capacityWalkDisplay_ = caps.walkCapacity;
    capacityRideDisplay_ = caps.rideCapacity;
    capacityFlyDisplay_ = caps.flyCapacity;
    capacitySwimDisplay_ = caps.swimCapacity;
    showRideCapacity_ = caps.hasRideSource;
    showFlyCapacity_ = caps.hasFlySource;
    showSwimCapacity_ = caps.hasSwimSource;
    hasCapacityValues_ = true;
    shipCapacityDisplay_ = ship.shipCapacity;
    shipFreeCapacityDisplay_ = ship.shipFreeCapacity;
    shipSkillNeedDisplay_ = ship.shipSkillNeed;
    shipOwnerSailingDisplay_ = ship.ownerSailContrib;
    hasShipCapacityValues_ = ship.hasCapacityValues;
    hasShipOwnerSkillValues_ = ship.hasOwnerSkillValues;
    shipIsFlying_ = ship.isFlying;

    unitWeightLabel_->setText(QString("Weight: %1").arg(caps.totalWeight));

    QString capacities = QString("Walk: %1").arg(caps.walkCapacity);
    if (showRideCapacity_)
    {
        capacities += QString(" Ride: %1").arg(caps.rideCapacity);
    }
    if (showFlyCapacity_)
    {
        capacities += QString(" Fly: %1").arg(caps.flyCapacity);
    }
    if (showSwimCapacity_)
    {
        capacities += QString(" Swim: %1").arg(caps.swimCapacity);
    }
    unitCapacitiesLabel_->setText(capacities);
}

void MapTabContentQt::updateUnitDetailsTabCaptions(int errorCount, int warningCount, int eventCount)
{
    if (!unitDetailsTabs_)
    {
        return;
    }

    unitDetailsTabs_->setTabText(2, "Orders");
    unitDetailsTabs_->setTabText(4, errorCount > 0 ? "Errors*" : "Errors");
    unitDetailsTabs_->setTabText(5, warningCount > 0 ? "*Warnings" : "Warnings");
    unitDetailsTabs_->setTabText(3, eventCount > 0 ? "Events*" : "Events");
}

void MapTabContentQt::updateUnitDetailsTabVisibility()
{
    // Qt tabs handle view switching natively; keep this for parity with Win32 structure.
}

void MapTabContentQt::updateSelectedUnitDetailsByNumber(int unitNumber)
{
    if (!appData_)
    {
        selectedUnitIsNew_ = false;
        if (mapCanvas_)
        {
            mapCanvas_->clearMovePathOverlay();
        }
        clearSelectedUnitDetails();
        return;
    }

    if (selectedUnitIsNew_)
    {
        const UnitNew* unitNew = appData_->unitNewRepository().findByNumberAndCoordinates(
            unitNumber, selectedRegionX_, selectedRegionY_, selectedZ_);
        if (!unitNew)
        {
            selectedUnitIsNew_ = false;
            clearSelectedUnitDetails();
            return;
        }

        selectedUnitLabel_->setText(toQString(
            unitNew->getUnitNameAfterOrders() +
            L", [New " + std::to_wstring(unitNew->getUnitNumber()) +
            L"] (origin unit: " + std::to_wstring(unitNew->getOriginUnit()) + L")"));
        unitCoordinatesLabel_->setText(QString("(%1,%2,%3)")
            .arg(unitNew->getXCoordinate())
            .arg(unitNew->getYCoordinate())
            .arg(unitNew->getZCoordinate()));
        unitFlagsLabel_->setText(toQString(L"Flags: " + StringUtils::joinLines(unitNew->getFlags(), L", ")));

        if (!unitNew->getWarnings().empty())
        {
            unitWarningLabel_->setText(toQString(unitNew->getWarnings().front()));
        }
        else
        {
            unitWarningLabel_->clear();
        }

        populateItemsForSelectedUnit(unitNew);
        populateSkillsList(unitNew);
        const int errorCount = populateErrorsList(nullptr);
        const int warningCount = populateWarningsList(unitNew);
        const int eventCount = populateUnitEventsList(nullptr);
        updateUnitDetailsTabCaptions(errorCount, warningCount, eventCount);
        updateUnitWeightAndCapacities(unitNew);

        if (ordersEditor_)
        {
            ordersEditor_->clear();
        }
        if (mapCanvas_)
        {
            mapCanvas_->clearMovePathOverlay();
        }
        setOrdersEditingEnabled(false);
        return;
    }

    const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
    if (!unit)
    {
        clearSelectedUnitDetails();
        return;
    }

    selectedUnitLabel_->setText(toQString(
        unit->getUnitNameAfterOrders() + L" [" + std::to_wstring(unit->getUnitNumber()) + L"]"));
    unitCoordinatesLabel_->setText(QString("(%1,%2,%3)")
        .arg(unit->getXCoordinate())
        .arg(unit->getYCoordinate())
        .arg(unit->getZCoordinate()));
    unitFlagsLabel_->setText(toQString(L"Flags: " + StringUtils::joinLines(unit->getFlags(), L", ")));

    if (!unit->getWarnings().empty())
    {
        unitWarningLabel_->setText(toQString(unit->getWarnings().front()));
    }
    else
    {
        unitWarningLabel_->clear();
    }

    populateItemsForSelectedUnit(unit);
    populateSkillsList(unit);
    const int errorCount = populateErrorsList(unit);
    const int warningCount = populateWarningsList(unit);
    const int eventCount = populateUnitEventsList(unit);
    updateUnitDetailsTabCaptions(errorCount, warningCount, eventCount);
    updateUnitWeightAndCapacities(unit);

    if (ordersEditor_)
    {
        ordersEditor_->setPlainText(toQString(StringUtils::joinLines(unit->getOrders()) + L"\r\n"));
    }

    std::vector<std::pair<int, int>> movePathCoordinates;
    bool movePathIsSail = false;
    bool movePathHasNegativeCapacity = false;
    bool movePathSailRouteInvalid = false;

    const auto& orders = unit->getOrders();
    for (const auto& order : orders)
    {
        const std::wstring trimmedOrder = StringUtils::trimWhitespace(order);
        const std::wstring lowerOrder = StringUtils::toLower(trimmedOrder);

        const std::size_t movePos = lowerOrder.find(L"move");
        const std::size_t advancePos = lowerOrder.find(L"advance");
        const std::size_t sailPos = lowerOrder.find(L"sail");
        if ((movePos != std::wstring::npos && movePos <= 2) ||
            (advancePos != std::wstring::npos && advancePos <= 2) ||
            (sailPos != std::wstring::npos && sailPos <= 2))
        {
            std::vector<std::wstring> directions;
            std::wstringstream stream(trimmedOrder);
            std::wstring token;
            bool foundMoveLikeCommand = false;
            bool isSailCommand = false;
            bool inStructure = (unit->getStructureId() > 0);

            while (stream >> token)
            {
                const std::wstring lowerToken = StringUtils::toLower(token);
                if (lowerToken == L"move" || lowerToken == L"advance" || lowerToken == L"sail" ||
                    (!lowerToken.empty() && lowerToken[0] == L'@' &&
                     (lowerToken.find(L"move") != std::wstring::npos ||
                      lowerToken.find(L"advance") != std::wstring::npos ||
                      lowerToken.find(L"sail") != std::wstring::npos)))
                {
                    foundMoveLikeCommand = true;
                    isSailCommand = (lowerToken.find(L"sail") != std::wstring::npos);
                }
                else if (foundMoveLikeCommand)
                {
                    const bool isNumericToken = !token.empty() &&
                        std::all_of(token.begin(), token.end(),
                            [](wchar_t ch) { return iswdigit(ch) != 0; });
                    if (isNumericToken)
                    {
                        inStructure = true;
                        continue;
                    }

                    if (lowerToken == L"in")
                    {
                        if (!inStructure)
                        {
                            continue;
                        }
                        continue;
                    }

                    const std::wstring normalized = HexDirectionUtils::normalizeHexDirection(lowerToken);
                    if (!normalized.empty())
                    {
                        directions.push_back(normalized);
                        inStructure = false;
                    }
                }
            }

            if (!directions.empty())
            {
                if (isSailCommand)
                {
                    if (!hasShipOwnerSkillValues_)
                    {
                        break;
                    }

                    movePathCoordinates = HexDirectionUtils::calculateMovePathCoordinates(
                        unit->getXCoordinate(),
                        unit->getYCoordinate(),
                        directions);
                    movePathIsSail = true;

                    bool routeInvalid = false;
                    if (!shipIsFlying_)
                    {
                        const RegionRepository& regionRepository = appData_->regionRepository();
                        const int unitZ = unit->getZCoordinate();
                        for (std::size_t si = 0; si + 1 < movePathCoordinates.size(); ++si)
                        {
                            const int sx = movePathCoordinates[si].first;
                            const int sy = movePathCoordinates[si].second;
                            const int ex = movePathCoordinates[si + 1].first;
                            const int ey = movePathCoordinates[si + 1].second;
                            const Region* startRegion = regionRepository.findByCoordinates(sx, sy, unitZ);
                            const Region* endRegion = regionRepository.findByCoordinates(ex, ey, unitZ);
                            const bool startOcean = startRegion && startRegion->isOcean();
                            const bool endOcean = endRegion && endRegion->isOcean();
                            if (!startOcean && !endOcean)
                            {
                                routeInvalid = true;
                                break;
                            }
                        }
                    }
                    movePathSailRouteInvalid = routeInvalid;

                    const bool shipSkillInsufficient =
                        hasShipOwnerSkillValues_ && shipOwnerSailingDisplay_ < shipSkillNeedDisplay_;
                    movePathHasNegativeCapacity = routeInvalid || (shipFreeCapacityDisplay_ < 0) || shipSkillInsufficient;
                }
                else
                {
                    movePathCoordinates = HexDirectionUtils::calculateMovePathCoordinates(
                        unit->getXCoordinate(),
                        unit->getYCoordinate(),
                        directions);
                    movePathIsSail = false;
                    movePathHasNegativeCapacity = (capacityWalkDisplay_ < 0);
                }
            }

            break;
        }
    }

    if (mapCanvas_)
    {
        mapCanvas_->setMovePathOverlay(movePathCoordinates,
                                       movePathIsSail,
                                       movePathHasNegativeCapacity,
                                       movePathSailRouteInvalid);
    }

    setOrdersEditingEnabled(canEditOrdersForUnit(unit));
}

void MapTabContentQt::clearSelectedUnitDetails()
{
    if (selectedUnitLabel_)
    {
        selectedUnitLabel_->clear();
    }
    if (unitCoordinatesLabel_)
    {
        unitCoordinatesLabel_->clear();
    }
    if (unitFlagsLabel_)
    {
        unitFlagsLabel_->clear();
    }
    if (unitWarningLabel_)
    {
        unitWarningLabel_->clear();
    }
    if (unitWeightLabel_)
    {
        unitWeightLabel_->clear();
    }
    if (unitCapacitiesLabel_)
    {
        unitCapacitiesLabel_->clear();
    }

    hasCapacityValues_ = false;
    capacityWalkDisplay_ = 0;
    capacityRideDisplay_ = 0;
    capacityFlyDisplay_ = 0;
    capacitySwimDisplay_ = 0;
    shipCapacityDisplay_ = 0;
    shipFreeCapacityDisplay_ = 0;
    shipSkillNeedDisplay_ = 0;
    shipOwnerSailingDisplay_ = 0;
    showRideCapacity_ = false;
    showFlyCapacity_ = false;
    showSwimCapacity_ = false;
    hasShipCapacityValues_ = false;
    hasShipOwnerSkillValues_ = false;
    shipIsFlying_ = false;

    if (unitItemsList_)
    {
        unitItemsList_->setRowCount(0);
    }
    if (unitSkillsList_)
    {
        unitSkillsList_->clear();
    }
    if (unitErrorsList_)
    {
        unitErrorsList_->clear();
    }
    if (unitWarningsList_)
    {
        unitWarningsList_->clear();
    }
    if (unitEventsList_)
    {
        unitEventsList_->clear();
    }
    if (ordersEditor_)
    {
        ordersEditor_->clear();
    }

    if (mapCanvas_)
    {
        mapCanvas_->clearMovePathOverlay();
    }

    setOrdersEditingEnabled(false);
    updateUnitDetailsTabCaptions(0, 0, 0);
}

void MapTabContentQt::setOrdersEditingEnabled(bool enabled)
{
    if (ordersEditor_)
    {
        // Keep editor focusable for UnitNew-origin flow, but make content read-only.
        ordersEditor_->setEnabled(true);
        ordersEditor_->setReadOnly(!enabled);
    }
    if (saveOrdersButton_)
    {
        saveOrdersButton_->setEnabled(enabled);
    }
}

bool MapTabContentQt::canEditOrdersForUnit(const Unit* unit) const
{
    if (!appData_ || !unit)
    {
        return false;
    }

    const auto& factionRepository = appData_->factionRepository();
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

    return mainFactionCount == 1 && unit->getFactionNumber() == mainFactionNumber;
}
