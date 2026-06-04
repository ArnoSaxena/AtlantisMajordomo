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
 * File: ItemRepository.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/ItemRepository.hpp"

#include <algorithm>
#include <cwctype>
#include <utility>

namespace
{
  std::wstring upperToken(std::wstring token)
  {
    std::transform(token.begin(), token.end(), token.begin(),
                   [](wchar_t ch) { return static_cast<wchar_t>(towupper(ch)); });
    return token;
  }
}

bool ItemRepository::add(std::wstring identifierToken,
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
                        bool man)
{
  identifierToken = upperToken(std::move(identifierToken));

  if (identifierToken.length() < 3)
  {
    lastError_ = L"Item identifier token must be at least 3 characters";
    return false;
  }

  if (findByIdentifierToken(identifierToken) != nullptr)
  {
    lastError_ = L"Item identifier token already exists";
    return false;
  }

  items_.emplace_back(std::move(identifierToken),
                      std::move(itemName),
                      weight,
                      meeleWeapon,
                      rangedWeapon,
                      armour,
                      resource,
                      mount,
                      moves,
                      walkCapacity,
                      rideCapacity,
                      swimCapacity,
                      flyCapacity,
                      man);
  lastError_.clear();
  return true;
}

bool ItemRepository::removeByIdentifierToken(const std::wstring& identifierToken)
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    items_.begin(),
    items_.end(),
    [&normalizedToken](const Item& item)
    {
      return item.getIdentifierToken() == normalizedToken;
    }
  );

  if (it == items_.end())
  {
    lastError_ = L"Item not found";
    return false;
  }

  items_.erase(it);
  lastError_.clear();
  return true;
}

Item* ItemRepository::findByIdentifierToken(const std::wstring& identifierToken)
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    items_.begin(),
    items_.end(),
    [&normalizedToken](const Item& item)
    {
      return item.getIdentifierToken() == normalizedToken;
    }
  );

  return it == items_.end() ? nullptr : &(*it);
}

const Item* ItemRepository::findByIdentifierToken(const std::wstring& identifierToken) const
{
  const std::wstring normalizedToken = upperToken(identifierToken);
  const auto it = std::find_if(
    items_.cbegin(),
    items_.cend(),
    [&normalizedToken](const Item& item)
    {
      return item.getIdentifierToken() == normalizedToken;
    }
  );

  return it == items_.cend() ? nullptr : &(*it);
}

const Item* ItemRepository::findByItemName(const std::wstring& itemName) const
{
  const auto it = std::find_if(
    items_.cbegin(),
    items_.cend(),
    [&itemName](const Item& item)
    {
      return item.getItemName() == itemName;
    }
  );

  return it == items_.cend() ? nullptr : &(*it);
}

void ItemRepository::clear()
{
  items_.clear();
  lastError_.clear();
}

std::size_t ItemRepository::size() const
{
  return items_.size();
}

const Item& ItemRepository::at(std::size_t index) const
{
  return items_.at(index);
}

const std::wstring& ItemRepository::getLastError() const
{
  return lastError_;
}

int ItemRepository::calculateTotalWeight(const std::map<std::wstring, int>& itemCounts) const
{
  int totalWeight = 0;

  for (const auto& [itemToken, count] : itemCounts)
  {
    if (count <= 0)
    {
      continue;
    }

    const std::wstring normalizedToken = upperToken(itemToken);
    const Item* item = findByIdentifierToken(normalizedToken);
    if (!item)
    {
      continue;
    }

    totalWeight += item->getWeight() * count;
  }

  return totalWeight;
}

int ItemRepository::calculateManItemCount(const std::map<std::wstring, int>& itemCounts) const
{
  int manCount = 0;

  for (const auto& [itemToken, count] : itemCounts)
  {
    if (count <= 0)
    {
      continue;
    }

    const std::wstring normalizedToken = upperToken(itemToken);
    const Item* item = findByIdentifierToken(normalizedToken);
    if (item && item->isMan())
    {
      manCount += count;
    }
  }

  return manCount;
}