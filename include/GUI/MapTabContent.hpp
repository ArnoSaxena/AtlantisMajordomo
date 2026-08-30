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
 * File: MapTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include "GUI/ControlIds.hpp"
#include "GUI/UiSizeProfile.hpp"
#include <array>
#include <functional>
#include <string>
#include <variant>
#include <vector>

class AppConfig;
class AppData;
class Region;
class Unit;
class UnitNew;

class MapTabContent
{
public:
  static constexpr int kUnitsListControlId = GUI::ControlIds::kMapUnitsList;
  static constexpr int kSaveOrdersButtonId = GUI::ControlIds::kMapSaveOrdersButton;
  static constexpr int kCheckOrdersButtonId = GUI::ControlIds::kMapCheckOrdersButton;
  static constexpr int kLastWarningButtonId = GUI::ControlIds::kMapLastWarningButton;
  static constexpr int kClearWarningButtonId = GUI::ControlIds::kMapClearWarningButton;
  static constexpr int kNextWarningButtonId = GUI::ControlIds::kMapNextWarningButton;
  static constexpr int kUnitSearchButtonId = GUI::ControlIds::kMapUnitSearchButton;
  static constexpr int kUnitSearchEditControlId = GUI::ControlIds::kMapUnitSearchEdit;
  static constexpr int kUnitItemsListControlId = GUI::ControlIds::kMapUnitItemsList;
  static constexpr int kUnitErrorsListControlId = GUI::ControlIds::kMapUnitErrorsList;
  static constexpr int kUnitDetailsTabsControlId = GUI::ControlIds::kMapUnitDetailsTabs;
  static constexpr int kUnitWarningsListControlId = GUI::ControlIds::kMapUnitWarningsList;
  static constexpr int kUnitEventsListControlId = GUI::ControlIds::kMapUnitEventsList;
  static constexpr int kUnitCapacitiesLabelControlId = GUI::ControlIds::kMapUnitCapacitiesLabel;

  MapTabContent() = default;
  ~MapTabContent() = default;

  MapTabContent(const MapTabContent&) = delete;
  MapTabContent& operator=(const MapTabContent&) = delete;
  MapTabContent(MapTabContent&&) = delete;
  MapTabContent& operator=(MapTabContent&&) = delete;

  bool create(HWND parentWindow, HINSTANCE instance, AppData& appData, AppConfig& appConfig);
  void resize(const RECT& displayRect);
  void setVisible(bool visible);
  void refresh();
  void commitPendingEdits();
  void refreshItemsForCurrentUnit();
  /** Selects, displays, and centers the specified map region. */
  void focusRegion(int x, int y, int z);
  void showZSelectionContextMenu(HWND ownerWindow, POINT screenPoint);
  bool handleNotify(const NMHDR* hdr);
  bool handleDrawItem(const DRAWITEMSTRUCT* drawItem);
  LRESULT getNotifyResult() const;
  bool handleCommand(int commandId, int notificationCode = 0);
  bool handleMouseMessage(UINT msg, WPARAM wp, LPARAM lp);

  /**
  * @brief Identifies which tab is the navigation target.
  *
  * -------------------------------------------------------------------------
  * HOW TO ADD A NEW NAVIGATION TARGET
  * -------------------------------------------------------------------------
  * Follow these steps in order. Example: adding a "Skills" target.
  *
  * 1. HERE — Add a new enumerator to NavigationTarget:
  *      Skills,
  *
  * 2. HERE — Add a new payload struct below BattleNavigationPayload, e.g.:
  *      struct SkillNavigationPayload
  *      {
  *        std::wstring skillToken;
  *      };
  *
  * 3. HERE — Extend the variant in NavigationRequest::payload with the new
  *    type and update its comment:
  *      std::variant<BattleNavigationPayload, SkillNavigationPayload> payload;
  *      NavigationTarget::Skills -> std::get<SkillNavigationPayload>(payload)
  *
  * 4. MapTabContent.cpp / onMapRightClick() — Add a context menu item and
  *    detect whether it should be enabled (grayed), then fire the callback:
  *      AppendMenuW(menu, MF_STRING, kRegionContextShowSkillCommandId, L"Show Skill");
  *      ...
  *      else if (selectedCommand == kRegionContextShowSkillCommandId && navigationCallback_)
  *      {
  *        navigationCallback_(NavigationRequest{
  *          NavigationTarget::Skills,
  *          SkillNavigationPayload{ someSkillToken }
  *        });
  *      }
  *    Also add the command ID constant in the anonymous namespace at the top
  *    of MapTabContent.cpp (e.g. constexpr UINT kRegionContextShowSkillCommandId = 4752;).
  *
  * 5. Target tab class (e.g. SkillsTabContent) — Add a focus method that
  *    accepts the payload data and selects the right item in its list:
  *      void focusSkillByToken(const std::wstring& skillToken);
  *
  * 6. MainWindowWin.cpp / setNavigationCallback lambda — Add a new case to the
  *    switch statement:
  *      case MapTabContent::NavigationTarget::Skills:
  *      {
  *        HWND tabCtrl = tabView_->getTabControl();
  *        if (tabCtrl && skillsTabIndex_ >= 0)
  *        {
  *          TabCtrl_SetCurSel(tabCtrl, skillsTabIndex_);
  *          tabView_->onSelectionChange();
  *          updateReportsTabVisibility();
  *        }
  *        if (skillsTabContent_)
  *        {
  *          const auto& p = std::get<MapTabContent::SkillNavigationPayload>(request.payload);
  *          skillsTabContent_->focusSkillByToken(p.skillToken);
  *        }
  *        break;
  *      }
  * -------------------------------------------------------------------------
  */
  enum class NavigationTarget
  {
    Battles,
    Skills,
    Items,
  };

  /**
  * @brief Payload for a Battles navigation request.
  * Carries the region coordinates and report period that were used to detect
  * the battle (the same values the map drawing uses for the red X marker).
  */
  struct BattleNavigationPayload
  {
    int x     { 0 };
    int y     { 0 };
    int z     { 0 };
    int month { 0 };
    int year  { 0 };
  };

  struct SkillNavigationPayload
  {
    std::wstring skillToken;
  };

  struct ItemNavigationPayload
  {
    std::wstring itemToken;
  };

  /**
  * @brief Unified navigation request passed to the navigation callback.
  *
  * The active alternative in payload matches the value of target:
  *   NavigationTarget::Battles -> std::get<BattleNavigationPayload>(payload)
  *   NavigationTarget::Skills  -> std::get<SkillNavigationPayload>(payload)
  *
  * Add a new payload struct and a new variant alternative here when a new
  * navigation target is introduced.
  */
  struct NavigationRequest
  {
    NavigationTarget                                           target;
    std::variant<BattleNavigationPayload, SkillNavigationPayload, ItemNavigationPayload> payload;
  };

  /**
  * @brief Sets the single callback invoked for all context-menu navigation
  *        actions. The callback receives a NavigationRequest whose target
  *        field identifies the destination and whose payload carries the
  *        data needed to focus the correct item there.
  *
  * Expected callback implementation (see MainWindow::handleMessage WM_CREATE):
  *   1. Switch on request.target.
  *   2. For NavigationTarget::Battles:
  *        a. TabCtrl_SetCurSel(tabCtrl, battlesTabIndex_) - switches the tab
  *           control selection to the Battles tab.
  *        b. WinTabView::onSelectionChange() - updates panel visibility inside
  *           WinTabView.
  *        c. MainWindow::updateReportsTabVisibility() - shows/hides all tab
  *           content controls to match the newly selected tab.
  *        d. BattlesTabContent::focusBattleByRegion(x, y, z, month, year) -
  *           selects the period and highlights the matching battle in the list.
  */
  void setNavigationCallback(std::function<void(const NavigationRequest&)> callback);

private:
  enum class DragMode
  {
    None,
    LeftRightSplit,
    DetailsMapSplit,
    TopBottomSplit,
  };

  struct RegionVisual
  {
    const Region* region { nullptr };
    std::array<POINT, 6> polygon {};
    POINT center { 0, 0 };
  };

  AppData* appData_ { nullptr };
  AppConfig* appConfig_ { nullptr };
  HWND mapCanvas_ { nullptr };
  HWND regionDateLabel_ { nullptr };
  HWND hoverRegionLabel_ { nullptr };
  HWND regionDetailsView_ { nullptr };
  HWND unitSearchLabel_ { nullptr };
  HWND unitSearchEdit_ { nullptr };
  HWND unitSearchButton_ { nullptr };
  HWND regionResourcesLabel_ { nullptr };
  HWND regionResourcesList_ { nullptr };
  HWND regionForSaleLabel_ { nullptr };
  HWND regionForSaleList_ { nullptr };
  HWND regionWantedLabel_ { nullptr };
  HWND regionWantedList_ { nullptr };
  HWND unitsList_ { nullptr };
  HWND lastWarningButton_ { nullptr };
  HWND clearWarningButton_ { nullptr };
  HWND nextWarningButton_ { nullptr };
  HWND warningsCountLabel_ { nullptr };
  HWND unitWeightLabel_ { nullptr };
  HWND unitCapacitiesLabel_ { nullptr };
  HWND unitItemsLabel_ { nullptr };
  HWND unitItemsList_ { nullptr };
  HWND unitSkillsLabel_ { nullptr };
  HWND unitSkillsList_ { nullptr };
  HWND unitDetailsTabs_ { nullptr };
  HWND unitErrorsList_ { nullptr };
  HWND unitWarningsList_ { nullptr };
  HWND unitEventsList_ { nullptr };
  HWND selectedUnitLabel_ { nullptr };
  HWND unitCoordinatesLabel_ { nullptr };
  HWND unitFlagsLabel_ { nullptr };
  HWND unitWarningLabel_ { nullptr };
  HWND ordersEditor_ { nullptr };
  HWND saveOrdersButton_ { nullptr };
  HWND checkOrdersButton_ { nullptr };
  RECT displayRect_ { 0, 0, 0, 0 };
  float leftPanelRatio_ { 0.75F };
  float detailsPanelRatio_ { 0.144F };
  float topPanelRatio_ { 0.75F };
  DragMode dragMode_ { DragMode::None };
  std::vector<int> availableZLevels_;
  int selectedZ_ { 1 };
  bool hasSelectedRegion_ { false };
  int selectedRegionX_ { 0 };
  int selectedRegionY_ { 0 };
  int selectedUnitNumber_ { 0 };
  bool selectedUnitIsNew_ { false };
  int scrollX_ { 0 };
  int scrollY_ { 0 };
  int contentWidth_ { 0 };
  int contentHeight_ { 0 };
  int capacityWalkDisplay_ { 0 };
  int capacityRideDisplay_ { 0 };
  int capacityFlyDisplay_ { 0 };
  int capacitySwimDisplay_ { 0 };
  int shipCapacityDisplay_ { 0 };
  int shipFreeCapacityDisplay_ { 0 };
  int shipSkillNeedDisplay_ { 0 };
  int shipOwnerSailingDisplay_ { 0 };
  bool showRideCapacity_ { false };
  bool showFlyCapacity_ { false };
  bool showSwimCapacity_ { false };
  bool isVisible_ { false };
  bool hasCapacityValues_ { false };
  bool hasShipCapacityValues_ { false };
  bool hasShipOwnerSkillValues_ { false };
  bool shipIsFlying_            { false };
  bool shipIsCapableOfFlying_   { false };
  LRESULT notifyResult_ { 0 };
  bool trackingMouseLeave_ { false };
  std::wstring hoverRegionText_;
  bool hasMapBounds_ { false };
  int mapMinX_ { 0 };
  int mapMaxX_ { 0 };
  int mapMinY_ { 0 };
  int mapMaxY_ { 0 };
  int mapLeftPaddingColumns_ { 0 };
  int mapRightPaddingColumns_ { 0 };
  std::vector<RegionVisual> visibleRegions_;
  std::vector<std::pair<int, int>> movePathCoordinates_; // (x, y) pairs for move path
  bool movePathIsSail_ { false };
  bool movePathHasNegativeCapacity_ { false };
  bool movePathSailRouteInvalid_ { false };
  int selectedUnitDetailsTab_ { 0 };
  int unitsListSortColumn_ { -1 };
  bool unitsListSortAscending_ { true };
  std::function<void(const NavigationRequest&)> navigationCallback_;

  static LRESULT CALLBACK mapCanvasWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  static LRESULT CALLBACK unitSearchEditSubclassProc(HWND hwnd,
                                                     UINT msg,
                                                     WPARAM wp,
                                                     LPARAM lp,
                                                     UINT_PTR subclassId,
                                                     DWORD_PTR refData);
  static LRESULT CALLBACK unitsListHeaderSubclassProc(HWND hwnd,
                                                      UINT msg,
                                                      WPARAM wp,
                                                      LPARAM lp,
                                                      UINT_PTR subclassId,
                                                      DWORD_PTR refData);
  static LRESULT CALLBACK ordersEditorSubclassProc(HWND hwnd,
                                                   UINT msg,
                                                   WPARAM wp,
                                                   LPARAM lp,
                                                   UINT_PTR subclassId,
                                                   DWORD_PTR refData);
  LRESULT handleMapCanvasMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  void recalculateVisibleMap();
  void updateMapScrollbars();
  void paintMap(HDC hdc) const;
  const RegionVisual* hitTestRegion(POINT pointInMapClient) const;
  bool hitTestMapCoordinate(POINT pointInMapClient, int& xCoordinate, int& yCoordinate) const;
  // buildHexagonPolygon moved to MapUtils::buildHexagonPolygon (Function/MapUtils.hpp)
  void populateUnitsForSelectedRegion();
  void populateItemsForSelectedUnit(const Unit* unit);
  void populateItemsForSelectedUnit(const UnitNew* unitNew);
  void populateSkillsList(const Unit* unit);
  void populateSkillsList(const UnitNew* unitNew);
  int populateErrorsList(const Unit* unit);
  int populateWarningsList(const Unit* unit);
  int populateWarningsList(const UnitNew* unitNew);
  int populateUnitEventsList(const Unit *unit);
  void updateUnitWeightAndCapacities(const Unit* unit);
  void updateUnitWeightAndCapacities(const UnitNew* unitNew);
  void updateUnitDetailsTabCaptions(int errorCount, int warningCount, int eventCount);
  void updateUnitDetailsTabVisibility();
  void clearUnitsList();
  void updateSelectedUnitFromList();
  void updateSelectedUnitDetailsByNumber(int unitNumber);
  void clearSelectedUnitDetails();
  void appendOrderLineToOrdersEditor(const std::wstring& orderLine);
  void navigateToSkillList(const std::wstring& skillToken);
  void navigateToItemList(const std::wstring& itemToken);
  void showSkillDescription(const std::wstring& skillToken);
  void showItemDescription(const std::wstring& itemToken);
  void saveOrdersToSelectedUnit();
  void runOrderChecksForMainFaction();
  void updateWarningsSummaryLabel();
  void selectUnitInMap(int unitNumber);
  bool focusOriginUnitForSelectedUnitNew();
  void searchAndSelectUnitById();
  void selectPreviousWarningUnit();
  void selectNextWarningUnit();
  void clearWarningsForSelectedUnit();
  bool canEditOrdersForUnit(const Unit* unit) const;
  void setOrdersEditingEnabled(bool enabled);
  void onMapLeftClick(POINT pointInMapClient);
  void onMapDoubleClick(POINT pointInMapClient);
  void onMapRightClick(POINT pointInMapClient);
  void showGiveToUnitDialog(HWND ownerWindow);
  void handleGiveDialogAccept(HWND dlgHwnd);
  void updateRegionDetailsView(const Region* region);
  void populateResourcesList(const Region* region);
  void populateForSaleList(const Region* region);
  void populateWantedList(const Region* region);
  void updateHoverTooltip(POINT pointInMapClient, const Region& region);
  void hideHoverTooltip();
  COLORREF getRegionFillColor(const std::wstring& regionType) const;
  RECT getLeftRightSplitterRect() const;
  RECT getDetailsMapSplitterRect() const;
  RECT getTopBottomSplitterRect() const;
  UiSizeProfile::Profile resolveRequestedUiProfile() const;
  UiSizeProfile::MapHexProfile resolveRequestedMapHexProfile() const;
  UiSizeProfile::Metrics resolveUiMetrics() const;
  int resolveScaledMapHexWidth() const;
  void applyListColumnWidths(const UiSizeProfile::Metrics& metrics,
                             int leftPanelWidth,
                             int rightPanelWidth,
                             int detailsWidth);
  void sortUnitsListByColumn(int columnIndex, bool ascending);
  void updateUnitsListSortHeaderMarkers();
  bool handleUnitsListHeaderDoubleClickNotify(int forcedColumnIndex = -1);
  bool handleUnitSkillsContextMenuNotify();
  bool handleUnitWarningsContextMenuNotify();
  bool handleRegionItemsContextMenuNotify(HWND sourceList);
};
