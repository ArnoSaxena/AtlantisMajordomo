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
 * File: StringUtils.cpp
 */
 
#include "Function/StringUtils.hpp"

#include <string>
#include <cwctype>
namespace StringUtils
{
    std::wstring trimWhitespace(const std::wstring& value)
    {
        const auto isNotSpace = [](wchar_t c) {
            return !std::iswspace(c);
        };

        auto first = std::find_if(value.begin(), value.end(), isNotSpace);
        if (first == value.end())
            return L"";

        auto last = std::find_if(value.rbegin(), value.rend(), isNotSpace).base();

        return std::wstring(first, last);
    }

    std::wstring toLower(std::wstring value)
    {
        for (auto& ch : value)
        {
            ch = static_cast<wchar_t>(std::towlower(ch));
        }
        return value;
    }

    std::wstring toUpper(std::wstring value)
    {
        for (auto& ch : value)
        {
            ch = static_cast<wchar_t>(std::towupper(ch));
        }
        return value;
    }

    int parseIntSafe(const std::wstring& text)
    {
        try
        {
            return std::stoi(trimWhitespace(text));
        }
        catch (...)
        {
            return 0;
        }
    }
}