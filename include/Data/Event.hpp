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
 * File: Event.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>

class Event
{
public:
  Event(int unitId, int eventId, std::wstring message, int month = 0, int year = 0);
  ~Event() = default;

  Event(const Event&) = default;
  Event& operator=(const Event&) = default;
  Event(Event&&) = default;
  Event& operator=(Event&&) = default;

  int getUnitId() const;
  int getEventId() const;
  const std::wstring& getMessage() const;
  const std::wstring& getIdentifier() const;
  int getMonth() const;
  int getYear() const;
  bool isErrorEvent() const;
  void setErrorEvent(bool errorEvent);

  static std::wstring calculateIdentifier(int unitId, const std::wstring& message, int month, int year);

private:
  int          unitId_ { 0 };
  int          eventId_ { 0 };
  std::wstring message_;
  std::wstring identifier_;
  int          month_ { 0 };
  int          year_ { 0 };
  bool         errorEvent_ { false };
};
