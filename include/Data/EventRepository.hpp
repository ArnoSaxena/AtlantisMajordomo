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
 * File: EventRepository.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Event.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

class EventRepository
{
public:
  EventRepository() = default;
  ~EventRepository() = default;

  EventRepository(const EventRepository&) = default;
  EventRepository& operator=(const EventRepository&) = default;
  EventRepository(EventRepository&&) = default;
  EventRepository& operator=(EventRepository&&) = default;

  bool add(Event eventValue);
  bool add(int unitId, const std::wstring& message);
  bool removeByIdentifier(const std::wstring& identifier);

  Event* findByIdentifier(const std::wstring& identifier);
  const Event* findByIdentifier(const std::wstring& identifier) const;
  std::vector<const Event*> findByUnitId(int unitId) const;
  std::vector<const Event*> findErrorsByUnitId(int unitId) const;
  std::vector<const Event*> findByPeriod(int month, int year) const;
  std::vector<std::pair<int, int>> getAvailablePeriods() const;
  bool getLatestPeriod(int& month, int& year) const;
  std::vector<const Event*> findLatestEventsByUnitId(int unitId) const;

  void clear();

  std::size_t size() const;
  const Event& at(std::size_t index) const;

  const std::wstring& getLastError() const;

  int getNextEventId() const;

private:
  std::vector<Event> events_;
  std::wstring       lastError_;
};
