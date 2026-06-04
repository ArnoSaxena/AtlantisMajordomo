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
 * File: OrderRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Order.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/****
 * @brief Repository for order entities.
 *
 * Stores orders for units, keyed by the issuing unit's number. Orders are
 * expected to be replaced in their entirety each turn, so no merging or
 * partial updates are supported.
 */
class OrderRepository
{
public:
    OrderRepository() = default;
    ~OrderRepository() = default;

    OrderRepository(const OrderRepository&) = default;
    OrderRepository& operator=(const OrderRepository&) = default;
    OrderRepository(OrderRepository&&) = default;
    OrderRepository& operator=(OrderRepository&&) = default;

    void addOrder(int issuingUnitNumber, bool fromNewUnit, int xCoordinate, int yCoordinate, int zCoordinate, std::wstring orderToken, std::wstring fullOrderText);
    const std::vector<Order> *getOrdersForUnit(int issuingUnitNumber, bool fromNewUnit) const;
    const std::vector<Order> &getOrdersForRegion(int xCoordinate, int yCoordinate, int zCoordinate) const;
    bool removeOrdersForUnit(int issuingUnitNumber, bool fromNewUnit);
    void clear();

private:  
    std::map<int, std::vector<Order>> orders_;
};