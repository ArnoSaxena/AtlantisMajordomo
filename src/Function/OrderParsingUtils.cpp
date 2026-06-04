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
 * File: OrderParsingUtils.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Function/OrderParsingUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/Item.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <cwctype>
#include <set>

namespace OrderParsingUtils
{
bool tryParseIntStrict(const std::wstring& value, int& parsed)
{
  try
  {
    std::size_t consumed = 0;
    parsed = std::stoi(value, &consumed);
    return consumed == value.size();
  }
  catch (...)
  {
    return false;
  }
}

bool tokenizeOrderLine(const std::wstring& line,
                       std::vector<std::wstring>& tokens,
                       std::vector<bool>& tokenWasQuoted)
{
  tokens.clear();
  tokenWasQuoted.clear();

  std::wstring currentToken;
  bool currentWasQuoted = false;
  bool inQuotes = false;
  bool escapeNext = false;

  for (std::size_t index = 0; index < line.size(); ++index)
  {
    const wchar_t ch = line[index];
    if (escapeNext)
    {
      currentToken.push_back(ch);
      escapeNext = false;
      continue;
    }

    if (ch == L'\\')
    {
      if (inQuotes)
      {
        escapeNext = true;
      }
      else
      {
        currentToken.push_back(ch);
      }
      continue;
    }

    if (ch == L'"')
    {
      inQuotes = !inQuotes;
      currentWasQuoted = true;
      continue;
    }

    if (!inQuotes && iswspace(ch))
    {
      if (!currentToken.empty())
      {
        tokens.push_back(currentToken);
        tokenWasQuoted.push_back(currentWasQuoted);
        currentToken.clear();
        currentWasQuoted = false;
      }
      continue;
    }

    currentToken.push_back(ch);
  }

  if (escapeNext || inQuotes)
  {
    return false;
  }

  if (!currentToken.empty())
  {
    tokens.push_back(currentToken);
    tokenWasQuoted.push_back(currentWasQuoted);
  }

  return true;
}

std::wstring normalizeItemTokenForWarning(std::wstring token)
{
  token = StringUtils::trimWhitespace(std::move(token));
  while (!token.empty() && !iswalnum(token.front()))
  {
    token.erase(token.begin());
  }
  while (!token.empty() && !iswalnum(token.back()))
  {
    token.pop_back();
  }
  return StringUtils::toUpper(std::move(token));
}

bool tryExtractOrderKeywordUpper(const std::wstring& orderLine, std::wstring& keyword)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return false;
  }

  std::size_t tokenIndex = 0;
  if (!tokens.front().empty() && tokens.front().front() == L'@')
  {
    if (tokens.front().size() == 1)
    {
      tokenIndex = 1;
    }
    else
    {
      tokens.front().erase(tokens.front().begin());
    }
  }

  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  keyword = StringUtils::toUpper(tokens[tokenIndex]);
  return !keyword.empty();
}

bool tryParseFormNewUnitLine(const std::wstring& orderLine, int& newUnitNumber)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return false;
  }

  std::size_t tokenIndex = 0;
  if (!tokens.front().empty() && tokens.front().front() == L'@')
  {
    if (tokens.front().size() == 1)
    {
      tokenIndex = 1;
    }
    else
    {
      tokens.front().erase(tokens.front().begin());
    }
  }

  if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"FORM")
  {
    return false;
  }

  ++tokenIndex;
  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  int parsedNumber = 0;
  if (!tryParseIntStrict(tokens[tokenIndex], parsedNumber) || parsedNumber <= 0)
  {
    return false;
  }

  newUnitNumber = parsedNumber;
  return true;
}

std::vector<int> extractFormNewUnitNumbers(const std::vector<std::wstring>& orders)
{
  std::vector<int> formUnitNumbers;
  std::set<int> seenUnitNumbers;
  bool insideFormBlock = false;
  int currentFormUnitNumber = 0;

  for (const std::wstring& order : orders)
  {
    if (!insideFormBlock)
    {
      int formUnitNumber = 0;
      if (tryParseFormNewUnitLine(order, formUnitNumber))
      {
        insideFormBlock = true;
        currentFormUnitNumber = formUnitNumber;
      }
      continue;
    }

    const std::wstring trimmed = StringUtils::trimWhitespace(order);
    if (StringUtils::toUpper(trimmed) == L"END")
    {
      if (currentFormUnitNumber > 0 && seenUnitNumbers.insert(currentFormUnitNumber).second)
      {
        formUnitNumbers.push_back(currentFormUnitNumber);
      }
      insideFormBlock = false;
      currentFormUnitNumber = 0;
    }
  }

  return formUnitNumbers;
}

std::vector<std::wstring> filterOrdersIgnoringFormBlocks(const std::vector<std::wstring>& orders)
{
  std::vector<std::wstring> filteredOrders;
  bool insideFormBlock = false;

  for (const std::wstring& order : orders)
  {
    if (!insideFormBlock)
    {
      int formUnitNumber = 0;
      if (tryParseFormNewUnitLine(order, formUnitNumber))
      {
        insideFormBlock = true;
        continue;
      }

      filteredOrders.push_back(order);
      continue;
    }

    const std::wstring trimmed = StringUtils::trimWhitespace(order);
    if (StringUtils::toUpper(trimmed) == L"END")
    {
      insideFormBlock = false;
    }
  }

  return filteredOrders;
}

std::vector<std::wstring> extractFormNewUnitBlock(const std::vector<std::wstring>& orders,
                                                  int formUnitNumber)
{
  std::vector<std::wstring> blockLines;
  bool insideFormBlock = false;

  for (const std::wstring& order : orders)
  {
    if (!insideFormBlock)
    {
      int parsedFormUnitNumber = 0;
      if (tryParseFormNewUnitLine(order, parsedFormUnitNumber) &&
          parsedFormUnitNumber == formUnitNumber)
      {
        insideFormBlock = true;
        blockLines.push_back(order);
      }
      continue;
    }

    blockLines.push_back(order);
    const std::wstring trimmed = StringUtils::trimWhitespace(order);
    if (StringUtils::toUpper(trimmed) == L"END")
    {
      break;
    }
  }

  return blockLines;
}

bool isMonthLongOrderLine(const std::wstring& orderLine)
{
  std::wstring keyword;
  if (!tryExtractOrderKeywordUpper(orderLine, keyword))
  {
    return false;
  }

  const std::vector<std::wstring> monthLongOrders = Commands::getFullMonthOrderKeywords();
  return std::find(monthLongOrders.begin(), monthLongOrders.end(), keyword) != monthLongOrders.end();
}

bool tryParseGiveTakeOrder(const std::wstring& orderLine, WarningGiveTakeOrder& parsedOrder)
{
  std::vector<std::wstring> tokens;
  std::vector<bool> tokenWasQuoted;
  if (!tokenizeOrderLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
  {
    return false;
  }

  std::size_t tokenIndex = 0;
  if (!tokens.front().empty() && tokens.front().front() == L'@')
  {
    if (tokens.front().size() == 1)
    {
      tokenIndex = 1;
    }
    else
    {
      tokens.front().erase(tokens.front().begin());
    }
  }

  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  const std::wstring commandToken = StringUtils::toUpper(tokens[tokenIndex]);
  if (commandToken == L"GIVE")
  {
    parsedOrder.isTake = false;
  }
  else if (commandToken == L"TAKE")
  {
    parsedOrder.isTake = true;
  }
  else
  {
    return false;
  }
  ++tokenIndex;

  if (parsedOrder.isTake)
  {
    // Accept both "TAKE FROM <unit> ..." and "TAKE <unit> ..." variants.
    if (tokenIndex < tokens.size() && StringUtils::toUpper(tokens[tokenIndex]) == L"FROM")
    {
      ++tokenIndex;
    }
  }

  if (tokenIndex >= tokens.size() || !tryParseIntStrict(tokens[tokenIndex], parsedOrder.otherUnitNumber) || parsedOrder.otherUnitNumber <= 0)
  {
    return false;
  }
  ++tokenIndex;

  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  const std::wstring amountToken = StringUtils::toUpper(tokens[tokenIndex]);
  if (amountToken == L"ALL")
  {
    parsedOrder.mode = WarningGiveMode::All;
    ++tokenIndex;
    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    parsedOrder.itemOperand = tokens[tokenIndex];
    parsedOrder.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;

    // Support optional trailing: ALL <item> EXCEPT <n>
    if (tokenIndex < tokens.size())
    {
      if (StringUtils::toUpper(tokens[tokenIndex]) != L"EXCEPT")
      {
        return false;
      }
      ++tokenIndex;

      if (tokenIndex >= tokens.size() || !tryParseIntStrict(tokens[tokenIndex], parsedOrder.exceptQuantity) || parsedOrder.exceptQuantity < 0)
      {
        return false;
      }
      parsedOrder.mode = WarningGiveMode::AllExcept;
      ++tokenIndex;
    }

    return tokenIndex == tokens.size();
  }

  if (amountToken == L"EXCEPT")
  {
    parsedOrder.mode = WarningGiveMode::AllExcept;
    ++tokenIndex;
    if (tokenIndex + 1 >= tokens.size())
    {
      return false;
    }

    if (!tryParseIntStrict(tokens[tokenIndex], parsedOrder.exceptQuantity) || parsedOrder.exceptQuantity < 0)
    {
      return false;
    }
    ++tokenIndex;

    parsedOrder.itemOperand = tokens[tokenIndex];
    parsedOrder.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;
    return tokenIndex == tokens.size();
  }

  parsedOrder.mode = WarningGiveMode::Quantity;
  if (!tryParseIntStrict(tokens[tokenIndex], parsedOrder.quantity) || parsedOrder.quantity <= 0)
  {
    // Alternate form: GIVE <unit> <item> <quantity>
    parsedOrder.itemOperand = tokens[tokenIndex];
    parsedOrder.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;

    if (tokenIndex >= tokens.size() || !tryParseIntStrict(tokens[tokenIndex], parsedOrder.quantity) || parsedOrder.quantity <= 0)
    {
      return false;
    }
    ++tokenIndex;
    return tokenIndex == tokens.size();
  }
  ++tokenIndex;

  if (tokenIndex >= tokens.size())
  {
    return false;
  }

  parsedOrder.itemOperand = tokens[tokenIndex];
  parsedOrder.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
  ++tokenIndex;

  return tokenIndex == tokens.size();
}

bool tryResolveItemTokenForWarning(const AppData& appData,
                                   const std::wstring& operand,
                                   bool operandWasQuoted,
                                   std::wstring& resolvedToken)
{
  std::wstring normalized = normalizeItemTokenForWarning(operand);
  if (!normalized.empty() && appData.itemRepository().findByIdentifierToken(normalized))
  {
    resolvedToken = normalized;
    return true;
  }

  std::wstring targetName = StringUtils::trimWhitespace(operand);
  bool treatAsQuotedName = operandWasQuoted;
  if (targetName.size() >= 2 && targetName.front() == L'"' && targetName.back() == L'"')
  {
    targetName = StringUtils::trimWhitespace(targetName.substr(1, targetName.size() - 2));
    treatAsQuotedName = true;
  }

  if (targetName.empty())
  {
    return false;
  }

  // Accept explicit quoted names and also unquoted names.
  (void)treatAsQuotedName;

  const std::wstring normalizedName = StringUtils::toUpper(targetName);
  for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
  {
    const Item& item = appData.itemRepository().at(index);
    if (StringUtils::toUpper(StringUtils::trimWhitespace(item.getItemName())) == normalizedName)
    {
      resolvedToken = normalizeItemTokenForWarning(item.getIdentifierToken());
      return !resolvedToken.empty();
    }
  }

  return false;
}
}
