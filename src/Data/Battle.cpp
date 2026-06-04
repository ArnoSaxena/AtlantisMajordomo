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
 * File: Battle.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Battle.hpp"

#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <utility>

namespace
{

std::wstring generateIdentifier()
{
  std::random_device randomDevice;
  std::mt19937_64 generator(randomDevice());
  std::uniform_int_distribution<std::uint64_t> distribution;

  const std::uint64_t first = distribution(generator);
  const std::uint64_t second = distribution(generator);

  std::wstringstream stream;
  stream << std::hex << std::setfill(L'0')
         << std::setw(8) << static_cast<std::uint32_t>(first >> 32) << L'-'
         << std::setw(4) << static_cast<std::uint32_t>((first >> 16) & 0xFFFF) << L'-'
         << std::setw(4) << static_cast<std::uint32_t>(first & 0xFFFF) << L'-'
         << std::setw(4) << static_cast<std::uint32_t>(second >> 48) << L'-'
         << std::setw(12) << static_cast<std::uint64_t>(second & 0x0000FFFFFFFFFFFFULL);
  return stream.str();
}

} // namespace

Battle::Battle()
  : identifier_(generateIdentifier())
{
}

const std::wstring& Battle::getIdentifier() const
{
  return identifier_;
}

const std::wstring& Battle::getFullText() const
{
  return fullText_;
}

int Battle::getAttackerUnitId() const
{
  return attackerUnitId_;
}

const std::wstring& Battle::getAttackerUnitName() const
{
  return attackerUnitName_;
}

int Battle::getDefenderUnitId() const
{
  return defenderUnitId_;
}

const std::wstring& Battle::getDefenderUnitName() const
{
  return defenderUnitName_;
}

int Battle::getRegionXCoordinate() const
{
  return regionXCoordinate_;
}

int Battle::getRegionYCoordinate() const
{
  return regionYCoordinate_;
}

int Battle::getRegionZCoordinate() const
{
  return regionZCoordinate_;
}

int Battle::getMonth() const
{
  return month_;
}

int Battle::getYear() const
{
  return year_;
}

const std::wstring& Battle::getRegionType() const
{
  return regionType_;
}

const std::wstring& Battle::getProvinceName() const
{
  return provinceName_;
}

bool Battle::isRegionFoundInRepository() const
{
  return regionFoundInRepository_;
}

int Battle::getAttackerLosses() const
{
  return attackerLosses_;
}

int Battle::getDefenderLosses() const
{
  return defenderLosses_;
}

const std::vector<int>& Battle::getAttackerDamagedUnitIds() const
{
  return attackerDamagedUnitIds_;
}

const std::vector<int>& Battle::getDefenderDamagedUnitIds() const
{
  return defenderDamagedUnitIds_;
}

const std::vector<BattleSpoil>& Battle::getSpoils() const
{
  return spoils_;
}

void Battle::setIdentifier(std::wstring identifier)
{
  // Identifier origin: Battle::Battle() assigns identifier_ using
  // generateIdentifier() (a random UUID-like value built from two uint64 draws).
  // This setter is intentionally kept for import/restore/merge flows where an
  // existing persisted identifier must be retained instead of generating a new one.
  identifier_ = std::move(identifier);
}

void Battle::setFullText(std::wstring fullText)
{
  fullText_ = std::move(fullText);
}

void Battle::setAttackerUnitId(int attackerUnitId)
{
  attackerUnitId_ = attackerUnitId;
}

void Battle::setAttackerUnitName(std::wstring attackerUnitName)
{
  attackerUnitName_ = std::move(attackerUnitName);
}

void Battle::setDefenderUnitId(int defenderUnitId)
{
  defenderUnitId_ = defenderUnitId;
}

void Battle::setDefenderUnitName(std::wstring defenderUnitName)
{
  defenderUnitName_ = std::move(defenderUnitName);
}

void Battle::setRegionCoordinates(int xCoordinate, int yCoordinate, int zCoordinate)
{
  regionXCoordinate_ = xCoordinate;
  regionYCoordinate_ = yCoordinate;
  regionZCoordinate_ = zCoordinate;
}

void Battle::setMonth(int month)
{
  month_ = month;
}

void Battle::setYear(int year)
{
  year_ = year;
}

void Battle::setRegionType(std::wstring regionType)
{
  regionType_ = std::move(regionType);
}

void Battle::setProvinceName(std::wstring provinceName)
{
  provinceName_ = std::move(provinceName);
}

void Battle::setRegionFoundInRepository(bool regionFoundInRepository)
{
  regionFoundInRepository_ = regionFoundInRepository;
}

void Battle::setAttackerLosses(int attackerLosses)
{
  attackerLosses_ = attackerLosses;
}

void Battle::setDefenderLosses(int defenderLosses)
{
  defenderLosses_ = defenderLosses;
}

void Battle::addAttackerDamagedUnitId(int unitId)
{
  attackerDamagedUnitIds_.push_back(unitId);
}

void Battle::addDefenderDamagedUnitId(int unitId)
{
  defenderDamagedUnitIds_.push_back(unitId);
}

void Battle::clearAttackerDamagedUnitIds()
{
  attackerDamagedUnitIds_.clear();
}

void Battle::clearDefenderDamagedUnitIds()
{
  defenderDamagedUnitIds_.clear();
}

void Battle::addSpoil(BattleSpoil spoil)
{
  spoils_.push_back(std::move(spoil));
}

void Battle::clearSpoils()
{
  spoils_.clear();
}
