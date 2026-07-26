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
 * File: MapTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QPoint>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "GUI/MapCanvasWidget.hpp"

#include <string>
#include <vector>

class AppConfig;
class AppData;
class Region;
class Unit;
class UnitNew;

// Forward-declared Qt widget types (no Qt headers pulled into user headers)
class QLabel;
class QLineEdit;
class QListWidget;
class QTreeWidget;
class QPlainTextEdit;
class QPushButton;
class QSplitter;
class QTabWidget;
class QTableWidget;
class QWidget;


/**
 * @brief Content widget for the "Map" tab (Qt / Linux build).
 *
 * Mirrors MapTabContent for the Win32 build.  Hosts the hex-map canvas
 * (MapCanvasWidget), a units list (QTableWidget), an orders editor
 * (QPlainTextEdit), unit-detail inner tabs (items, skills, errors, warnings,
 * events), region detail lists, and order-checking / warning-navigation
 * controls.
 *
 * Navigation actions (battle link, skill link) triggered from the map
 * context menu are emitted as Qt signals so MainWindow can wire them to the
 * target tab content widgets without coupling MapTabContentQt to those classes.
 *
 * Implementation is split across multiple .cpp files that mirror the Win32
 * MapTabContent_*.cpp split (steps 7.2 – 7.9):
 *
 *   MapTabContentQt_Layout.cpp      (7.2) — constructor + QSplitter wiring
 *   MapTabContentQt_UnitDetails.cpp (7.3) — unit list + inner detail tabs
 *   MapTabContentQt_Orders.cpp      (7.4) — orders editor + save/check logic
 *   MapTabContentQt_OrderChecks.cpp (7.5) — warning navigation
 *   MapTabContentQt_RegionDetails.cpp (7.6) — region info pane
 *   MapTabContentQt_Navigation.cpp  (7.7) — unit search + selectUnitInMap
 *   MapTabContentQt_Events.cpp      (7.8) — slot wiring (connects all signals)
 *   MapTabContentQt_MapCanvas.cpp   (7.9) — MapCanvasWidget + paintEvent
 */
class MapTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit MapTabContentQt(AppData& appData, AppConfig& appConfig,
                              QWidget* parent = nullptr);
    ~MapTabContentQt() override = default;

    MapTabContentQt(const MapTabContentQt&) = delete;
    MapTabContentQt& operator=(const MapTabContentQt&) = delete;
    MapTabContentQt(MapTabContentQt&&) = delete;
    MapTabContentQt& operator=(MapTabContentQt&&) = delete;

    /** Repopulates all panels from the current AppData state. */
    void refresh();

    /** Saves any pending orders edits for the currently selected unit. */
    void commitPendingEdits();

    /** Refreshes the items panel for the currently selected unit without
     *  rebuilding the whole view (called after item data changes). */
    void refreshItemsForCurrentUnit();

signals:
    /**
     * @brief Emitted when the user chooses "Show Battle Report" from the map
     *        context menu. The receiver should switch to the Battles tab and
     *        call BattlesTabContentQt::focusBattleByRegion().
     */
    void navigateToBattle(int x, int y, int z, int month, int year);

    /**
     * @brief Emitted when the user navigates to a skill from the map context
     *        (e.g. a skill link in a unit summary). The receiver should switch
     *        to the Skills tab and call SkillsTabContentQt::focusSkillByToken().
     */
    void navigateToSkill(const QString& skillToken);

    /**
     * @brief Emitted when the user navigates to an item from map region item
     *        lists. The receiver should switch to the Items tab and call
     *        ItemsTabContentQt::focusItemByToken().
     */
    void navigateToItem(const QString& itemToken);

private slots:
    // -----------------------------------------------------------------------
    // Map canvas events — wired in MapTabContentQt_Events.cpp (7.8)
    // (signals emitted by MapCanvasWidget; implemented in 7.9)
    // -----------------------------------------------------------------------
    void onMapRegionLeftClicked(int regionX, int regionY);
    void onMapRegionDoubleClicked(int regionX, int regionY);
    void onMapRegionRightClicked(QPoint screenPos, int regionX, int regionY);
    void onMapNoRegionClicked();
    void onZSelectionRequested(QPoint screenPos);

    // -----------------------------------------------------------------------
    // Units list — wired in 7.8; implemented in 7.3
    // -----------------------------------------------------------------------
    void onUnitsSelectionChanged();
    void onUnitsHeaderSectionDoubleClicked(int logicalIndex);

    // -----------------------------------------------------------------------
    // Unit-detail inner tabs — wired in 7.8; implemented in 7.3
    // -----------------------------------------------------------------------
    void onUnitDetailsTabChanged(int index);

    // -----------------------------------------------------------------------
    // Orders buttons — wired in 7.8; implemented in 7.4
    // -----------------------------------------------------------------------
    void onSaveOrdersClicked();
    void onCheckOrdersClicked();

    // -----------------------------------------------------------------------
    // Warning-navigation buttons — wired in 7.8; implemented in 7.5
    // -----------------------------------------------------------------------
    void onPrevWarningClicked();
    void onNextWarningClicked();
    void onClearWarningClicked();

    // -----------------------------------------------------------------------
    // Unit search — wired in 7.8; implemented in 7.7
    // -----------------------------------------------------------------------
    void onSearchUnitClicked();

    // -----------------------------------------------------------------------
    // Context menus — wired in 7.8
    // -----------------------------------------------------------------------
    void onOrdersEditorContextMenuRequested(const QPoint& pos);
    void onUnitSkillsContextMenuRequested(const QPoint& pos);
    void onUnitWarningsContextMenuRequested(const QPoint& pos);
    void onRegionResourcesContextMenuRequested(const QPoint& pos);
    void onRegionForSaleContextMenuRequested(const QPoint& pos);
    void onRegionWantedContextMenuRequested(const QPoint& pos);

private:
    // -----------------------------------------------------------------------
    // AppData / AppConfig references
    // -----------------------------------------------------------------------
    AppData*   appData_   { nullptr };
    AppConfig* appConfig_ { nullptr };

    // -----------------------------------------------------------------------
    // Layout splitters — created in MapTabContentQt_Layout.cpp (7.2)
    // -----------------------------------------------------------------------
    QSplitter* mainSplitter_       { nullptr };  // horizontal: left (details+map+units) | right (unit panel)
    QSplitter* detailsMapSplitter_ { nullptr };  // horizontal within left: region details | map canvas

    // -----------------------------------------------------------------------
    // Map canvas — MapCanvasWidget implemented in 7.9; see forward decl above
    // -----------------------------------------------------------------------
    MapCanvasWidget* mapCanvas_ { nullptr };

    // -----------------------------------------------------------------------
    // Region details pane — implemented in MapTabContentQt_RegionDetails.cpp (7.6)
    // -----------------------------------------------------------------------
    QLabel*         regionDateLabel_      { nullptr };
    QLabel*         hoverRegionLabel_     { nullptr };   // hovered-region name
    QPlainTextEdit* regionDetailsView_    { nullptr };   // read-only region report text
    QLabel*         regionResourcesLabel_ { nullptr };
    QTreeWidget*    regionResourcesList_  { nullptr };
    QLabel*         regionForSaleLabel_   { nullptr };
    QTreeWidget*    regionForSaleList_    { nullptr };
    QLabel*         regionWantedLabel_    { nullptr };
    QTreeWidget*    regionWantedList_     { nullptr };

    // -----------------------------------------------------------------------
    // Units list — implemented in MapTabContentQt_UnitDetails.cpp (7.3)
    // -----------------------------------------------------------------------
    QTableWidget* unitsList_  { nullptr };

    // Unit-detail inner tab widget and its tab pages
    QTabWidget*   unitDetailsTabs_    { nullptr };
    QTableWidget* unitItemsList_      { nullptr };   // Items tab (columns: Token, Name, Amount, after com.)
    QListWidget* unitSkillsList_     { nullptr };
    QListWidget* unitErrorsList_     { nullptr };
    QListWidget* unitWarningsList_   { nullptr };
    QListWidget* unitEventsList_     { nullptr };

    // Unit summary / status labels
    QLabel* selectedUnitLabel_     { nullptr };
    QLabel* unitCoordinatesLabel_  { nullptr };
    QLabel* unitFlagsLabel_        { nullptr };
    QLabel* unitWarningLabel_      { nullptr };
    QLabel* unitWeightLabel_          { nullptr };
    QLabel* unitCapacitiesLabel_       { nullptr };
    QLabel* unitShipCapacityLabel_     { nullptr };

    // -----------------------------------------------------------------------
    // Orders editor — implemented in MapTabContentQt_Orders.cpp (7.4)
    // -----------------------------------------------------------------------
    QPlainTextEdit* ordersEditor_     { nullptr };
    QPushButton*    saveOrdersButton_ { nullptr };
    QPushButton*    checkOrdersButton_{ nullptr };

    // -----------------------------------------------------------------------
    // Warning navigation — implemented in MapTabContentQt_OrderChecks.cpp (7.5)
    // -----------------------------------------------------------------------
    QPushButton* prevWarningButton_  { nullptr };
    QPushButton* clearWarningButton_ { nullptr };
    QPushButton* nextWarningButton_  { nullptr };
    QLabel*      warningsCountLabel_ { nullptr };

    // -----------------------------------------------------------------------
    // Unit search — implemented in MapTabContentQt_Navigation.cpp (7.7)
    // -----------------------------------------------------------------------
    QLineEdit*   unitSearchEdit_   { nullptr };
    QPushButton* unitSearchButton_ { nullptr };

    // -----------------------------------------------------------------------
    // Selection / display state
    // -----------------------------------------------------------------------
    std::vector<int> availableZLevels_;
    int  selectedZ_            { 1 };
    bool hasSelectedRegion_    { false };
    int  selectedRegionX_      { 0 };
    int  selectedRegionY_      { 0 };
    int  selectedUnitNumber_   { 0 };
    bool selectedUnitIsNew_    { false };
    int  unitsListSortColumn_  { -1 };
    bool unitsListSortAscending_ { true };
    QStringList unitsListBaseHeaderLabels_;

    // Capacity display values (computed in updateUnitWeightAndCapacities)
    int  capacityWalkDisplay_       { 0 };
    int  capacityRideDisplay_       { 0 };
    int  capacityFlyDisplay_        { 0 };
    int  capacitySwimDisplay_       { 0 };
    int  shipCapacityDisplay_       { 0 };
    int  shipFreeCapacityDisplay_   { 0 };
    int  shipSkillNeedDisplay_      { 0 };
    int  shipOwnerSailingDisplay_   { 0 };
    bool showRideCapacity_          { false };
    bool showFlyCapacity_           { false };
    bool showSwimCapacity_          { false };
    bool hasCapacityValues_         { false };
    bool hasShipCapacityValues_     { false };
    bool hasShipOwnerSkillValues_   { false };
    bool shipIsFlying_              { false };

    // -----------------------------------------------------------------------
    // Private methods — implemented across the various _*.cpp files
    // -----------------------------------------------------------------------

    // 7.3 — Unit list population and detail panel
    void populateUnitsForSelectedRegion();
    void populateItemsForSelectedUnit(const Unit* unit);
    void populateItemsForSelectedUnit(const UnitNew* unitNew);
    void populateSkillsList(const Unit* unit);
    void populateSkillsList(const UnitNew* unitNew);
    int  populateErrorsList(const Unit* unit);
    int  populateWarningsList(const Unit* unit);
    int  populateWarningsList(const UnitNew* unitNew);
    int  populateUnitEventsList(const Unit* unit);
    void updateUnitWeightAndCapacities(const Unit* unit);
    void updateUnitWeightAndCapacities(const UnitNew* unitNew);
    void updateUnitDetailsTabCaptions(int errorCount, int warningCount, int eventCount);
    void updateUnitDetailsTabVisibility();
    void clearUnitsList();
    void updateSelectedUnitFromList();
    void updateSelectedUnitDetailsByNumber(int unitNumber);
    void clearSelectedUnitDetails();
    void sortUnitsListByColumn(int columnIndex, bool ascending);
    void updateUnitsListSortHeaderMarkers();

    // 7.4 — Orders editor
    void appendOrderLineToOrdersEditor(const std::wstring& orderLine);
    void saveOrdersToSelectedUnit();
    bool canEditOrdersForUnit(const Unit* unit) const;
    void setOrdersEditingEnabled(bool enabled);

    // 7.5 — Order checks / warning navigation
    void runOrderChecksForMainFaction();
    void updateWarningsSummaryLabel();
    void selectPreviousWarningUnit();
    void selectNextWarningUnit();
    void clearWarningsForSelectedUnit();

    // 7.6 — Region details
    void updateRegionDetailsView(const Region* region);
    void populateResourcesList(const Region* region);
    void populateForSaleList(const Region* region);
    void populateWantedList(const Region* region);

    // 7.7 — Navigation / unit search
    void selectUnitInMap(int unitNumber);
    bool focusOriginUnitForSelectedUnitNew();
    void searchAndSelectUnitById();
    void handleRegionItemContextMenuRequested(QTreeWidget* sourceList, const QPoint& pos);

    // 7.9 — Map canvas helpers
    void navigateToSkillList(const std::wstring& skillToken);
    void showSkillDescription(const std::wstring& skillToken);
};
