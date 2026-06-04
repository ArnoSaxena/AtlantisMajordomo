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
 * File: DataSerializer.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <string>
#include <memory>

class AppData;

/**
* @brief Handles serialization and deserialization of AppData to/from files.
*
* Provides functionality to save the complete application state (all repositories)
* to a JSON file and load it back.
*/
class DataSerializer
{
public:
  DataSerializer() = default;
  ~DataSerializer() = default;

  DataSerializer(const DataSerializer&) = delete;
  DataSerializer& operator=(const DataSerializer&) = delete;
  DataSerializer(DataSerializer&&) = delete;
  DataSerializer& operator=(DataSerializer&&) = delete;

  /**
  * @brief Saves AppData to a JSON file.
  * @param[in] appData   Application data to save.
  * @param[in] filePath  Path where to save the file.
  * @return true on success, false on error.
  */
  static bool saveToFile(const AppData& appData, const std::wstring& filePath);

  /**
  * @brief Loads AppData from a JSON file.
  * @param[in,out] appData   Application data to populate.
  * @param[in]     filePath  Path to the file to load.
  * @return true on success, false on error.
  */
  static bool loadFromFile(AppData& appData, const std::wstring& filePath);

  /**
  * @brief Gets the last error message from serialization/deserialization.
  * @return Error message string.
  */
  static const std::wstring& getLastError();

private:
  static std::wstring lastError_;
};
