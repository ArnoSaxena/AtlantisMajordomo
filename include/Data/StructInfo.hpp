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
 * File: StructInfo.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>

/**
* @brief Data model for a structure data entity.
*
* StructureId is the stable identifier and cannot be modified after
* construction. 
*/
class StructInfo
{
public:
  StructInfo(std::wstring structureType,
            int needs,
            int magesCapacity,
            bool isShip,
            bool isFlying);
  ~StructInfo() = default;

  StructInfo(const StructInfo&) = default;
  StructInfo& operator=(const StructInfo&) = default;
  StructInfo(StructInfo&&) = default;
  StructInfo& operator=(StructInfo&&) = default;

  const std::wstring& getStructureType() const;
  int getNeeds() const;
  int getMagesCapacity() const;
  bool isShip() const;
  bool protoIsFlying() const;
  const std::wstring& getItemIdentifierToken() const;

  void setStructureType(std::wstring structureType);
  void setNeeds(int needs);
  void setMagesCapacity(int magesCapacity);
  void setIsShip(bool isShip);
  void setIsFlying(bool isFlying);
  void setItemIdentifierToken(std::wstring itemIdentifierToken);

  static std::wstring extractRoadDirectionFromStructureType(const std::wstring& structureType);

private:
  std::wstring  structureType_;
  int           needs_ { 0 };
  int           magesCapacity_ { 0 };
  bool          isShip_;
  bool          isFlying_;
  std::wstring  itemIdentifierToken_;
};
