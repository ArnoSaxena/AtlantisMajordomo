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
 * File: Order.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Order.hpp"

#include <utility>

int Order::getIssuingUnitNumber() const
{
    return issuingUnitNumber_;
}

bool Order::getFromNewUnit() const
{
    return fromNewUnit_;
}

int Order::getXCoordinate() const
{
    return xCoordinate_;
}

int Order::getYCoordinate() const
{
    return yCoordinate_;
}

int Order::getZCoordinate() const
{
    return zCoordinate_;
}

const std::wstring& Order::getOrderToken() const
{
    return orderToken_;
}

const std::wstring& Order::getFullOrderText() const
{
    return fullOrderText_;
}

void Order::setIssuingUnitNumber(int issuingUnitNumber)
{
    issuingUnitNumber_ = issuingUnitNumber;
}

void Order::setFromNewUnit(bool fromNewUnit)
{
    fromNewUnit_ = fromNewUnit;
}

void Order::setXCoordinate(int xCoordinate)
{
    xCoordinate_ = xCoordinate;
}

void Order::setYCoordinate(int yCoordinate)
{
    yCoordinate_ = yCoordinate;
}

void Order::setZCoordinate(int zCoordinate)
{
    zCoordinate_ = zCoordinate;
}

void Order::setOrderToken(std::wstring orderToken)
{
    orderToken_ = orderToken;
}

void Order::setFullOrderText(std::wstring fullOrderText)
{
    fullOrderText_ = fullOrderText;
}