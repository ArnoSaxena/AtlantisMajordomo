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
 * File: MapCanvasWidget.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QColor>
#include <QPoint>
#include <QPolygon>
#include <QWidget>

#include <string>
#include <vector>

class AppConfig;
class AppData;
class Region;

class MapCanvasWidget : public QWidget
{
    Q_OBJECT

public:
    struct RegionVisual
    {
        const Region* region { nullptr };
        QPoint center;
        QPolygon polygon;
    };

    explicit MapCanvasWidget(AppData& appData, AppConfig& appConfig, QWidget* parent = nullptr);

    void setSelectedZ(int selectedZ);
    void setSelectedRegion(bool hasSelectedRegion, int selectedRegionX, int selectedRegionY);
    void recalculateVisibleMap();

    const std::vector<int>& availableZLevels() const { return availableZLevels_; }
    int selectedZ() const { return selectedZ_; }
    int contentWidth() const { return contentWidth_; }
    int contentHeight() const { return contentHeight_; }

    const RegionVisual* hitTestRegion(const QPoint& pointInMapClient) const;
    bool hitTestMapCoordinate(const QPoint& pointInMapClient, int& xCoordinate, int& yCoordinate) const;
    QColor getRegionFillColor(const std::wstring& regionType) const;

signals:
    void mapRegionLeftClicked(int regionX, int regionY);
    void mapRegionDoubleClicked(int regionX, int regionY);
    void mapRegionRightClicked(QPoint screenPos, int regionX, int regionY);
    void mapNoRegionClicked();
    void zSelectionRequested(QPoint screenPos);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    AppData* appData_ { nullptr };
    AppConfig* appConfig_ { nullptr };

    std::vector<RegionVisual> visibleRegions_;
    std::vector<int> availableZLevels_;

    int selectedZ_ { 1 };
    bool hasSelectedRegion_ { false };
    int selectedRegionX_ { 0 };
    int selectedRegionY_ { 0 };

    bool hasMapBounds_ { false };
    int mapMinX_ { 0 };
    int mapMaxX_ { 0 };
    int mapMinY_ { 0 };
    int mapMaxY_ { 0 };
    int mapLeftPaddingColumns_ { 0 };
    int mapRightPaddingColumns_ { 0 };

    int contentWidth_ { 0 };
    int contentHeight_ { 0 };
    int scrollX_ { 0 };
    int scrollY_ { 0 };
};
