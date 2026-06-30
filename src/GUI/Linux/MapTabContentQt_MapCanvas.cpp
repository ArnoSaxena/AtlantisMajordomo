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
 * File: MapTabContentQt_MapCanvas.cpp
 *
 * Step 7.9.1 - MapCanvasWidget skeleton + hit-test + coordinate math.
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "GUI/MapCanvasWidget.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/StructInfo.hpp"
#include "Data/Structure.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Function/HexDirectionUtils.hpp"
#include "Function/MapUtils.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <set>

namespace
{
constexpr int kMargin = 8;

QPolygon toQPolygon(const std::array<POINT, 6>& points)
{
    QPolygon polygon;
    polygon.reserve(static_cast<int>(points.size()));
    for (const POINT& point : points)
    {
        polygon << QPoint(static_cast<int>(point.x), static_cast<int>(point.y));
    }
    return polygon;
}

QPoint getRoadEndpointForDirection(const QPolygon& polygon, const std::wstring& direction)
{
    auto midpoint = [&polygon](int firstIndex, int secondIndex) -> QPoint
    {
        return QPoint((polygon[firstIndex].x() + polygon[secondIndex].x()) / 2,
                      (polygon[firstIndex].y() + polygon[secondIndex].y()) / 2);
    };

    if (direction == L"N")
    {
        return midpoint(1, 2);
    }
    if (direction == L"NE")
    {
        return midpoint(2, 3);
    }
    if (direction == L"SE")
    {
        return midpoint(3, 4);
    }
    if (direction == L"S")
    {
        return midpoint(4, 5);
    }
    if (direction == L"SW")
    {
        return midpoint(5, 0);
    }

    return midpoint(0, 1);
}

} // namespace

MapCanvasWidget::MapCanvasWidget(AppData& appData, AppConfig& appConfig, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
    , appConfig_(&appConfig)
    , horizontalScrollBar_(new QScrollBar(Qt::Horizontal, this))
    , verticalScrollBar_(new QScrollBar(Qt::Vertical, this))
{
    setMouseTracking(true);

    horizontalScrollBar_->setSingleStep(20);
    verticalScrollBar_->setSingleStep(20);

    connect(horizontalScrollBar_, &QScrollBar::valueChanged, this, [this](int value)
    {
        scrollX_ = value;
        update();
    });
    connect(verticalScrollBar_, &QScrollBar::valueChanged, this, [this](int value)
    {
        scrollY_ = value;
        update();
    });

    recalculateVisibleMap();
    hideHoverTooltip();
}

void MapCanvasWidget::setSelectedZ(int selectedZ)
{
    if (selectedZ_ == selectedZ)
    {
        return;
    }

    selectedZ_ = selectedZ;
    recalculateVisibleMap();
    update();
}

void MapCanvasWidget::setSelectedRegion(bool hasSelectedRegion, int selectedRegionX, int selectedRegionY)
{
    hasSelectedRegion_ = hasSelectedRegion;
    selectedRegionX_ = selectedRegionX;
    selectedRegionY_ = selectedRegionY;
}

void MapCanvasWidget::recalculateVisibleMap()
{
    visibleRegions_.clear();
    availableZLevels_.clear();
    hasMapBounds_ = false;

    if (!appData_ || !appConfig_)
    {
        updateScrollbars();
        return;
    }

    const auto& regionRepository = appData_->regionRepository();
    if (regionRepository.size() == 0)
    {
        contentWidth_ = 0;
        contentHeight_ = 0;
        hasSelectedRegion_ = false;
        updateScrollbars();
        return;
    }

    std::set<int> zSet;
    for (std::size_t index = 0; index < regionRepository.size(); ++index)
    {
        zSet.insert(regionRepository.at(index).getZCoordinate());
    }

    availableZLevels_.assign(zSet.begin(), zSet.end());
    if (availableZLevels_.empty())
    {
        selectedZ_ = 1;
    }
    else if (std::find(availableZLevels_.begin(), availableZLevels_.end(), selectedZ_) == availableZLevels_.end())
    {
        selectedZ_ = availableZLevels_.front();
    }

    std::vector<const Region*> zRegions;
    zRegions.reserve(regionRepository.size());

    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    bool firstRegion = true;

    for (std::size_t index = 0; index < regionRepository.size(); ++index)
    {
        const Region& region = regionRepository.at(index);
        if (region.getZCoordinate() != selectedZ_)
        {
            continue;
        }

        zRegions.push_back(&region);
        if (firstRegion)
        {
            minX = maxX = region.getXCoordinate();
            minY = maxY = region.getYCoordinate();
            firstRegion = false;
        }
        else
        {
            minX = (std::min)(minX, region.getXCoordinate());
            maxX = (std::max)(maxX, region.getXCoordinate());
            minY = (std::min)(minY, region.getYCoordinate());
            maxY = (std::max)(maxY, region.getYCoordinate());
        }
    }

    if (zRegions.empty())
    {
        contentWidth_ = 0;
        contentHeight_ = 0;
        hasSelectedRegion_ = false;
        updateScrollbars();
        return;
    }

    bool leftRolloverDiscovered = false;
    bool rightRolloverDiscovered = false;
    for (const Region* region : zRegions)
    {
        if (!region)
        {
            continue;
        }

        for (const auto& direction : region->getExitDirections())
        {
            if (HexDirectionUtils::isWestDirection(direction) && region->getXCoordinate() <= minX + 1)
            {
                for (const Region* candidate : zRegions)
                {
                    if (!candidate)
                    {
                        continue;
                    }
                    if (candidate->getXCoordinate() >= maxX - 1 &&
                        std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
                    {
                        leftRolloverDiscovered = true;
                        break;
                    }
                }
            }

            if (HexDirectionUtils::isEastDirection(direction) && region->getXCoordinate() >= maxX - 1)
            {
                for (const Region* candidate : zRegions)
                {
                    if (!candidate)
                    {
                        continue;
                    }
                    if (candidate->getXCoordinate() <= minX + 1 &&
                        std::abs(candidate->getYCoordinate() - region->getYCoordinate()) <= 2)
                    {
                        rightRolloverDiscovered = true;
                        break;
                    }
                }
            }
        }
    }

    const int leftPaddingColumns = leftRolloverDiscovered ? 0 : 3;
    const int rightPaddingColumns = rightRolloverDiscovered ? 0 : 3;

    mapMinX_ = minX;
    mapMaxX_ = maxX;
    mapMinY_ = minY;
    mapMaxY_ = maxY;
    mapLeftPaddingColumns_ = leftPaddingColumns;
    mapRightPaddingColumns_ = rightPaddingColumns;
    hasMapBounds_ = true;

    const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
    const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
    const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
    const int rowStep = hexHeight;

    int maxCenterY = 0;
    for (const Region* region : zRegions)
    {
        if (!region)
        {
            continue;
        }

        const int mapColumn = (region->getXCoordinate() - minX) + leftPaddingColumns;
        const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

        const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
        const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

        RegionVisual visual;
        visual.region = region;
        visual.center = QPoint(centerX, centerY);
        visual.polygon = toQPolygon(MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth));
        visibleRegions_.push_back(visual);
        maxCenterY = (std::max)(maxCenterY, centerY);
    }

    if (mapLeftPaddingColumns_ > 0)
    {
        for (const Region* region : zRegions)
        {
            if (!region)
            {
                continue;
            }

            const int wrappedLeftX = region->getXCoordinate() - (maxX - minX + 1);
            if (wrappedLeftX < (minX - mapLeftPaddingColumns_) || wrappedLeftX >= minX)
            {
                continue;
            }

            const int mapColumn = (wrappedLeftX - minX) + leftPaddingColumns;
            const double mapRow = static_cast<double>(region->getYCoordinate() - minY) / 2.0;

            const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
            const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

            RegionVisual wrappedVisual;
            wrappedVisual.region = region;
            wrappedVisual.center = QPoint(centerX, centerY);
            wrappedVisual.polygon = toQPolygon(MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth));
            visibleRegions_.push_back(wrappedVisual);
            maxCenterY = (std::max)(maxCenterY, centerY);
        }
    }

    const int totalColumns = (maxX - minX + 1) + leftPaddingColumns + rightPaddingColumns;
    contentWidth_ = kMargin * 2 + (std::max)(1, totalColumns) * columnStep + hexWidth;
    contentHeight_ = kMargin * 2 + maxCenterY + hexHeight;

    if (hasSelectedRegion_)
    {
        const bool selectedStillVisible = std::any_of(
            visibleRegions_.begin(),
            visibleRegions_.end(),
            [this](const RegionVisual& visual)
            {
                return visual.region != nullptr &&
                       visual.region->getXCoordinate() == selectedRegionX_ &&
                       visual.region->getYCoordinate() == selectedRegionY_;
            });

        if (!selectedStillVisible)
        {
            hasSelectedRegion_ = false;
        }
    }

    updateScrollbars();
}

bool MapCanvasWidget::centerOnRegion(int regionX, int regionY)
{
    const RegionVisual* targetRegion = nullptr;
    for (const RegionVisual& visual : visibleRegions_)
    {
        if (visual.region &&
            visual.region->getXCoordinate() == regionX &&
            visual.region->getYCoordinate() == regionY)
        {
            targetRegion = &visual;
            break;
        }
    }

    if (!targetRegion)
    {
        return false;
    }

    const QRect viewport = viewportRect();
    if (viewport.isEmpty())
    {
        return false;
    }

    const int maxScrollX = (std::max)(0, contentWidth_ - viewport.width());
    const int maxScrollY = (std::max)(0, contentHeight_ - viewport.height());
    const int targetX = targetRegion->center.x() - (viewport.width() / 2);
    const int targetY = targetRegion->center.y() - (viewport.height() / 2);

    const int clampedX = std::clamp(targetX, 0, maxScrollX);
    const int clampedY = std::clamp(targetY, 0, maxScrollY);
    horizontalScrollBar_->setValue(clampedX);
    verticalScrollBar_->setValue(clampedY);
    return true;
}

void MapCanvasWidget::setMovePathOverlay(const std::vector<std::pair<int, int>>& coordinates,
                                         bool isSail,
                                         bool hasNegativeCapacity,
                                         bool sailRouteInvalid)
{
    movePathCoordinates_ = coordinates;
    movePathIsSail_ = isSail;
    movePathHasNegativeCapacity_ = hasNegativeCapacity;
    movePathSailRouteInvalid_ = sailRouteInvalid;
    update();
}

void MapCanvasWidget::clearMovePathOverlay()
{
    movePathCoordinates_.clear();
    movePathIsSail_ = false;
    movePathHasNegativeCapacity_ = false;
    movePathSailRouteInvalid_ = false;
    update();
}

void MapCanvasWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    const QRect viewport = viewportRect();
    if (viewport.isEmpty())
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipRect(viewport);
    painter.fillRect(viewport, palette().color(QPalette::Base));

    if (appConfig_ && hasMapBounds_)
    {
        std::set<std::pair<int, int>> occupiedCoordinates;
        int coordinateParity = 0;
        bool hasCoordinateParity = false;
        for (const RegionVisual& visual : visibleRegions_)
        {
            if (!visual.region)
            {
                continue;
            }

            const int regionX = visual.region->getXCoordinate();
            const int regionY = visual.region->getYCoordinate();
            occupiedCoordinates.emplace(regionX, regionY);
            if (!hasCoordinateParity)
            {
                coordinateParity = (regionX + regionY) & 1;
                hasCoordinateParity = true;
            }
        }

        const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
        const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
        const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
        const int rowStep = hexHeight;

        painter.setPen(QPen(QColor(192, 192, 192), 1));
        painter.setBrush(Qt::NoBrush);

        for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
        {
            for (int y = mapMinY_; y <= mapMaxY_; ++y)
            {
                if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
                {
                    continue;
                }

                if (occupiedCoordinates.find({ x, y }) != occupiedCoordinates.end())
                {
                    continue;
                }

                const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
                const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
                const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2) - scrollX_;
                const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2) - scrollY_;

                QPolygon emptyPolygon = toQPolygon(MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth));
                if (!emptyPolygon.boundingRect().intersects(viewport))
                {
                    continue;
                }

                painter.drawPolygon(emptyPolygon);
            }
        }
    }

    int latestBattlePeriodMonth = 0;
    int latestBattlePeriodYear = 0;
    if (appData_)
    {
        const auto& reportRepository = appData_->reportRepository();
        for (std::size_t i = 0; i < reportRepository.size(); ++i)
        {
            const Report& report = reportRepository.at(i);
            const int reportMonth = report.getMonth();
            const int reportYear = report.getYear();
            if (reportMonth >= 1 && reportMonth <= 12 && reportYear > 0)
            {
                if (reportYear > latestBattlePeriodYear ||
                    (reportYear == latestBattlePeriodYear && reportMonth > latestBattlePeriodMonth))
                {
                    latestBattlePeriodMonth = reportMonth;
                    latestBattlePeriodYear = reportYear;
                }
            }
        }
    }
    const bool hasLatestBattlePeriod =
        latestBattlePeriodMonth >= 1 && latestBattlePeriodMonth <= 12 && latestBattlePeriodYear > 0;

    painter.setPen(QPen(QColor(0, 0, 0), 2));

    for (const RegionVisual& visual : visibleRegions_)
    {
        if (!visual.region)
        {
            continue;
        }

        QPolygon translated = visual.polygon.translated(-scrollX_, -scrollY_);
        if (!translated.boundingRect().intersects(viewport))
        {
            continue;
        }

        painter.setBrush(QBrush(getRegionFillColor(visual.region->getRegionType())));
        painter.drawPolygon(translated);

        if (appConfig_ && !visual.region->getPeasantType().empty() && visual.region->getPeasantNumber() > 0)
        {
            const std::array<int, 3> peasantRgb = appConfig_->getPeasantColour(visual.region->getPeasantType());
            const QColor peasantColor(
                std::clamp(peasantRgb[0], 0, 255),
                std::clamp(peasantRgb[1], 0, 255),
                std::clamp(peasantRgb[2], 0, 255));

            const int centerX = visual.center.x() - scrollX_;
            const int centerY = visual.center.y() - scrollY_;
            const QPoint neCorner = translated[2];
            const QPoint eCorner = translated[3];

            const double dxNe = static_cast<double>(neCorner.x() - centerX);
            const double dyNe = static_cast<double>(neCorner.y() - centerY);
            const double radialDistance = std::sqrt(dxNe * dxNe + dyNe * dyNe);
            const double fraction = (radialDistance > 0.0)
                ? std::min(1.0, std::max(0.20, 10.0 / radialDistance))
                : 0.20;

            auto lerp = [](int from, int to, double factor) -> int
            {
                return static_cast<int>(std::lround(from + factor * static_cast<double>(to - from)));
            };

            QPolygon wedge;
            wedge << neCorner
                  << eCorner
                  << QPoint(lerp(centerX, eCorner.x(), 1.0 - fraction),
                            lerp(centerY, eCorner.y(), 1.0 - fraction))
                  << QPoint(lerp(centerX, neCorner.x(), 1.0 - fraction),
                            lerp(centerY, neCorner.y(), 1.0 - fraction));

            painter.setPen(QPen(QColor(0, 0, 0), 1));
            painter.setBrush(QBrush(peasantColor));
            painter.drawPolygon(wedge);
            painter.setPen(QPen(QColor(0, 0, 0), 2));
        }

        if (appData_ && appConfig_)
        {
            std::set<std::wstring> availableExitDirections;
            for (const auto& exitDirection : visual.region->getExitDirections())
            {
                const std::wstring normalizedDirection = HexDirectionUtils::normalizeHexDirection(exitDirection);
                if (!normalizedDirection.empty())
                {
                    availableExitDirections.insert(normalizedDirection);
                }
            }

            if (!availableExitDirections.empty())
            {
                std::set<std::wstring> roadDirectionsToDraw;
                const auto& structureRepository = appData_->structureRepository();
                for (std::size_t structureIndex = 0; structureIndex < structureRepository.size(); ++structureIndex)
                {
                    const Structure& structure = structureRepository.at(structureIndex);
                    if (structure.getXCoordinate() != visual.region->getXCoordinate() ||
                        structure.getYCoordinate() != visual.region->getYCoordinate() ||
                        structure.getZCoordinate() != selectedZ_)
                    {
                        continue;
                    }

                    const std::wstring roadDirection = HexDirectionUtils::extractRoadDirectionFromStructure(structure);
                    if (roadDirection.empty() || availableExitDirections.find(roadDirection) == availableExitDirections.end())
                    {
                        continue;
                    }

                    roadDirectionsToDraw.insert(roadDirection);
                }

                if (!roadDirectionsToDraw.empty())
                {
                    const std::array<int, 3> roadColor = appConfig_->getRoadColor();
                    painter.setPen(QPen(QColor(std::clamp(roadColor[0], 0, 255),
                                               std::clamp(roadColor[1], 0, 255),
                                               std::clamp(roadColor[2], 0, 255)),
                                        4));
                    painter.setBrush(Qt::NoBrush);

                    const int centerX = visual.center.x() - scrollX_;
                    const int centerY = visual.center.y() - scrollY_;
                    for (const auto& direction : roadDirectionsToDraw)
                    {
                        const QPoint endpoint = getRoadEndpointForDirection(visual.polygon, direction);
                        painter.drawLine(QPoint(centerX, centerY),
                                         QPoint(endpoint.x() - scrollX_, endpoint.y() - scrollY_));
                    }

                    painter.setPen(QPen(QColor(0, 0, 0), 2));
                }
            }
        }

        if (visual.region->getContainsSettlement() && appConfig_)
        {
            const std::wstring settlementType = StringUtils::toLower(visual.region->getSettlementType());
            const int markerDiameter = (std::max)(4, (std::max)(12, appConfig_->getMapHexWidth()) / 4);
            const int coreDiameter = (std::max)(2, markerDiameter / 4);
            const int townRingDiameter = markerDiameter;
            const int cityInnerRingDiameter = (std::max)(coreDiameter + 3, markerDiameter * 2 / 3);
            const int cityOuterRingDiameter = markerDiameter;
            const int centerX = visual.center.x() - scrollX_;
            const int centerY = visual.center.y() - scrollY_;

            painter.setPen(QPen(QColor(0, 0, 0), 2));
            painter.setBrush(QBrush(QColor(0, 0, 0)));

            if (settlementType == L"village")
            {
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(QPoint(centerX, centerY), markerDiameter / 2, markerDiameter / 2);
                painter.setBrush(QBrush(QColor(0, 0, 0)));
            }
            else if (settlementType == L"town")
            {
                painter.drawEllipse(QPoint(centerX, centerY), coreDiameter / 2, coreDiameter / 2);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(QPoint(centerX, centerY), townRingDiameter / 2, townRingDiameter / 2);
            }
            else if (settlementType == L"city")
            {
                painter.drawEllipse(QPoint(centerX, centerY), coreDiameter / 2, coreDiameter / 2);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(QPoint(centerX, centerY), cityInnerRingDiameter / 2, cityInnerRingDiameter / 2);
                painter.drawEllipse(QPoint(centerX, centerY), cityOuterRingDiameter / 2, cityOuterRingDiameter / 2);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(QPoint(centerX, centerY), markerDiameter / 2, markerDiameter / 2);
                painter.setBrush(QBrush(QColor(0, 0, 0)));
            }
        }

        if (hasLatestBattlePeriod && appData_)
        {
            const bool hasBattle = appData_->battleRepository().hasBattleInRegionForPeriod(
                visual.region->getXCoordinate(),
                visual.region->getYCoordinate(),
                visual.region->getZCoordinate(),
                latestBattlePeriodMonth,
                latestBattlePeriodYear);

            if (hasBattle)
            {
                const int centerX = visual.center.x() - scrollX_;
                const int centerY = visual.center.y() - scrollY_;

                const int hexWidth = (std::max)(12, appConfig_ ? appConfig_->getMapHexWidth() : 12);
                const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
                const int crossSize = (std::max)(4, (((hexHeight * 2) / 5) * 2) / 3);
                const int halfCross = crossSize / 2;
                const int bottomMargin = (std::max)(4, hexHeight / 8);
                const int crossCenterY = centerY + (hexHeight / 2) - bottomMargin - halfCross;

                painter.setPen(QPen(QColor(200, 0, 0), 3));
                painter.drawLine(QPoint(centerX - halfCross, crossCenterY - halfCross),
                                 QPoint(centerX + halfCross, crossCenterY + halfCross));
                painter.drawLine(QPoint(centerX - halfCross, crossCenterY + halfCross),
                                 QPoint(centerX + halfCross, crossCenterY - halfCross));
                painter.setPen(QPen(QColor(0, 0, 0), 2));
            }
        }
    }

    if (appData_ && appConfig_)
    {
        const std::array<int, 3> markerColor = appConfig_->getStructureMarkerColor();
        const QColor defaultMarkerColor(std::clamp(markerColor[0], 0, 255),
                                        std::clamp(markerColor[1], 0, 255),
                                        std::clamp(markerColor[2], 0, 255));
        const QColor caravanseraiMarkerColor(255, 165, 0);
        const QColor shipMarkerColor(173, 216, 230);

        for (const RegionVisual& visual : visibleRegions_)
        {
            if (!visual.region)
            {
                continue;
            }

            QPolygon translated = visual.polygon.translated(-scrollX_, -scrollY_);
            if (!translated.boundingRect().intersects(viewport))
            {
                continue;
            }

            const auto structuresInRegion = appData_->structureRepository().findByCoordinates(
                visual.region->getXCoordinate(),
                visual.region->getYCoordinate(),
                visual.region->getZCoordinate());
            if (structuresInRegion.empty())
            {
                continue;
            }

            bool hasNonRoadNonShipStructure = false;
            bool hasShipStructure = false;
            bool hasFlyingShipStructure = false;
            for (const Structure* structure : structuresInRegion)
            {
                if (!structure)
                {
                    continue;
                }

                const bool isRoadStructure = !HexDirectionUtils::extractRoadDirectionFromStructure(*structure).empty();
                const StructInfo* structInfo = appData_->structInfoRepository().findByType(structure->getStructureType());
                const bool isShipStructure = structInfo && structInfo->isShip();

                if (isShipStructure)
                {
                    hasShipStructure = true;
                    if (structure->isFlying())
                    {
                        hasFlyingShipStructure = true;
                    }
                }

                if (!isRoadStructure && !isShipStructure)
                {
                    hasNonRoadNonShipStructure = true;
                }
            }

            const bool hasCaravanserai = !appData_->structureRepository().findByCoordinatesAndType(
                visual.region->getXCoordinate(),
                visual.region->getYCoordinate(),
                visual.region->getZCoordinate(),
                L"caravanserai").empty();

            const bool hasShaft = !appData_->structureRepository().findByCoordinatesAndType(
                visual.region->getXCoordinate(),
                visual.region->getYCoordinate(),
                visual.region->getZCoordinate(),
                L"shaft").empty();

            if (!hasNonRoadNonShipStructure && !hasShipStructure)
            {
                continue;
            }

            const QPoint northEndpoint = getRoadEndpointForDirection(visual.polygon, L"N");
            const QPoint northWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"NW");
            const QPoint borderMidpoint((northEndpoint.x() + northWestEndpoint.x()) / 2,
                                        (northEndpoint.y() + northWestEndpoint.y()) / 2);
            const QPoint southEndpoint = getRoadEndpointForDirection(visual.polygon, L"S");
            const QPoint southWestEndpoint = getRoadEndpointForDirection(visual.polygon, L"SW");
            const QPoint borderMidpointBottomLeft((southEndpoint.x() + southWestEndpoint.x()) / 2,
                                                  (southEndpoint.y() + southWestEndpoint.y()) / 2);

            const QPoint centerPoint = visual.center;
            const int markerCenterX = borderMidpoint.x() + (centerPoint.x() - borderMidpoint.x()) / 4 - scrollX_;
            const int markerCenterY = borderMidpoint.y() + (centerPoint.y() - borderMidpoint.y()) / 4 - scrollY_;
            const int shipMarkerCenterX = borderMidpointBottomLeft.x() + (centerPoint.x() - borderMidpointBottomLeft.x()) / 4 - scrollX_;
            const int shipMarkerCenterY = borderMidpointBottomLeft.y() + (centerPoint.y() - borderMidpointBottomLeft.y()) / 4 - scrollY_;
            const int markerRadius = 2;

            if (hasNonRoadNonShipStructure)
            {
                const QColor topMarkerColor = hasCaravanserai ? caravanseraiMarkerColor : defaultMarkerColor;
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(topMarkerColor));
                painter.drawEllipse(QPoint(markerCenterX, markerCenterY), markerRadius, markerRadius);

                if (hasShaft)
                {
                    painter.setPen(QPen(QColor(50, 50, 50), 1));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawEllipse(QPoint(markerCenterX, markerCenterY), markerRadius, markerRadius);
                }
            }

            if (hasShipStructure)
            {
                painter.setPen(hasFlyingShipStructure ? QPen(QColor(0, 0, 0), 1) : Qt::NoPen);
                painter.setBrush(QBrush(shipMarkerColor));
                painter.drawEllipse(QPoint(shipMarkerCenterX, shipMarkerCenterY), markerRadius, markerRadius);
            }
        }
    }

    if (hasSelectedRegion_ && appConfig_)
    {
        const std::array<int, 3> selectedColor = appConfig_->getSelectedRegionBorderColor();
        painter.setPen(QPen(QColor(std::clamp(selectedColor[0], 0, 255),
                                   std::clamp(selectedColor[1], 0, 255),
                                   std::clamp(selectedColor[2], 0, 255)),
                            3));
        painter.setBrush(Qt::NoBrush);

        for (const RegionVisual& visual : visibleRegions_)
        {
            if (!visual.region ||
                visual.region->getXCoordinate() != selectedRegionX_ ||
                visual.region->getYCoordinate() != selectedRegionY_)
            {
                continue;
            }

            const QPolygon translated = visual.polygon.translated(-scrollX_, -scrollY_);
            painter.drawPolygon(translated);
            break;
        }
    }

    if (movePathCoordinates_.size() > 1)
    {
        const QColor defaultArrowColor = movePathIsSail_ ? QColor(173, 216, 230) : QColor(144, 238, 144);
        const QColor arrowColor = movePathHasNegativeCapacity_ ? QColor(255, 0, 0) : defaultArrowColor;
        const QColor arrowBorderColor(0, 0, 0);

        for (std::size_t i = 0; i + 1 < movePathCoordinates_.size(); ++i)
        {
            const int x1 = movePathCoordinates_[i].first;
            const int y1 = movePathCoordinates_[i].second;
            const int x2 = movePathCoordinates_[i + 1].first;
            const int y2 = movePathCoordinates_[i + 1].second;

            QPoint startCenter;
            QPoint endCenter;
            bool startFound = false;
            bool endFound = false;
            for (const RegionVisual& visual : visibleRegions_)
            {
                if (visual.region && visual.region->getXCoordinate() == x1 && visual.region->getYCoordinate() == y1)
                {
                    startCenter = visual.center;
                    startFound = true;
                }
                if (visual.region && visual.region->getXCoordinate() == x2 && visual.region->getYCoordinate() == y2)
                {
                    endCenter = visual.center;
                    endFound = true;
                }
                if (startFound && endFound)
                {
                    break;
                }
            }

            if (!startFound || !endFound)
            {
                continue;
            }

            startCenter -= QPoint(scrollX_, scrollY_);
            endCenter -= QPoint(scrollX_, scrollY_);

            const double dx = static_cast<double>(endCenter.x() - startCenter.x());
            const double dy = static_cast<double>(endCenter.y() - startCenter.y());
            const double length = std::sqrt(dx * dx + dy * dy);

            int adjustedStartX = startCenter.x();
            int adjustedStartY = startCenter.y();
            int adjustedEndX = endCenter.x();
            int adjustedEndY = endCenter.y();

            if (length > 0.0)
            {
                const double ux = dx / length;
                const double uy = dy / length;
                const double shorteningAmount = (std::max)(1.0, length * 0.25);
                adjustedStartX = static_cast<int>(std::lround(startCenter.x() + ux * shorteningAmount));
                adjustedStartY = static_cast<int>(std::lround(startCenter.y() + uy * shorteningAmount));
                adjustedEndX = static_cast<int>(std::lround(endCenter.x() - ux * shorteningAmount));
                adjustedEndY = static_cast<int>(std::lround(endCenter.y() - uy * shorteningAmount));
            }

            int shaftEndX = adjustedEndX;
            int shaftEndY = adjustedEndY;
            if (length > 0.0)
            {
                const double ux = dx / length;
                const double uy = dy / length;
                const double arrowLength = 10.0;
                shaftEndX = static_cast<int>(std::lround(adjustedEndX - ux * arrowLength));
                shaftEndY = static_cast<int>(std::lround(adjustedEndY - uy * arrowLength));
            }

            painter.setPen(QPen(arrowBorderColor, 8));
            painter.setBrush(Qt::NoBrush);
            painter.drawLine(QPoint(adjustedStartX, adjustedStartY), QPoint(shaftEndX, shaftEndY));

            painter.setPen(QPen(arrowColor, 4));
            painter.drawLine(QPoint(adjustedStartX, adjustedStartY), QPoint(shaftEndX, shaftEndY));

            if (length > 0.0)
            {
                const double ux = dx / length;
                const double uy = dy / length;
                const double arrowLength = 10.0;
                const double arrowWidth = 6.0;

                const QPoint arrowTip(adjustedEndX, adjustedEndY);
                const QPoint arrowBase1(
                    static_cast<int>(std::lround(adjustedEndX - ux * arrowLength + uy * arrowWidth)),
                    static_cast<int>(std::lround(adjustedEndY - uy * arrowLength - ux * arrowWidth)));
                const QPoint arrowBase2(
                    static_cast<int>(std::lround(adjustedEndX - ux * arrowLength - uy * arrowWidth)),
                    static_cast<int>(std::lround(adjustedEndY - uy * arrowLength + ux * arrowWidth)));

                QPolygon arrowHead;
                arrowHead << arrowTip << arrowBase1 << arrowBase2;
                painter.setPen(QPen(arrowBorderColor, 2));
                painter.setBrush(QBrush(arrowColor));
                painter.drawPolygon(arrowHead);
            }
        }
    }
}

const MapCanvasWidget::RegionVisual* MapCanvasWidget::hitTestRegion(const QPoint& pointInMapClient) const
{
    const QPoint mapPoint(pointInMapClient.x() + scrollX_, pointInMapClient.y() + scrollY_);

    for (const RegionVisual& visual : visibleRegions_)
    {
        if (visual.polygon.containsPoint(mapPoint, Qt::WindingFill))
        {
            return &visual;
        }
    }

    return nullptr;
}

bool MapCanvasWidget::hitTestMapCoordinate(const QPoint& pointInMapClient, int& xCoordinate, int& yCoordinate) const
{
    if (!hasMapBounds_ || !appConfig_)
    {
        return false;
    }

    const QPoint mapPoint(pointInMapClient.x() + scrollX_, pointInMapClient.y() + scrollY_);

    int coordinateParity = 0;
    bool hasCoordinateParity = false;
    for (const RegionVisual& visual : visibleRegions_)
    {
        if (!visual.region)
        {
            continue;
        }

        coordinateParity = (visual.region->getXCoordinate() + visual.region->getYCoordinate()) & 1;
        hasCoordinateParity = true;
        break;
    }

    const int hexWidth = (std::max)(12, appConfig_->getMapHexWidth());
    const int hexHeight = (std::max)(14, static_cast<int>(std::lround(static_cast<double>(hexWidth) * std::sqrt(3.0) / 2.0)));
    const int columnStep = (std::max)(10, static_cast<int>(std::lround(hexWidth * 0.75)));
    const int rowStep = hexHeight;

    for (int x = mapMinX_ - mapLeftPaddingColumns_; x <= mapMaxX_ + mapRightPaddingColumns_; ++x)
    {
        for (int y = mapMinY_; y <= mapMaxY_; ++y)
        {
            if (hasCoordinateParity && (((x + y) & 1) != coordinateParity))
            {
                continue;
            }

            const int mapColumn = (x - mapMinX_) + mapLeftPaddingColumns_;
            const double mapRow = static_cast<double>(y - mapMinY_) / 2.0;
            const int centerX = kMargin + mapColumn * columnStep + (hexWidth / 2);
            const int centerY = kMargin + static_cast<int>(std::lround(mapRow * rowStep)) + (hexHeight / 2);

            const QPolygon polygon = toQPolygon(MapUtils::buildHexagonPolygon(centerX, centerY, hexWidth));
            if (polygon.containsPoint(mapPoint, Qt::WindingFill))
            {
                xCoordinate = HexDirectionUtils::wrapMapXCoordinate(x, mapMinX_, mapMaxX_);
                yCoordinate = y;
                return true;
            }
        }
    }

    return false;
}

QColor MapCanvasWidget::getRegionFillColor(const std::wstring& regionType) const
{
    if (!appConfig_)
    {
        return QColor(192, 192, 192);
    }

    const std::array<int, 3> rgb = appConfig_->getRegionColor(regionType);
    const int red = std::clamp(rgb[0], 0, 255);
    const int green = std::clamp(rgb[1], 0, 255);
    const int blue = std::clamp(rgb[2], 0, 255);
    return QColor(red, green, blue);
}

void MapCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if (!event)
    {
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        onMapLeftClick(event->position().toPoint());
    }
    else if (event->button() == Qt::RightButton)
    {
        onMapRightClick(event->position().toPoint(), event->globalPosition().toPoint());
    }

    QWidget::mousePressEvent(event);
}

void MapCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!event || event->button() != Qt::LeftButton)
    {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    onMapDoubleClick(event->position().toPoint());

    QWidget::mouseDoubleClickEvent(event);
}

void MapCanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event)
    {
        updateHoverTooltip(event->position().toPoint());
    }

    QWidget::mouseMoveEvent(event);
}

void MapCanvasWidget::leaveEvent(QEvent* event)
{
    hideHoverTooltip();
    QWidget::leaveEvent(event);
}

void MapCanvasWidget::resizeEvent(QResizeEvent* event)
{
    updateScrollbars();
    QWidget::resizeEvent(event);
}

void MapCanvasWidget::wheelEvent(QWheelEvent* event)
{
    if (!event)
    {
        return;
    }

    QScrollBar* targetScrollBar = nullptr;
    if (event->modifiers() & Qt::ShiftModifier)
    {
        targetScrollBar = horizontalScrollBar_->isVisible() ? horizontalScrollBar_ : verticalScrollBar_;
    }
    else
    {
        targetScrollBar = verticalScrollBar_->isVisible() ? verticalScrollBar_ : horizontalScrollBar_;
    }

    if (!targetScrollBar || !targetScrollBar->isVisible())
    {
        QWidget::wheelEvent(event);
        return;
    }

    int delta = event->angleDelta().y();
    if (delta == 0)
    {
        delta = event->angleDelta().x();
    }

    if (delta != 0)
    {
        const int step = (std::max)(1, targetScrollBar->singleStep());
        const int wheelSteps = delta / 120;
        const int effectiveSteps = (wheelSteps != 0) ? wheelSteps : ((delta > 0) ? 1 : -1);
        targetScrollBar->setValue(targetScrollBar->value() - (effectiveSteps * step));
        event->accept();
        return;
    }

    QWidget::wheelEvent(event);
}

QRect MapCanvasWidget::viewportRect() const
{
    const int vBarWidth = (verticalScrollBar_ && verticalScrollBar_->isVisible())
        ? verticalScrollBar_->sizeHint().width()
        : 0;
    const int hBarHeight = (horizontalScrollBar_ && horizontalScrollBar_->isVisible())
        ? horizontalScrollBar_->sizeHint().height()
        : 0;

    const int viewportWidth = (std::max)(0, width() - vBarWidth);
    const int viewportHeight = (std::max)(0, height() - hBarHeight);
    return QRect(0, 0, viewportWidth, viewportHeight);
}

void MapCanvasWidget::updateScrollbars()
{
    if (!horizontalScrollBar_ || !verticalScrollBar_)
    {
        return;
    }

    const int availableWidth = (std::max)(0, width());
    const int availableHeight = (std::max)(0, height());
    const int vBarWidth = verticalScrollBar_->sizeHint().width();
    const int hBarHeight = horizontalScrollBar_->sizeHint().height();

    bool needVertical = contentHeight_ > availableHeight;
    bool needHorizontal = contentWidth_ > (availableWidth - (needVertical ? vBarWidth : 0));
    needVertical = contentHeight_ > (availableHeight - (needHorizontal ? hBarHeight : 0));

    const int viewportWidth = (std::max)(0, availableWidth - (needVertical ? vBarWidth : 0));
    const int viewportHeight = (std::max)(0, availableHeight - (needHorizontal ? hBarHeight : 0));

    horizontalScrollBar_->setVisible(needHorizontal);
    verticalScrollBar_->setVisible(needVertical);

    if (needHorizontal)
    {
        horizontalScrollBar_->setGeometry(0, viewportHeight, viewportWidth, hBarHeight);
    }
    if (needVertical)
    {
        verticalScrollBar_->setGeometry(viewportWidth, 0, vBarWidth, viewportHeight);
    }

    const int maxScrollX = (std::max)(0, contentWidth_ - viewportWidth);
    const int maxScrollY = (std::max)(0, contentHeight_ - viewportHeight);
    scrollX_ = std::clamp(scrollX_, 0, maxScrollX);
    scrollY_ = std::clamp(scrollY_, 0, maxScrollY);

    horizontalScrollBar_->setRange(0, maxScrollX);
    horizontalScrollBar_->setPageStep(viewportWidth);
    horizontalScrollBar_->setValue(scrollX_);

    verticalScrollBar_->setRange(0, maxScrollY);
    verticalScrollBar_->setPageStep(viewportHeight);
    verticalScrollBar_->setValue(scrollY_);
}

void MapCanvasWidget::onMapLeftClick(const QPoint& pointInMapClient)
{
    if (!viewportRect().contains(pointInMapClient))
    {
        return;
    }

    const RegionVisual* region = hitTestRegion(pointInMapClient);
    if (region && region->region)
    {
        emit mapRegionLeftClicked(region->region->getXCoordinate(), region->region->getYCoordinate());
    }
    else
    {
        emit mapNoRegionClicked();
    }
}

void MapCanvasWidget::onMapDoubleClick(const QPoint& pointInMapClient)
{
    if (!viewportRect().contains(pointInMapClient))
    {
        return;
    }

    const RegionVisual* region = hitTestRegion(pointInMapClient);
    if (!region || !region->region)
    {
        return;
    }

    emit mapRegionDoubleClicked(region->region->getXCoordinate(), region->region->getYCoordinate());
    (void)centerOnRegion(region->region->getXCoordinate(), region->region->getYCoordinate());
}

void MapCanvasWidget::onMapRightClick(const QPoint& pointInMapClient, const QPoint& screenPos)
{
    if (!viewportRect().contains(pointInMapClient))
    {
        return;
    }

    const RegionVisual* region = hitTestRegion(pointInMapClient);
    if (region && region->region)
    {
        emit mapRegionRightClicked(screenPos,
                                   region->region->getXCoordinate(),
                                   region->region->getYCoordinate());
    }
    else
    {
        emit zSelectionRequested(screenPos);
    }
}

void MapCanvasWidget::updateHoverTooltip(const QPoint& pointInMapClient)
{
    if (!viewportRect().contains(pointInMapClient))
    {
        hideHoverTooltip();
        return;
    }

    const RegionVisual* region = hitTestRegion(pointInMapClient);
    if (region && region->region)
    {
        const QString hoverText = QString::fromStdWString(
            L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
                region->region->getXCoordinate(),
                region->region->getYCoordinate(),
                region->region->getZCoordinate()));
        if (hoverText_ != hoverText)
        {
            hoverText_ = hoverText;
            emit hoverTextChanged(hoverText_);
        }
        return;
    }

    int xCoordinate = 0;
    int yCoordinate = 0;
    if (hitTestMapCoordinate(pointInMapClient, xCoordinate, yCoordinate))
    {
        const QString hoverText = QString::fromStdWString(
            L"Hover: " + CoordinateFormattingUtils::formatCoordinates(
                xCoordinate,
                yCoordinate,
                selectedZ_));
        if (hoverText_ != hoverText)
        {
            hoverText_ = hoverText;
            emit hoverTextChanged(hoverText_);
        }
        return;
    }

    hideHoverTooltip();
}

void MapCanvasWidget::hideHoverTooltip()
{
    static const QString kDefaultHoverText("Hover: -");
    if (hoverText_ != kDefaultHoverText)
    {
        hoverText_ = kDefaultHoverText;
        emit hoverTextChanged(hoverText_);
    }
}
