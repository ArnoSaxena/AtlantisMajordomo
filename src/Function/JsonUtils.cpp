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
 * File: JsonUtils.cpp
 */
 
#include "Function/JsonUtils.hpp"
#include "Function/StringUtils.hpp"

#include <array>
#include <sstream>
#include <string>
#include <iomanip>

namespace JsonUtils
{
  void appendUnicodeEscape(std::wstring& result, unsigned int codeUnit)
  {
    std::wstringstream stream;
    stream << L"\\u" << std::uppercase << std::hex << std::setw(4) << std::setfill(L'0') << codeUnit;
    result += stream.str();
  }

  bool tryParseHex4(const std::wstring& str, size_t startIndex, std::uint32_t& codeUnit)
  {
    if (startIndex + 4 > str.size())
    {
      return false;
    }

    std::uint32_t value = 0;
    for (size_t index = startIndex; index < startIndex + 4; ++index)
    {
      const wchar_t ch = str[index];
      value <<= 4;
      if (ch >= L'0' && ch <= L'9')
      {
        value |= static_cast<std::uint32_t>(ch - L'0');
      }
      else if (ch >= L'a' && ch <= L'f')
      {
        value |= static_cast<std::uint32_t>(10 + (ch - L'a'));
      }
      else if (ch >= L'A' && ch <= L'F')
      {
        value |= static_cast<std::uint32_t>(10 + (ch - L'A'));
      }
      else
      {
        return false;
      }
    }

    codeUnit = value;
    return true;
  }

  void appendCodePoint(std::wstring& result, std::uint32_t codePoint)
  {
    if constexpr (sizeof(wchar_t) >= 4)
    {
      result.push_back(static_cast<wchar_t>(codePoint));
      return;
    }

    if (codePoint <= 0xFFFF)
    {
      result.push_back(static_cast<wchar_t>(codePoint));
      return;
    }

    codePoint -= 0x10000;
    const std::uint32_t highSurrogate = 0xD800 + (codePoint >> 10);
    const std::uint32_t lowSurrogate = 0xDC00 + (codePoint & 0x3FF);
    result.push_back(static_cast<wchar_t>(highSurrogate));
    result.push_back(static_cast<wchar_t>(lowSurrogate));
  }

  std::wstring escapeJsonString(const std::wstring& str)
  {
    std::wstring result;
    for (wchar_t ch : str)
    {
      switch (ch)
      {
        case L'"':  result += L"\\\""; break;
        case L'\\': result += L"\\\\"; break;
        case L'\b': result += L"\\b";  break;
        case L'\f': result += L"\\f";  break;
        case L'\n': result += L"\\n";  break;
        case L'\r': result += L"\\r";  break;
        case L'\t': result += L"\\t";  break;
        default:
        {
          const unsigned int codeUnit = static_cast<unsigned int>(ch);
          if (codeUnit < 0x20 || (codeUnit >= 0xD800 && codeUnit <= 0xDFFF))
          {
            appendUnicodeEscape(result, codeUnit);
          }
          else
          {
            result += ch;
          }
          break;
        }
      }
    }
    return result;
  }

  bool unescapeJsonString(const std::wstring& str, std::wstring& result)
  {
    result.clear();
    for (size_t i = 0; i < str.length(); ++i)
    {
      const wchar_t current = str[i];
      if (current < 0x20)
      {
        return false;
      }

      if (current != L'\\')
      {
        result += current;
        continue;
      }

      if (i + 1 >= str.length())
      {
        return false;
      }

      ++i;
      const wchar_t escapeType = str[i];
      switch (escapeType)
      {
        case L'"': result += L'"'; break;
        case L'\\': result += L'\\'; break;
        case L'/': result += L'/'; break;
        case L'b': result += L'\b'; break;
        case L'f': result += L'\f'; break;
        case L'n': result += L'\n'; break;
        case L'r': result += L'\r'; break;
        case L't': result += L'\t'; break;
        case L'u':
        {
          std::uint32_t codeUnit = 0;
          if (!tryParseHex4(str, i + 1, codeUnit))
          {
            return false;
          }
          i += 4;

          if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF)
          {
            if (i + 6 >= str.length() || str[i + 1] != L'\\' || str[i + 2] != L'u')
            {
              return false;
            }

            std::uint32_t lowSurrogate = 0;
            if (!tryParseHex4(str, i + 3, lowSurrogate))
            {
              return false;
            }
            if (lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF)
            {
              return false;
            }

            const std::uint32_t codePoint = 0x10000 + (((codeUnit - 0xD800) << 10) | (lowSurrogate - 0xDC00));
            appendCodePoint(result, codePoint);
            i += 6;
          }
          else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF)
          {
            return false;
          }
          else
          {
            appendCodePoint(result, codeUnit);
          }
          break;
        }
        default:
          return false;
      }
    }

    return true;
  }



  bool extractJsonStringField(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& value)
{
  std::wstring jsonValue;
  if (!extractJsonFieldValue(json, fieldName, jsonValue))
  {
    return false;
  }

  if (jsonValue.size() < 2 || jsonValue.front() != L'"' || jsonValue.back() != L'"')
  {
    return false;
  }
  return JsonUtils::unescapeJsonString(jsonValue.substr(1, jsonValue.size() - 2), value);
}

  bool extractJsonFieldValue(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& value)
{
  const std::wstring token = L"\"" + fieldName + L"\"";
  const size_t keyPos = json.find(token);
  if (keyPos == std::wstring::npos)
  {
    return false;
  }

  const size_t colonPos = json.find(L':', keyPos + token.size());
  if (colonPos == std::wstring::npos)
  {
    return false;
  }

  size_t valueStart = colonPos + 1;
  while (valueStart < json.size()
        && (json[valueStart] == L' ' || json[valueStart] == L'\t'
            || json[valueStart] == L'\n' || json[valueStart] == L'\r'))
  {
    ++valueStart;
  }

  if (valueStart >= json.size())
  {
    return false;
  }

  size_t valueEnd = valueStart;
  if (json[valueStart] == L'{')
  {
    int braceDepth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = valueStart; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        inString = !inString;
        continue;
      }
      if (inString)
      {
        continue;
      }

      if (ch == L'{')
      {
        ++braceDepth;
      }
      else if (ch == L'}')
      {
        --braceDepth;
        if (braceDepth == 0)
        {
          valueEnd = i + 1;
          break;
        }
      }
    }
  }
  else if (json[valueStart] == L'[')
  {
    int bracketDepth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = valueStart; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        inString = !inString;
        continue;
      }
      if (inString)
      {
        continue;
      }

      if (ch == L'[')
      {
        ++bracketDepth;
      }
      else if (ch == L']')
      {
        --bracketDepth;
        if (bracketDepth == 0)
        {
          valueEnd = i + 1;
          break;
        }
      }
    }
  }
  else if (json[valueStart] == L'"')
  {
    bool escaped = false;
    for (size_t i = valueStart + 1; i < json.size(); ++i)
    {
      const wchar_t ch = json[i];
      if (escaped)
      {
        escaped = false;
        continue;
      }
      if (ch == L'\\')
      {
        escaped = true;
        continue;
      }
      if (ch == L'"')
      {
        valueEnd = i + 1;
        break;
      }
    }
  }
  else
  {
    valueEnd = valueStart;
    while (valueEnd < json.size() && json[valueEnd] != L',' && json[valueEnd] != L'}')
    {
      ++valueEnd;
    }
  }

  if (valueEnd <= valueStart || valueEnd > json.size())
  {
    return false;
  }

  value = StringUtils::trimWhitespace(json.substr(valueStart, valueEnd - valueStart));
  return !value.empty();
}

  bool extractJsonObjectField(const std::wstring& json,
                                      const std::wstring& fieldName,
                                      std::wstring& objectContent)
{
  std::wstring jsonValue;
  if (!extractJsonFieldValue(json, fieldName, jsonValue))
  {
    return false;
  }

  if (jsonValue.size() < 2 || jsonValue.front() != L'{' || jsonValue.back() != L'}')
  {
    return false;
  }

  objectContent = jsonValue.substr(1, jsonValue.size() - 2);
  return true;
}

  bool parseJsonInteger(const std::wstring& jsonValue, int& parsedValue)
{
  try
  {
    parsedValue = std::stoi(StringUtils::trimWhitespace(jsonValue));
    return true;
  }
  catch (...)
  {
    return false;
  }
}

  bool parseRgbColorArray(const std::wstring& jsonArray,
                                  std::array<int, 3>& rgbColor)
{
  const std::wstring trimmedArray = StringUtils::trimWhitespace(jsonArray);
  if (trimmedArray.size() < 2 || trimmedArray.front() != L'[' || trimmedArray.back() != L']')
  {
    return false;
  }

  const std::wstring values = trimmedArray.substr(1, trimmedArray.size() - 2);
  size_t start = 0;
  for (int channel = 0; channel < 3; ++channel)
  {
    const size_t commaPos = values.find(L',', start);
    size_t end = std::wstring::npos;

    if (channel < 2)
    {
      if (commaPos == std::wstring::npos)
      {
        return false;
      }
      end = commaPos;
    }
    else
    {
      end = values.size();
    }

    const std::wstring token = StringUtils::trimWhitespace(values.substr(start, end - start));
    if (token.empty())
    {
      return false;
    }

    try
    {
      rgbColor[channel] = std::stoi(token);
    }
    catch (...)
    {
      return false;
    }

    if (channel < 2)
    {
      start = commaPos + 1;
    }
  }

  return true;
}

}