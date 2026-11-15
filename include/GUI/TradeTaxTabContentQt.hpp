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
 * File: TradeTaxTabContentQt.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include "Function/TradeTaxUtils.hpp"

class AppData;
class QButtonGroup;
class QLabel;
class QPushButton;
class QTableWidget;

class TradeTaxTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit TradeTaxTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~TradeTaxTabContentQt() override = default;

    TradeTaxTabContentQt(const TradeTaxTabContentQt&) = delete;
    TradeTaxTabContentQt& operator=(const TradeTaxTabContentQt&) = delete;
    TradeTaxTabContentQt(TradeTaxTabContentQt&&) = delete;
    TradeTaxTabContentQt& operator=(TradeTaxTabContentQt&&) = delete;

    void refresh();

signals:
    void navigateToRegion(int x, int y, int z);

private:
    enum class Mode
    {
        Tax,
        Resources,
    };

    void setMode(Mode mode);
    void updateSummaryLabel();
    void updateRows();
    void populateTaxRows();
    void populateResourceRows();
    void setRowBackground(int row, TradeTaxUtils::RowSeverity severity);

    AppData* appData_ { nullptr };
    QLabel* summaryLabel_ { nullptr };
    QButtonGroup* buttonGroup_ { nullptr };
    QPushButton* taxButton_ { nullptr };
    QPushButton* resourcesButton_ { nullptr };
    QTableWidget* table_ { nullptr };
    Mode mode_ { Mode::Tax };
};
