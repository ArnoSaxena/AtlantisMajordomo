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
 * File: EventsTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/EventsTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Event.hpp"
#include "Data/EventRepository.hpp"
#include "Function/MonthUtils.hpp"

#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cstddef>
#include <string>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

EventsTabContentQt::EventsTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    dateCombo_ = new QComboBox(this);
    dateCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    eventsList_ = new QTableWidget(this);
    eventsList_->setColumnCount(2);
    eventsList_->setHorizontalHeaderLabels({ "Unit Id", "Message" });
    eventsList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    eventsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    eventsList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventsList_->setAlternatingRowColors(true);
    eventsList_->verticalHeader()->setVisible(false);
    eventsList_->horizontalHeader()->setStretchLastSection(true);
    eventsList_->setColumnWidth(0, 90);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);
    layout->addWidget(dateCombo_, 0);
    layout->addWidget(eventsList_, 1);
    setLayout(layout);

    connect(dateCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EventsTabContentQt::onDateComboChanged);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void EventsTabContentQt::refresh()
{
    updateDateCombo();
    updateEventsList();
}

// ---------------------------------------------------------------------------
// Private slot
// ---------------------------------------------------------------------------

void EventsTabContentQt::onDateComboChanged(int index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= availablePeriods_.size())
    {
        selectedMonth_ = 0;
        selectedYear_  = 0;
    }
    else
    {
        const auto [month, year] = availablePeriods_[static_cast<std::size_t>(index)];
        selectedMonth_ = month;
        selectedYear_  = year;
    }

    updateEventsList();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void EventsTabContentQt::updateDateCombo()
{
    if (!appData_)
        return;

    const int previousMonth = selectedMonth_;
    const int previousYear  = selectedYear_;

    availablePeriods_ = appData_->eventRepository().getAvailablePeriods();

    // Block signals while repopulating to avoid spurious onDateComboChanged calls.
    dateCombo_->blockSignals(true);
    dateCombo_->clear();

    if (availablePeriods_.empty())
    {
        dateCombo_->addItem("Date: -");
        dateCombo_->setEnabled(false);
        dateCombo_->blockSignals(false);
        selectedMonth_ = 0;
        selectedYear_  = 0;
        return;
    }

    dateCombo_->setEnabled(true);
    int selectedIndex = 0;
    for (int i = 0; i < static_cast<int>(availablePeriods_.size()); ++i)
    {
        const auto [month, year] = availablePeriods_[static_cast<std::size_t>(i)];
        const std::wstring text = MonthUtils::monthNumberToNameOr(month, L"Unknown")
                                  + L" " + std::to_wstring(year);
        dateCombo_->addItem(QString::fromStdWString(text));

        if (month == previousMonth && year == previousYear)
            selectedIndex = i;
    }

    dateCombo_->blockSignals(false);
    dateCombo_->setCurrentIndex(selectedIndex);
    // Manually sync the selected period (setCurrentIndex may not fire the slot
    // when blockSignals was active).
    const auto [month, year] = availablePeriods_[static_cast<std::size_t>(selectedIndex)];
    selectedMonth_ = month;
    selectedYear_  = year;
}

void EventsTabContentQt::updateEventsList()
{
    eventsList_->setRowCount(0);

    if (!appData_ || selectedMonth_ < 1 || selectedMonth_ > 12 || selectedYear_ <= 0)
        return;

    const std::vector<const Event*> events =
        appData_->eventRepository().findByPeriod(selectedMonth_, selectedYear_);

    eventsList_->setRowCount(static_cast<int>(events.size()));

    int row = 0;
    for (const Event* ev : events)
    {
        if (!ev)
            continue;

        const QString unitIdText = QString::number(ev->getUnitId());
        const QString messageText = QString::fromStdWString(ev->getMessage());

        QTableWidgetItem* unitIdItem  = new QTableWidgetItem(unitIdText);
        QTableWidgetItem* messageItem = new QTableWidgetItem(messageText);

        if (ev->isErrorEvent())
        {
            const QColor errorColour(200, 0, 0);
            unitIdItem->setForeground(QBrush(errorColour));
            messageItem->setForeground(QBrush(errorColour));
        }

        eventsList_->setItem(row, 0, unitIdItem);
        eventsList_->setItem(row, 1, messageItem);
        ++row;
    }
}
