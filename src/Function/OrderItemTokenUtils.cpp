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
 * File: OrderItemTokenUtils.cpp
 */

#include "Function/OrderItemTokenUtils.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/Order.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"

namespace OrderItemTokenUtils
{

bool tryResolveOrderItemToken(const AppData& appData,
                              const std::wstring& operand,
                              std::wstring& resolvedToken)
{
    const std::wstring normalizedOperand = StringUtils::normalizeToken(operand);
    if (normalizedOperand.empty())
    {
        return false;
    }

    if (const Item* exactItem = appData.itemRepository().findByIdentifierToken(normalizedOperand))
    {
        resolvedToken = exactItem->getIdentifierToken();
        return true;
    }

    for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
    {
        const Item& item = appData.itemRepository().at(index);
        const std::wstring normalizedName = StringUtils::normalizeToken(item.getItemName());
        const std::wstring normalizedPluralName = StringUtils::normalizeToken(item.getItemNamePlural());
        if (normalizedOperand == normalizedName || normalizedOperand == normalizedPluralName)
        {
            resolvedToken = item.getIdentifierToken();
            return true;
        }
    }

    return false;
}

bool tryConsumeUnitReference(const std::vector<std::wstring>& tokens, std::size_t& tokenIndex)
{
    if (tokenIndex >= tokens.size())
    {
        return false;
    }

    if (StringUtils::toUpper(tokens[tokenIndex]) == L"NEW")
    {
        ++tokenIndex;
        if (tokenIndex >= tokens.size())
        {
            return false;
        }
    }

    int unitNumber = 0;
    if (!OrderParsingUtils::tryParseIntStrict(tokens[tokenIndex], unitNumber) || unitNumber <= 0)
    {
        return false;
    }

    ++tokenIndex;
    return true;
}

std::set<std::wstring> collectTouchedItemTokensForUnit(const AppData& appData,
                                                       int unitNumber,
                                                       bool isNewUnit)
{
    std::set<std::wstring> touchedTokens;
    const std::vector<Order>* orders = appData.orderRepository().getOrdersForUnit(unitNumber, isNewUnit);
    if (!orders)
    {
        return touchedTokens;
    }

    for (const Order& order : *orders)
    {
        std::vector<std::wstring> tokens;
        std::vector<bool> quoted;
        if (!OrderParsingUtils::tokenizeOrderLine(order.getFullOrderText(), tokens, quoted) || tokens.empty())
        {
            continue;
        }

        std::size_t tokenIndex = 0;
        if (!tokens[0].empty() && tokens[0][0] == L'@')
        {
            if (tokens[0].size() > 1)
            {
                tokens[0] = tokens[0].substr(1);
            }
            else
            {
                ++tokenIndex;
            }
        }

        if (tokenIndex >= tokens.size())
        {
            continue;
        }

        const std::wstring command = StringUtils::toUpper(tokens[tokenIndex]);
        std::size_t itemTokenIndex = std::wstring::npos;

        if (command == L"PRODUCE")
        {
            if ((tokenIndex + 1) < tokens.size())
            {
                int amount = 0;
                const bool hasAmount = OrderParsingUtils::tryParseIntStrict(tokens[tokenIndex + 1], amount) && amount > 0;
                itemTokenIndex = hasAmount ? (tokenIndex + 2) : (tokenIndex + 1);
            }
        }
        else if (command == L"BUY" || command == L"SELL")
        {
            itemTokenIndex = tokenIndex + 2;
        }
        else if (command == L"TRANSPORT" || command == L"DISTRIBUTE")
        {
            std::size_t cursor = tokenIndex + 1;
            if (!tryConsumeUnitReference(tokens, cursor) || cursor >= tokens.size())
            {
                continue;
            }

            const std::wstring quantityToken = StringUtils::toUpper(tokens[cursor]);
            ++cursor;
            if (quantityToken == L"ALL" || quantityToken == L"EXCEPT")
            {
                itemTokenIndex = cursor;
            }
            else
            {
                int quantity = 0;
                if (!OrderParsingUtils::tryParseIntStrict(quantityToken, quantity) || quantity <= 0)
                {
                    continue;
                }
                itemTokenIndex = cursor;
            }
        }

        if (itemTokenIndex == std::wstring::npos || itemTokenIndex >= tokens.size())
        {
            continue;
        }

        std::wstring resolvedToken;
        if (tryResolveOrderItemToken(appData, tokens[itemTokenIndex], resolvedToken) && !resolvedToken.empty())
        {
            touchedTokens.insert(resolvedToken);
        }
    }

    return touchedTokens;
}

} // namespace OrderItemTokenUtils
