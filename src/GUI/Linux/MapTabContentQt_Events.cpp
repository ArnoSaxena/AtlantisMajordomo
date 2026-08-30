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
#include "Data/Battle.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/Report.hpp"
#include "Data/Skill.hpp"
#include "Data/Unit.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/ReadOnlyPopupService.hpp"
#include "Function/StringUtils.hpp"
#include "Function/SkillFormattingUtils.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <string>

namespace
{

void showReadOnlyTextPopup(QWidget* owner,
                           const QString& title,
                           const QString& text,
                           bool softWrap)
{
    QDialog dialog(owner);
    dialog.setWindowTitle(title);
    dialog.resize(760, 560);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto* textEdit = new QPlainTextEdit(&dialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(text);
    textEdit->setLineWrapMode(softWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    layout->addWidget(textEdit, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    dialog.exec();
}

class QtReadOnlyPopupBackend : public ReadOnlyPopupService::IBackend
{
public:
    explicit QtReadOnlyPopupBackend(QWidget* owner)
        : owner_(owner)
    {
    }

    void show(const ReadOnlyPopupService::PopupRequest& request) override
    {
        showReadOnlyTextPopup(owner_,
                              QString::fromStdWString(request.title),
                              QString::fromStdWString(request.text),
                              request.softWrap);
    }

private:
    QWidget* owner_ { nullptr };
};

bool getLatestReportPeriod(const AppData* appData, int& month, int& year)
{
    month = 0;
    year = 0;
    if (!appData)
    {
        return false;
    }

    const auto& reportRepository = appData->reportRepository();
    for (std::size_t i = 0; i < reportRepository.size(); ++i)
    {
        const Report& report = reportRepository.at(i);
        const int reportMonth = report.getMonth();
        const int reportYear = report.getYear();
        if (reportMonth < 1 || reportMonth > 12 || reportYear <= 0)
        {
            continue;
        }

        if (reportYear > year || (reportYear == year && reportMonth > month))
        {
            month = reportMonth;
            year = reportYear;
        }
    }

    return month >= 1 && month <= 12 && year > 0;
}

const Item* resolveItemFromRegionListRow(const AppData* appData,
                                         const QTreeWidgetItem* row,
                                         std::wstring& normalizedToken)
{
    normalizedToken.clear();
    if (!appData || !row)
    {
        return nullptr;
    }

    const std::wstring rawToken = StringUtils::trimWhitespace(row->text(0).toStdWString());
    if (rawToken.empty())
    {
        return nullptr;
    }

    normalizedToken = StringUtils::toUpper(rawToken);
    return appData->itemRepository().findByIdentifierToken(normalizedToken);
}

} // namespace

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

void MapTabContentQt::onMapRegionRightClicked(QPoint screenPos, int regionX, int regionY)
{
    if (!appData_)
    {
        return;
    }

    const Region* region = appData_->regionRepository().findByCoordinates(regionX, regionY, selectedZ_);
    if (!region)
    {
        region = appData_->regionRepository().findByCoordinates(regionX, regionY);
    }
    if (!region)
    {
        return;
    }

    int latestReportMonth = 0;
    int latestReportYear = 0;
    const bool hasLatestPeriod = getLatestReportPeriod(appData_, latestReportMonth, latestReportYear);
    const bool hasBattle = hasLatestPeriod &&
        appData_->battleRepository().hasBattleInRegionForPeriod(
            regionX,
            regionY,
            region->getZCoordinate(),
            latestReportMonth,
            latestReportYear);

    QMenu menu(this);
    QAction* regionReportAction = menu.addAction("Show Region Report");
    QAction* battleReportAction = menu.addAction("Show Battle Report");
    battleReportAction->setEnabled(hasBattle);

    QAction* selected = menu.exec(screenPos);
    if (selected == regionReportAction)
    {
        QtReadOnlyPopupBackend popupBackend(this);
        ReadOnlyPopupService::show(popupBackend,
                                   ReadOnlyPopupService::PopupRequest{
                                       L"Region Report",
                                       StringUtils::toCRLF(region->getRegionReport()),
                                       false
                                   });
        return;
    }

    if (selected == battleReportAction && hasBattle)
    {
        std::wstring combinedBattleText;
        const auto battles = appData_->battleRepository().findByPeriod(latestReportMonth, latestReportYear);
        for (const Battle* battle : battles)
        {
            if (!battle)
            {
                continue;
            }

            if (battle->getRegionXCoordinate() == regionX &&
                battle->getRegionYCoordinate() == regionY &&
                battle->getRegionZCoordinate() == region->getZCoordinate())
            {
                if (!combinedBattleText.empty())
                {
                    combinedBattleText += L"\n\n----------------------------------------\n\n";
                }
                combinedBattleText += battle->getFullText();
            }
        }

        if (combinedBattleText.empty())
        {
            // Fall back to tab navigation when no detailed battle text is available.
            emit navigateToBattle(regionX, regionY, region->getZCoordinate(), latestReportMonth, latestReportYear);
            return;
        }

        QtReadOnlyPopupBackend popupBackend(this);
        ReadOnlyPopupService::show(popupBackend,
                                   ReadOnlyPopupService::PopupRequest{
                                       L"Battle Report",
                                       StringUtils::toCRLF(combinedBattleText),
                                       false
                                   });
    }
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

void MapTabContentQt::onZSelectionRequested(QPoint screenPos)
{
    if (!mapCanvas_)
    {
        return;
    }

    const std::vector<int>& zLevels = mapCanvas_->availableZLevels();
    QMenu menu(this);

    if (zLevels.empty())
    {
        QAction* emptyAction = menu.addAction("No Z levels");
        emptyAction->setEnabled(false);
        menu.exec(screenPos);
        return;
    }

    QList<QAction*> zActions;
    zActions.reserve(static_cast<qsizetype>(zLevels.size()));
    for (int z : zLevels)
    {
        QAction* action = menu.addAction(QString("Z=%1").arg(z));
        action->setCheckable(true);
        action->setChecked(z == selectedZ_);
        zActions.push_back(action);
    }

    QAction* selected = menu.exec(screenPos);
    if (!selected)
    {
        return;
    }

    for (int i = 0; i < zActions.size(); ++i)
    {
        if (zActions[i] != selected)
        {
            continue;
        }

        selectedZ_ = zLevels[static_cast<std::size_t>(i)];
        hasSelectedRegion_ = false;
        selectedRegionX_ = 0;
        selectedRegionY_ = 0;
        selectedUnitNumber_ = 0;
        selectedUnitIsNew_ = false;
        if (mapCanvas_)
        {
            mapCanvas_->setSelectedRegion(false, 0, 0);
            mapCanvas_->clearMovePathOverlay();
        }
        refresh();
        updateRegionDetailsView(nullptr);
        clearSelectedUnitDetails();
        return;
    }
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
    QAction* giveAction = menu->addAction("Give...");

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
    else if (selected == giveAction && appData_)
    {
        showGiveToUnitDialog();
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

void MapTabContentQt::onUnitWarningsContextMenuRequested(const QPoint& pos)
{
    if (!unitWarningsList_ || !appData_)
    {
        return;
    }

    QListWidgetItem* item = unitWarningsList_->itemAt(pos);
    if (!item)
    {
        return;
    }

    unitWarningsList_->setCurrentItem(item);

    QMenu menu(unitWarningsList_);
    QAction* clearAction = menu.addAction("Clear");
    QAction* selected = menu.exec(unitWarningsList_->mapToGlobal(pos));
    if (selected == clearAction)
    {
        clearWarningsForSelectedUnit();
    }
}

void MapTabContentQt::onRegionResourcesContextMenuRequested(const QPoint& pos)
{
    handleRegionItemContextMenuRequested(regionResourcesList_, pos);
}

void MapTabContentQt::onRegionForSaleContextMenuRequested(const QPoint& pos)
{
    handleRegionItemContextMenuRequested(regionForSaleList_, pos);
}

void MapTabContentQt::onRegionWantedContextMenuRequested(const QPoint& pos)
{
    handleRegionItemContextMenuRequested(regionWantedList_, pos);
}

void MapTabContentQt::handleRegionItemContextMenuRequested(QTreeWidget* sourceList, const QPoint& pos)
{
    if (!sourceList || !appData_)
    {
        return;
    }

    QTreeWidgetItem* row = sourceList->itemAt(pos);
    if (!row)
    {
        return;
    }

    std::wstring itemToken;
    const Item* item = resolveItemFromRegionListRow(appData_, row, itemToken);
    if (!item)
    {
        return;
    }

    QMenu menu(sourceList);
    QAction* showDescriptionAction = menu.addAction("Item Description");
    QAction* openItemsTabAction = menu.addAction("Items Tab");
    QAction* selected = menu.exec(sourceList->mapToGlobal(pos));
    if (selected == showDescriptionAction)
    {
        std::wstring description = item->getFullText();
        if (description.empty())
        {
            description = L"No description available.";
        }

        QtReadOnlyPopupBackend popupBackend(this);
        ReadOnlyPopupService::show(popupBackend,
                                   ReadOnlyPopupService::PopupRequest{
                                       L"Item Description " + item->getItemName() + L" [" + item->getIdentifierToken() + L"]",
                                       StringUtils::toCRLF(description),
                                       true
                                   });
    }
    else if (selected == openItemsTabAction)
    {
        emit navigateToItem(QString::fromStdWString(itemToken));
    }
}

void MapTabContentQt::showSkillDescription(const std::wstring& skillToken)
{
    if (!appData_ || skillToken.empty())
    {
        return;
    }

    const SkillFormattingUtils::SkillDescriptionContent content =
        SkillFormattingUtils::buildSkillDescriptionContent(*appData_, skillToken);
    if (!content.found)
    {
        QtReadOnlyPopupBackend popupBackend(this);
        ReadOnlyPopupService::show(popupBackend,
                                   ReadOnlyPopupService::PopupRequest{
                                       L"Skill Description",
                                       content.notFoundMessage,
                                       true
                                   });
        return;
    }

    QtReadOnlyPopupBackend popupBackend(this);
    ReadOnlyPopupService::show(popupBackend,
                               ReadOnlyPopupService::PopupRequest{
                                   content.title,
                                   StringUtils::toCRLF(content.description),
                                   true
                               });
}
