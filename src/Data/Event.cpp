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
 * File: Event.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Event.hpp"

#include <iomanip>
#include <sstream>

namespace
{
  std::wstring makeChecksumSource(int unitId, const std::wstring& message, int month, int year)
  {
    return std::to_wstring(unitId) + L":" + message + L":" + std::to_wstring(month) + L":" + std::to_wstring(year);
  }
}

Event::Event(int unitId, int eventId, std::wstring message, int month, int year)
  : unitId_(unitId)
  , eventId_(eventId)
  , message_(std::move(message))
  , month_(month)
  , year_(year)
  , identifier_(calculateIdentifier(unitId_, message_, month_, year_))
{
}

int Event::getUnitId() const
{
  return unitId_;
}

int Event::getEventId() const
{
  return eventId_;
}

const std::wstring& Event::getMessage() const
{
  return message_;
}

const std::wstring& Event::getIdentifier() const
{
  return identifier_;
}

int Event::getMonth() const
{
  return month_;
}

int Event::getYear() const
{
  return year_;
}

std::wstring Event::calculateIdentifier(int unitId, const std::wstring& message, int month, int year)
{
  constexpr unsigned long long offsetBasis = 14695981039346656037ull;
  constexpr unsigned long long prime = 1099511628211ull;

  unsigned long long hash = offsetBasis;
  const std::wstring checksumSource = makeChecksumSource(unitId, message, month, year);
  for (wchar_t ch : checksumSource)
  {
    hash ^= static_cast<unsigned long long>(ch);
    hash *= prime;
  }

  std::wostringstream stream;
  stream << std::hex << std::setw(16) << std::setfill(L'0') << hash;
  return stream.str();
}

bool Event::isErrorEvent() const
{
  return errorEvent_;
}

void Event::setErrorEvent(bool errorEvent)
{
  errorEvent_ = errorEvent;
}
