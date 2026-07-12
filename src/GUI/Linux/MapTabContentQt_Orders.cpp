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
 * File: MapTabContentQt_Orders.cpp
 *
 * Step 7.4 - orders editor + save/edit logic.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
#include "Function/StringUtils.hpp"

#include <QAction>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>

#include <string>
#include <vector>

void MapTabContentQt::onSaveOrdersClicked()
{
    saveOrdersToSelectedUnit();
}

void MapTabContentQt::saveOrdersToSelectedUnit()
{
    if (!appData_ || selectedUnitNumber_ == 0 || !ordersEditor_)
    {
        return;
    }

    Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    if (!unit || !canEditOrdersForUnit(unit))
    {
        return;
    }

    const std::wstring text = ordersEditor_->toPlainText().toStdWString();
    const std::vector<std::wstring> orders = StringUtils::splitLines(text);
    unit->setOrders(orders);

    // Keep stale OrderRepository cleanup based on current UnitNew state, then recalc
    // after UnitNew entries are rebuilt from the just-saved FORM/END blocks.
    OrderBusinessLogic::syncOrderRepositoryForSavedUnit(*appData_, selectedUnitNumber_, false);

    // Handle FORM/END blocks: extract new unit numbers and remove previous UnitNew entries
    // originating from this unit, then create new UnitNew snapshot entries.
    appData_->unitNewRepository().removeByOriginUnit(selectedUnitNumber_);

    const std::vector<int> formUnitNumbers =
        OrderParsingUtils::extractFormNewUnitNumbers(orders);

    for (int formUnitNumber : formUnitNumbers)
    {
        // Create a UnitNew snapshot for each newly formed unit.
        // The snapshot will be orderless and marked with the origin unit number.
        const int x = unit->getXCoordinate();
        const int y = unit->getYCoordinate();
        const int z = unit->getZCoordinate();
        const std::wstring formUnitName = L"New Unit";

        appData_->unitNewRepository().add(
            formUnitNumber,
            formUnitName,
            unit->getStructureId(),  // structureId - inherit from origin unit
            x, y, z,
            unit->getFlags(),  // flags
            std::map<std::wstring, int>(),  // itemCounts
            0,  // weight
            0,  // capacityWalk
            0,  // capacityRide
            0,  // capacityFly
            0,  // capacitySwim
            std::map<std::wstring, int>(),  // skills
            unit->getMonth(),  // month
            unit->getYear(),  // year
            selectedUnitNumber_  // originUnit
        );
    }

    CommandSimulationService::recalculateAfterOrdersValues(*appData_);
    OrderWarningService::runForMainFaction(*appData_);

    updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
    populateUnitsForSelectedRegion();
    updateWarningsSummaryLabel();
}

void MapTabContentQt::appendOrderLineToOrdersEditor(const std::wstring& orderLine)
{
    if (!ordersEditor_)
    {
        return;
    }

    const std::wstring trimmedOrderLine = StringUtils::trimWhitespace(orderLine);
    if (trimmedOrderLine.empty())
    {
        return;
    }

    QString ordersText = ordersEditor_->toPlainText();

    if (!ordersText.isEmpty())
    {
        const QChar lastChar = ordersText.at(ordersText.length() - 1);
        if (lastChar != QChar(L'\n') && lastChar != QChar(L'\r'))
        {
            ordersText += QChar(L'\n');
        }
    }

    ordersText += QString::fromStdWString(trimmedOrderLine);
    if (ordersText.isEmpty() || ordersText.at(ordersText.length() - 1) != QChar(L'\n'))
    {
        ordersText += QChar(L'\n');
    }

    ordersEditor_->setPlainText(ordersText);

    // Move cursor to end
    QTextCursor cursor = ordersEditor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    ordersEditor_->setTextCursor(cursor);
    ordersEditor_->setFocus();
}

