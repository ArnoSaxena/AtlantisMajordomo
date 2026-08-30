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
 * File: BattlesTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/BattlesTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Battle.hpp"
#include "Data/BattleRepository.hpp"
#include "Function/BattleFormattingUtils.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QVBoxLayout>

#include <cstddef>
#include <string>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

BattlesTabContentQt::BattlesTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    // ---- Left pane: date combo + battles list ------------------------------
    QLabel* dateLabel = new QLabel("Date:", this);

    dateCombo_ = new QComboBox(this);
    dateCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    QHBoxLayout* dateRow = new QHBoxLayout;
    dateRow->setContentsMargins(0, 0, 0, 0);
    dateRow->setSpacing(6);
    dateRow->addWidget(dateLabel);
    dateRow->addWidget(dateCombo_);
    dateRow->addStretch();

    battlesList_ = new QListWidget(this);
    battlesList_->setSelectionMode(QAbstractItemView::SingleSelection);
    battlesList_->setContextMenuPolicy(Qt::CustomContextMenu);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);
    leftLayout->addLayout(dateRow);
    leftLayout->addWidget(battlesList_, 1);

    QWidget* leftWidget = new QWidget(this);
    leftWidget->setLayout(leftLayout);

    // ---- Right pane: summary + full report ---------------------------------
    summaryEdit_ = new QPlainTextEdit(this);
    summaryEdit_->setReadOnly(true);
    summaryEdit_->setMaximumHeight(90);

    fullReportEdit_ = new QPlainTextEdit(this);
    fullReportEdit_->setReadOnly(true);

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);
    rightLayout->addWidget(summaryEdit_, 0);
    rightLayout->addWidget(fullReportEdit_, 1);

    QWidget* rightWidget = new QWidget(this);
    rightWidget->setLayout(rightLayout);

    // ---- Splitter ----------------------------------------------------------
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // ---- Connections -------------------------------------------------------
    connect(dateCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BattlesTabContentQt::onDateComboChanged);
    connect(battlesList_, &QListWidget::currentRowChanged,
            this, &BattlesTabContentQt::onBattleSelectionChanged);
        connect(battlesList_, &QListWidget::customContextMenuRequested,
            this, &BattlesTabContentQt::onBattleContextMenuRequested);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void BattlesTabContentQt::refresh()
{
    updateDateSelector();
    updateBattleList();
    updateBattleDetails(battlesList_->currentRow());
}

void BattlesTabContentQt::focusBattleByRegion(int x, int y, int z, int month, int year)
{
    if (!appData_)
        return;

    if (!appData_->battleRepository().hasBattleInRegionForPeriod(x, y, z, month, year))
        return;

    selectedMonth_ = month;
    selectedYear_  = year;
    updateDateSelector();
    updateBattleList();

    for (int row = 0; row < static_cast<int>(visibleBattles_.size()); ++row)
    {
        const Battle* battle = visibleBattles_[static_cast<std::size_t>(row)];
        if (battle &&
            battle->getRegionXCoordinate() == x &&
            battle->getRegionYCoordinate() == y &&
            battle->getRegionZCoordinate() == z)
        {
            battlesList_->setCurrentRow(row);
            battlesList_->scrollToItem(battlesList_->currentItem());
            break;
        }
    }

    updateBattleDetails(battlesList_->currentRow());
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void BattlesTabContentQt::onDateComboChanged(int index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= availablePeriods_.size())
        return;

    selectedMonth_ = availablePeriods_[static_cast<std::size_t>(index)].first;
    selectedYear_  = availablePeriods_[static_cast<std::size_t>(index)].second;
    updateBattleList();
    updateBattleDetails(battlesList_->currentRow());
}

void BattlesTabContentQt::onBattleSelectionChanged()
{
    updateBattleDetails(battlesList_->currentRow());
}

void BattlesTabContentQt::onBattleContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* item = battlesList_->itemAt(pos);
    if (!item)
        return;

    const int row = battlesList_->row(item);
    if (row < 0 || static_cast<std::size_t>(row) >= visibleBattles_.size())
        return;

    const Battle* battle = visibleBattles_[static_cast<std::size_t>(row)];
    if (!battle)
        return;

    battlesList_->setCurrentRow(row);
    QMenu menu(this);
    QAction* mapAction = menu.addAction("Map");
    if (menu.exec(battlesList_->viewport()->mapToGlobal(pos)) == mapAction)
    {
        emit navigateToMap(battle->getRegionXCoordinate(),
                           battle->getRegionYCoordinate(),
                           battle->getRegionZCoordinate());
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void BattlesTabContentQt::updateDateSelector()
{
    if (!appData_)
        return;

    availablePeriods_ = appData_->battleRepository().getAvailablePeriodsDescending();

    dateCombo_->blockSignals(true);
    dateCombo_->clear();

    if (availablePeriods_.empty())
    {
        selectedMonth_ = 0;
        selectedYear_  = 0;
        dateCombo_->blockSignals(false);
        return;
    }

    int desiredIndex = 0;
    for (int i = 0; i < static_cast<int>(availablePeriods_.size()); ++i)
    {
        const auto [month, year] = availablePeriods_[static_cast<std::size_t>(i)];
        const std::wstring text = BattleFormattingUtils::formatPeriod(month, year);
        dateCombo_->addItem(QString::fromStdWString(text));

        if (selectedMonth_ > 0 && selectedYear_ > 0 &&
            month == selectedMonth_ && year == selectedYear_)
        {
            desiredIndex = i;
        }
    }

    dateCombo_->blockSignals(false);
    dateCombo_->setCurrentIndex(desiredIndex);

    selectedMonth_ = availablePeriods_[static_cast<std::size_t>(desiredIndex)].first;
    selectedYear_  = availablePeriods_[static_cast<std::size_t>(desiredIndex)].second;
}

void BattlesTabContentQt::updateBattleList()
{
    battlesList_->blockSignals(true);
    battlesList_->clear();
    visibleBattles_.clear();

    if (!appData_ || selectedMonth_ <= 0 || selectedYear_ <= 0)
    {
        battlesList_->blockSignals(false);
        return;
    }

    visibleBattles_ = appData_->battleRepository().findByPeriod(selectedMonth_, selectedYear_);

    for (const Battle* battle : visibleBattles_)
    {
        if (!battle)
            continue;
        const std::wstring text = BattleFormattingUtils::formatBattleListEntry(*battle);
        battlesList_->addItem(QString::fromStdWString(text));
    }

    battlesList_->blockSignals(false);

    if (!visibleBattles_.empty())
        battlesList_->setCurrentRow(0);
}

void BattlesTabContentQt::updateBattleDetails(int selectedRow)
{
    if (selectedRow < 0 || static_cast<std::size_t>(selectedRow) >= visibleBattles_.size())
    {
        summaryEdit_->clear();
        fullReportEdit_->clear();
        return;
    }

    const Battle* battle = visibleBattles_[static_cast<std::size_t>(selectedRow)];
    if (!battle)
    {
        summaryEdit_->clear();
        fullReportEdit_->clear();
        return;
    }

    summaryEdit_->setPlainText(
        QString::fromStdWString(BattleFormattingUtils::formatSummary(*battle)));
    fullReportEdit_->setPlainText(
        QString::fromStdWString(battle->getFullText()));
}
