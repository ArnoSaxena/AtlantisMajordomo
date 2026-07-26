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
 * File: ItemsTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/ItemsTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/ItemRepository.hpp"
#include "Function/ItemOrderingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
constexpr int kTokenRole        = Qt::UserRole;
constexpr int kMultilineMinH    = 75;
constexpr int kFullTextMinH     = 100;
} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ItemsTabContentQt::ItemsTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    // ---- Left pane: items list ---------------------------------------------
    itemsList_ = new QListWidget(this);
    itemsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    itemsList_->setMinimumWidth(170);

    // ---- Right pane: scrollable form ---------------------------------------

    // Single-line fields
    tokenEdit_ = new QLineEdit(this);
    tokenEdit_->setReadOnly(true);

    nameEdit_             = new QLineEdit(this);
    weightEdit_           = new QLineEdit(this);
    movesEdit_            = new QLineEdit(this);
    walkCapacityEdit_     = new QLineEdit(this);
    rideCapacityEdit_     = new QLineEdit(this);
    swimCapacityEdit_     = new QLineEdit(this);
    flyCapacityEdit_      = new QLineEdit(this);
    shipSpeedEdit_        = new QLineEdit(this);
    shipSailingSkillEdit_ = new QLineEdit(this);
    magesStudyEdit_       = new QLineEdit(this);
    defaultSkillMaxEdit_  = new QLineEdit(this);

    // Checkboxes (two rows of three)
    meeleWeaponCheck_  = new QCheckBox("Meele Weapon",  this);
    rangedWeaponCheck_ = new QCheckBox("Ranged Weapon", this);
    armourCheck_       = new QCheckBox("Armour",        this);
    resourceCheck_     = new QCheckBox("Resource",      this);
    mountCheck_        = new QCheckBox("Mount",         this);
    manCheck_          = new QCheckBox("Is Man",        this);

    QWidget* checkboxWidget = new QWidget(this);
    QGridLayout* checkboxGrid = new QGridLayout(checkboxWidget);
    checkboxGrid->setContentsMargins(0, 0, 0, 0);
    checkboxGrid->setSpacing(8);
    checkboxGrid->addWidget(meeleWeaponCheck_,  0, 0);
    checkboxGrid->addWidget(rangedWeaponCheck_, 0, 1);
    checkboxGrid->addWidget(armourCheck_,       0, 2);
    checkboxGrid->addWidget(resourceCheck_,     1, 0);
    checkboxGrid->addWidget(mountCheck_,        1, 1);
    checkboxGrid->addWidget(manCheck_,          1, 2);
    checkboxGrid->setColumnStretch(3, 1);
    checkboxWidget->setLayout(checkboxGrid);

    // Multiline fields
    skillsMaxEdit_ = new QPlainTextEdit(this);
    skillsMaxEdit_->setMinimumHeight(kMultilineMinH);

    resourcesEdit_ = new QPlainTextEdit(this);
    resourcesEdit_->setMinimumHeight(kMultilineMinH);

    productionSkillEdit_ = new QPlainTextEdit(this);
    productionSkillEdit_->setMinimumHeight(kMultilineMinH);

    productionHelpEdit_ = new QPlainTextEdit(this);
    productionHelpEdit_->setMinimumHeight(kMultilineMinH);

    fullTextEdit_ = new QPlainTextEdit(this);
    fullTextEdit_->setReadOnly(true);
    fullTextEdit_->setMinimumHeight(kFullTextMinH);

    saveButton_ = new QPushButton("Save", this);
    saveButton_->setFixedWidth(120);

    // ---- Assemble form layout -----------------------------------------------

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(6);
    formLayout->addRow("Item ID:",                         tokenEdit_);
    formLayout->addRow("Name:",                            nameEdit_);
    formLayout->addRow("Weight:",                          weightEdit_);
    formLayout->addRow("",                                 checkboxWidget);
    formLayout->addRow("Moves:",                           movesEdit_);
    formLayout->addRow("Walk Cap.:",                       walkCapacityEdit_);
    formLayout->addRow("Ride Cap.:",                       rideCapacityEdit_);
    formLayout->addRow("Swim Cap.:",                       swimCapacityEdit_);
    formLayout->addRow("Fly Cap.:",                        flyCapacityEdit_);
    formLayout->addRow("Ship Speed (hexes/month):",        shipSpeedEdit_);
    formLayout->addRow("Ship Sailing Skill Required:",     shipSailingSkillEdit_);
    formLayout->addRow("Mages Study Above L2:",            magesStudyEdit_);
    formLayout->addRow("Default Skill Max:",               defaultSkillMaxEdit_);

    QHBoxLayout* saveRow = new QHBoxLayout;
    saveRow->setContentsMargins(0, 0, 0, 0);
    saveRow->addStretch();
    saveRow->addWidget(saveButton_);

    QVBoxLayout* formContainer = new QVBoxLayout;
    formContainer->setContentsMargins(4, 4, 4, 4);
    formContainer->setSpacing(6);
    formContainer->addLayout(formLayout);
    formContainer->addWidget(new QLabel("Skills Max (SKILL:LEVEL per line):", this));
    formContainer->addWidget(skillsMaxEdit_);
    formContainer->addWidget(new QLabel("Resources (TOKEN:AMOUNT per line):", this));
    formContainer->addWidget(resourcesEdit_);
    formContainer->addWidget(new QLabel("Production Skills (SKILL:LEVEL per line):", this));
    formContainer->addWidget(productionSkillEdit_);
    formContainer->addWidget(new QLabel("Production Help (ITEM:AMOUNT per line):", this));
    formContainer->addWidget(productionHelpEdit_);
    formContainer->addWidget(new QLabel("Full Text (display only):", this));
    formContainer->addWidget(fullTextEdit_);
    formContainer->addLayout(saveRow);
    formContainer->addStretch();

    QWidget* formWidget = new QWidget(this);
    formWidget->setLayout(formContainer);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(formWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // ---- Splitter ----------------------------------------------------------
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(itemsList_);
    splitter->addWidget(scrollArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({170, 500});

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // ---- Connections -------------------------------------------------------
    connect(itemsList_, &QListWidget::currentRowChanged,
            this, &ItemsTabContentQt::onItemSelectionChanged);
    connect(saveButton_, &QPushButton::clicked,
            this, &ItemsTabContentQt::onSaveClicked);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void ItemsTabContentQt::refresh()
{
    updateItemsList();
}

void ItemsTabContentQt::focusItemByToken(const std::wstring& itemToken)
{
    if (!appData_ || itemToken.empty())
    {
        return;
    }

    selectedItemToken_ = itemToken;
    updateItemsList();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void ItemsTabContentQt::onItemSelectionChanged()
{
    // Ignore separator items (they carry no token data).
    const QListWidgetItem* item = itemsList_->currentItem();
    if (!item || item->data(kTokenRole).toString().isEmpty())
        return;

    updateSelectedItemFromList();
}

void ItemsTabContentQt::onSaveClicked()
{
    saveSelectedItem();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void ItemsTabContentQt::updateItemsList()
{
    QSignalBlocker listBlocker(itemsList_);
    itemsList_->clear();

    if (!appData_)
        return;

    const auto& repository = appData_->itemRepository();

    const ItemOrderingUtils::OrderedItemGroups orderedGroups =
        ItemOrderingUtils::buildOrderedItemGroups(repository);

    int selectedRow = -1;
    int rowIndex    = 0;

    auto addItemRow = [&](const Item* it)
    {
        const QString token = QString::fromStdWString(it->getIdentifierToken());
        QListWidgetItem* listItem = new QListWidgetItem(token);
        listItem->setData(kTokenRole, token);
        itemsList_->addItem(listItem);

        if (it->getIdentifierToken() == selectedItemToken_)
            selectedRow = rowIndex;
        ++rowIndex;
    };

    for (const Item* it : orderedGroups.manItems)
        addItemRow(it);

    // Visual separator between man-type and non-man items
    if (!orderedGroups.manItems.empty() && !orderedGroups.otherItems.empty())
    {
        QListWidgetItem* sep = new QListWidgetItem(QString(32, QChar(0x2500)));
        sep->setFlags(Qt::NoItemFlags);
        sep->setForeground(QApplication::palette().color(QPalette::Mid));
        itemsList_->addItem(sep);
        ++rowIndex;
    }

    for (const Item* it : orderedGroups.otherItems)
        addItemRow(it);

    if (selectedRow >= 0)
    {
        itemsList_->setCurrentRow(selectedRow);
        itemsList_->scrollToItem(itemsList_->item(selectedRow));
        updateSelectedItemFromList();
    }
    else
    {
        selectedItemToken_.clear();
        clearFields();
    }
}

void ItemsTabContentQt::updateSelectedItemFromList()
{
    const QListWidgetItem* listItem = itemsList_->currentItem();
    if (!listItem)
    {
        selectedItemToken_.clear();
        clearFields();
        return;
    }

    const QString tokenStr = listItem->data(kTokenRole).toString();
    if (tokenStr.isEmpty())
        return;  // separator item

    selectedItemToken_ = tokenStr.toStdWString();

    const Item* item = appData_->itemRepository().findByIdentifierToken(selectedItemToken_);
    loadItemToFields(item);
}

void ItemsTabContentQt::loadItemToFields(const Item* item)
{
    if (!item)
    {
        clearFields();
        return;
    }

    tokenEdit_->setText(QString::fromStdWString(item->getIdentifierToken()));
    nameEdit_->setText(QString::fromStdWString(item->getItemName()));
    weightEdit_->setText(QString::number(item->getWeight()));

    meeleWeaponCheck_->setChecked(item->isMeeleWeapon());
    rangedWeaponCheck_->setChecked(item->isRangedWeapon());
    armourCheck_->setChecked(item->isArmour());
    resourceCheck_->setChecked(item->isResource());
    mountCheck_->setChecked(item->isMount());
    manCheck_->setChecked(item->isMan());

    movesEdit_->setText(QString::number(item->getMoves()));
    walkCapacityEdit_->setText(QString::number(item->getWalkCapacity()));
    rideCapacityEdit_->setText(QString::number(item->getRideCapacity()));
    swimCapacityEdit_->setText(QString::number(item->getSwimCapacity()));
    flyCapacityEdit_->setText(QString::number(item->getFlyCapacity()));
    shipSpeedEdit_->setText(QString::number(item->getShipSpeedHexesPerMonth()));
    shipSailingSkillEdit_->setText(QString::number(item->getShipSailingSkillRequired()));
    magesStudyEdit_->setText(QString::number(item->getMagesStudy()));
    defaultSkillMaxEdit_->setText(QString::number(item->getDefaultSkillMax()));

    skillsMaxEdit_->setPlainText(
        QString::fromStdWString(StringUtils::formatStringIntMap(item->getSkillsMax())));
    resourcesEdit_->setPlainText(
        QString::fromStdWString(StringUtils::formatStringIntMap(item->getResources())));
    productionSkillEdit_->setPlainText(
        QString::fromStdWString(StringUtils::formatStringIntMap(item->getProductionSkill())));
    productionHelpEdit_->setPlainText(
        QString::fromStdWString(StringUtils::formatStringIntMap(item->getProductionHelp())));
    fullTextEdit_->setPlainText(
        QString::fromStdWString(item->getFullText()));
}

void ItemsTabContentQt::clearFields()
{
    tokenEdit_->clear();
    nameEdit_->clear();
    weightEdit_->clear();
    movesEdit_->clear();
    walkCapacityEdit_->clear();
    rideCapacityEdit_->clear();
    swimCapacityEdit_->clear();
    flyCapacityEdit_->clear();
    shipSpeedEdit_->clear();
    shipSailingSkillEdit_->clear();
    magesStudyEdit_->clear();
    defaultSkillMaxEdit_->clear();
    skillsMaxEdit_->clear();
    resourcesEdit_->clear();
    productionSkillEdit_->clear();
    productionHelpEdit_->clear();
    fullTextEdit_->clear();

    meeleWeaponCheck_->setChecked(false);
    rangedWeaponCheck_->setChecked(false);
    armourCheck_->setChecked(false);
    resourceCheck_->setChecked(false);
    mountCheck_->setChecked(false);
    manCheck_->setChecked(false);
}

void ItemsTabContentQt::saveSelectedItem()
{
    if (!appData_ || selectedItemToken_.empty())
        return;

    Item* item = appData_->itemRepository().findByIdentifierToken(selectedItemToken_);
    if (!item)
        return;

    // Token is immutable — warn if the user tried to edit it.
    const std::wstring editedToken =
        StringUtils::trimWhitespace(tokenEdit_->text().toStdWString());
    if (!editedToken.empty() && editedToken != selectedItemToken_)
    {
        QMessageBox::warning(this, "Items",
            "Item ID is immutable in this editor. Other fields were saved.");
    }

    item->setItemName(nameEdit_->text().toStdWString());
    item->setWeight(StringUtils::parseIntSafe(weightEdit_->text().toStdWString()));
    item->setMeeleWeapon(meeleWeaponCheck_->isChecked());
    item->setRangedWeapon(rangedWeaponCheck_->isChecked());
    item->setArmour(armourCheck_->isChecked());
    item->setResource(resourceCheck_->isChecked());
    item->setMount(mountCheck_->isChecked());
    item->setMoves(StringUtils::parseIntSafe(movesEdit_->text().toStdWString()));
    item->setWalkCapacity(StringUtils::parseIntSafe(walkCapacityEdit_->text().toStdWString()));
    item->setRideCapacity(StringUtils::parseIntSafe(rideCapacityEdit_->text().toStdWString()));
    item->setSwimCapacity(StringUtils::parseIntSafe(swimCapacityEdit_->text().toStdWString()));
    item->setFlyCapacity(StringUtils::parseIntSafe(flyCapacityEdit_->text().toStdWString()));
    item->setShipSpeedHexesPerMonth(
        StringUtils::parseIntSafe(shipSpeedEdit_->text().toStdWString()));
    item->setShipSailingSkillRequired(
        StringUtils::parseIntSafe(shipSailingSkillEdit_->text().toStdWString()));
    item->setMagesStudy(StringUtils::parseIntSafe(magesStudyEdit_->text().toStdWString()));
    item->setDefaultSkillMax(
        StringUtils::parseIntSafe(defaultSkillMaxEdit_->text().toStdWString()));
    item->setMan(manCheck_->isChecked());
    item->setSkillsMax(
        StringUtils::parseStringIntMap(skillsMaxEdit_->toPlainText().toStdWString()));
    item->setResources(
        StringUtils::parseStringIntMap(resourcesEdit_->toPlainText().toStdWString()));
    item->setProductionSkill(
        StringUtils::parseStringIntMap(productionSkillEdit_->toPlainText().toStdWString()));
    item->setProductionHelp(
        StringUtils::parseStringIntMap(productionHelpEdit_->toPlainText().toStdWString()));

    updateItemsList();
}
