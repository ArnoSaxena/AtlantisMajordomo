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
 * File: EventRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/EventRepository.hpp"

#include <functional>
#include <set>

bool EventRepository::add(Event eventValue)
{
  const std::wstring& identifier = eventValue.getIdentifier();
  if (findByIdentifier(identifier) != nullptr)
  {
    lastError_ = L"Event with the same identifier already exists";
    return false;
  }

  events_.push_back(std::move(eventValue));
  lastError_.clear();
  return true;
}

bool EventRepository::add(int unitId, const std::wstring& message)
{
  int eventId = getNextEventId();
  return add(Event(unitId, eventId, message));
}

bool EventRepository::removeByIdentifier(const std::wstring& identifier)
{
  for (auto iterator = events_.begin(); iterator != events_.end(); ++iterator)
  {
    if (iterator->getIdentifier() == identifier)
    {
      events_.erase(iterator);
      lastError_.clear();
      return true;
    }
  }

  lastError_ = L"Event not found";
  return false;
}

Event* EventRepository::findByIdentifier(const std::wstring& identifier)
{
  for (Event& eventValue : events_)
  {
    if (eventValue.getIdentifier() == identifier)
    {
      return &eventValue;
    }
  }

  return nullptr;
}

const Event* EventRepository::findByIdentifier(const std::wstring& identifier) const
{
  for (const Event& eventValue : events_)
  {
    if (eventValue.getIdentifier() == identifier)
    {
      return &eventValue;
    }
  }

  return nullptr;
}

std::vector<const Event*> EventRepository::findByUnitId(int unitId) const
{
  std::vector<const Event*> results;
  for (const Event& eventValue : events_)
  {
    if (eventValue.getUnitId() == unitId)
    {
      results.push_back(&eventValue);
    }
  }

  return results;
}

std::vector<const Event*> EventRepository::findLatestEventsByUnitId(int unitId) const
{
  int latestYear {0};
  int latestMonth {0};
  getLatestPeriod(latestMonth, latestYear);

  std::vector<const Event*> results;
  for (const Event& eventValue : events_)
  {
    if (eventValue.getUnitId() == unitId 
        && eventValue.getYear() == latestYear 
        && eventValue.getMonth() == latestMonth)
    {
        results.push_back(&eventValue);
    }
  }
  return results;
}

std::vector<const Event*> EventRepository::findErrorsByUnitId(int unitId) const
{
  std::vector<const Event*> results;
  for (const Event& eventValue : events_)
  {
    if (eventValue.getUnitId() == unitId && eventValue.isErrorEvent())
    {
      results.push_back(&eventValue);
    }
  }

  return results;
}

std::vector<const Event*> EventRepository::findByPeriod(int month, int year) const
{
  std::vector<const Event*> results;
  for (const Event& eventValue : events_)
  {
    if (eventValue.getMonth() == month && eventValue.getYear() == year)
    {
      results.push_back(&eventValue);
    }
  }

  return results;
}

std::vector<std::pair<int, int>> EventRepository::getAvailablePeriods() const
{
  std::set<std::pair<int, int>, std::greater<>> periods;
  for (const Event& eventValue : events_)
  {
    const int eventMonth = eventValue.getMonth();
    const int eventYear = eventValue.getYear();
    if (eventMonth < 1 || eventMonth > 12 || eventYear <= 0)
    {
      continue;
    }

    periods.insert({eventYear, eventMonth});
  }

  std::vector<std::pair<int, int>> results;
  results.reserve(periods.size());
  for (const auto& period : periods)
  {
    results.push_back({period.second, period.first});
  }

  return results;
}

bool EventRepository::getLatestPeriod(int& month, int& year) const
{
  std::set<std::pair<int, int>, std::greater<>> periods;
  for (const Event& eventValue : events_)
  {
    const int eventMonth = eventValue.getMonth();
    const int eventYear = eventValue.getYear();
    if (eventMonth < 1 || eventMonth > 12 || eventYear <= 0)
    {
      continue;
    }

    periods.insert({eventYear, eventMonth});
  }

  if (periods.empty())
  {
    return false;
  }

  const auto latest = *periods.begin();
  year = latest.first;
  month = latest.second;
  return true;
}

void EventRepository::clear()
{
  events_.clear();
  lastError_.clear();
}

std::size_t EventRepository::size() const
{
  return events_.size();
}

const Event& EventRepository::at(std::size_t index) const
{
  return events_.at(index);
}

const std::wstring& EventRepository::getLastError() const
{
  return lastError_;
}

int EventRepository::getNextEventId() const
{
    if (events_.empty())
        return 1;

    int maxId = 0;
    for (const auto& event : events_)
    {
        if (event.getEventId() > maxId)
        {
            maxId = event.getEventId();
        }
    }
    return maxId + 1;
}
