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
 * File: MapTabContentQt_Navigation.cpp
 *
 * Step 7.7 - unit search and selection callbacks.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Region.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"

#include <QLineEdit>
#include <QMessageBox>
#include <QTableWidget>

#include <string>

void MapTabContentQt::navigateToSkillList(const std::wstring& skillToken)
{
    if (skillToken.empty())
    {
        return;
    }

    // Emit the Qt signal for skills navigation
    emit navigateToSkill(QString::fromStdWString(skillToken));
}

void MapTabContentQt::selectUnitInMap(int unitNumber)
{
    if (!appData_)
    {
        return;
    }

    const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
    if (!unit)
    {
        QMessageBox::warning(
            unitSearchEdit_,
            "Unit Not Found",
            QString("Unit %1 was not found in the database.").arg(unitNumber)
        );
        return;
    }

    // Update selected region
    selectedZ_ = unit->getZCoordinate();
    hasSelectedRegion_ = true;
    selectedRegionX_ = unit->getXCoordinate();
    selectedRegionY_ = unit->getYCoordinate();
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedZ(selectedZ_);
        mapCanvas_->setSelectedRegion(hasSelectedRegion_, selectedRegionX_, selectedRegionY_);
    }

    // Refresh the entire map tab (units list, details, region view)
    refresh();

    // Update region details view
    const Region* region = appData_->regionRepository().findByCoordinates(
        selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!region)
    {
        region = appData_->regionRepository().findByCoordinates(
            selectedRegionX_, selectedRegionY_);
    }
    updateRegionDetailsView(region);

    // Find and select the unit row in the units list
    if (unitsList_)
    {
        // Clear any existing selection
        for (int row = 0; row < unitsList_->rowCount(); ++row)
        {
            unitsList_->setItemSelected(unitsList_->item(row, 0), false);
        }

        // Search for the unit by its number (stored in Qt::UserRole of first column)
        for (int row = 0; row < unitsList_->rowCount(); ++row)
        {
            QTableWidgetItem* item = unitsList_->item(row, 0);
            if (item)
            {
                const int itemValue = item->data(Qt::UserRole).toInt();
                if (itemValue == unitNumber)
                {
                    // Found the unit row; select it
                    unitsList_->selectRow(row);
                    unitsList_->scrollToItem(item, QAbstractItemView::EnsureVisible);
                    unitsList_->setFocus();
                    updateSelectedUnitFromList();
                    break;
                }
            }
        }
    }

    // TODO (step 7.9.2): Map canvas scrolling to center on the selected region.
    // Currently, mapCanvas_ is a QWidget* placeholder. Once 7.9.1 instantiates
    // MapCanvasWidget, this section will scroll the map to the region's center
    // and invalidate for repainting.
}

bool MapTabContentQt::focusOriginUnitForSelectedUnitNew()
{
    if (!appData_ || !selectedUnitIsNew_ || selectedUnitNumber_ == 0)
    {
        return false;
    }

    const UnitNew* unitNew = appData_->unitNewRepository().findByNumberAndCoordinates(
        selectedUnitNumber_, selectedRegionX_, selectedRegionY_, selectedZ_);
    if (!unitNew)
    {
        return false;
    }

    const int originUnitNumber = unitNew->getOriginUnit();
    if (originUnitNumber <= 0)
    {
        return false;
    }

    selectedUnitIsNew_ = false;
    selectUnitInMap(originUnitNumber);
    if (ordersEditor_)
    {
        ordersEditor_->setFocus();
    }
    return true;
}

void MapTabContentQt::searchAndSelectUnitById()
{
    if (!appData_ || !unitSearchEdit_)
    {
        return;
    }

    const QString unitIdText = unitSearchEdit_->text();
    if (unitIdText.isEmpty())
    {
        return;
    }

    bool ok = false;
    int unitNumber = unitIdText.toInt(&ok);
    if (!ok || unitNumber <= 0)
    {
        return;
    }

    const Unit* unit = appData_->unitRepository().findByNumber(unitNumber);
    if (!unit)
    {
        QMessageBox::warning(
            unitSearchEdit_,
            "Unit Not Found",
            QString("Unit %1 was not found in the database.").arg(unitNumber)
        );
        return;
    }

    selectUnitInMap(unitNumber);
}
