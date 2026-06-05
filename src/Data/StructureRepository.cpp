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
 * File: StructureRepository.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/StructureRepository.hpp"

#include <algorithm>
#include <cwctype>

bool StructureRepository::add(int structureId,
                              int xCoordinate,
                              int yCoordinate,
                              int zCoordinate,
                              std::wstring structureType,
                              std::wstring structureName,
                              bool isClosed,
                              int month,
                              int year)
{
  // Check if a structure with this ID and coordinates combination already exists
  if (findByIdAndCoordinates(structureId, xCoordinate, yCoordinate, zCoordinate) != nullptr)
  {
    lastError_ = L"A structure with the same ID and coordinates already exists";
    return false;
  }

  structures_.emplace_back(structureId,
                          xCoordinate,
                          yCoordinate,
                          zCoordinate,
                          std::move(structureType),
                          std::move(structureName),
                          isClosed,
                          month,
                          year);
  lastError_.clear();
  return true;
}

bool StructureRepository::addOrUpdateIfLater(int structureId,
                                            int xCoordinate,
                                            int yCoordinate,
                                            int zCoordinate,
                                            std::wstring structureType,
                                            std::wstring structureName,
                                            bool isClosed,
                                            int month,
                                            int year)
{
  Structure* existing = findByIdAndCoordinates(structureId, xCoordinate, yCoordinate, zCoordinate);
  if (existing == nullptr)
  {
    return add(structureId,
              xCoordinate,
              yCoordinate,
              zCoordinate,
              std::move(structureType),
              std::move(structureName),
              isClosed,
              month,
              year);
  }

  const bool isLater = (year > existing->getYear()) ||
                      (year == existing->getYear() && month > existing->getMonth());
  if (!isLater)
  {
    lastError_.clear();
    return true;
  }

  existing->setStructureType(std::move(structureType));
  existing->setStructureName(std::move(structureName));
  existing->setIsClosed(isClosed);
  existing->setMonth(month);
  existing->setYear(year);
  lastError_.clear();
  return true;
}

bool StructureRepository::removeByIdAndCoordinates(int structureId,
                                                  int xCoordinate,
                                                  int yCoordinate,
                                                  int zCoordinate)
{
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [structureId, xCoordinate, yCoordinate, zCoordinate](const Structure& s)
                        {
                          return s.getStructureId() == structureId &&
                                  s.getXCoordinate() == xCoordinate &&
                                  s.getYCoordinate() == yCoordinate &&
                                  s.getZCoordinate() == zCoordinate;
                        });

  if (it != structures_.end())
  {
    structures_.erase(it);
    lastError_.clear();
    return true;
  }

  lastError_ = L"Structure not found";
  return false;
}

Structure* StructureRepository::findByIdAndCoordinates(int structureId,
                                                      int xCoordinate,
                                                      int yCoordinate,
                                                      int zCoordinate)
{
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [structureId, xCoordinate, yCoordinate, zCoordinate](const Structure& s)
                        {
                          return s.getStructureId() == structureId &&
                                  s.getXCoordinate() == xCoordinate &&
                                  s.getYCoordinate() == yCoordinate &&
                                  s.getZCoordinate() == zCoordinate;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

const Structure* StructureRepository::findByIdAndCoordinates(int structureId,
                                                            int xCoordinate,
                                                            int yCoordinate,
                                                            int zCoordinate) const
{
  auto it = std::find_if(structures_.begin(), structures_.end(),
                        [structureId, xCoordinate, yCoordinate, zCoordinate](const Structure& s)
                        {
                          return s.getStructureId() == structureId &&
                                  s.getXCoordinate() == xCoordinate &&
                                  s.getYCoordinate() == yCoordinate &&
                                  s.getZCoordinate() == zCoordinate;
                        });

  return (it != structures_.end()) ? &(*it) : nullptr;
}

std::vector<const Structure*> StructureRepository::findByCoordinates(int xCoordinate,
                                                                     int yCoordinate,
                                                                     int zCoordinate) const
{
  std::vector<const Structure*> results;
  for (const Structure& structure : structures_)
  {
    if (structure.getXCoordinate() == xCoordinate &&
        structure.getYCoordinate() == yCoordinate &&
        structure.getZCoordinate() == zCoordinate)
    {
      results.push_back(&structure);
    }
  }

  return results;
}

// TODO: we need to move this method to a more general utility class if we want to use it in other places as well.
static bool iequals(const std::wstring& a, const std::wstring& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::towlower(a[i]) != std::towlower(b[i])) return false;
    return true;
}

std::vector<const Structure*> StructureRepository::findByCoordinatesAndType(int xCoordinate,
                                                                     int yCoordinate,
                                                                     int zCoordinate,
                                                                     const std::wstring& structureType) const
{
  std::vector<const Structure*> candidates = findByCoordinates(xCoordinate, yCoordinate, zCoordinate);
  std::vector<const Structure*> results;
  for (const Structure* structure : candidates)
  {
    if (iequals(structure->getStructureType(), structureType))
    {
      results.push_back(structure);
    }
  } 
  return results;
}


void StructureRepository::clear()
{
  structures_.clear();
  lastError_.clear();
}

std::size_t StructureRepository::size() const
{
  return structures_.size();
}

Structure& StructureRepository::at(std::size_t index)
{
  return structures_.at(index);
}

const Structure& StructureRepository::at(std::size_t index) const
{
  return structures_.at(index);
}

const std::wstring& StructureRepository::getLastError() const
{
  return lastError_;
}
