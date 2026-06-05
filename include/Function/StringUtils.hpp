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
 * File: StringUtils.hpp
 */
 
#pragma once

#include <string>

namespace StringUtils
{
    std::wstring trimWhitespace(const std::wstring& value);
    std::wstring toLower(std::wstring value);
    std::wstring toUpper(std::wstring value);
    int parseIntSafe(const std::wstring& text);
    void appendUnicodeEscape(std::wstring& result, unsigned int codeUnit);
}