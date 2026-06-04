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
 * File: OrderRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/OrderRepository.hpp"

#include <algorithm>

void OrderRepository::addOrder(
    int issuingUnitNumber, 
    bool fromNewUnit, 
    int xCoordinate, 
    int yCoordinate, 
    int zCoordinate, 
    std::wstring orderToken, 
    std::wstring fullOrderText)
{
    orders_[issuingUnitNumber].emplace_back(
        issuingUnitNumber, 
        fromNewUnit,
        orderToken, 
        xCoordinate, 
        yCoordinate, 
        zCoordinate, 
        fullOrderText);
}

const std::vector<Order>* OrderRepository::getOrdersForUnit(int issuingUnitNumber, bool fromNewUnit) const
{
    auto it = orders_.find(issuingUnitNumber);
    if (it == orders_.end())
    {
        return nullptr;
    }

    thread_local std::vector<Order> filteredOrders;
    filteredOrders.clear();
    for (const auto& order : it->second)
    {
        if (order.getFromNewUnit() == fromNewUnit)
        {
            filteredOrders.push_back(order);
        }
    }

    return filteredOrders.empty() ? nullptr : &filteredOrders;
}

const std::vector<Order>& OrderRepository::getOrdersForRegion(int xCoordinate, int yCoordinate, int zCoordinate) const
{
    static const std::vector<Order> emptyOrders;
    thread_local std::vector<Order> regionOrders;

    regionOrders.clear();
    for (const auto& [unitNumber, unitOrders] : orders_)
    {
        for (const auto& order : unitOrders)
        {
            if (order.getXCoordinate() == xCoordinate &&
                order.getYCoordinate() == yCoordinate &&
                order.getZCoordinate() == zCoordinate)
            {
                regionOrders.push_back(order);
            }
        }
    }

    return regionOrders.empty() ? emptyOrders : regionOrders;
}

bool OrderRepository::removeOrdersForUnit(int issuingUnitNumber, bool fromNewUnit)
{
    auto it = orders_.find(issuingUnitNumber);
    if (it == orders_.end())
    {
        return false;
    }

    auto& unitOrders = it->second;
    const auto oldSize = unitOrders.size();
    unitOrders.erase(
        std::remove_if(
            unitOrders.begin(),
            unitOrders.end(),
            [fromNewUnit](const Order& order)
            {
                return order.getFromNewUnit() == fromNewUnit;
            }),
        unitOrders.end());

    if (unitOrders.empty())
    {
        orders_.erase(it);
    }

    return unitOrders.size() != oldSize;
}

void OrderRepository::clear()
{
    orders_.clear();
}
