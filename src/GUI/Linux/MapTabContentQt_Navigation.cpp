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
#include "Data/UnitNew.hpp"
#include "Function/MapNavigationUtils.hpp"

#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
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

void MapTabContentQt::focusRegion(int x, int y, int z)
{
    if (!appData_)
    {
        return;
    }

    const Region* region = appData_->regionRepository().findByCoordinates(x, y, z);
    if (!region)
    {
        return;
    }

    selectedZ_ = z;
    hasSelectedRegion_ = true;
    selectedRegionX_ = x;
    selectedRegionY_ = y;
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedZ(selectedZ_);
        mapCanvas_->setSelectedRegion(true, selectedRegionX_, selectedRegionY_);
    }

    refresh();
    updateRegionDetailsView(region);
    if (mapCanvas_)
    {
        (void)mapCanvas_->centerOnRegion(selectedRegionX_, selectedRegionY_);
    }
}

void MapTabContentQt::selectUnitInMap(int unitNumber)
{
    if (!appData_)
    {
        return;
    }

    MapNavigationUtils::UnitSelectionContext context {};
    if (!MapNavigationUtils::tryBuildUnitSelectionContext(*appData_, unitNumber, context))
    {
        QMessageBox::warning(
            unitSearchEdit_,
            "Unit Not Found",
            QString::fromStdWString(MapNavigationUtils::buildUnitNotFoundMessage(unitNumber))
        );
        return;
    }

    // Update selected region
    selectedZ_ = context.zCoordinate;
    hasSelectedRegion_ = true;
    selectedRegionX_ = context.xCoordinate;
    selectedRegionY_ = context.yCoordinate;
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedZ(selectedZ_);
        mapCanvas_->setSelectedRegion(hasSelectedRegion_, selectedRegionX_, selectedRegionY_);
    }

    // Refresh the entire map tab (units list, details, region view)
    refresh();

    // Update region details view
    updateRegionDetailsView(context.region);

    // Find and select the unit row in the units list
    if (unitsList_)
    {
        // Qt6 removed setItemSelected; clearSelection is the direct replacement.
        unitsList_->clearSelection();

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

    if (mapCanvas_)
    {
        (void)mapCanvas_->centerOnRegion(selectedRegionX_, selectedRegionY_);
    }
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

    const MapNavigationUtils::UnitSearchResult searchResult =
        MapNavigationUtils::resolveUnitSearch(*appData_, unitSearchEdit_->text().toStdWString());
    if (searchResult.status == MapNavigationUtils::UnitSearchStatus::Found)
    {
        selectUnitInMap(searchResult.unitNumber);
        return;
    }

    if (searchResult.status == MapNavigationUtils::UnitSearchStatus::NotFound)
    {
        QMessageBox::warning(
            unitSearchEdit_,
            "Unit Not Found",
            QString::fromStdWString(MapNavigationUtils::buildUnitNotFoundMessage(searchResult.unitNumber))
        );
    }
}
