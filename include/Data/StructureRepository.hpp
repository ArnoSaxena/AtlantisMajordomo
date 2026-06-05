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
 * File: StructureRepository.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include "Data/Structure.hpp"

#include <cstddef>
#include <string>
#include <vector>

/**
* @brief Repository for structure entities.
*
* Enforces uniqueness on the combination of structureId and coordinates (x, y, z).
*/
class StructureRepository
{
public:
  StructureRepository() = default;
  ~StructureRepository() = default;

  StructureRepository(const StructureRepository&) = default;
  StructureRepository& operator=(const StructureRepository&) = default;
  StructureRepository(StructureRepository&&) = default;
  StructureRepository& operator=(StructureRepository&&) = default;

  /**
  * @brief Add a new structure to the repository.
  *
  * Returns false if a structure with the same (id, x, y, z) combination
  * already exists. Sets lastError_ in that case.
  */
  bool add(int structureId,
          int xCoordinate,
          int yCoordinate,
          int zCoordinate,
          std::wstring structureType,
          std::wstring structureName,
          bool isClosed,
          int month,
          int year);

  bool addOrUpdateIfLater(int structureId,
                          int xCoordinate,
                          int yCoordinate,
                          int zCoordinate,
                          std::wstring structureType,
                          std::wstring structureName,
                          bool isClosed,
                          int month,
                          int year);

  /**
  * @brief Remove a structure by its ID and coordinates.
  */
  bool removeByIdAndCoordinates(int structureId,
                                int xCoordinate,
                                int yCoordinate,
                                int zCoordinate);

  /**
  * @brief Find a structure by ID and coordinates.
  */
  Structure* findByIdAndCoordinates(int structureId,
                                    int xCoordinate,
                                    int yCoordinate,
                                    int zCoordinate);

  /**
  * @brief Find a structure by ID and coordinates (const version).
  */
  const Structure* findByIdAndCoordinates(int structureId,
                                          int xCoordinate,
                                          int yCoordinate,
                                          int zCoordinate) const;

  std::vector<const Structure*> findByCoordinates(int xCoordinate,
                                                  int yCoordinate,
                                                  int zCoordinate) const;

  std::vector<const Structure*> findByCoordinatesAndType(int xCoordinate,
                                                         int yCoordinate,
                                                         int zCoordinate,
                                                         const std::wstring& structureType) const;

  void clear();

  std::size_t size() const;
  Structure& at(std::size_t index);
  const Structure& at(std::size_t index) const;

  const std::wstring& getLastError() const;

private:
  std::vector<Structure> structures_;
  std::wstring           lastError_;
};
