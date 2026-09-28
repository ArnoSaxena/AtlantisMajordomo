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
 * File: TradeTaxTabContentQt.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/TradeTaxTabContentQt.hpp"

#include "Function/CoordinateFormattingUtils.hpp"
#include "Data/AppData.hpp"

#include <QButtonGroup>
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <set>

namespace
{
// Selected radio button background.
const QColor kSelectedButtonBackgroundColor(0, 96, 32);
// Selected radio button text.
const QColor kSelectedButtonTextColor(255, 255, 255);
// Tax/resource row is fully handled or exhausted.
const QColor kSatisfiedRowColor(204, 255, 204);
// Tax row has less than one full potential taxer remaining.
const QColor kPartialTaxRowColor(255, 245, 180);
// Tax/resource row still needs attention.
const QColor kAttentionRowColor(255, 210, 210);

QString toQString(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

QTableWidgetItem* makeItem(const QString& text)
{
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

QColor colorForSeverity(TradeTaxUtils::RowSeverity severity)
{
    switch (severity)
    {
        case TradeTaxUtils::RowSeverity::Green:
            return kSatisfiedRowColor;
        case TradeTaxUtils::RowSeverity::Yellow:
            return kPartialTaxRowColor;
        case TradeTaxUtils::RowSeverity::Red:
        default:
            return kAttentionRowColor;
    }
}
}

TradeTaxTabContentQt::TradeTaxTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    summaryLabel_ = new QLabel("Tax Regions: 0, Trade Regions: 0", this);

    taxButton_ = new QPushButton("Tax", this);
    resourcesButton_ = new QPushButton("Resources", this);
    taxButton_->setCheckable(true);
    resourcesButton_->setCheckable(true);
    taxButton_->setChecked(true);

    const QString selectedStyle = QString(
        "QPushButton:checked { background-color: rgb(%1,%2,%3); color: rgb(%4,%5,%6); }")
        .arg(kSelectedButtonBackgroundColor.red())
        .arg(kSelectedButtonBackgroundColor.green())
        .arg(kSelectedButtonBackgroundColor.blue())
        .arg(kSelectedButtonTextColor.red())
        .arg(kSelectedButtonTextColor.green())
        .arg(kSelectedButtonTextColor.blue());
    taxButton_->setStyleSheet(selectedStyle);
    resourcesButton_->setStyleSheet(selectedStyle);

    buttonGroup_ = new QButtonGroup(this);
    buttonGroup_->setExclusive(true);
    buttonGroup_->addButton(taxButton_, 0);
    buttonGroup_->addButton(resourcesButton_, 1);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(6);
    buttonLayout->addWidget(taxButton_);
    buttonLayout->addWidget(resourcesButton_);
    buttonLayout->addStretch(1);

    table_ = new QTableWidget(this);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);
    layout->addWidget(summaryLabel_);
    layout->addLayout(buttonLayout);
    layout->addWidget(table_, 1);
    setLayout(layout);

    connect(buttonGroup_, &QButtonGroup::idClicked, this, [this](int id)
    {
        setMode(id == 0 ? Mode::Tax : Mode::Resources);
    });

    connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int)
    {
        const QTableWidgetItem* coordinateItem = table_->item(row, 0);
        if (!coordinateItem)
        {
            return;
        }

        int x = 0;
        int y = 0;
        int z = 1;
        if (CoordinateFormattingUtils::tryParseCoordinates(coordinateItem->text().toStdWString(), x, y, z))
        {
            emit navigateToRegion(x, y, z);
        }
    });

    updateRows();
}

void TradeTaxTabContentQt::refresh()
{
    updateSummaryLabel();
    updateRows();
}

void TradeTaxTabContentQt::setMode(Mode mode)
{
    if (mode_ == mode)
    {
        return;
    }

    mode_ = mode;
    updateRows();
}

void TradeTaxTabContentQt::updateSummaryLabel()
{
    if (!appData_ || !summaryLabel_)
    {
        return;
    }

    const std::vector<TradeTaxUtils::TaxRow> taxRows = TradeTaxUtils::buildTaxRows(*appData_);
    const std::vector<TradeTaxUtils::ResourceRow> resourceRows = TradeTaxUtils::buildResourceRows(*appData_);
    std::set<std::wstring> tradeRegions;
    for (const TradeTaxUtils::ResourceRow& row : resourceRows)
    {
        tradeRegions.insert(row.coordinates);
    }

    summaryLabel_->setText(QString("Tax Regions: %1, Trade Regions: %2")
        .arg(taxRows.size())
        .arg(tradeRegions.size()));
}

void TradeTaxTabContentQt::updateRows()
{
    table_->clear();
    table_->setRowCount(0);
    table_->setColumnCount(4);

    if (mode_ == Mode::Tax)
    {
        table_->setHorizontalHeaderLabels({ "coordinates", "max tax", "untaxed", "potential taxers" });
        populateTaxRows();
    }
    else
    {
        table_->setHorizontalHeaderLabels({ "coordinates", "resource", "max amount", "after com" });
        populateResourceRows();
    }

    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
}

void TradeTaxTabContentQt::populateTaxRows()
{
    if (!appData_)
    {
        return;
    }

    const std::vector<TradeTaxUtils::TaxRow> rows = TradeTaxUtils::buildTaxRows(*appData_);
    table_->setRowCount(static_cast<int>(rows.size()));
    for (int rowIndex = 0; rowIndex < static_cast<int>(rows.size()); ++rowIndex)
    {
        const TradeTaxUtils::TaxRow& row = rows[static_cast<std::size_t>(rowIndex)];
        table_->setItem(rowIndex, 0, makeItem(toQString(row.coordinates)));
        table_->setItem(rowIndex, 1, makeItem(QString::number(row.maxTax)));
        table_->setItem(rowIndex, 2, makeItem(QString::number(row.untaxed)));
        table_->setItem(rowIndex, 3, makeItem(QString::number(row.potentialTaxers)));
        setRowBackground(rowIndex, row.severity);
    }
}

void TradeTaxTabContentQt::populateResourceRows()
{
    if (!appData_)
    {
        return;
    }

    const std::vector<TradeTaxUtils::ResourceRow> rows = TradeTaxUtils::buildResourceRows(*appData_);
    table_->setRowCount(static_cast<int>(rows.size()));
    for (int rowIndex = 0; rowIndex < static_cast<int>(rows.size()); ++rowIndex)
    {
        const TradeTaxUtils::ResourceRow& row = rows[static_cast<std::size_t>(rowIndex)];
        table_->setItem(rowIndex, 0, makeItem(toQString(row.coordinates)));
        table_->setItem(rowIndex, 1, makeItem(toQString(row.resourceToken)));
        table_->setItem(rowIndex, 2, makeItem(QString::number(row.maxAmount)));
        table_->setItem(rowIndex, 3, makeItem(QString::number(row.afterCommandsAmount)));
        setRowBackground(rowIndex, row.severity);
    }
}

void TradeTaxTabContentQt::setRowBackground(int row, TradeTaxUtils::RowSeverity severity)
{
    const QBrush brush(colorForSeverity(severity));
    for (int column = 0; column < table_->columnCount(); ++column)
    {
        QTableWidgetItem* item = table_->item(row, column);
        if (item)
        {
            item->setBackground(brush);
        }
    }
}
