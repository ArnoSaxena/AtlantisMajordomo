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
 * File: ItemRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Item.hpp"

#include <cstddef>
#include <string>
#include <vector>

/**
* @brief Repository for item entities.
*
* Uses identifier token as the stable identifier and enforces uniqueness.
*/
class ItemRepository
{
public:
  ItemRepository() = default;
  ~ItemRepository() = default;

  ItemRepository(const ItemRepository&) = default;
  ItemRepository& operator=(const ItemRepository&) = default;
  ItemRepository(ItemRepository&&) = default;
  ItemRepository& operator=(ItemRepository&&) = default;

  bool add(std::wstring identifierToken,
          std::wstring itemName,
          int weight,
          bool meeleWeapon,
          bool rangedWeapon,
          bool armour,
          bool resource,
          bool mount,
          int moves,
          int walkCapacity,
          int rideCapacity,
          int swimCapacity,
          int flyCapacity,
          bool man);

  bool removeByIdentifierToken(const std::wstring& identifierToken);

  Item* findByIdentifierToken(const std::wstring& identifierToken);
  const Item* findByIdentifierToken(const std::wstring& identifierToken) const;

  const Item* findByItemName(const std::wstring& itemName) const;

  void clear();

  std::size_t size() const;
  const Item& at(std::size_t index) const;

  const std::wstring& getLastError() const;

  int calculateTotalWeight(const std::map<std::wstring, int>& itemCounts) const;
  int calculateManItemCount(const std::map<std::wstring, int>& itemCounts) const;

private:
  std::vector<Item> items_;
  std::wstring      lastError_;
};
