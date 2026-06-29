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
#include "Function/StringUtils.hpp"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>

#include <algorithm>
#include <cwctype>

// Helper to format a row with 2-3 columns for display in QListWidget
static QString formatResourceRow(const QString& col1, const QString& col2, const QString& col3 = QString())
{
    if (col3.isEmpty())
    {
        return QString("%1  %2").arg(col1, col2);
    }
    return QString("%1  %2  %3").arg(col1, col2, col3);
}

// Helper for 4-column rows (for-sale, wanted)
static QString formatForSaleRow(const QString& col1, const QString& col2, const QString& col3, const QString& col4)
{
    return QString("%1  %2  %3  %4").arg(col1, col2, col3, col4);
}

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

    std::wstring details;
    details += L"Coordinates: " + CoordinateFormattingUtils::formatCoordinates(
        region->getXCoordinate(),
        region->getYCoordinate(),
        region->getZCoordinate()
    ) + L"\n";
    details += L"Region Type: " + region->getRegionType() + L"\n";
    details += L"Peasants: " + region->getPeasantType() + L"\n";
    details += L"Province: " + region->getProvinceName() + L"\n";

    if (region->getContainsSettlement())
    {
        details += L"Settlement Type: " + region->getSettlementType() + L"\n";
        details += L"Settlement Name: " + region->getSettlementName();
    }

    // Display structure of selected unit if it's in this region
    if (appData_ && selectedUnitNumber_ > 0)
    {
        const Unit* selectedUnit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
        const int selectedDisplayStructureId = selectedUnit
            ? selectedUnit->getFutureStructureId()
            : 0;

        if (selectedUnit &&
            selectedUnit->getXCoordinate() == region->getXCoordinate() &&
            selectedUnit->getYCoordinate() == region->getYCoordinate() &&
            selectedUnit->getZCoordinate() == selectedZ_ &&
            selectedDisplayStructureId > 0)
        {
            const Structure* structure = appData_->structureRepository().findByIdAndCoordinates(
                selectedDisplayStructureId,
                selectedUnit->getXCoordinate(),
                selectedUnit->getYCoordinate(),
                selectedUnit->getZCoordinate());

            if (structure)
            {
                details += L"\nStructure: " + structure->getStructureType() + L" [" + std::to_wstring(selectedDisplayStructureId) + L"]";
                if (!structure->getStructureName().empty())
                {
                    details += L" - " + structure->getStructureName();
                }

                const StructInfo* structInfo = appData_->structInfoRepository().findByType(structure->getStructureType());
                if (structInfo && structInfo->getNeeds() > 0)
                {
                    details += L", needs " + std::to_wstring(structInfo->getNeeds());
                }

                const auto& fleetItems = structure->getFleetItems();
                if (!fleetItems.empty())
                {
                    for (const auto& itemEntry : fleetItems)
                    {
                        const std::wstring& itemToken = itemEntry.first;
                        const int amount = itemEntry.second;
                        const StructInfo* itemStructInfo = appData_->structInfoRepository().findByItemIdentifierToken(itemToken);
                        const std::wstring itemType = itemStructInfo ? itemStructInfo->getStructureType() : itemToken;
                        details += L"\n  " + std::to_wstring(amount) + L" " + itemType + L" [" + itemToken + L"]";
                    }
                }
            }
        }
    }

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

    const auto& resources = region->getResources();
    const auto& afterCommandResources = region->getResourcesAfterOrders();
    const int entertainmentAfterCommands = region->getEntertainmentAfterOrders();
    const int taxesAfterCommands = region->getTaxableIncomeAfterOrders();
    const int workWagesAfterCommands = region->getWagesAfterOrders();

    auto insertResourceRow = [this](const std::wstring& itemName, int amount, int amountAfterCommands)
    {
        const QString name = QString::fromStdWString(itemName);
        const QString before = QString::number(amount);
        const QString after = QString::number(amountAfterCommands);
        const QString row = formatResourceRow(name, before, after);

        auto* item = new QListWidgetItem(row);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        regionResourcesList_->addItem(item);
    };

    insertResourceRow(L"Entertainment", region->getEntertainment(), entertainmentAfterCommands);
    insertResourceRow(L"Taxes", region->getTaxableIncome(), taxesAfterCommands);
    insertResourceRow(L"Work wages", region->getWagesMax(), workWagesAfterCommands);

    for (const auto& [token, amount] : resources)
    {
        int amountAfterCommands = amount;
        auto afterIt = afterCommandResources.find(token);
        if (afterIt == afterCommandResources.end())
        {
            std::wstring tokenUpper = token;
            for (wchar_t& ch : tokenUpper)
            {
                ch = static_cast<wchar_t>(towupper(ch));
            }
            afterIt = afterCommandResources.find(tokenUpper);
        }
        if (afterIt != afterCommandResources.end())
        {
            amountAfterCommands = afterIt->second;
        }

        insertResourceRow(token, amount, amountAfterCommands);
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

    const auto& forSale = region->getForSale();
    const auto& afterCommandForSale = region->getForSaleAfterOrders();

    // Build a sorted list: men items first, then the rest, each group in token order.
    std::vector<std::wstring> sortedTokens;
    sortedTokens.reserve(forSale.size());
    for (const auto& [token, amountPrice] : forSale)
    {
        sortedTokens.push_back(token);
    }

    if (appData_)
    {
        std::stable_sort(sortedTokens.begin(), sortedTokens.end(),
            [this](const std::wstring& a, const std::wstring& b)
            {
                const Item* ia = appData_->itemRepository().findByIdentifierToken(a);
                const Item* ib = appData_->itemRepository().findByIdentifierToken(b);
                const bool manA = ia && ia->isMan();
                const bool manB = ib && ib->isMan();
                if (manA != manB)
                {
                    return manA > manB;
                }
                return false;
            });
    }

    for (const auto& token : sortedTokens)
    {
        const auto& amountPrice = forSale.at(token);
        const QString tokenStr = QString::fromStdWString(token);
        const QString amountStr = QString::number(amountPrice.first);
        const QString priceStr = QString::number(amountPrice.second);

        int amountAfterCommands = amountPrice.first;
        auto afterIt = afterCommandForSale.find(token);
        if (afterIt == afterCommandForSale.end())
        {
            std::wstring tokenUpper = token;
            for (wchar_t& ch : tokenUpper)
            {
                ch = static_cast<wchar_t>(towupper(ch));
            }
            afterIt = afterCommandForSale.find(tokenUpper);
        }
        if (afterIt != afterCommandForSale.end())
        {
            amountAfterCommands = afterIt->second.first;
        }
        const QString amountAfterStr = QString::number(amountAfterCommands);

        const QString row = formatForSaleRow(tokenStr, amountStr, priceStr, amountAfterStr);
        auto* item = new QListWidgetItem(row);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        regionForSaleList_->addItem(item);
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

    const auto& wanted = region->getWanted();
    const auto& afterCommandWanted = region->getWantedAfterOrders();

    for (const auto& [token, amountPrice] : wanted)
    {
        const QString tokenStr = QString::fromStdWString(token);
        const QString amountStr = QString::number(amountPrice.first);
        const QString priceStr = QString::number(amountPrice.second);

        int amountAfterCommands = amountPrice.first;
        auto afterIt = afterCommandWanted.find(token);
        if (afterIt == afterCommandWanted.end())
        {
            std::wstring tokenUpper = token;
            for (wchar_t& ch : tokenUpper)
            {
                ch = static_cast<wchar_t>(towupper(ch));
            }
            afterIt = afterCommandWanted.find(tokenUpper);
        }
        if (afterIt != afterCommandWanted.end())
        {
            amountAfterCommands = afterIt->second.first;
        }
        const QString amountAfterStr = QString::number(amountAfterCommands);

        const QString row = formatForSaleRow(tokenStr, amountStr, priceStr, amountAfterStr);
        auto* item = new QListWidgetItem(row);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        regionWantedList_->addItem(item);
    }
}
