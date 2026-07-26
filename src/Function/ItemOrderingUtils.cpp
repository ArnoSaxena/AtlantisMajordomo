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
 * File: ItemOrderingUtils.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/ItemOrderingUtils.hpp"

#include "Data/Item.hpp"
#include "Data/ItemRepository.hpp"

#include <algorithm>

namespace ItemOrderingUtils
{

OrderedItemGroups buildOrderedItemGroups(const ItemRepository& repository)
{
  std::vector<const Item*> leadItems;
  std::vector<const Item*> otherManItems;
  std::vector<const Item*> otherItems;

  for (std::size_t i = 0; i < repository.size(); ++i)
  {
    const Item& item = repository.at(i);
    if (item.getIdentifierToken() == L"LEAD")
    {
      leadItems.push_back(&item);
    }
    else if (item.isMan())
    {
      otherManItems.push_back(&item);
    }
    else
    {
      otherItems.push_back(&item);
    }
  }

  auto sortByToken = [](const Item* left, const Item* right)
  {
    return left->getIdentifierToken() < right->getIdentifierToken();
  };
  std::sort(otherManItems.begin(), otherManItems.end(), sortByToken);
  std::sort(otherItems.begin(), otherItems.end(), sortByToken);

  OrderedItemGroups groups {};
  groups.manItems.insert(groups.manItems.end(), leadItems.begin(), leadItems.end());
  groups.manItems.insert(groups.manItems.end(), otherManItems.begin(), otherManItems.end());
  groups.otherItems = std::move(otherItems);
  return groups;
}

} // namespace ItemOrderingUtils
