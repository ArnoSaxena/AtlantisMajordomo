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
 * File: Structure.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <map>

/**
* @brief Data model for a structure entity.
*
* StructureId is the stable identifier and cannot be modified after
* construction. The combination of structureId and coordinates (x, y, z)
* must be unique within the repository.
*/
class Structure
{
public:
  Structure(int structureId,
            int xCoordinate,
            int yCoordinate,
            int zCoordinate,
            std::wstring structureType,
            std::wstring structureName,
            bool isClosed,
            int month,
            int year);
  ~Structure() = default;

  Structure(const Structure&) = default;
  Structure& operator=(const Structure&) = default;
  Structure(Structure&&) = default;
  Structure& operator=(Structure&&) = default;

  int getStructureId() const;
  int getXCoordinate() const;
  int getYCoordinate() const;
  int getZCoordinate() const;
  const std::wstring& getStructureType() const;
  const std::wstring& getStructureName() const;
  bool isClosed() const;
  bool isFlying() const;
  int getOwnerUnitId() const;
  const std::map<std::wstring, int>& getFleetItems() const;
  const std::map<std::wstring, int>& getFleetItemsAfterOrders() const;
  int getMonth() const;
  int getYear() const;

  void setStructureType(std::wstring structureType);
  void setStructureName(std::wstring structureName);
  void setIsClosed(bool isClosed);
  void setOwnerUnitId(int ownerUnitId);
  void setFleetItems(std::map<std::wstring, int> fleetItems);
  void setFleetItemsAfterOrders(std::map<std::wstring, int> fleetItemsAfterOrders);
  void setFleetItem(std::wstring itemToken, int amount);
  void setFleetItemAfterOrders(std::wstring itemToken, int amount);
  void clearFleetItems();
  void clearFleetItemsAfterOrders();
  void setMonth(int month);
  void setYear(int year);

private:
  int           structureId_;
  int           xCoordinate_;
  int           yCoordinate_;
  int           zCoordinate_;
  std::wstring  structureType_;
  std::wstring  structureName_;
  bool          isClosed_;
  int           ownerUnitId_ { 0 };
  std::map<std::wstring, int> fleetItems_;
  std::map<std::wstring, int> fleetItemsAfterOrders_;
  int           month_ { 0 };
  int           year_ { 0 };
};
