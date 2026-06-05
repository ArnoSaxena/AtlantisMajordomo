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
 * File: JsonUtils.hpp
 */
 
#pragma once

#include <string>

namespace JsonUtils
{
    void appendUnicodeEscape(std::wstring& result, unsigned int codeUnit);
    bool tryParseHex4(const std::wstring& str, size_t startIndex, std::uint32_t& codeUnit);
    void appendCodePoint(std::wstring& result, std::uint32_t codePoint);
    std::wstring escapeJsonString(const std::wstring& str);
    bool unescapeJsonString(const std::wstring& str, std::wstring& result);
    bool extractJsonFieldValue(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& value);
    bool extractJsonStringField(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& value);
    bool extractJsonObjectField(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& objectContent);
    bool parseJsonInteger(const std::wstring& jsonValue, int& parsedValue);
    bool parseRgbColorArray(const std::wstring& jsonArray,
                                    std::array<int, 3>& rgbColor);
}