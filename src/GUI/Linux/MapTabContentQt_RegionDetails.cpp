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
 * File: MapTabContentQt_RegionDetails.cpp
 *
 * Step 7.6 - region info pane and resource/for-sale/wanted lists.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/StructInfo.hpp"
#include "Data/Structure.hpp"
#include "Data/Unit.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "Function/MapRegionDetailsUtils.hpp"
#include "Function/StringUtils.hpp"

#include <QLabel>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <algorithm>
#include <cwctype>

void MapTabContentQt::updateRegionDetailsView(const Region* region)
{
    if (!regionDetailsView_)
    {
        return;
    }

    if (!region)
    {
        if (regionDateLabel_)
        {
            regionDateLabel_->setText(QString::fromStdWString(AppDataUtils::buildDateLabelText(appData_)));
        }
        regionDetailsView_->setPlainText("No region selected");
        populateResourcesList(nullptr);
        populateForSaleList(nullptr);
        populateWantedList(nullptr);
        return;
    }

    if (regionDateLabel_)
    {
        regionDateLabel_->setText(QString::fromStdWString(AppDataUtils::buildDateLabelText(appData_)));
    }

    const std::wstring details = MapRegionDetailsUtils::buildRegionSummaryText(
        *region,
        appData_,
        selectedUnitNumber_,
        selectedZ_,
        L"\n");

    regionDetailsView_->setPlainText(QString::fromStdWString(details));
    populateResourcesList(region);
    populateForSaleList(region);
    populateWantedList(region);
}

void MapTabContentQt::populateResourcesList(const Region* region)
{
    if (!regionResourcesList_)
    {
        return;
    }

    regionResourcesList_->clear();

    if (!region)
    {
        return;
    }

    auto insertResourceRow = [this](const std::wstring& itemName, int amount, int amountAfterCommands)
    {
        auto* treeItem = new QTreeWidgetItem(regionResourcesList_);
        treeItem->setText(0, QString::fromStdWString(itemName));
        treeItem->setText(1, QString::number(amount));
        treeItem->setText(2, QString::number(amountAfterCommands));
        treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsSelectable);
        // Right-align the numeric columns.
        treeItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        treeItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    };

    const std::vector<MapRegionDetailsUtils::ResourceRow> rows =
        MapRegionDetailsUtils::buildResourcesRows(*region);
    for (const MapRegionDetailsUtils::ResourceRow& row : rows)
    {
        insertResourceRow(row.token, row.amount, row.amountAfterOrders);
    }
}

void MapTabContentQt::populateForSaleList(const Region* region)
{
    if (!regionForSaleList_)
    {
        return;
    }

    regionForSaleList_->clear();

    if (!region)
    {
        return;
    }

    const std::vector<MapRegionDetailsUtils::ForSaleRow> rows =
        MapRegionDetailsUtils::buildForSaleRows(*region, appData_);

    for (const MapRegionDetailsUtils::ForSaleRow& row : rows)
    {
        auto* treeItem = new QTreeWidgetItem(regionForSaleList_);
        treeItem->setText(0, QString::fromStdWString(row.token));
        treeItem->setText(1, QString::number(row.amount));
        treeItem->setText(2, QString::number(row.price));
        treeItem->setText(3, QString::number(row.amountAfterOrders));
        treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsSelectable);
        treeItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        treeItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
        treeItem->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
    }
}

void MapTabContentQt::populateWantedList(const Region* region)
{
    if (!regionWantedList_)
    {
        return;
    }

    regionWantedList_->clear();

    if (!region)
    {
        return;
    }

    const std::vector<MapRegionDetailsUtils::WantedRow> rows =
        MapRegionDetailsUtils::buildWantedRows(*region);

    for (const MapRegionDetailsUtils::WantedRow& row : rows)
    {
        auto* treeItem = new QTreeWidgetItem(regionWantedList_);
        treeItem->setText(0, QString::fromStdWString(row.token));
        treeItem->setText(1, QString::number(row.amount));
        treeItem->setText(2, QString::number(row.price));
        treeItem->setText(3, QString::number(row.amountAfterOrders));
        treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsSelectable);
        treeItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        treeItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
        treeItem->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
    }
}
