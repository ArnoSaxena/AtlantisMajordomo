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
 * File: MapTabContentQt_OrderChecks.cpp
 *
 * Step 7.5 - warning navigation and order-check orchestration.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Unit.hpp"
#include "Function/OrderChecksUtils.hpp"

#include <QLabel>

void MapTabContentQt::runOrderChecksForMainFaction()
{
    if (!appData_)
    {
        return;
    }

    OrderChecksUtils::runOrderChecksForMainFaction(
        *appData_,
        selectedUnitNumber_,
        [this]() { saveOrdersToSelectedUnit(); },
        [this]() { populateUnitsForSelectedRegion(); },
        [this]() { updateWarningsSummaryLabel(); },
        [this](int unitNumber) { updateSelectedUnitDetailsByNumber(unitNumber); });
}

void MapTabContentQt::updateWarningsSummaryLabel()
{
    if (!warningsCountLabel_ || !appData_)
    {
        return;
    }

    const int warningCount = appData_->unitRepository().countTotalWarnings();
    const std::wstring text = L"Warnings: " + std::to_wstring(warningCount);
    warningsCountLabel_->setText(QString::fromStdWString(text));
}

void MapTabContentQt::onPrevWarningClicked()
{
    selectPreviousWarningUnit();
}

void MapTabContentQt::onNextWarningClicked()
{
    selectNextWarningUnit();
}

void MapTabContentQt::onClearWarningClicked()
{
    clearWarningsForSelectedUnit();
}

void MapTabContentQt::selectPreviousWarningUnit()
{
    if (!appData_)
    {
        return;
    }

    const int previousWarningUnitNumber =
        OrderChecksUtils::selectPreviousWarningUnitNumber(*appData_, selectedUnitNumber_);
    if (previousWarningUnitNumber == 0)
    {
        return;
    }

    selectUnitInMap(previousWarningUnitNumber);
}

void MapTabContentQt::selectNextWarningUnit()
{
    if (!appData_)
    {
        return;
    }

    const int nextWarningUnitNumber =
        OrderChecksUtils::selectNextWarningUnitNumber(*appData_, selectedUnitNumber_);
    if (nextWarningUnitNumber == 0)
    {
        return;
    }

    selectUnitInMap(nextWarningUnitNumber);
}

void MapTabContentQt::clearWarningsForSelectedUnit()
{
    if (!appData_ || selectedUnitNumber_ == 0)
    {
        return;
    }

    Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    if (!unit)
    {
        return;
    }

    unit->clearWarnings();
    populateUnitsForSelectedRegion();
    updateWarningsSummaryLabel();
    updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
}
