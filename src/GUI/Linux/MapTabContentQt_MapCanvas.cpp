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
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Function/HexDirectionUtils.hpp"
#include "Function/MapUtils.hpp"

#include <QMouseEvent>

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

} // namespace

MapCanvasWidget::MapCanvasWidget(AppData& appData, AppConfig& appConfig, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
    , appConfig_(&appConfig)
{
    setMouseTracking(true);
    recalculateVisibleMap();
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
        return;
    }

    const auto& regionRepository = appData_->regionRepository();
    if (regionRepository.size() == 0)
    {
        contentWidth_ = 0;
        contentHeight_ = 0;
        hasSelectedRegion_ = false;
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

    const RegionVisual* region = hitTestRegion(event->position().toPoint());
    if (event->button() == Qt::LeftButton)
    {
        if (region && region->region)
        {
            emit mapRegionLeftClicked(region->region->getXCoordinate(), region->region->getYCoordinate());
        }
        else
        {
            emit mapNoRegionClicked();
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        if (region && region->region)
        {
            emit mapRegionRightClicked(event->globalPosition().toPoint(),
                                       region->region->getXCoordinate(),
                                       region->region->getYCoordinate());
        }
        else
        {
            emit zSelectionRequested(event->globalPosition().toPoint());
        }
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

    const RegionVisual* region = hitTestRegion(event->position().toPoint());
    if (region && region->region)
    {
        emit mapRegionDoubleClicked(region->region->getXCoordinate(), region->region->getYCoordinate());
    }

    QWidget::mouseDoubleClickEvent(event);
}
