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
 * File: Order.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>
#include <vector>

class Order
{
public:
    Order(int issuingUnitNumber,
        bool fromNewUnit,
        std::wstring orderToken, 
        int xCoordinate, 
        int yCoordinate, 
        int zCoordinate, 
        std::wstring fullOrderText): 
        issuingUnitNumber_(issuingUnitNumber),
        fromNewUnit_(fromNewUnit),
        xCoordinate_(xCoordinate),
        yCoordinate_(yCoordinate),
        zCoordinate_(zCoordinate),
        orderToken_(orderToken),
        fullOrderText_(fullOrderText) {}
    ~Order() = default;

    Order(const Order&) = default;
    Order& operator=(const Order&) = default;
    Order(Order&&) = default;
    Order& operator=(Order&&) = default;

    int getIssuingUnitNumber() const;
    bool getFromNewUnit() const;
    int getXCoordinate() const;
    int getYCoordinate() const;
    int getZCoordinate() const;
    const std::wstring& getOrderToken() const;
    const std::wstring& getFullOrderText() const;

    void setIssuingUnitNumber(int issuingUnitNumber);
    void setFromNewUnit(bool fromNewUnit);
    void setXCoordinate(int xCoordinate);
    void setYCoordinate(int yCoordinate);
    void setZCoordinate(int zCoordinate);
    void setOrderToken(std::wstring orderToken);
    void setFullOrderText(std::wstring fullOrderText);

private:
    int issuingUnitNumber_{ 0 };
    bool fromNewUnit_ {false};
    int xCoordinate_ { 0 };
    int yCoordinate_ { 0 };
    int zCoordinate_ { 0 };
    std::wstring orderToken_;
    std::wstring fullOrderText_;

};