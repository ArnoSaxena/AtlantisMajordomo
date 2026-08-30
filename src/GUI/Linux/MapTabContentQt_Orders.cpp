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
// Qt widgets used by the Give dialog
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>

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

void MapTabContentQt::showGiveToUnitDialog()
{
    if (!appData_ || selectedUnitNumber_ == 0 || !ordersEditor_)
    {
        return;
    }

    const Unit* originUnit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
    if (!originUnit)
    {
        return;
    }

    QDialog dlg(nullptr);
    dlg.setWindowTitle("Give to unit");
    QVBoxLayout* v = new QVBoxLayout(&dlg);

    QHBoxLayout* h1 = new QHBoxLayout();
    QLabel* lbl = new QLabel("Give to unit:");
    QLineEdit* edit = new QLineEdit();
    edit->setPlaceholderText("Unit number or NEW n");
    h1->addWidget(lbl);
    h1->addWidget(edit);
    v->addLayout(h1);

    QHBoxLayout* h2 = new QHBoxLayout();
    QLabel* lbl2 = new QLabel("Item:");
    QComboBox* combo = new QComboBox();
    h2->addWidget(lbl2);
    h2->addWidget(combo);
    v->addLayout(h2);

    // Populate combo with origin unit items, SILV first if present
    std::vector<std::wstring> tokens;
    for (const auto& p : originUnit->getItems())
    {
        if (p.second <= 0) continue;
        tokens.push_back(p.first);
    }
    auto it = std::find_if(tokens.begin(), tokens.end(), [](const std::wstring& t){ return StringUtils::toUpper(t) == L"SILV"; });
    if (it != tokens.end())
    {
        std::wstring s = *it; tokens.erase(it); tokens.insert(tokens.begin(), s);
    }
    for (const auto& t : tokens) combo->addItem(QString::fromStdWString(t));
    if (!tokens.empty()) combo->setCurrentIndex(0);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch(1);
    QPushButton* give = new QPushButton("Give");
    QPushButton* cancel = new QPushButton("Cancel");
    buttons->addWidget(give);
    buttons->addWidget(cancel);
    v->addLayout(buttons);

    connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(give, &QPushButton::clicked, [&]() {
        const QString editText = edit->text().trimmed();
        if (editText.isEmpty())
        {
            QMessageBox::warning(nullptr, "Give", "Invalid receiving unit reference.");
            return;
        }

        bool isNewRef = false;
        int target = 0;
        const QString upper = editText.toUpper();
        if (upper.startsWith("NEW "))
        {
            bool ok = false;
            const int v = editText.mid(4).trimmed().toInt(&ok);
            if (!ok || v <= 0)
            {
                QMessageBox::warning(nullptr, "Give", "Invalid NEW unit index.");
                return;
            }
            isNewRef = true;
            target = v;
        }
        else
        {
            bool ok = false;
            const int v = editText.toInt(&ok);
            if (!ok || v <= 0)
            {
                QMessageBox::warning(nullptr, "Give", "Invalid receiving unit number.");
                return;
            }
            target = v;
        }
        const QString itemStr = combo->currentText();
        if (itemStr.isEmpty())
        {
            QMessageBox::warning(nullptr, "Give", "No item selected.");
            return;
        }
        std::wstring itemToken = itemStr.toStdWString();

        const std::wstring giveLine = OrderBusinessLogic::buildGiveCommand(
            *appData_, selectedUnitNumber_, target, isNewRef, itemToken);
        appendOrderLineToOrdersEditor(giveLine);
        dlg.accept();
        return;
    });

    dlg.exec();
}

