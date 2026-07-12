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
 * File: MapTabContentQt_Layout.cpp
 *
 * Step 7.2 — constructor and QSplitter wiring.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapTabContentQt.hpp"
#include "GUI/MapCanvasWidget.hpp"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

MapTabContentQt::MapTabContentQt(AppData&   appData,
                                  AppConfig& appConfig,
                                  QWidget*   parent)
    : QWidget(parent)
    , appData_  (&appData)
    , appConfig_(&appConfig)
{
    // -----------------------------------------------------------------------
    // Top-level layout
    // -----------------------------------------------------------------------
    auto* topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    // -----------------------------------------------------------------------
    // Outer horizontal splitter: left (details + map + units) | right (unit panel)
    // -----------------------------------------------------------------------
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    topLayout->addWidget(mainSplitter_);

    // -----------------------------------------------------------------------
    // Left container
    // -----------------------------------------------------------------------
    auto* leftContainer = new QWidget(mainSplitter_);
    leftContainer->setMinimumWidth(420);
    mainSplitter_->addWidget(leftContainer);

    auto* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(4, 4, 4, 4);
    leftLayout->setSpacing(4);

    // Inner horizontal splitter: region-details column | map canvas
    detailsMapSplitter_ = new QSplitter(Qt::Horizontal, leftContainer);
    leftLayout->addWidget(detailsMapSplitter_, 3); // stretch 3 so map area dominates

    // -----------------------------------------------------------------------
    // Details pane (left half of inner splitter)
    // Region date, hover label, read-only region text, resource / sale / wanted lists.
    // Content is populated by MapTabContentQt_RegionDetails.cpp (step 7.6).
    // -----------------------------------------------------------------------
    auto* detailsPane = new QWidget(detailsMapSplitter_);
    detailsPane->setMinimumWidth(150);
    detailsMapSplitter_->addWidget(detailsPane);

    auto* detailsPaneLayout = new QVBoxLayout(detailsPane);
    detailsPaneLayout->setContentsMargins(0, 0, 2, 0);
    detailsPaneLayout->setSpacing(2);

    regionDateLabel_   = new QLabel("\u2014", detailsPane);       // em-dash placeholder
    hoverRegionLabel_  = new QLabel("Hover: -", detailsPane);

    regionDetailsView_ = new QPlainTextEdit(detailsPane);
    regionDetailsView_->setReadOnly(true);
    regionDetailsView_->setMinimumHeight(60);
    regionDetailsView_->setPlaceholderText("No region selected");

    // Resources tree: Item | Amount | After Cmd
    regionResourcesList_ = new QTreeWidget(detailsPane);
    regionResourcesList_->setMinimumHeight(40);
    regionResourcesList_->setColumnCount(3);
    regionResourcesList_->setHeaderLabels({"Item", "Amount", "After Cmd"});
    regionResourcesList_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    regionResourcesList_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    regionResourcesList_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    regionResourcesList_->setRootIsDecorated(false);
    regionResourcesList_->setSelectionMode(QAbstractItemView::NoSelection);

    // For Sale tree: Token | Amount | Price | After Cmd
    regionForSaleList_ = new QTreeWidget(detailsPane);
    regionForSaleList_->setMinimumHeight(40);
    regionForSaleList_->setColumnCount(4);
    regionForSaleList_->setHeaderLabels({"Token", "Amount", "Price", "After Cmd"});
    regionForSaleList_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    regionForSaleList_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    regionForSaleList_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    regionForSaleList_->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    regionForSaleList_->setRootIsDecorated(false);
    regionForSaleList_->setSelectionMode(QAbstractItemView::NoSelection);

    // Wanted tree: Token | Amount | Price | After Cmd
    regionWantedList_ = new QTreeWidget(detailsPane);
    regionWantedList_->setMinimumHeight(40);
    regionWantedList_->setColumnCount(4);
    regionWantedList_->setHeaderLabels({"Token", "Amount", "Price", "After Cmd"});
    regionWantedList_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    regionWantedList_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    regionWantedList_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    regionWantedList_->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    regionWantedList_->setRootIsDecorated(false);
    regionWantedList_->setSelectionMode(QAbstractItemView::NoSelection);

    detailsPaneLayout->addWidget(regionDateLabel_);
    detailsPaneLayout->addWidget(hoverRegionLabel_);
    detailsPaneLayout->addWidget(regionDetailsView_, 2);
    detailsPaneLayout->addWidget(regionResourcesList_, 1);
    detailsPaneLayout->addWidget(regionForSaleList_, 1);
    detailsPaneLayout->addWidget(regionWantedList_, 1);

    // -----------------------------------------------------------------------
    // Map canvas (right half of inner splitter)
    // Step 7.9.1 introduces the MapCanvasWidget skeleton.
    // -----------------------------------------------------------------------
    mapCanvas_ = new MapCanvasWidget(*appData_, *appConfig_, detailsMapSplitter_);
    mapCanvas_->setMinimumWidth(200);
    mapCanvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    detailsMapSplitter_->addWidget(mapCanvas_);

    // Details : map initial split — 20 % : 80 %
    detailsMapSplitter_->setSizes({160, 640});
    detailsMapSplitter_->setStretchFactor(0, 0);
    detailsMapSplitter_->setStretchFactor(1, 1);

    // -----------------------------------------------------------------------
    // Button row: check / warning navigation / unit search
    // (below the map, above the units list)
    // -----------------------------------------------------------------------
    auto* buttonRowWidget = new QWidget(leftContainer);
    buttonRowWidget->setFixedHeight(30);
    leftLayout->addWidget(buttonRowWidget);

    auto* buttonRowLayout = new QHBoxLayout(buttonRowWidget);
    buttonRowLayout->setContentsMargins(0, 0, 0, 0);
    buttonRowLayout->setSpacing(4);

    checkOrdersButton_  = new QPushButton("Check Orders",  buttonRowWidget);
    prevWarningButton_  = new QPushButton("\u25C4 Prev",   buttonRowWidget);  // ◄ Prev
    clearWarningButton_ = new QPushButton("Clear",          buttonRowWidget);
    nextWarningButton_  = new QPushButton("Next \u25BA",   buttonRowWidget);  // Next ►
    warningsCountLabel_ = new QLabel("Warnings: 0",         buttonRowWidget);
    unitSearchEdit_     = new QLineEdit(buttonRowWidget);
    unitSearchEdit_->setPlaceholderText("Unit id\u2026");
    unitSearchEdit_->setFixedWidth(90);
    unitSearchButton_ = new QPushButton("Search", buttonRowWidget);

    buttonRowLayout->addWidget(checkOrdersButton_);
    buttonRowLayout->addWidget(prevWarningButton_);
    buttonRowLayout->addWidget(clearWarningButton_);
    buttonRowLayout->addWidget(nextWarningButton_);
    buttonRowLayout->addWidget(warningsCountLabel_);
    buttonRowLayout->addStretch();
    buttonRowLayout->addWidget(unitSearchEdit_);
    buttonRowLayout->addWidget(unitSearchButton_);

    // -----------------------------------------------------------------------
    // Units list (QTableWidget — filled by MapTabContentQt_UnitDetails.cpp, step 7.3)
    // Columns mirror the Win32 LVS_REPORT: #, Name, Faction, Faction Name,
    // Structure, Men, Silver, Flags, Skills, ! (errors), D (warnings).
    // -----------------------------------------------------------------------
    unitsList_ = new QTableWidget(0, 11, leftContainer);
    unitsList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    unitsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    unitsList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    unitsList_->setAlternatingRowColors(true);
    unitsList_->verticalHeader()->setVisible(false);
    unitsList_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    unitsList_->setHorizontalHeaderLabels({
        "#", "Name", "Faction", "Faction Name",
        "Structure", "Men", "Silver", "Flags", "Skills", "!", "D"
    });
    unitsList_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    unitsList_->setColumnWidth(0, 50);
    unitsList_->setColumnWidth(1, 180);
    unitsList_->setColumnWidth(2, 50);
    unitsList_->setColumnWidth(3, 120);
    unitsList_->setColumnWidth(4, 150);
    unitsList_->setColumnWidth(5, 60);
    unitsList_->setColumnWidth(6, 70);
    unitsList_->setColumnWidth(7, 200);
    unitsList_->setColumnWidth(8, 200);
    unitsList_->setColumnWidth(9, 28);
    unitsList_->setColumnWidth(10, 28);
    unitsList_->setMinimumHeight(80);
    leftLayout->addWidget(unitsList_, 1);

    // -----------------------------------------------------------------------
    // Right panel: unit summary labels + detail tabs
    // -----------------------------------------------------------------------
    auto* rightPanel = new QWidget(mainSplitter_);
    rightPanel->setMinimumWidth(250);
    mainSplitter_->addWidget(rightPanel);

    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(2);

    selectedUnitLabel_    = new QLabel(rightPanel);
    unitCoordinatesLabel_ = new QLabel(rightPanel);
    unitFlagsLabel_       = new QLabel(rightPanel);
    unitFlagsLabel_->setWordWrap(true);
    unitWarningLabel_     = new QLabel(rightPanel);
    unitWeightLabel_      = new QLabel(rightPanel);
    unitCapacitiesLabel_  = new QLabel(rightPanel);
    unitCapacitiesLabel_->setWordWrap(true);
    unitShipCapacityLabel_ = new QLabel(rightPanel);
    unitShipCapacityLabel_->setWordWrap(true);
    unitShipCapacityLabel_->hide();

    rightLayout->addWidget(selectedUnitLabel_);
    rightLayout->addWidget(unitCoordinatesLabel_);
    rightLayout->addWidget(unitFlagsLabel_);
    rightLayout->addWidget(unitWarningLabel_);
    rightLayout->addWidget(unitWeightLabel_);
    rightLayout->addWidget(unitCapacitiesLabel_);
    rightLayout->addWidget(unitShipCapacityLabel_);

    // -----------------------------------------------------------------------
    // Vertical splitter: upper (Items + Skills) | lower (tab widget)
    // Items and Skills are shown permanently above the tab widget so they
    // are always visible regardless of the active tab.
    // -----------------------------------------------------------------------
    auto* detailSplitter = new QSplitter(Qt::Vertical, rightPanel);
    rightLayout->addWidget(detailSplitter, 1);

    // --- Upper part: permanent Items list and Skills list ---
    auto* upperDetailsWidget = new QWidget(detailSplitter);
    auto* upperDetailsLayout = new QVBoxLayout(upperDetailsWidget);
    upperDetailsLayout->setContentsMargins(0, 0, 0, 0);
    upperDetailsLayout->setSpacing(2);
    detailSplitter->addWidget(upperDetailsWidget);

    // Items list — mirrors Win32 unitItemsList_ (LVS_REPORT, 4 columns)
    auto* itemsHeaderLabel = new QLabel("Items:", upperDetailsWidget);
    upperDetailsLayout->addWidget(itemsHeaderLabel);

    unitItemsList_ = new QTableWidget(0, 4, upperDetailsWidget);
    unitItemsList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    unitItemsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    unitItemsList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    unitItemsList_->verticalHeader()->setVisible(false);
    unitItemsList_->setHorizontalHeaderLabels({"Token", "Name", "Amount", "after com."});
    unitItemsList_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    unitItemsList_->setColumnWidth(0, 70);
    unitItemsList_->setColumnWidth(1, 100);
    unitItemsList_->setColumnWidth(2, 60);
    unitItemsList_->setColumnWidth(3, 70);
    upperDetailsLayout->addWidget(unitItemsList_, 1);

    // Skills list — mirrors Win32 unitSkillsList_
    auto* skillsHeaderLabel = new QLabel("Skills:", upperDetailsWidget);
    upperDetailsLayout->addWidget(skillsHeaderLabel);

    unitSkillsList_ = new QListWidget(upperDetailsWidget);
    upperDetailsLayout->addWidget(unitSkillsList_, 1);

    // --- Lower part: Orders / Events / Errors / Warnings tabs ---
    // Tabs: Orders | Events | Errors | Warnings
    // Data population is implemented in steps 7.3 and 7.4.
    unitDetailsTabs_ = new QTabWidget(detailSplitter);
    detailSplitter->addWidget(unitDetailsTabs_);

    // Orders tab — mirrors Win32 ordersEditor_ + saveOrdersButton_
    {
        auto* ordersPage   = new QWidget(unitDetailsTabs_);
        auto* ordersLayout = new QVBoxLayout(ordersPage);
        ordersLayout->setContentsMargins(0, 0, 0, 0);
        ordersLayout->setSpacing(4);

        ordersEditor_ = new QPlainTextEdit(ordersPage);
        ordersEditor_->setEnabled(false);
        ordersLayout->addWidget(ordersEditor_, 1);

        saveOrdersButton_ = new QPushButton("Save Orders", ordersPage);
        saveOrdersButton_->setEnabled(false);
        ordersLayout->addWidget(saveOrdersButton_);

        unitDetailsTabs_->addTab(ordersPage, "Orders");  // index 0
    }

    // Events tab — mirrors Win32 unitEventsList_
    unitEventsList_ = new QListWidget(unitDetailsTabs_);
    unitDetailsTabs_->addTab(unitEventsList_, "Events");  // index 1

    // Errors tab — mirrors Win32 unitErrorsList_
    unitErrorsList_ = new QListWidget(unitDetailsTabs_);
    unitDetailsTabs_->addTab(unitErrorsList_, "Errors");  // index 2

    // Warnings tab — mirrors Win32 unitWarningsList_
    unitWarningsList_ = new QListWidget(unitDetailsTabs_);
    unitDetailsTabs_->addTab(unitWarningsList_, "Warnings");  // index 3

    unitDetailsTabs_->setCurrentIndex(0); // start on Orders tab

    // Give items+skills roughly 55 % and the tab widget 45 % of the available space.
    detailSplitter->setStretchFactor(0, 11);
    detailSplitter->setStretchFactor(1, 9);

        connect(unitsList_, &QTableWidget::itemSelectionChanged,
            this, &MapTabContentQt::onUnitsSelectionChanged);
        connect(unitDetailsTabs_, &QTabWidget::currentChanged,
            this, &MapTabContentQt::onUnitDetailsTabChanged);
        connect(saveOrdersButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onSaveOrdersClicked);
        connect(checkOrdersButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onCheckOrdersClicked);
        connect(prevWarningButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onPrevWarningClicked);
        connect(nextWarningButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onNextWarningClicked);
        connect(clearWarningButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onClearWarningClicked);
        connect(unitSearchButton_, &QPushButton::clicked,
            this, &MapTabContentQt::onSearchUnitClicked);
        connect(unitSearchEdit_, &QLineEdit::returnPressed,
            this, &MapTabContentQt::onSearchUnitClicked);

        ordersEditor_->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ordersEditor_, &QPlainTextEdit::customContextMenuRequested,
            this, &MapTabContentQt::onOrdersEditorContextMenuRequested);

        unitSkillsList_->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(unitSkillsList_, &QListWidget::customContextMenuRequested,
            this, &MapTabContentQt::onUnitSkillsContextMenuRequested);

            connect(mapCanvas_, &MapCanvasWidget::mapRegionLeftClicked,
                this, &MapTabContentQt::onMapRegionLeftClicked);
            connect(mapCanvas_, &MapCanvasWidget::mapRegionDoubleClicked,
                this, &MapTabContentQt::onMapRegionDoubleClicked);
            connect(mapCanvas_, &MapCanvasWidget::mapRegionRightClicked,
                this, &MapTabContentQt::onMapRegionRightClicked);
            connect(mapCanvas_, &MapCanvasWidget::mapNoRegionClicked,
                this, &MapTabContentQt::onMapNoRegionClicked);
            connect(mapCanvas_, &MapCanvasWidget::zSelectionRequested,
                this, &MapTabContentQt::onZSelectionRequested);
            connect(mapCanvas_, &MapCanvasWidget::hoverTextChanged,
                this,
                [this](const QString& hoverText)
                {
                    if (hoverRegionLabel_)
                    {
                        hoverRegionLabel_->setText(hoverText);
                    }
                });

    // -----------------------------------------------------------------------
    // Outer splitter initial sizes: left 75 % | right 25 %
    // -----------------------------------------------------------------------
    mainSplitter_->setSizes({750, 250});
    mainSplitter_->setStretchFactor(0, 3);
    mainSplitter_->setStretchFactor(1, 1);
}

