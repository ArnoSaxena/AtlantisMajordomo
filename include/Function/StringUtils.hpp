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

#include <array>
#include <map>
#include <string>
#include <vector>

namespace StringUtils
{
    // Strips a leading UTF-8/UTF-16 BOM character if present.
    std::wstring trimBom(std::wstring value);
    // Trims leading and trailing whitespace.
    std::wstring trimWhitespace(const std::wstring& value);
    // Splits a comma-separated string into trimmed tokens.
    std::vector<std::wstring> splitByComma(const std::wstring& text);
    // Joins lines with the given separator (default CRLF).
    std::wstring joinLines(const std::vector<std::wstring>& lines, const std::wstring& separator = L"\r\n");
    // Splits text into non-empty, trimmed lines, treating both CRLF and LF as line endings.
    std::vector<std::wstring> splitLines(const std::wstring& text);
    // Returns a lowercased copy of the string.
    std::wstring toLower(std::wstring value);
    // Returns an uppercased copy of the string.
    std::wstring toUpper(std::wstring value);
    // Trims whitespace, strips leading/trailing non-alphanumeric characters, and uppercases; used to canonicalize item/skill tokens for comparison.
    std::wstring normalizeToken(std::wstring value);
    // Parses the whole trimmed string as an int, returning 0 if it is not a valid number.
    int parseIntSafe(const std::wstring& text);
    // Finds the first leading numeric run (optionally after non-numeric prefix characters) and parses it; returns false if none is found.
    bool tryParseLeadingInteger(const std::wstring& text, long long& value);
    // Formats an RGB triplet as "R, G, B".
    std::wstring formatRgbColor(const std::array<int, 3>& rgb);
    // Parses an RGB triplet with values in the inclusive range 0 through 255.
    bool tryParseRgbColor(const std::wstring& text, std::array<int, 3>& rgb);
    // Normalizes lone LF line endings to CRLF, leaving existing CRLF pairs untouched.
    std::wstring toCRLF(const std::wstring& input);
    // Formats a string->int map as CRLF-separated "token:amount" entries.
    std::wstring formatStringIntMap(const std::map<std::wstring, int>& data);
    // Parses "token:amount" entries (one per line) back into a string->int map.
    std::map<std::wstring, int> parseStringIntMap(const std::wstring& text);
}