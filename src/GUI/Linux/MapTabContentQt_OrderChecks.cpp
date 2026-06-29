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
#include "Function/AppDataUtils.hpp"
#include "Function/OrderWarningService.hpp"

#include <QLabel>

#include <algorithm>
#include <vector>

void MapTabContentQt::runOrderChecksForMainFaction()
{
    if (!appData_)
    {
        return;
    }

    saveOrdersToSelectedUnit();
    OrderWarningService::runForMainFaction(*appData_);

    populateUnitsForSelectedRegion();
    updateWarningsSummaryLabel();
    if (selectedUnitNumber_ != 0)
    {
        updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
    }
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

    std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(*appData_);

    if (warningUnits.empty())
    {
        return;
    }

    std::sort(warningUnits.begin(), warningUnits.end());

    int selectedIndex = -1;
    for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
    {
        if (warningUnits[static_cast<std::size_t>(index)] == selectedUnitNumber_)
        {
            selectedIndex = index;
            break;
        }
    }

    int targetIndex = selectedIndex - 1;
    if (selectedIndex < 0)
    {
        targetIndex = static_cast<int>(warningUnits.size()) - 1;
    }
    else if (targetIndex < 0)
    {
        targetIndex = static_cast<int>(warningUnits.size()) - 1;
    }

    selectUnitInMap(warningUnits[static_cast<std::size_t>(targetIndex)]);
}

void MapTabContentQt::selectNextWarningUnit()
{
    if (!appData_)
    {
        return;
    }

    std::vector<int> warningUnits = AppDataUtils::getWarningUnitNumbersForLatestPeriod(*appData_);

    if (warningUnits.empty())
    {
        return;
    }

    std::sort(warningUnits.begin(), warningUnits.end());

    int selectedIndex = -1;
    for (int index = 0; index < static_cast<int>(warningUnits.size()); ++index)
    {
        if (warningUnits[static_cast<std::size_t>(index)] == selectedUnitNumber_)
        {
            selectedIndex = index;
            break;
        }
    }

    int targetIndex = selectedIndex + 1;
    if (selectedIndex < 0)
    {
        targetIndex = 0;
    }
    else if (targetIndex >= static_cast<int>(warningUnits.size()))
    {
        targetIndex = 0;
    }

    selectUnitInMap(warningUnits[static_cast<std::size_t>(targetIndex)]);
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
