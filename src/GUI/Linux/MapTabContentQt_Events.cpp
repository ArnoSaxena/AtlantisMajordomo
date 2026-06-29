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
 * File: MapTabContentQt_Events.cpp
 *
 * Step 7.8 - Qt signal routing and event handlers.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Region.hpp"
#include "Data/Skill.hpp"
#include "Data/Unit.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/StringUtils.hpp"

#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextCursor>

#include <string>

void MapTabContentQt::commitPendingEdits()
{
    saveOrdersToSelectedUnit();
}

void MapTabContentQt::onCheckOrdersClicked()
{
    runOrderChecksForMainFaction();
}

void MapTabContentQt::onSearchUnitClicked()
{
    searchAndSelectUnitById();
}

void MapTabContentQt::onMapRegionLeftClicked(int regionX, int regionY)
{
    if (!appData_)
    {
        return;
    }

    hasSelectedRegion_ = true;
    selectedRegionX_ = regionX;
    selectedRegionY_ = regionY;
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedRegion(true, selectedRegionX_, selectedRegionY_);
    }

    refresh();

    const Region* region = appData_->regionRepository().findByCoordinates(regionX, regionY, selectedZ_);
    if (!region)
    {
        region = appData_->regionRepository().findByCoordinates(regionX, regionY);
    }
    updateRegionDetailsView(region);
}

void MapTabContentQt::onMapRegionDoubleClicked(int regionX, int regionY)
{
    onMapRegionLeftClicked(regionX, regionY);
}

void MapTabContentQt::onMapRegionRightClicked(QPoint /*screenPos*/, int /*regionX*/, int /*regionY*/)
{
    // Context menu actions for region and battle reports are implemented in step 7.9.5,
    // once the custom MapCanvasWidget is active.
}

void MapTabContentQt::onMapNoRegionClicked()
{
    hasSelectedRegion_ = false;
    selectedRegionX_ = 0;
    selectedRegionY_ = 0;
    selectedUnitNumber_ = 0;
    selectedUnitIsNew_ = false;
    if (mapCanvas_)
    {
        mapCanvas_->setSelectedRegion(false, 0, 0);
    }

    clearUnitsList();
    clearSelectedUnitDetails();
    updateRegionDetailsView(nullptr);
}

void MapTabContentQt::onZSelectionRequested(QPoint /*screenPos*/)
{
    // Z-level context menu is implemented in step 7.9.5.
}

void MapTabContentQt::onOrdersEditorContextMenuRequested(const QPoint& pos)
{
    if (!ordersEditor_)
    {
        return;
    }

    QMenu* menu = ordersEditor_->createStandardContextMenu();
    menu->addSeparator();
    QAction* formNewUnitAction = menu->addAction("Form New Unit");

    QAction* selected = menu->exec(ordersEditor_->mapToGlobal(pos));
    if (selected == formNewUnitAction && appData_)
    {
        int x = selectedRegionX_;
        int y = selectedRegionY_;
        int z = selectedZ_;

        if (x == 0 && y == 0)
        {
            const Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
            if (unit)
            {
                x = unit->getXCoordinate();
                y = unit->getYCoordinate();
                z = unit->getZCoordinate();
            }
        }

        const int newNumber = OrderBusinessLogic::computeNextNewUnitNumber(appData_, x, y, z);
        QString currentText = ordersEditor_->toPlainText();
        if (!currentText.isEmpty() && !currentText.endsWith("\n"))
        {
            currentText += "\n";
        }

        currentText += QString::fromStdWString(L"FORM " + std::to_wstring(newNumber) + L"\n\nEND\n");
        ordersEditor_->setPlainText(currentText);

        QTextCursor cursor = ordersEditor_->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::Up);
        cursor.movePosition(QTextCursor::StartOfLine);
        ordersEditor_->setTextCursor(cursor);
        ordersEditor_->setFocus();
    }

    delete menu;
}

void MapTabContentQt::onUnitSkillsContextMenuRequested(const QPoint& pos)
{
    if (!unitSkillsList_ || !appData_)
    {
        return;
    }

    QListWidgetItem* item = unitSkillsList_->itemAt(pos);
    if (!item)
    {
        return;
    }

    std::wstring skillToken = StringUtils::trimWhitespace(item->text().toStdWString());
    if (skillToken.empty())
    {
        return;
    }

    const std::size_t colonPos = skillToken.find(L':');
    if (colonPos != std::wstring::npos)
    {
        skillToken = StringUtils::trimWhitespace(skillToken.substr(0, colonPos));
    }
    if (skillToken.empty())
    {
        return;
    }

    QMenu menu(unitSkillsList_);
    QAction* addStudyOrderAction = menu.addAction("Add Study Order");
    QAction* showSkillDescriptionAction = menu.addAction("Skill Description");
    QAction* openSkillTabAction = menu.addAction("Skill Tab");

    QAction* selected = menu.exec(unitSkillsList_->mapToGlobal(pos));
    if (selected == addStudyOrderAction)
    {
        Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
        if (unit && canEditOrdersForUnit(unit) && ordersEditor_ && !ordersEditor_->isReadOnly())
        {
            appendOrderLineToOrdersEditor(L"study " + skillToken);
        }
    }
    else if (selected == openSkillTabAction)
    {
        navigateToSkillList(skillToken);
    }
    else if (selected == showSkillDescriptionAction)
    {
        showSkillDescription(skillToken);
    }
}

void MapTabContentQt::showSkillDescription(const std::wstring& skillToken)
{
    if (!appData_ || skillToken.empty())
    {
        return;
    }

    const Skill* skill = appData_->skillRepository().findByIdentifierToken(skillToken);
    if (!skill)
    {
        QMessageBox::information(this,
                                 "Skill Description",
                                 QString("No description found for skill '%1'.")
                                     .arg(QString::fromStdWString(skillToken)));
        return;
    }

    std::wstring description = skill->getAllLevelDescriptions();
    if (description.empty())
    {
        description = L"No description available.";
    }

    const QString title = QString::fromStdWString(skill->getName()) +
                          " [" + QString::fromStdWString(skill->getIdentifierToken()) + "]";
    QMessageBox::information(this,
                             "Skill Description",
                             title + "\n\n" + QString::fromStdWString(description));
}
