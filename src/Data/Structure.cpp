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
 * File: Structure.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Structure.hpp"
#include "Data/AppData.hpp"
#include "Function/StringUtils.hpp"

Structure::Structure(int structureId,
                    int xCoordinate,
                    int yCoordinate,
                    int zCoordinate,
                    std::wstring structureType,
                    std::wstring structureName,
                    bool isClosed,
                    int month,
                    int year)
  : structureId_(structureId)
  , xCoordinate_(xCoordinate)
  , yCoordinate_(yCoordinate)
  , zCoordinate_(zCoordinate)
  , structureType_(std::move(structureType))
  , structureName_(std::move(structureName))
  , isClosed_(isClosed)
  , month_(month)
  , year_(year)
{
}

int Structure::getStructureId() const
{
  return structureId_;
}

int Structure::getXCoordinate() const
{
  return xCoordinate_;
}

int Structure::getYCoordinate() const
{
  return yCoordinate_;
}

int Structure::getZCoordinate() const
{
  return zCoordinate_;
}

const std::wstring& Structure::getStructureType() const
{
  return structureType_;
}

const std::wstring& Structure::getStructureName() const
{
  return structureName_;
}

const std::map<std::wstring, int>& Structure::getFleetItems() const
{
  return fleetItems_;
}

const std::map<std::wstring, int>& Structure::getFleetItemsAfterOrders() const
{
  return fleetItemsAfterOrders_;
}

bool Structure::isFlying() const
{
  AppData& appData = AppData::getInstance();
  const StructInfo* structInfo = appData.structInfoRepository().findByType(structureType_);
  if (!structInfo)
  {
    return false;
  }

  const std::wstring lowerType = StringUtils::toLower(structureType_);
  if (lowerType != L"fleet")
  {
    return structInfo->protoIsFlying();
  }

  if (fleetItems_.empty())
  {
    return false;
  }

  const StructInfoRepository& structInfoRepository = appData.structInfoRepository();
  for (const auto& [itemToken, amount] : fleetItems_)
  {
    if (amount <= 0)
    {
      continue;
    }

    const StructInfo* shipStructInfo = structInfoRepository.findByItemIdentifierToken(itemToken);
    if (!shipStructInfo || !shipStructInfo->protoIsFlying())
    {
      return false;
    }
  }

  return true;
}

bool Structure::isClosed() const
{
  return isClosed_;
}

int Structure::getOwnerUnitId() const
{
  return ownerUnitId_;
}

int Structure::getMonth() const
{
  return month_;
}

int Structure::getYear() const
{
  return year_;
}

void Structure::setStructureType(std::wstring structureType)
{
  structureType_ = std::move(structureType);
}

void Structure::setStructureName(std::wstring structureName)
{
  structureName_ = std::move(structureName);
}


void Structure::setIsClosed(bool isClosed)
{
  isClosed_ = isClosed;
}

void Structure::setOwnerUnitId(int ownerUnitId)
{
  ownerUnitId_ = ownerUnitId;
}

void Structure::setFleetItems(std::map<std::wstring, int> fleetItems)
{
  fleetItems_ = std::move(fleetItems);
}

void Structure::setFleetItemsAfterOrders(std::map<std::wstring, int> fleetItemsAfterOrders)
{
  fleetItemsAfterOrders_ = std::move(fleetItemsAfterOrders);
}

void Structure::setFleetItem(std::wstring fleetItemToken, int amount)
{
  if (fleetItemToken.empty() || amount <= 0)
  {
    return;
  }
  fleetItems_[std::move(fleetItemToken)] = amount;
}

void Structure::setFleetItemAfterOrders(std::wstring fleetItemToken, int amount)
{
  if (fleetItemToken.empty() || amount <= 0)
  {
    return;
  }
  fleetItemsAfterOrders_[std::move(fleetItemToken)] = amount;
}

void Structure::clearFleetItems()
{
  fleetItems_.clear();
}

void Structure::clearFleetItemsAfterOrders()
{
  fleetItemsAfterOrders_.clear();
}

void Structure::setMonth(int month)
{
  month_ = month;
}

void Structure::setYear(int year)
{
  year_ = year;
}
