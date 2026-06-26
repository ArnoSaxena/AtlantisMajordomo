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
 * File: FactionsTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/FactionsTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Faction.hpp"
#include "Data/FactionRepository.hpp"
#include "Function/FactionAttitudeUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/StringUtils.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
// UserRole key used to store faction number on list/table items.
constexpr int kFactionNumberRole = Qt::UserRole;

// Build the display label for a faction as it appears in the factions list.
QString factionListLabel(const Faction& f)
{
    QString label;
    if (f.isMainFaction())
        label = "* ";
    if (!f.getName().empty())
        label += QString::fromStdWString(f.getName())
                 + " (" + QString::number(f.getFactionNumber()) + ")";
    else
        label += QString::number(f.getFactionNumber());
    return label;
}

} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FactionsTabContentQt::FactionsTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    // ---- Left pane: factions list ------------------------------------------
    factionsList_ = new QListWidget(this);
    factionsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    factionsList_->setMinimumWidth(220);

    // ---- Right pane — faction info form ------------------------------------
    factionNumberEdit_ = new QLineEdit(this);
    factionNumberEdit_->setReadOnly(true);

    factionNameEdit_             = new QLineEdit(this);
    mainFactionCheck_            = new QCheckBox("Main Faction", this);
    monthEdit_                  = new QLineEdit(this);
    yearEdit_                   = new QLineEdit(this);
    passwordEdit_               = new QLineEdit(this);
    taxedTradedCurrentEdit_      = new QLineEdit(this);
    taxedTradedMaxEdit_          = new QLineEdit(this);
    quartermastersCurrentEdit_   = new QLineEdit(this);
    quartermastersMaxEdit_       = new QLineEdit(this);
    magesCurrentEdit_            = new QLineEdit(this);
    magesMaxEdit_                = new QLineEdit(this);
    apprenticesCurrentEdit_      = new QLineEdit(this);
    apprenticesMaxEdit_          = new QLineEdit(this);
    saveButton_                  = new QPushButton("Save", this);
    saveButton_->setFixedWidth(120);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(6);
    formLayout->addRow("Faction Number:",            factionNumberEdit_);
    formLayout->addRow("Name:",                      factionNameEdit_);
    formLayout->addRow(mainFactionCheck_);
    formLayout->addRow("Month:",                     monthEdit_);
    formLayout->addRow("Year:",                      yearEdit_);
    formLayout->addRow("Password:",                  passwordEdit_);
    formLayout->addRow("Taxed/Traded Regions Current:", taxedTradedCurrentEdit_);
    formLayout->addRow("Taxed/Traded Regions Max:",  taxedTradedMaxEdit_);
    formLayout->addRow("Quartermasters Current:",    quartermastersCurrentEdit_);
    formLayout->addRow("Quartermasters Max:",        quartermastersMaxEdit_);
    formLayout->addRow("Mages Current:",             magesCurrentEdit_);
    formLayout->addRow("Mages Max:",                 magesMaxEdit_);
    formLayout->addRow("Apprentices Current:",       apprenticesCurrentEdit_);
    formLayout->addRow("Apprentices Max:",           apprenticesMaxEdit_);

    QHBoxLayout* saveRow = new QHBoxLayout;
    saveRow->setContentsMargins(0, 0, 0, 0);
    saveRow->addStretch();
    saveRow->addWidget(saveButton_);

    QVBoxLayout* formContainer = new QVBoxLayout;
    formContainer->setContentsMargins(4, 4, 4, 4);
    formContainer->setSpacing(6);
    formContainer->addLayout(formLayout);
    formContainer->addLayout(saveRow);
    formContainer->addStretch();

    QWidget* formWidget = new QWidget(this);
    formWidget->setLayout(formContainer);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(formWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // ---- Attitudes panel (visible only for main faction) -------------------
    defaultAttitudeCombo_ = new QComboBox(this);
    for (const char* label : {"Hostile", "Unfriendly", "Neutral", "Friendly", "Ally"})
        defaultAttitudeCombo_->addItem(label);

    unclaimedSilverEdit_ = new QLineEdit(this);
    unclaimedSilverEdit_->setReadOnly(true);

    commandUnitEdit_       = new QLineEdit(this);
    commandUnitSaveButton_ = new QPushButton("Save", this);
    commandUnitSaveButton_->setFixedWidth(70);

    QHBoxLayout* commandRow = new QHBoxLayout;
    commandRow->setContentsMargins(0, 0, 0, 0);
    commandRow->setSpacing(6);
    commandRow->addWidget(commandUnitEdit_, 1);
    commandRow->addWidget(commandUnitSaveButton_);
    QWidget* commandWidget = new QWidget(this);
    commandWidget->setLayout(commandRow);

    attitudesTable_ = new QTableWidget(0, 2, this);
    attitudesTable_->setHorizontalHeaderLabels({"Faction", "Attitude"});
    attitudesTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    attitudesTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    attitudesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    attitudesTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    attitudesTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    attitudesTable_->verticalHeader()->setVisible(false);
    attitudesTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    attitudesTable_->setMinimumHeight(200);

    QFormLayout* attitudesFormLayout = new QFormLayout;
    attitudesFormLayout->setContentsMargins(0, 0, 0, 0);
    attitudesFormLayout->setSpacing(6);
    attitudesFormLayout->addRow("Default Attitude:",              defaultAttitudeCombo_);
    attitudesFormLayout->addRow("Unclaimed Silver (actual/after orders):", unclaimedSilverEdit_);
    attitudesFormLayout->addRow("Faction Command Unit:",          commandWidget);

    QVBoxLayout* attitudesLayout = new QVBoxLayout;
    attitudesLayout->setContentsMargins(0, 0, 0, 0);
    attitudesLayout->setSpacing(6);
    attitudesLayout->addLayout(attitudesFormLayout);
    attitudesLayout->addWidget(new QLabel("Declared Attitudes (right-click to change):", this));
    attitudesLayout->addWidget(attitudesTable_, 1);

    attitudesWidget_ = new QWidget(this);
    attitudesWidget_->setLayout(attitudesLayout);
    attitudesWidget_->setMinimumWidth(280);
    attitudesWidget_->setVisible(false);

    // ---- Right container: form + attitudes side by side --------------------
    QHBoxLayout* rightLayout = new QHBoxLayout;
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);
    rightLayout->addWidget(scrollArea, 1);
    rightLayout->addWidget(attitudesWidget_, 0);

    QWidget* rightContainer = new QWidget(this);
    rightContainer->setLayout(rightLayout);

    // ---- Main splitter -----------------------------------------------------
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(factionsList_);
    splitter->addWidget(rightContainer);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({220, 600});

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // ---- Connections -------------------------------------------------------
    connect(factionsList_, &QListWidget::currentRowChanged,
            this, &FactionsTabContentQt::onFactionSelectionChanged);
    connect(defaultAttitudeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FactionsTabContentQt::onDefaultAttitudeComboChanged);
    connect(attitudesTable_, &QWidget::customContextMenuRequested,
            this, &FactionsTabContentQt::onAttitudesContextMenu);
    connect(commandUnitSaveButton_, &QPushButton::clicked,
            this, &FactionsTabContentQt::onCommandUnitSaveClicked);
    connect(saveButton_, &QPushButton::clicked,
            this, &FactionsTabContentQt::onSaveClicked);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void FactionsTabContentQt::refresh()
{
    updateFactionsList();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void FactionsTabContentQt::onFactionSelectionChanged()
{
    updateSelectedFactionFromList();
}

void FactionsTabContentQt::onDefaultAttitudeComboChanged(int index)
{
    if (index < 0)
        return;

    Faction* faction = appData_
        ? appData_->factionRepository().findByNumber(selectedFactionNumber_)
        : nullptr;
    if (!faction || !faction->isMainFaction())
        return;

    const std::wstring selectedText = defaultAttitudeCombo_->itemText(index).toStdWString();
    handleDefaultAttitudeSelection(*faction, selectedText);
}

void FactionsTabContentQt::onAttitudesContextMenu(const QPoint& pos)
{
    if (!appData_)
        return;

    Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
    if (!faction || !faction->isMainFaction())
        return;

    QTableWidgetItem* hitItem = attitudesTable_->itemAt(pos);
    if (!hitItem)
        return;

    const int row = attitudesTable_->row(hitItem);
    attitudesTable_->selectRow(row);

    QTableWidgetItem* col0 = attitudesTable_->item(row, 0);
    if (!col0)
        return;
    const int targetFactionNumber = col0->data(kFactionNumberRole).toInt();

    QMenu menu(this);
    QAction* defaultAct     = menu.addAction("Default");
    menu.addSeparator();
    QAction* hostileAct     = menu.addAction("Hostile");
    QAction* unfriendlyAct  = menu.addAction("Unfriendly");
    QAction* neutralAct     = menu.addAction("Neutral");
    QAction* friendlyAct    = menu.addAction("Friendly");
    QAction* allyAct        = menu.addAction("Ally");

    QAction* selected = menu.exec(attitudesTable_->viewport()->mapToGlobal(pos));
    if (!selected)
        return;

    std::wstring selectedValue;
    if      (selected == defaultAct)     selectedValue = L"Default";
    else if (selected == hostileAct)     selectedValue = L"Hostile";
    else if (selected == unfriendlyAct)  selectedValue = L"Unfriendly";
    else if (selected == neutralAct)     selectedValue = L"Neutral";
    else if (selected == friendlyAct)    selectedValue = L"Friendly";
    else if (selected == allyAct)        selectedValue = L"Ally";
    else return;

    applyAttitudeContextSelection(targetFactionNumber, selectedValue);
}

void FactionsTabContentQt::onCommandUnitSaveClicked()
{
    if (!appData_)
        return;

    Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
    if (!faction)
        return;

    const int requestedCommandUnitNumber =
        StringUtils::parseIntSafe(commandUnitEdit_->text().toStdWString());

    const auto resolution = OrderBusinessLogic::resolveFactionCommandUnit(
        *appData_,
        faction->getFactionNumber(),
        requestedCommandUnitNumber);

    if (resolution.usedFallback)
    {
        QMessageBox::warning(this, "Invalid Command Unit",
            "Selected command unit is not part of the main faction. "
            "The default command unit (smallest unit number) was selected.");
    }

    faction->setCommandUnitNumber(resolution.commandUnitNumber);
    commandUnitEdit_->setText(
        resolution.commandUnitNumber > 0
            ? QString::number(resolution.commandUnitNumber)
            : QString());

    saveAttitudeEdits();

    OrderBusinessLogic::rewriteFactionDeclareOrders(
        *appData_,
        faction->getFactionNumber(),
        resolution.commandUnitNumber,
        originalDefaultAttitudeText_,
        originalDeclaredAttitudesText_);

    updateAttitudesList(faction);
}

void FactionsTabContentQt::onSaveClicked()
{
    saveSelectedFaction();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void FactionsTabContentQt::updateFactionsList()
{
    QSignalBlocker listBlocker(factionsList_);
    factionsList_->clear();

    if (!appData_)
        return;

    const auto& repository = appData_->factionRepository();

    std::vector<const Faction*> sorted;
    sorted.reserve(repository.size());
    for (std::size_t i = 0; i < repository.size(); ++i)
        sorted.push_back(&repository.at(i));

    std::stable_sort(sorted.begin(), sorted.end(),
        [](const Faction* a, const Faction* b)
        {
            if (a->isMainFaction() != b->isMainFaction())
                return a->isMainFaction();
            return a->getFactionNumber() < b->getFactionNumber();
        });

    int selectedRow = -1;
    for (int row = 0; row < static_cast<int>(sorted.size()); ++row)
    {
        const Faction* f = sorted[static_cast<std::size_t>(row)];
        QListWidgetItem* item = new QListWidgetItem(factionListLabel(*f));
        item->setData(kFactionNumberRole, f->getFactionNumber());
        factionsList_->addItem(item);

        if (f->getFactionNumber() == selectedFactionNumber_)
            selectedRow = row;
    }

    // Auto-select first entry if no previous selection found
    if (selectedRow < 0 && factionsList_->count() > 0)
        selectedRow = 0;

    if (selectedRow >= 0)
    {
        factionsList_->setCurrentRow(selectedRow);
        updateSelectedFactionFromList();
    }
    else
    {
        selectedFactionNumber_ = 0;
        clearFields();
    }
}

void FactionsTabContentQt::updateSelectedFactionFromList()
{
    QListWidgetItem* item = factionsList_->currentItem();
    if (!item)
    {
        selectedFactionNumber_ = 0;
        clearFields();
        return;
    }

    selectedFactionNumber_ = item->data(kFactionNumberRole).toInt();
    pendingAttitudeEdits_.clear();

    const Faction* faction = appData_
        ? appData_->factionRepository().findByNumber(selectedFactionNumber_)
        : nullptr;
    loadFactionToFields(faction);
}

void FactionsTabContentQt::loadFactionToFields(const Faction* faction)
{
    if (!faction)
    {
        clearFields();
        return;
    }

    captureOriginalAttitudeSnapshot(faction);

    factionNumberEdit_->setText(QString::number(faction->getFactionNumber()));
    factionNameEdit_->setText(QString::fromStdWString(faction->getName()));
    mainFactionCheck_->setChecked(faction->isMainFaction());
    monthEdit_->setText(QString::number(faction->getMonth()));
    yearEdit_->setText(QString::number(faction->getYear()));
    passwordEdit_->setText(QString::fromStdWString(faction->getPassword()));

    taxedTradedCurrentEdit_->setText(
        QString::number(faction->getTaxedOrTradedRegionsCurrent()));
    taxedTradedMaxEdit_->setText(
        QString::number(faction->getTaxedOrTradedRegionsMax()));
    quartermastersCurrentEdit_->setText(
        QString::number(faction->getQuartermastersCurrent()));
    quartermastersMaxEdit_->setText(
        QString::number(faction->getQuartermastersMax()));
    magesCurrentEdit_->setText(
        QString::number(faction->getMagesCurrent()));
    magesMaxEdit_->setText(
        QString::number(faction->getMagesMax()));
    apprenticesCurrentEdit_->setText(
        QString::number(faction->getApprenticesCurrent()));
    apprenticesMaxEdit_->setText(
        QString::number(faction->getApprenticesMax()));

    updateAttitudesPanel(faction);
}

void FactionsTabContentQt::captureOriginalAttitudeSnapshot(const Faction* faction)
{
    originalDeclaredAttitudesText_.clear();
    originalDefaultAttitudeText_ = L"Neutral";

    if (!faction)
        return;

    const int factionNumber = faction->getFactionNumber();
    if (originalDefaultAttitudeByFactionText_.find(factionNumber)
            == originalDefaultAttitudeByFactionText_.end())
    {
        originalDefaultAttitudeByFactionText_[factionNumber] =
            FactionAttitudeUtils::attitudeToText(faction->getDefaultAttitude());

        std::map<int, std::wstring> declaredText;
        for (const auto& [targetNum, attitude] : faction->getDeclaredAttitudes())
            declaredText[targetNum] = FactionAttitudeUtils::attitudeToText(attitude);
        originalDeclaredAttitudesByFactionText_[factionNumber] = std::move(declaredText);
    }

    originalDefaultAttitudeText_ = originalDefaultAttitudeByFactionText_[factionNumber];
    originalDeclaredAttitudesText_ = originalDeclaredAttitudesByFactionText_[factionNumber];
}

void FactionsTabContentQt::updateAttitudesPanel(const Faction* faction)
{
    if (!faction || !faction->isMainFaction())
    {
        {
            QSignalBlocker b(defaultAttitudeCombo_);
            defaultAttitudeCombo_->setCurrentIndex(-1);
        }
        attitudesTable_->setRowCount(0);
        unclaimedSilverEdit_->clear();
        commandUnitEdit_->clear();
        attitudesWidget_->setVisible(false);
        return;
    }

    attitudesWidget_->setVisible(true);

    // Default attitude combo
    {
        QSignalBlocker b(defaultAttitudeCombo_);
        const QString attText =
            QString::fromWCharArray(
                FactionAttitudeUtils::attitudeToText(faction->getDefaultAttitude()));
        const int idx = defaultAttitudeCombo_->findText(attText);
        defaultAttitudeCombo_->setCurrentIndex(idx >= 0 ? idx : 2);  // fallback = Neutral
    }

    // Command unit
    int commandUnitNumber = 0;
    if (appData_)
        commandUnitNumber = faction->resolveCommandUnitNumber(appData_->unitRepository());
    commandUnitEdit_->setText(
        commandUnitNumber > 0 ? QString::number(commandUnitNumber) : QString());

    // Unclaimed silver
    const int actual      = faction->getUnclaimedSilver();
    const int afterOrders = faction->getUnclaimedSilverAfterOrders();
    unclaimedSilverEdit_->setText(
        QString("%1 (%2)").arg(actual).arg(afterOrders));

    updateAttitudesList(faction);
}

void FactionsTabContentQt::updateAttitudesList(const Faction* faction)
{
    attitudesTable_->setRowCount(0);
    if (!appData_ || !faction || !faction->isMainFaction())
        return;

    const Faction::Attitude defaultAttitude = faction->getDefaultAttitude();
    const std::map<int, Faction::Attitude>& declared = faction->getDeclaredAttitudes();
    const auto& repository = appData_->factionRepository();

    std::vector<const Faction*> others;
    others.reserve(repository.size());
    for (std::size_t i = 0; i < repository.size(); ++i)
    {
        const Faction& f = repository.at(i);
        if (f.getFactionNumber() != faction->getFactionNumber())
            others.push_back(&f);
    }
    std::sort(others.begin(), others.end(),
        [](const Faction* a, const Faction* b)
        {
            return a->getFactionNumber() < b->getFactionNumber();
        });

    for (const Faction* target : others)
    {
        const int targetNum = target->getFactionNumber();

        // Determine effective attitude (pending edits override stored data)
        Faction::Attitude resolvedAttitude = defaultAttitude;
        bool resolvedFromDefault = true;

        const auto declaredIt = declared.find(targetNum);
        if (declaredIt != declared.end())
        {
            resolvedAttitude     = declaredIt->second;
            resolvedFromDefault  = false;
        }

        const auto pendingIt = pendingAttitudeEdits_.find(targetNum);
        if (pendingIt != pendingAttitudeEdits_.end())
        {
            if (pendingIt->second.useDefault)
            {
                resolvedAttitude    = defaultAttitude;
                resolvedFromDefault = true;
            }
            else
            {
                resolvedAttitude    =
                    FactionAttitudeUtils::textToAttitude(pendingIt->second.attitudeText);
                resolvedFromDefault = false;
            }
        }

        const int row = attitudesTable_->rowCount();
        attitudesTable_->insertRow(row);

        // Column 0: faction name + number
        QString factionLabel;
        if (!target->getName().empty())
            factionLabel = QString::fromStdWString(target->getName())
                           + " (" + QString::number(targetNum) + ")";
        else
            factionLabel = QString::number(targetNum);

        auto* col0 = new QTableWidgetItem(factionLabel);
        col0->setData(kFactionNumberRole, targetNum);
        attitudesTable_->setItem(row, 0, col0);

        // Column 1: attitude text, with "(default)" suffix when not explicitly declared
        QString attText =
            QString::fromWCharArray(
                FactionAttitudeUtils::attitudeToText(resolvedAttitude));
        if (resolvedFromDefault)
            attText += " (default)";
        attitudesTable_->setItem(row, 1, new QTableWidgetItem(attText));
    }
}

void FactionsTabContentQt::handleDefaultAttitudeSelection(
    Faction& faction, const std::wstring& selectedAttitudeText)
{
    faction.setDefaultAttitude(FactionAttitudeUtils::textToAttitude(selectedAttitudeText));
    updateAttitudesList(&faction);
}

void FactionsTabContentQt::applyAttitudeContextSelection(
    int targetFactionNumber, const std::wstring& selectedValue)
{
    if (!appData_ || targetFactionNumber <= 0)
        return;

    Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
    if (!faction || !faction->isMainFaction())
        return;

    const auto& declared     = faction->getDeclaredAttitudes();
    const auto  declaredIt   = declared.find(targetFactionNumber);
    const bool  hadDeclared  = (declaredIt != declared.end());

    if (selectedValue == L"Default")
    {
        if (hadDeclared)
            pendingAttitudeEdits_[targetFactionNumber] = PendingAttitudeEdit{ true, L"" };
        else
            pendingAttitudeEdits_.erase(targetFactionNumber);
    }
    else
    {
        const Faction::Attitude selectedAttitude =
            FactionAttitudeUtils::textToAttitude(selectedValue);
        const bool unchanged =
            hadDeclared && declaredIt->second == selectedAttitude;
        if (unchanged)
            pendingAttitudeEdits_.erase(targetFactionNumber);
        else
            pendingAttitudeEdits_[targetFactionNumber] =
                PendingAttitudeEdit{ false,
                    FactionAttitudeUtils::attitudeToText(selectedAttitude) };
    }

    updateAttitudesList(faction);
}

void FactionsTabContentQt::saveAttitudeEdits()
{
    if (!appData_ || pendingAttitudeEdits_.empty())
        return;

    Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
    if (!faction || !faction->isMainFaction())
        return;

    std::vector<OrderBusinessLogic::DeclaredAttitudeChange> changes;
    changes.reserve(pendingAttitudeEdits_.size());
    for (const auto& [targetNum, edit] : pendingAttitudeEdits_)
    {
        OrderBusinessLogic::DeclaredAttitudeChange change;
        change.targetFactionNumber = targetNum;
        change.useDefault          = edit.useDefault;
        change.attitudeText        = edit.attitudeText;
        changes.push_back(std::move(change));
    }

    if (OrderBusinessLogic::applyDeclaredAttitudeChanges(
            *appData_, faction->getFactionNumber(), changes))
    {
        pendingAttitudeEdits_.clear();
        updateAttitudesList(faction);
    }
}

void FactionsTabContentQt::clearFields()
{
    pendingAttitudeEdits_.clear();
    originalDeclaredAttitudesText_.clear();
    originalDefaultAttitudeText_ = L"Neutral";

    factionNumberEdit_->clear();
    factionNameEdit_->clear();
    mainFactionCheck_->setChecked(false);
    monthEdit_->clear();
    yearEdit_->clear();
    passwordEdit_->clear();
    taxedTradedCurrentEdit_->clear();
    taxedTradedMaxEdit_->clear();
    quartermastersCurrentEdit_->clear();
    quartermastersMaxEdit_->clear();
    magesCurrentEdit_->clear();
    magesMaxEdit_->clear();
    apprenticesCurrentEdit_->clear();
    apprenticesMaxEdit_->clear();

    {
        QSignalBlocker b(defaultAttitudeCombo_);
        defaultAttitudeCombo_->setCurrentIndex(-1);
    }
    attitudesTable_->setRowCount(0);
    unclaimedSilverEdit_->clear();
    commandUnitEdit_->clear();
    attitudesWidget_->setVisible(false);
}

void FactionsTabContentQt::saveSelectedFaction()
{
    if (!appData_ || selectedFactionNumber_ <= 0)
        return;

    Faction* faction = appData_->factionRepository().findByNumber(selectedFactionNumber_);
    if (!faction)
        return;

    faction->setName(
        StringUtils::trimWhitespace(factionNameEdit_->text().toStdWString()));
    faction->setMonth(
        StringUtils::parseIntSafe(monthEdit_->text().toStdWString()));
    faction->setYear(
        StringUtils::parseIntSafe(yearEdit_->text().toStdWString()));
    faction->setPassword(passwordEdit_->text().toStdWString());
    faction->setTaxedOrTradedRegionsCurrent(
        StringUtils::parseIntSafe(taxedTradedCurrentEdit_->text().toStdWString()));
    faction->setTaxedOrTradedRegionsMax(
        StringUtils::parseIntSafe(taxedTradedMaxEdit_->text().toStdWString()));
    faction->setQuartermastersCurrent(
        StringUtils::parseIntSafe(quartermastersCurrentEdit_->text().toStdWString()));
    faction->setQuartermastersMax(
        StringUtils::parseIntSafe(quartermastersMaxEdit_->text().toStdWString()));
    faction->setMagesCurrent(
        StringUtils::parseIntSafe(magesCurrentEdit_->text().toStdWString()));
    faction->setMagesMax(
        StringUtils::parseIntSafe(magesMaxEdit_->text().toStdWString()));
    faction->setApprenticesCurrent(
        StringUtils::parseIntSafe(apprenticesCurrentEdit_->text().toStdWString()));
    faction->setApprenticesMax(
        StringUtils::parseIntSafe(apprenticesMaxEdit_->text().toStdWString()));

    if (faction->isMainFaction())
    {
        const int idx = defaultAttitudeCombo_->currentIndex();
        if (idx >= 0)
        {
            faction->setDefaultAttitude(
                FactionAttitudeUtils::textToAttitude(
                    defaultAttitudeCombo_->itemText(idx).toStdWString()));
        }
        faction->setCommandUnitNumber(
            StringUtils::parseIntSafe(commandUnitEdit_->text().toStdWString()));
    }

    // Propagate main-faction flag: only one faction can be main at a time.
    const bool selectedAsMain = mainFactionCheck_->isChecked();
    auto& repository = appData_->factionRepository();
    if (selectedAsMain)
    {
        for (std::size_t i = 0; i < repository.size(); ++i)
        {
            Faction* candidate = repository.findByNumber(repository.at(i).getFactionNumber());
            if (candidate)
                candidate->setMainFaction(
                    candidate->getFactionNumber() == selectedFactionNumber_);
        }
    }
    else
    {
        faction->setMainFaction(false);
    }

    refresh();
}
