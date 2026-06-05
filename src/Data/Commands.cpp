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
 * File: Commands.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "Data/Commands.hpp"

#include "Data/AppData.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/Skill.hpp"
#include "Data/Unit.hpp"
#include "Data/UnitNew.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
  /**
  * @brief Default set of commands that consume a unit's full month.
  */
  const std::vector<std::wstring> kDefaultFullMonthOrderKeywords = {
    L"ADVANCE",
    L"BUILD",
    L"ENTERTAIN",
    L"MOVE",
    L"PILLAGE",
    L"PRODUCE",
    L"SAIL",
    L"STUDY",
    L"TAX",
    L"TEACH",
    L"WORK"
  };

  /**
  * @brief Runtime-configurable month-long keyword list used by simulation and warnings.
  */
  std::vector<std::wstring> gFullMonthOrderKeywords = kDefaultFullMonthOrderKeywords;

  /**
  * @brief Quantity semantics for GIVE/TAKE-like commands.
  */
  enum class GiveMode
  {
    Quantity, /**< Transfer exactly N items. */
    All, /**< Transfer all available items. */
    AllExcept /**< Transfer all except N items. */
  };

  /**
  * @brief GIVE command category recognized by the parser.
  */
  enum class GiveKind
  {
    ItemTransfer, /**< Standard item transfer between units. */
    UnitTransfer /**< Unit ownership transfer syntax (recognized but not simulated). */
  };

  /**
  * @brief Parsed GIVE/TAKE command payload.
  */
  struct GiveCommand
  {
    bool isTake { false }; /**< True for TAKE, false for GIVE. */
    GiveKind kind { GiveKind::ItemTransfer }; /**< Parsed GIVE/TAKE command kind. */
    int receiverUnitId { 0 }; /**< Target unit number as parsed from the order. */
    bool receiverIsNewUnit { false }; /**< True when the target is a NEW unit syntax. */
    int transferredUnitId { 0 }; /**< Unit id for GIVE UNIT syntax when present. */
    std::wstring itemToken; /**< Resolved normalized item token. */
    std::wstring itemOperand; /**< Raw item operand from the order text. */
    bool itemOperandWasQuoted { false }; /**< True when operand was originally quoted. */
    GiveMode mode { GiveMode::Quantity }; /**< Quantity mode for transfer amount. */
    int quantity { 0 }; /**< Parsed explicit amount for Quantity mode. */
    int exceptQuantity { 0 }; /**< Parsed keep amount for AllExcept mode. */
  };

  /**
  * @brief Parsed STUDY command payload.
  */
  struct StudyCommand
  {
    std::wstring skillToken; /**< Raw skill operand used to resolve the skill to study. */
  };

  /**
  * @brief Parsed TEACH command payload.
  */
  struct TeachCommand
  {
    std::vector<int> studentUnitNumbers; /**< List of unit numbers to teach. */
  };

  /**
  * @brief Parsed PRODUCE command payload.
  */
  struct ProduceCommand
  {
    std::wstring itemOperand; /**< Raw producible item operand from the command. */
    bool itemOperandWasQuoted { false }; /**< True when item operand was quoted. */
    std::wstring itemToken; /**< Resolved normalized produced item token. */
    bool hasRequestedAmount { false }; /**< Whether the command requested an explicit cap. */
    int requestedAmount { 0 }; /**< Explicit requested cap for production output. */
  };

  /**
  * @brief Parsed TRANSPORT/DISTRIBUTE command payload.
  */
  struct TransportCommand
  {
    int targetUnitId { 0 }; /**< Destination unit number. */
    bool targetIsNewUnit { false }; /**< True when the destination is a NEW unit syntax. */
    std::wstring itemOperand; /**< Raw item operand from the order text. */
    bool itemOperandWasQuoted { false }; /**< True when item operand was quoted. */
    std::wstring itemToken; /**< Resolved normalized item token. */
    GiveMode mode { GiveMode::Quantity }; /**< Quantity mode for transport amount. */
    int quantity { 0 }; /**< Parsed explicit amount for Quantity mode. */
    int exceptQuantity { 0 }; /**< Parsed keep amount for AllExcept mode. */
  };

  /**
  * @brief Parsed BUY/SELL command payload.
  */
  struct TradeCommand
  {
    bool isBuy { true }; /**< True for BUY, false for SELL. */
    std::wstring itemOperand; /**< Raw trade item operand from the order text. */
    bool itemOperandWasQuoted { false }; /**< True when item operand was quoted. */
    std::wstring itemToken; /**< Resolved normalized trade item token. */
    GiveMode mode { GiveMode::Quantity }; /**< Quantity mode (Quantity or All). */
    int quantity { 0 }; /**< Parsed explicit amount for Quantity mode. */
  };

  /**
  * @brief Parsed MOVE/ADVANCE command payload.
  */
  struct MoveCommand
  {
    std::vector<std::wstring> directions; /**< Direction sequence (N, S, NE, NW, SE, SW). */
    int targetStructureId { -1 }; /**< Final structure target: -1 none, -2 IN, or explicit id. */
  };

  /**
  * @brief Marker payload for LEAVE command.
  */
  struct LeaveCommand
  {};

  /**
  * @brief Parsed ENTER command payload.
  */
  struct EnterCommand
  {
    int structureId { 0 }; /**< Destination structure id. */
  };

  /**
  * @brief Parsed SAIL command payload.
  */
  struct SailCommand
  {
    std::vector<std::wstring> directions; /**< Direction sequence (N, S, NE, NW, SE, SW). */
  };

  /**
  * @brief Parsed ENTERTAIN command payload.
  */
  struct EntertainCommand
  {};

  /**
  * @brief Parsed NAME UNIT command payload.
  */
  struct NameUnitCommand
  {
    std::wstring newName; /**< Requested new unit name. */
  };

  /**
  * @brief Command categories understood by simulation.
  */
  enum class SimulatedCommandKind
  {
    GiveTake, /**< GIVE/TAKE transfer effects. */
    Transport, /**< TRANSPORT/DISTRIBUTE effects. */
    Trade, /**< BUY/SELL effects. */
    Tax, /**< TAX income effects. */
    Sell, /**< Reserved legacy value for SELL phase routing. */
    Buy, /**< Reserved legacy value for BUY phase routing. */
    Move, /**< MOVE/ADVANCE command category. */
    Sail, /**< SAIL command category. */
    Produce, /**< PRODUCE command effects. */
    Entertain, /**< ENTERTAIN income effects. */
    Study, /**< STUDY silver-consumption effects. */
    Teach, /**< TEACH command effects. */
    NameUnit, /**< NAME UNIT command effects. */
    Work, /**< WORK wage effects. */
    Leave, /**< LEAVE command category. */
    Enter, /**< ENTER command category. */
    NoEffect /**< Parsed but intentionally ignored command. */
  };

  /**
  * @brief Union-like container that stores a parsed command and its payload.
  */
  struct ParsedOrderCommand
  {
    SimulatedCommandKind kind { SimulatedCommandKind::NoEffect }; /**< Parsed command category. */
    GiveCommand give; /**< Parsed payload for GiveTake kind. */
    TransportCommand transport; /**< Parsed payload for Transport kind. */
    TradeCommand trade; /**< Parsed payload for Trade kind. */
    MoveCommand move; /**< Parsed payload for Move kind. */
    SailCommand sail; /**< Parsed payload for Sail kind. */
    ProduceCommand produce; /**< Parsed payload for Produce kind. */
    EntertainCommand entertain; /**< Parsed payload for Entertain kind. */
    StudyCommand study; /**< Parsed payload for Study kind. */
    TeachCommand teach; /**< Parsed payload for Teach kind. */
    NameUnitCommand nameUnit; /**< Parsed payload for NameUnit kind. */
    LeaveCommand leave; /**< Parsed payload for Leave kind. */
    EnterCommand enter; /**< Parsed payload for Enter kind. */
  };

  /**
  * @brief Mutable simulation state for all units/economy in and near a region.
  */
  struct RegionCommandSimulation
  {
    std::map<int, const Unit*> regionUnits; /**< Units physically in the simulated region. */
    std::map<int, const UnitNew*> regionNewUnits; /**< NEW units physically in the simulated region. */
    std::map<int, const Unit*> nearbyUnits; /**< Units within transport range (distance <= 2). */
    std::map<int, const UnitNew*> nearbyNewUnits; /**< NEW units within transport range (distance <= 2). */
    std::map<int, std::map<std::wstring, int>> itemCountsByUnit; /**< Working item counts per nearby normal unit. */
    std::map<int, std::map<std::wstring, int>> itemCountsByNewUnit; /**< Working item counts per nearby NEW unit. */
    std::map<int, std::map<std::wstring, int>> skillsByUnit; /**< Working skill days per nearby normal unit. */
    std::map<int, std::map<std::wstring, int>> skillsByNewUnit; /**< Working skill days per nearby NEW unit. */
    std::map<int, std::wstring> unitNamesByUnit; /**< Working display name per nearby normal unit. */
    std::map<int, std::wstring> unitNamesByNewUnit; /**< Working display name per nearby NEW unit. */
    std::map<std::wstring, int> remainingResources; /**< Remaining natural resources after production. */
    std::map<std::wstring, std::pair<int, int>> remainingForSale; /**< Remaining for-sale entries {amount, price}. */
    std::map<std::wstring, std::pair<int, int>> remainingWanted; /**< Remaining wanted entries {amount, price}. */
    int remainingTaxableIncome { 0 }; /**< Remaining taxable income pool. */
    int remainingEntertainment { 0 }; /**< Remaining entertainment income pool. */
    int remainingWorkWages { 0 }; /**< Remaining wages pool for WORK. */
  };

  /**
  * @brief Phase ordering used to execute commands deterministically.
  */
  enum class CommandPhase
  {
    GiveTake, /**< Resolve GIVE/TAKE exchanges first. */
    Tax, /**< Resolve TAX income. */
    Sell, /**< Resolve SELL market operations. */
    Buy, /**< Resolve BUY market operations. */
    Move, /**< Reserved for movement-only sequencing. */
    Produce, /**< Resolve PRODUCE and STUDY costs. */
    Entertain, /**< Resolve ENTERTAIN income. */
    Name, /**< Resolve NAME UNIT updates. */
    Work, /**< Resolve WORK wages. */
    Transport /**< Resolve TRANSPORT/DISTRIBUTE transfers last. */
  };

  /**
  * @brief Parsed command entry queued for phase-based execution.
  */
  struct ScheduledCommand
  {
    int originUnitNumber { 0 }; /**< Unit number that issued the command. */
    const Unit* originUnit { nullptr }; /**< Unit snapshot pointer from repository (null for new units). */
    bool originIsLocal { false }; /**< True when unit is in the exact target region. */
    ParsedOrderCommand parsedCommand; /**< Parsed command payload to execute. */
    bool isNewUnit { false }; /**< True when the origin is a UnitNew (FORM block unit). */
  };

  /** @brief Normalizes token formatting and letter case for comparisons. */
  std::wstring normalizeItemToken(std::wstring token);

  /**
  * @brief Resolves the per-man monthly silver study cost for a skill.
  *
  * Prefers level 1 cost and falls back to the first positive level cost.
  */
  int resolveStudyCostPerManMonth(const Skill& skill)
  {
    return skill.getStudyCost();
  }

  /**
  * @brief Resolves a skill using identifier token first, then by display name.
  */
  const Skill* findSkillByTokenOrNameNormalized(const AppData& appData,
                                                const std::wstring& operand)
  {
    const std::wstring trimmedOperand = StringUtils::trimWhitespace(operand);
    const std::wstring normalizedToken = normalizeItemToken(trimmedOperand);
    if (!normalizedToken.empty())
    {
      if (const Skill* exactByIdentifier = appData.skillRepository().findByIdentifier(normalizedToken))
      {
        return exactByIdentifier;
      }

      for (std::size_t skillIndex = 0; skillIndex < appData.skillRepository().size(); ++skillIndex)
      {
        const Skill& candidate = appData.skillRepository().at(skillIndex);
        if (normalizeItemToken(candidate.getIdentifierToken()) == normalizedToken)
        {
          return &candidate;
        }
      }
    }

    const std::wstring normalizedName = StringUtils::toUpper(trimmedOperand);
    for (std::size_t skillIndex = 0; skillIndex < appData.skillRepository().size(); ++skillIndex)
    {
      const Skill& candidate = appData.skillRepository().at(skillIndex);
      if (StringUtils::toUpper(StringUtils::trimWhitespace(candidate.getName())) == normalizedName)
      {
        return &candidate;
      }
    }

    // Last-resort fallback for short aliases (e.g. "comb" -> "Combat"),
    // but only when the match is unique to avoid ambiguous resolution.
    if (!normalizedToken.empty())
    {
      const Skill* uniquePrefixMatch = nullptr;
      for (std::size_t skillIndex = 0; skillIndex < appData.skillRepository().size(); ++skillIndex)
      {
        const Skill& candidate = appData.skillRepository().at(skillIndex);
        const std::wstring candidateIdentifier = normalizeItemToken(candidate.getIdentifierToken());
        const std::wstring candidateName = normalizeItemToken(candidate.getName());

        const bool prefixMatchesIdentifier =
          candidateIdentifier.size() >= normalizedToken.size() &&
          candidateIdentifier.rfind(normalizedToken, 0) == 0;
        const bool prefixMatchesName =
          candidateName.size() >= normalizedToken.size() &&
          candidateName.rfind(normalizedToken, 0) == 0;

        if (!prefixMatchesIdentifier && !prefixMatchesName)
        {
          continue;
        }

        if (!uniquePrefixMatch)
        {
          uniquePrefixMatch = &candidate;
          continue;
        }

        // Ambiguous prefix: do not guess.
        return nullptr;
      }

      if (uniquePrefixMatch)
      {
        return uniquePrefixMatch;
      }
    }

    return nullptr;
  }

  /**
  * @brief Normalizes a token by trimming punctuation/space and upper-casing.
  */
  std::wstring normalizeItemToken(std::wstring token)
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

  /**
  * @brief Parses an integer and ensures the whole input is consumed.
  */
  bool tryParseInt(const std::wstring& text, int& value)
  {
    try
    {
      std::size_t parsedLength = 0;
      int parsed = std::stoi(text, &parsedLength);
      if (parsedLength != text.size())
      {
        return false;
      }

      value = parsed;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  /**
  * @brief Parses a unit reference token, allowing optional NEW prefix before the number.
  */
  bool tryParseUnitReference(const std::vector<std::wstring>& tokens,
                             std::size_t& tokenIndex,
                             int& unitNumber,
                             bool& isNewUnit)
  {
    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    const std::wstring upperToken = StringUtils::toUpper(tokens[tokenIndex]);
    if (upperToken == L"NEW")
    {
      ++tokenIndex;
      if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], unitNumber) || unitNumber <= 0)
      {
        return false;
      }

      isNewUnit = true;
      ++tokenIndex;
      return true;
    }

    if (!tryParseInt(tokens[tokenIndex], unitNumber) || unitNumber <= 0)
    {
      return false;
    }

    isNewUnit = false;
    ++tokenIndex;
    return true;
  }

  /**
  * @brief Tokenizes an order line, preserving quoted-token metadata.
  */
  bool tokenizeCommandLine(const std::wstring& line,
                          std::vector<std::wstring>& tokens,
                          std::vector<bool>& tokenWasQuoted)
  {
    tokens.clear();
    tokenWasQuoted.clear();

    const std::wstring input =  StringUtils::trimWhitespace(line);
    if (input.empty())
    {
      return true;
    }

    std::wstring currentToken;
    bool currentWasQuoted = false;
    bool inQuotes = false;
    bool escapeNext = false;

    for (std::size_t i = 0; i < input.size(); ++i)
    {
      const wchar_t ch = input[i];

      if (escapeNext)
      {
        currentToken.push_back(ch);
        escapeNext = false;
        continue;
      }

      if (ch == L'\\')
      {
        // In quoted strings, keep standard escaping behavior.
        if (inQuotes)
        {
          escapeNext = true;
          continue;
        }

        // Be tolerant of legacy backslash-quoted values like \"silver\".
        if (i + 1 < input.size() && input[i + 1] == L'"')
        {
          continue;
        }

        currentToken.push_back(ch);
        continue;
      }

      if (ch == L'"')
      {
        inQuotes = !inQuotes;
        currentWasQuoted = true;
        continue;
      }

      if (!inQuotes && ch == L';')
      {
        // Inline comments: ignore everything after ';' outside quotes.
        break;
      }

      if (!inQuotes && ch == L'@')
      {
        // Support trailing inline comments like "TAX @ note".
        // Preserve legacy leading @command syntax by keeping parsing alive
        // when '@' is the first non-space character.
        const bool isLeadingCommandPrefix = tokens.empty() && currentToken.empty();
        if (isLeadingCommandPrefix)
        {
          continue;
        }

        break;
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

  /**
  * @brief Parses a comma-separated keyword list into normalized tokens.
  */
  std::vector<std::wstring> parseCsvKeywordList(const std::wstring& csvKeywords)
  {
    std::vector<std::wstring> parsedKeywords;
    std::wstring current;

    auto flushToken = [&parsedKeywords](std::wstring token)
    {
      token = StringUtils::trimWhitespace(std::move(token));
      if (token.empty())
      {
        return;
      }

      parsedKeywords.push_back(StringUtils::toUpper(std::move(token)));
    };

    for (const wchar_t ch : csvKeywords)
    {
      if (ch == L',')
      {
        flushToken(current);
        current.clear();
        continue;
      }

      current.push_back(ch);
    }

    flushToken(current);
    return parsedKeywords;
  }

  /**
  * @brief Joins normalized keywords into a comma-separated string.
  */
  std::wstring joinCsvKeywordList(const std::vector<std::wstring>& keywords)
  {
    std::wstring csv;
    for (std::size_t index = 0; index < keywords.size(); ++index)
    {
      if (index > 0)
      {
        csv += L", ";
      }
      csv += keywords[index];
    }

    return csv;
  }

  /**
  * @brief Extracts and upper-cases the first command keyword from an order line.
  */
  bool tryExtractCommandKeyword(const std::wstring& orderLine, std::wstring& commandKeyword)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(orderLine, tokens, tokenWasQuoted) || tokens.empty())
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

    commandKeyword = StringUtils::toUpper(tokens[tokenIndex]);
    return !commandKeyword.empty();
  }

  /**
  * @brief Returns true when an order keyword is configured as month-long.
  */
  bool isFullMonthOrderLine(const std::wstring& orderLine)
  {
    std::wstring commandKeyword;
    if (!tryExtractCommandKeyword(orderLine, commandKeyword))
    {
      return false;
    }

    for (const std::wstring& keyword : gFullMonthOrderKeywords)
    {
      if (commandKeyword == keyword)
      {
        return true;
      }
    }

    return false;
  }

  /**
  * @brief Resolves an item operand to a normalized repository token.
  *
  * Resolution tries identifier token first, then falls back to item name matching.
  */
  bool tryResolveItemToken(const AppData& appData,
                          const std::wstring& operand,
                          bool operandWasQuoted,
                          std::wstring& resolvedToken)
  {
    std::wstring normalizedOperand = normalizeItemToken(operand);
    if (!normalizedOperand.empty() && appData.itemRepository().findByIdentifierToken(normalizedOperand))
    {
      resolvedToken = normalizedOperand;
      return true;
    }

    if (!normalizedOperand.empty())
    {
      for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
      {
        const Item& candidate = appData.itemRepository().at(index);
        if (normalizeItemToken(candidate.getIdentifierToken()) == normalizedOperand)
        {
          resolvedToken = normalizeItemToken(candidate.getIdentifierToken());
          return !resolvedToken.empty();
        }
      }
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

    // Accept explicit quoted names and also unquoted names for convenience.
    // This keeps command simulation tolerant of orders like: GIVE 123 10 silver
    // while preserving token-first behavior when identifiers are used.
    (void)treatAsQuotedName;

    const std::wstring normalizedTargetName = StringUtils::toUpper(targetName);

    for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
    {
      const Item& item = appData.itemRepository().at(index);
      if (StringUtils::toUpper(StringUtils::trimWhitespace(item.getItemName())) == normalizedTargetName)
      {
        resolvedToken = normalizeItemToken(item.getIdentifierToken());
        return !resolvedToken.empty();
      }
    }

    return false;
  }

  /**
  * @brief Normalizes inventory keys to upper-case tokens and removes non-positive entries.
  */
  std::map<std::wstring, int> normalizeItemCountsMap(const std::map<std::wstring, int>& itemCounts)
  {
    std::map<std::wstring, int> normalized;
    for (const auto& [token, amount] : itemCounts)
    {
      if (amount <= 0)
      {
        continue;
      }

      const std::wstring normalizedToken = normalizeItemToken(token);
      if (normalizedToken.empty())
      {
        continue;
      }

      normalized[normalizedToken] += amount;
    }

    return normalized;
  }

  /**
  * @brief Parses a GIVE/TAKE command line into a structured payload.
  */
  bool tryParseGiveCommand(const std::wstring& line, GiveCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    const std::wstring verbToken = StringUtils::toUpper(tokens[tokenIndex]);
    if (verbToken == L"GIVE")
    {
      command.isTake = false;
    }
    else if (verbToken == L"TAKE")
    {
      command.isTake = true;
    }
    else
    {
      return false;
    }
    ++tokenIndex;

    if (command.isTake)
    {
      // Only accept the explicit "TAKE FROM <unit> ..." syntax.
      if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"FROM")
      {
        return false;
      }
      ++tokenIndex;
    }

    if (!tryParseUnitReference(tokens, tokenIndex, command.receiverUnitId, command.receiverIsNewUnit))
    {
      return false;
    }

    // GIVE [unit id] UNIT [unit id] - valid syntax in Atlantis, but unit transfer
    // effects are not modeled yet. Recognize and ignore safely.
    if (!command.isTake && tokenIndex < tokens.size() && StringUtils::toUpper(tokens[tokenIndex]) == L"UNIT")
    {
      ++tokenIndex;
      if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], command.transferredUnitId) || command.transferredUnitId <= 0)
      {
        return false;
      }
      ++tokenIndex;

      command.kind = GiveKind::UnitTransfer;
      return tokenIndex == tokens.size();
    }

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    const std::wstring quantityToken = tokens[tokenIndex++];
    const std::wstring upperQuantityToken = StringUtils::toUpper(quantityToken);
    if (upperQuantityToken == L"ALL")
    {
      command.mode = GiveMode::All;
      command.quantity = 0;

      if (tokenIndex >= tokens.size())
      {
        return false;
      }
      command.itemOperand = tokens[tokenIndex];
      command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
      ++tokenIndex;

      if (tokenIndex < tokens.size())
      {
        if (StringUtils::toUpper(tokens[tokenIndex]) != L"EXCEPT")
        {
          return false;
        }
        ++tokenIndex;

        if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], command.exceptQuantity) || command.exceptQuantity < 0)
        {
          return false;
        }
        ++tokenIndex;

        command.mode = GiveMode::AllExcept;
      }
      else
      {
        command.exceptQuantity = 0;
      }

      return tokenIndex == tokens.size();
    }

    if (upperQuantityToken == L"EXCEPT")
    {
      if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], command.exceptQuantity) || command.exceptQuantity < 0)
      {
        return false;
      }
      ++tokenIndex;

      if (tokenIndex >= tokens.size())
      {
        return false;
      }

      command.mode = GiveMode::AllExcept;
      command.quantity = 0;
      command.itemOperand = tokens[tokenIndex];
      command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
      ++tokenIndex;
      return tokenIndex == tokens.size();
    }

    if (!tryParseInt(quantityToken, command.quantity) || command.quantity <= 0)
    {
      // Alternate form: GIVE <unit> <item> <quantity>
      command.itemOperand = quantityToken;
      command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex - 1];
      if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], command.quantity) || command.quantity <= 0)
      {
        return false;
      }
      ++tokenIndex;
      command.mode = GiveMode::Quantity;
      command.exceptQuantity = 0;
      return tokenIndex == tokens.size();
    }

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    command.mode = GiveMode::Quantity;
    command.itemOperand = tokens[tokenIndex];
    command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;
    command.exceptQuantity = 0;
    return tokenIndex == tokens.size();
  }

  /**
  * @brief Parses a PRODUCE command line, including optional amount cap.
  */
  bool tryParseProduceCommand(const std::wstring& line, ProduceCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"PRODUCE")
    {
      return false;
    }
    ++tokenIndex;

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    int requestedAmount = 0;
    if (tryParseInt(tokens[tokenIndex], requestedAmount))
    {
      if (requestedAmount <= 0)
      {
        return false;
      }

      command.hasRequestedAmount = true;
      command.requestedAmount = requestedAmount;
      ++tokenIndex;

      if (tokenIndex >= tokens.size())
      {
        return false;
      }
    }
    else
    {
      command.hasRequestedAmount = false;
      command.requestedAmount = 0;
    }

    command.itemOperand = tokens[tokenIndex];
    command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;

    return tokenIndex == tokens.size();
  }

  /**
  * @brief Parses a TRANSPORT or DISTRIBUTE command line.
  */
  bool tryParseTransportCommand(const std::wstring& line, TransportCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    const std::wstring commandKeyword = StringUtils::toUpper(tokens[tokenIndex]);
    if (commandKeyword != L"TRANSPORT" && commandKeyword != L"DISTRIBUTE")
    {
      return false;
    }
    ++tokenIndex;

    if (!tryParseUnitReference(tokens, tokenIndex, command.targetUnitId, command.targetIsNewUnit))
    {
      return false;
    }

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    const std::wstring quantityToken = tokens[tokenIndex++];
    const std::wstring upperQuantityToken = StringUtils::toUpper(quantityToken);
    if (upperQuantityToken == L"ALL")
    {
      command.mode = GiveMode::All;
      command.quantity = 0;

      if (tokenIndex >= tokens.size())
      {
        return false;
      }

      command.itemOperand = tokens[tokenIndex];
      command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
      ++tokenIndex;

      if (tokenIndex < tokens.size())
      {
        if (StringUtils::toUpper(tokens[tokenIndex]) != L"EXCEPT")
        {
          return false;
        }
        ++tokenIndex;

        if (tokenIndex >= tokens.size() || !tryParseInt(tokens[tokenIndex], command.exceptQuantity) || command.exceptQuantity < 0)
        {
          return false;
        }
        ++tokenIndex;

        command.mode = GiveMode::AllExcept;
      }
      else
      {
        command.exceptQuantity = 0;
      }

      return tokenIndex == tokens.size();
    }

    if (!tryParseInt(quantityToken, command.quantity) || command.quantity <= 0)
    {
      return false;
    }

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    command.mode = GiveMode::Quantity;
    command.itemOperand = tokens[tokenIndex];
    command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;
    command.exceptQuantity = 0;
    return tokenIndex == tokens.size();
  }

  /**
  * @brief Parses a BUY or SELL command line.
  */
  bool tryParseTradeCommand(const std::wstring& line, TradeCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    const std::wstring verbToken = StringUtils::toUpper(tokens[tokenIndex]);
    if (verbToken == L"BUY")
    {
      command.isBuy = true;
    }
    else if (verbToken == L"SELL")
    {
      command.isBuy = false;
    }
    else
    {
      return false;
    }
    ++tokenIndex;

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    const std::wstring quantityToken = tokens[tokenIndex++];
    if (StringUtils::toUpper(quantityToken) == L"ALL")
    {
      command.mode = GiveMode::All;
      command.quantity = 0;
    }
    else
    {
      if (!tryParseInt(quantityToken, command.quantity) || command.quantity <= 0)
      {
        return false;
      }
      command.mode = GiveMode::Quantity;
    }

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    command.itemOperand = tokens[tokenIndex];
    command.itemOperandWasQuoted = tokenWasQuoted[tokenIndex];
    ++tokenIndex;
    return tokenIndex == tokens.size();
  }

  /**
  * @brief Parses a MOVE or ADVANCE command with directions and optional structure target.
  */
  bool tryParseMoveCommand(const std::wstring& line, MoveCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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
    if (commandToken != L"MOVE" && commandToken != L"ADVANCE")
    {
      return false;
    }
    ++tokenIndex;

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    // Parse directions: N, S, NE, NW, SE, SW
    static const std::vector<std::wstring> validDirections = { L"N", L"S", L"NE", L"NW", L"SE", L"SW" };
    
    while (tokenIndex < tokens.size())
    {
      const std::wstring token = StringUtils::toUpper(tokens[tokenIndex]);
      
      // Check if this is a direction
      bool isDirection = false;
      for (const auto& validDir : validDirections)
      {
        if (token == validDir)
        {
          isDirection = true;
          command.directions.push_back(token);
          break;
        }
      }
      
      if (isDirection)
      {
        ++tokenIndex;
        continue;
      }
      
      // Check if this is "IN"
      if (token == L"IN")
      {
        command.targetStructureId = -2; // Special value for "IN"
        ++tokenIndex;
        break;
      }
      
      // Try to parse as a number (structure ID)
      try
      {
        int structureId = std::stoi(token);
        command.targetStructureId = structureId;
        ++tokenIndex;
        break;
      }
      catch (...)
      {
        return false; // Invalid token
      }
    }

    return !command.directions.empty();
  }

  /**
  * @brief Parses a SAIL command and validates all directions.
  */
  bool tryParseSailCommand(const std::wstring& line, SailCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"SAIL")
    {
      return false;
    }
    ++tokenIndex;

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    static const std::vector<std::wstring> validDirections = { L"N", L"S", L"NE", L"NW", L"SE", L"SW" };

    while (tokenIndex < tokens.size())
    {
      const std::wstring direction = StringUtils::toUpper(tokens[tokenIndex]);
      bool isValid = false;
      for (const auto& validDir : validDirections)
      {
        if (direction == validDir)
        {
          isValid = true;
          break;
        }
      }

      if (!isValid)
      {
        return false;
      }

      command.directions.push_back(direction);
      ++tokenIndex;
    }

    return !command.directions.empty();
  }

  /**
  * @brief Returns true when the unit owns the ship structure it is currently inside.
  */
  bool isUnitShipOwnerAtCurrentLocation(const AppData& appData, const Unit& unit)
  {
    if (unit.getStructureId() <= 0)
    {
      return false;
    }

    const Structure* structure = appData.structureRepository().findByIdAndCoordinates(
      unit.getStructureId(), unit.getXCoordinate(), unit.getYCoordinate(), unit.getZCoordinate());
    const StructInfo* structInfo =
      structure ? appData.structInfoRepository().findByType(structure->getStructureType()) : nullptr;
    if (!structure || !structInfo || !structInfo->isShip())
    {
      return false;
    }

    return structure->getOwnerUnitId() == unit.getUnitNumber();
  }

  /**
  * @brief Computes doubled hex-grid distance between two coordinates.
  */
  int doubledHexDistance(int x1, int y1, int x2, int y2)
  {
    const int dx = std::abs(x1 - x2);
    const int dy = std::abs(y1 - y2);
    if (dy <= dx)
    {
      return dx;
    }

    return dx + ((dy - dx) / 2);
  }

  /**
  * @brief Returns true when two units are within allowed transport distance.
  */
  bool isWithinTransportRange(const Unit& sourceUnit, const Unit& targetUnit)
  {
    if (sourceUnit.getZCoordinate() != targetUnit.getZCoordinate())
    {
      return false;
    }

    return doubledHexDistance(sourceUnit.getXCoordinate(),
                              sourceUnit.getYCoordinate(),
                              targetUnit.getXCoordinate(),
                              targetUnit.getYCoordinate()) <= 2;
  }

  bool isWithinTransportRange(const Unit& sourceUnit, const UnitNew& targetUnit)
  {
    if (sourceUnit.getZCoordinate() != targetUnit.getZCoordinate())
    {
      return false;
    }

    return doubledHexDistance(sourceUnit.getXCoordinate(),
                              sourceUnit.getYCoordinate(),
                              targetUnit.getXCoordinate(),
                              targetUnit.getYCoordinate()) <= 2;
  }

  bool isWithinTransportRange(const UnitNew& sourceUnit, const Unit& targetUnit)
  {
    if (sourceUnit.getZCoordinate() != targetUnit.getZCoordinate())
    {
      return false;
    }

    return doubledHexDistance(sourceUnit.getXCoordinate(),
                              sourceUnit.getYCoordinate(),
                              targetUnit.getXCoordinate(),
                              targetUnit.getYCoordinate()) <= 2;
  }

  bool isWithinTransportRange(const UnitNew& sourceUnit, const UnitNew& targetUnit)
  {
    if (sourceUnit.getZCoordinate() != targetUnit.getZCoordinate())
    {
      return false;
    }

    return doubledHexDistance(sourceUnit.getXCoordinate(),
                              sourceUnit.getYCoordinate(),
                              targetUnit.getXCoordinate(),
                              targetUnit.getYCoordinate()) <= 2;
  }

  /**
  * @brief Parses a LEAVE command that must contain only the keyword.
  */
  bool tryParseLeaveCommand(const std::wstring& line)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    return tokenIndex < tokens.size() && StringUtils::toUpper(tokens[tokenIndex]) == L"LEAVE" && tokens.size() == tokenIndex + 1;
  }

  /**
  * @brief Parses an ENTER command with a target structure id.
  */
  bool tryParseEnterCommand(const std::wstring& line, EnterCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
    {
      return false;
    }

    std::size_t tokenIndex = 0;
    if (!tokens.empty() && !tokens.front().empty() && tokens.front().front() == L'@')
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"ENTER")
    {
      return false;
    }
    ++tokenIndex;

    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    try
    {
      command.structureId = std::stoi(tokens[tokenIndex]);
      ++tokenIndex;
      return tokenIndex == tokens.size(); // Must have exactly one parameter
    }
    catch (...)
    {
      return false;
    }
  }

  /**
  * @brief Checks whether a unit can legally perform transport via caravanserai rules.
  */
  bool unitOwnsCaravanseraiAndHasQuam(const AppData& appData, int unitNumber)
  {
    const Unit* unit = appData.unitRepository().findByNumber(unitNumber);
    if (!unit)
    {
      return false;
    }

    if (Skill::trainingDaysToLevel(unit->getSkillDays(L"QUAM")) <= 0)
    {
      return false;
    }

    const StructureRepository& structureRepository = appData.structureRepository();
    for (std::size_t index = 0; index < structureRepository.size(); ++index)
    {
      const Structure& structure = structureRepository.at(index);
      if (structure.getOwnerUnitId() != unitNumber)
      {
        continue;
      }

      const std::wstring normalizedType = StringUtils::toUpper(StringUtils::trimWhitespace(structure.getStructureType()));
      const std::wstring normalizedName = StringUtils::toUpper(StringUtils::trimWhitespace(structure.getStructureName()));
      if (normalizedType == L"CARAVANSERAI" || normalizedName == L"CARAVANSERAI")
      {
        return true;
      }
    }

    return false;
  }

  /**
  * @brief Returns true when a line contains exactly one command keyword token.
  */
  bool isSingleKeywordCommand(const std::wstring& line, const wchar_t* keyword)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
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

    if (StringUtils::toUpper(tokens[tokenIndex]) != keyword)
    {
      return false;
    }

    ++tokenIndex;
    return tokenIndex == tokens.size();
  }

  /** @brief Returns true for single-token TAX command lines. */
  bool isTaxCommand(const std::wstring& line)       { return isSingleKeywordCommand(line, L"TAX"); }
  /**
  * @brief Parses ENTERTAIN command with optional order tag prefix.
  */
  bool tryParseEntertainCommand(const std::wstring& line, EntertainCommand& command)
  {
    command = EntertainCommand {};

    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"ENTERTAIN")
    {
      return false;
    }

    ++tokenIndex;
    return tokenIndex == tokens.size();
  }
  /** @brief Returns true for single-token WORK command lines. */
  bool isWorkCommand(const std::wstring& line)       { return isSingleKeywordCommand(line, L"WORK"); }

  /**
  * @brief Parses NAME UNIT [new name] and ignores other NAME variants.
  */
  bool tryParseNameUnitCommand(const std::wstring& line, NameUnitCommand& command)
  {
    command = NameUnitCommand {};

    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
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

    if ((tokenIndex + 2) >= tokens.size())
    {
      return false;
    }

    if (StringUtils::toUpper(tokens[tokenIndex]) != L"NAME")
    {
      return false;
    }

    ++tokenIndex;
    if (StringUtils::toUpper(tokens[tokenIndex]) != L"UNIT")
    {
      return false;
    }

    ++tokenIndex;
    std::wstring newName;
    while (tokenIndex < tokens.size())
    {
      const std::wstring piece = StringUtils::trimWhitespace(tokens[tokenIndex]);
      if (!piece.empty())
      {
        if (!newName.empty())
        {
          newName += L" ";
        }
        newName += piece;
      }
      ++tokenIndex;
    }

    command.newName = StringUtils::trimWhitespace(newName);
    return !command.newName.empty();
  }

  /**
  * @brief Parses a STUDY command and captures the studied skill operand.
  */
  bool tryParseStudyCommand(const std::wstring& line, StudyCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"STUDY")
    {
      return false;
    }

    ++tokenIndex;
    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    command.skillToken = StringUtils::trimWhitespace(tokens[tokenIndex]);
    return !command.skillToken.empty();
  }

  /**
  * @brief Parses a TEACH command and captures the list of student unit numbers.
  */
  bool tryParseTeachCommand(const std::wstring& line, TeachCommand& command)
  {
    std::vector<std::wstring> tokens;
    std::vector<bool> tokenWasQuoted;
    if (!tokenizeCommandLine(line, tokens, tokenWasQuoted) || tokens.empty())
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

    if (tokenIndex >= tokens.size() || StringUtils::toUpper(tokens[tokenIndex]) != L"TEACH")
    {
      return false;
    }

    ++tokenIndex;
    // TEACH requires at least one student unit number
    if (tokenIndex >= tokens.size())
    {
      return false;
    }

    // Parse all remaining tokens as student unit numbers
    while (tokenIndex < tokens.size())
    {
      int unitNumber = 0;
      if (tryParseInt(tokens[tokenIndex], unitNumber) && unitNumber > 0)
      {
        command.studentUnitNumbers.push_back(unitNumber);
      }
      ++tokenIndex;
    }

    return !command.studentUnitNumbers.empty();
  }

  /**
  * @brief Attempts to parse any command currently supported by simulation.
  */
  bool tryParseSupportedOrderCommand(const std::wstring& line, ParsedOrderCommand& parsed)
  {
    GiveCommand giveCommand;
    if (tryParseGiveCommand(line, giveCommand))
    {
      parsed.give = std::move(giveCommand);
      if (parsed.give.kind == GiveKind::UnitTransfer)
      {
        parsed.kind = SimulatedCommandKind::NoEffect;
      }
      else
      {
        parsed.kind = SimulatedCommandKind::GiveTake;
      }
      return true;
    }

    TransportCommand transportCommand;
    if (tryParseTransportCommand(line, transportCommand))
    {
      parsed.kind = SimulatedCommandKind::Transport;
      parsed.transport = std::move(transportCommand);
      return true;
    }

    TradeCommand tradeCommand;
    if (tryParseTradeCommand(line, tradeCommand))
    {
      parsed.kind = SimulatedCommandKind::Trade;
      parsed.trade = std::move(tradeCommand);
      return true;
    }

    MoveCommand moveCommand;
    if (tryParseMoveCommand(line, moveCommand))
    {
      parsed.kind = SimulatedCommandKind::Move;
      parsed.move = std::move(moveCommand);
      return true;
    }

    SailCommand sailCommand;
    if (tryParseSailCommand(line, sailCommand))
    {
      parsed.kind = SimulatedCommandKind::Sail;
      parsed.sail = std::move(sailCommand);
      return true;
    }

    if (tryParseLeaveCommand(line))
    {
      parsed.kind = SimulatedCommandKind::Leave;
      return true;
    }

    EnterCommand enterCommand;
    if (tryParseEnterCommand(line, enterCommand))
    {
      parsed.kind = SimulatedCommandKind::Enter;
      parsed.enter = std::move(enterCommand);
      return true;
    }

    if (isTaxCommand(line))
    {
      parsed.kind = SimulatedCommandKind::Tax;
      return true;
    }

    StudyCommand studyCommand;
    if (tryParseStudyCommand(line, studyCommand))
    {
      parsed.kind = SimulatedCommandKind::Study;
      parsed.study = std::move(studyCommand);
      return true;
    }

    TeachCommand teachCommand;
    if (tryParseTeachCommand(line, teachCommand))
    {
      parsed.kind = SimulatedCommandKind::Teach;
      parsed.teach = std::move(teachCommand);
      return true;
    }

    NameUnitCommand nameUnitCommand;
    if (tryParseNameUnitCommand(line, nameUnitCommand))
    {
      parsed.kind = SimulatedCommandKind::NameUnit;
      parsed.nameUnit = std::move(nameUnitCommand);
      return true;
    }

    EntertainCommand entertainCommand;
    if (tryParseEntertainCommand(line, entertainCommand))
    {
      parsed.kind = SimulatedCommandKind::Entertain;
      parsed.entertain = std::move(entertainCommand);
      return true;
    }

    if (isWorkCommand(line))
    {
      parsed.kind = SimulatedCommandKind::Work;
      return true;
    }

    ProduceCommand produceCommand;
    if (tryParseProduceCommand(line, produceCommand))
    {
      parsed.kind = SimulatedCommandKind::Produce;
      parsed.produce = std::move(produceCommand);
      return true;
    }

    return false;
  }

  /**
  * @brief Returns true when a unit appears to have COMB level 1 or higher.
  */
  bool hasCombSkillAtLeastOne(const Unit& unit)
  {
    auto lineHasCombAtLeastOne = [](const std::wstring& skillLine) -> bool
    {
      const std::wstring upperLine = StringUtils::toUpper(skillLine);
      std::size_t pos = std::wstring::npos;
      const std::size_t bracketCombPos = upperLine.find(L"[COMB]");
      if (bracketCombPos != std::wstring::npos)
      {
        pos = bracketCombPos + 6;
      }
      else
      {
        const std::size_t combatPos = upperLine.find(L"COMBAT");
        if (combatPos != std::wstring::npos)
        {
          pos = combatPos + 6;
        }
        else
        {
          const std::size_t plainCombPos = upperLine.find(L"COMB");
          if (plainCombPos == std::wstring::npos)
          {
            return false;
          }
          pos = plainCombPos + 4;
        }
      }

      while (pos < upperLine.size() && !iswdigit(upperLine[pos]))
      {
        ++pos;
      }

      if (pos >= upperLine.size())
      {
        return false;
      }

      std::size_t startDigits = pos;
      while (pos < upperLine.size() && iswdigit(upperLine[pos]))
      {
        ++pos;
      }

      if (pos == startDigits)
      {
        return false;
      }

      int level = 0;
      if (tryParseInt(upperLine.substr(startDigits, pos - startDigits), level) && level >= 1)
      {
        return true;
      }

      return false;
    };

    // Skills parsed from "Skills:" lines.
    for (const auto& [skillToken, days] : unit.getSkills())
    {
      if (lineHasCombAtLeastOne(skillToken))
      {
        return true;
      }

      if (StringUtils::toUpper(skillToken) == L"COMB")
      {
        const int level = Skill::trainingDaysToLevel(days);
        if (level >= 1)
        {
          return true;
        }
      }
    }

    // Fallback for reports where "combat n" appears in unit header flags text.
    for (const std::wstring& flagLine : unit.getFlags())
    {
      if (lineHasCombAtLeastOne(flagLine))
      {
        return true;
      }
    }

    return false;
  }

  /**
  * @brief Finds an item by token while tolerating case differences.
  */
  const Item* findItemByTokenNormalized(const AppData& appData, const std::wstring& itemToken)
  {
    const std::wstring normalizedToken = normalizeItemToken(itemToken);
    if (normalizedToken.empty())
    {
      return nullptr;
    }

    if (const Item* exact = appData.itemRepository().findByIdentifierToken(normalizedToken))
    {
      return exact;
    }

    for (std::size_t index = 0; index < appData.itemRepository().size(); ++index)
    {
      const Item& candidate = appData.itemRepository().at(index);
      if (normalizeItemToken(candidate.getIdentifierToken()) == normalizedToken)
      {
        return &candidate;
      }
    }

    return nullptr;
  }

  /**
  * @brief Returns the per-item skill limit for a skill token.
  *
  * Returns 0 when the item has no applicable limit (no per-skill entry and defaultSkillMax is 0).
  */
  int getManItemSkillLimit(const Item& item, const std::wstring& skillToken)
  {
    const std::wstring normalizedSkillToken = normalizeItemToken(skillToken);
    for (const auto& [limitedSkillToken, levelLimit] : item.getSkillsMax())
    {
      if (normalizeItemToken(limitedSkillToken) == normalizedSkillToken)
      {
        return levelLimit > 0 ? levelLimit : 2;
      }
    }

    const int defaultLimit = item.getDefaultSkillMax();
    return defaultLimit > 0 ? defaultLimit : 2;
  }

  /**
  * @brief Resolves the lowest applicable skill level limit across all man item types in a unit.
  *
  * Returns nullopt when no man item imposes a positive limit on the skill.
  */
  std::optional<int> resolveUnitSkillLevelLimit(const AppData& appData,
                                                const std::map<std::wstring, int>& itemCounts,
                                                const std::wstring& skillToken)
  {
    std::optional<int> lowestLimit;

    for (const auto& [itemToken, amount] : itemCounts)
    {
      if (amount <= 0)
      {
        continue;
      }

      const Item* item = findItemByTokenNormalized(appData, itemToken);
      if (!item || !item->isMan())
      {
        continue;
      }

      const int itemLimit = getManItemSkillLimit(*item, skillToken);

      if (!lowestLimit.has_value())
      {
        lowestLimit = itemLimit;
      }
      else
      {
        lowestLimit = (std::min)(*lowestLimit, itemLimit);
      }
    }

    return lowestLimit;
  }

  /**
  * @brief Returns the maximum cumulative training days allowed for a skill level limit.
  */
  int maxTrainingDaysForLevelLimit(int level)
  {
    if (level <= 0)
    {
      return 0;
    }

    const int clampedLevel = (std::min)(level, Skill::kMaxLevel);
    int maxDaysAtLimit = 0;
    for (int rank = 1; rank <= clampedLevel; ++rank)
    {
      maxDaysAtLimit += rank * 30;
    }

    return maxDaysAtLimit;
  }

  bool hasStudyPrerequisites(const std::map<std::wstring, int>& unitSkills, const Skill& studiedSkill)
  {
    for (const SkillPrerequisite& prerequisite : studiedSkill.getPrerequisites())
    {
      if (prerequisite.token.empty() || prerequisite.requiredLevel <= 0)
      {
        continue;
      }

      int knownLevel = 0;
      for (const auto& [unitSkillToken, days] : unitSkills)
      {
        if (unitSkillToken != prerequisite.token)
        {
          continue;
        }
        return Skill::trainingDaysToLevel(days);
      }      

      if (knownLevel < prerequisite.requiredLevel)
      {
        return false;
      }
    }

    return true;
  }

  /**
  * @brief Finds the best unit skill level that satisfies an item's production requirements.
  */
  int getBestQualifiedProductionSkillLevel(const Unit& unit, const Item& item)
  {
    int bestQualifiedLevel = 0;

    for (const auto& [requiredSkillToken, requiredLevel] : item.getProductionSkill())
    {
      const int unitSkillLevel = Skill::trainingDaysToLevel(unit.getSkillDays(requiredSkillToken));
      if (unitSkillLevel >= requiredLevel)
      {
        bestQualifiedLevel = std::max(bestQualifiedLevel, unitSkillLevel);
      }
    }

    return bestQualifiedLevel;
  }

  /**
  * @brief Maps a parsed command to its deterministic execution phase.
  */
  CommandPhase getCommandPhase(const ParsedOrderCommand& parsedCommand)
  {
    switch (parsedCommand.kind)
    {
      case SimulatedCommandKind::GiveTake:
        return CommandPhase::GiveTake;

      case SimulatedCommandKind::Tax:
        return CommandPhase::Tax;

      case SimulatedCommandKind::Trade:
        return parsedCommand.trade.isBuy ? CommandPhase::Buy : CommandPhase::Sell;

      case SimulatedCommandKind::Move:
        return CommandPhase::Move;

      case SimulatedCommandKind::Sail:
        return CommandPhase::Move;

      case SimulatedCommandKind::Produce:
        return CommandPhase::Produce;

      case SimulatedCommandKind::Entertain:
        return CommandPhase::Entertain;

      case SimulatedCommandKind::Study:
        return CommandPhase::Produce;

      case SimulatedCommandKind::NameUnit:
        return CommandPhase::Name;

      case SimulatedCommandKind::Work:
        return CommandPhase::Work;

      case SimulatedCommandKind::Leave:
        return CommandPhase::Move;

      case SimulatedCommandKind::Enter:
        return CommandPhase::Move;

      case SimulatedCommandKind::Transport:
        return CommandPhase::Transport;

      case SimulatedCommandKind::NoEffect:
      default:
        return CommandPhase::Work;
    }
  }

  /**
  * @brief Applies one scheduled command to the mutable simulation state.
  */
  void executeScheduledCommand(const AppData& appData,
                               const Region* region,
                               RegionCommandSimulation& simulation,
                               const ScheduledCommand& scheduledCommand)
  {
    if (!scheduledCommand.originUnit && !scheduledCommand.isNewUnit)
    {
      return;
    }

    const int originUnitNumber = scheduledCommand.originUnitNumber;
    const Unit* originUnit = scheduledCommand.originUnit;
    const bool originIsLocal = scheduledCommand.originIsLocal;
    const bool isNewUnit = scheduledCommand.isNewUnit;
    const ParsedOrderCommand& parsedCommand = scheduledCommand.parsedCommand;

    if (!originIsLocal && parsedCommand.kind != SimulatedCommandKind::Transport)
    {
      return;
    }

    switch (parsedCommand.kind)
    {
      case SimulatedCommandKind::NoEffect:
      {
        return;
      }

      case SimulatedCommandKind::Transport:
      {
        TransportCommand transportCommand = parsedCommand.transport;
        if (!tryResolveItemToken(appData,
                                 transportCommand.itemOperand,
                                 transportCommand.itemOperandWasQuoted,
                                 transportCommand.itemToken))
        {
          return;
        }

        const Unit* destinationUnit = nullptr;
        const UnitNew* destinationNewUnit = nullptr;
        if (transportCommand.targetIsNewUnit)
        {
          const auto destinationNewIt = simulation.nearbyNewUnits.find(transportCommand.targetUnitId);
          if (destinationNewIt == simulation.nearbyNewUnits.end() || destinationNewIt->second == nullptr)
          {
            return;
          }
          destinationNewUnit = destinationNewIt->second;
        }
        else
        {
          const auto destinationUnitIt = simulation.nearbyUnits.find(transportCommand.targetUnitId);
          if (destinationUnitIt == simulation.nearbyUnits.end() || destinationUnitIt->second == nullptr)
          {
            return;
          }
          destinationUnit = destinationUnitIt->second;
        }

        if (transportCommand.targetIsNewUnit)
        {
          if (!isWithinTransportRange(*originUnit, *destinationNewUnit))
          {
            return;
          }
        }
        else
        {
          if (!isWithinTransportRange(*originUnit, *destinationUnit))
          {
            return;
          }
        }

        const bool originIsQualifiedOwner = unitOwnsCaravanseraiAndHasQuam(appData, originUnitNumber);
        const bool destinationIsQualifiedOwner =
          unitOwnsCaravanseraiAndHasQuam(appData, transportCommand.targetUnitId);
        if (!originIsQualifiedOwner && !destinationIsQualifiedOwner)
        {
          return;
        }

        auto sourceCountsIt = simulation.itemCountsByUnit.find(originUnitNumber);
        if (sourceCountsIt == simulation.itemCountsByUnit.end())
        {
          return;
        }

        std::map<std::wstring, int>* destinationCounts = nullptr;
        if (transportCommand.targetIsNewUnit)
        {
          const auto destinationCountsNewIt = simulation.itemCountsByNewUnit.find(transportCommand.targetUnitId);
          if (destinationCountsNewIt == simulation.itemCountsByNewUnit.end())
          {
            return;
          }
          destinationCounts = &destinationCountsNewIt->second;
        }
        else
        {
          const auto destinationCountsIt = simulation.itemCountsByUnit.find(transportCommand.targetUnitId);
          if (destinationCountsIt == simulation.itemCountsByUnit.end())
          {
            return;
          }
          destinationCounts = &destinationCountsIt->second;
        }

        std::map<std::wstring, int>& sourceCountsRef = sourceCountsIt->second;
        std::map<std::wstring, int>& destinationCountsRef = *destinationCounts;

        const auto availableIt = sourceCountsRef.find(transportCommand.itemToken);
        const int availableAmount = (availableIt == sourceCountsRef.end()) ? 0 : availableIt->second;
        if (availableAmount <= 0)
        {
          return;
        }

        int transferAmount = 0;
        switch (transportCommand.mode)
        {
          case GiveMode::Quantity:
            if (availableAmount < transportCommand.quantity)
            {
              return;
            }
            transferAmount = transportCommand.quantity;
            break;

          case GiveMode::All:
            transferAmount = availableAmount;
            break;

          case GiveMode::AllExcept:
            transferAmount = std::max(0, availableAmount - transportCommand.exceptQuantity);
            break;
        }

        if (transferAmount <= 0)
        {
          return;
        }

        const int remaining = availableAmount - transferAmount;
        if (remaining > 0)
        {
          sourceCountsRef[transportCommand.itemToken] = remaining;
        }
        else
        {
          sourceCountsRef.erase(transportCommand.itemToken);
        }

        destinationCountsRef[transportCommand.itemToken] += transferAmount;
        return;
      }

      case SimulatedCommandKind::Trade:
      {
        TradeCommand tradeCommand = parsedCommand.trade;
        if (!tryResolveItemToken(appData,
                                 tradeCommand.itemOperand,
                                 tradeCommand.itemOperandWasQuoted,
                                 tradeCommand.itemToken))
        {
          return;
        }

        std::map<std::wstring, int>* originCountsPtr = nullptr;
        std::map<std::wstring, int>* originSkillsPtr = nullptr;
        if (isNewUnit)
        {
          auto it = simulation.itemCountsByNewUnit.find(originUnitNumber);
          if (it == simulation.itemCountsByNewUnit.end())
          {
            return;
          }
          originCountsPtr = &it->second;

          auto skillsIt = simulation.skillsByNewUnit.find(originUnitNumber);
          if (skillsIt != simulation.skillsByNewUnit.end())
          {
            originSkillsPtr = &skillsIt->second;
          }
        }
        else
        {
          auto originCountsIt = simulation.itemCountsByUnit.find(originUnitNumber);
          if (originCountsIt == simulation.itemCountsByUnit.end())
          {
            return;
          }
          originCountsPtr = &originCountsIt->second;

          auto skillsIt = simulation.skillsByUnit.find(originUnitNumber);
          if (skillsIt != simulation.skillsByUnit.end())
          {
            originSkillsPtr = &skillsIt->second;
          }
        }
        std::map<std::wstring, int>& originCounts = *originCountsPtr;

        if (tradeCommand.isBuy)
        {
          auto forSaleIt = simulation.remainingForSale.find(tradeCommand.itemToken);
          if (forSaleIt == simulation.remainingForSale.end())
          {
            return;
          }

          const int availableAmount = std::max(0, forSaleIt->second.first);
          const int price = std::max(0, forSaleIt->second.second);
          if (availableAmount <= 0)
          {
            return;
          }

          const int desiredAmount =
            (tradeCommand.mode == GiveMode::All) ? availableAmount : std::min(availableAmount, tradeCommand.quantity);
          const int silverAvailable = std::max(0, originCounts[L"SILV"]);
          const int affordableAmount = (price > 0) ? (silverAvailable / price) : desiredAmount;
          const int boughtAmount = std::min(desiredAmount, affordableAmount);
          if (boughtAmount <= 0)
          {
            return;
          }

          const int manCountBeforeBuy = appData.itemRepository().calculateManItemCount(originCounts);
          originCounts[tradeCommand.itemToken] += boughtAmount;

          const Item* boughtItem = appData.itemRepository().findByIdentifierToken(tradeCommand.itemToken);
          if (boughtItem && boughtItem->isMan() && originSkillsPtr)
          {
            const int manCountAfterBuy = manCountBeforeBuy + boughtAmount;
            if (manCountAfterBuy > 0)
            {
              if (manCountBeforeBuy <= 0)
              {
                originSkillsPtr->clear();
              }
              else
              {
                std::vector<std::wstring> eraseTokens;
                for (auto& [skillToken, days] : *originSkillsPtr)
                {
                  const long long weightedDays =
                    static_cast<long long>(days) * static_cast<long long>(manCountBeforeBuy);
                  const int blendedDays = static_cast<int>(weightedDays / manCountAfterBuy);
                  if (blendedDays > 0)
                  {
                    days = blendedDays;
                  }
                  else
                  {
                    eraseTokens.push_back(skillToken);
                  }
                }

                for (const std::wstring& skillToken : eraseTokens)
                {
                  originSkillsPtr->erase(skillToken);
                }
              }
            }
          }

          if (price > 0)
          {
            const int remainingSilver = silverAvailable - (boughtAmount * price);
            if (remainingSilver > 0)
            {
              originCounts[L"SILV"] = remainingSilver;
            }
            else
            {
              originCounts.erase(L"SILV");
            }
          }

          forSaleIt->second.first = availableAmount - boughtAmount;
          return;
        }

        auto wantedIt = simulation.remainingWanted.find(tradeCommand.itemToken);
        if (wantedIt == simulation.remainingWanted.end())
        {
          return;
        }

        const auto availableItemIt = originCounts.find(tradeCommand.itemToken);
        const int availableItemAmount = (availableItemIt == originCounts.end()) ? 0 : availableItemIt->second;
        const int wantedAmount = std::max(0, wantedIt->second.first);
        const int price = std::max(0, wantedIt->second.second);
        if (availableItemAmount <= 0 || wantedAmount <= 0)
        {
          return;
        }

        const int desiredAmount =
          (tradeCommand.mode == GiveMode::All) ? availableItemAmount : std::min(availableItemAmount, tradeCommand.quantity);
        const int soldAmount = std::min(desiredAmount, wantedAmount);
        if (soldAmount <= 0)
        {
          return;
        }

        const int remainingItemAmount = availableItemAmount - soldAmount;
        if (remainingItemAmount > 0)
        {
          originCounts[tradeCommand.itemToken] = remainingItemAmount;
        }
        else
        {
          originCounts.erase(tradeCommand.itemToken);
        }

        originCounts[L"SILV"] += soldAmount * price;
        wantedIt->second.first = wantedAmount - soldAmount;
        return;
      }

      case SimulatedCommandKind::Tax:
      {
        if (simulation.remainingTaxableIncome <= 0 || !hasCombSkillAtLeastOne(*originUnit))
        {
          return;
        }

        auto originCountsIt = simulation.itemCountsByUnit.find(originUnitNumber);
        if (originCountsIt == simulation.itemCountsByUnit.end())
        {
          return;
        }

        const int manCount = appData.itemRepository().calculateManItemCount(originCountsIt->second);
        if (manCount <= 0)
        {
          return;
        }

        const int silverCapacity = manCount * 50;
        const int silverAmount = std::min(silverCapacity, simulation.remainingTaxableIncome);
        if (silverAmount <= 0)
        {
          return;
        }

        originCountsIt->second[L"SILV"] += silverAmount;
        simulation.remainingTaxableIncome -= silverAmount;
        return;
      }

      case SimulatedCommandKind::GiveTake:
      {
        GiveCommand giveCommand = parsedCommand.give;

        if (!tryResolveItemToken(appData, giveCommand.itemOperand, giveCommand.itemOperandWasQuoted, giveCommand.itemToken))
        {
          return;
        }

        int sourceUnitNumber = originUnitNumber;
        int destinationUnitNumber = giveCommand.receiverUnitId;
        bool sourceIsNewUnit = false;
        bool destinationIsNewUnit = false;
        if (giveCommand.isTake)
        {
          sourceUnitNumber = giveCommand.receiverUnitId;
          destinationUnitNumber = originUnitNumber;
          sourceIsNewUnit = giveCommand.receiverIsNewUnit;
          destinationIsNewUnit = false;

          if (giveCommand.receiverIsNewUnit)
          {
            const auto sourceNewUnitIt = simulation.regionNewUnits.find(sourceUnitNumber);
            if (sourceNewUnitIt == simulation.regionNewUnits.end() || sourceNewUnitIt->second == nullptr)
            {
              return;
            }

            const UnitNew* sourceNewUnit = sourceNewUnitIt->second;
            const int sourceNewOriginUnit = sourceNewUnit->getOriginUnit();
            if (sourceNewOriginUnit <= 0)
            {
              return;
            }

            const auto sourceOriginUnitIt = simulation.regionUnits.find(sourceNewOriginUnit);
            if (sourceOriginUnitIt == simulation.regionUnits.end() || sourceOriginUnitIt->second == nullptr)
            {
              return;
            }

            if (sourceOriginUnitIt->second->getFactionNumber() != originUnit->getFactionNumber())
            {
              return;
            }
          }
          else
          {
            const auto sourceUnitIt = simulation.regionUnits.find(sourceUnitNumber);
            if (sourceUnitIt == simulation.regionUnits.end() || sourceUnitIt->second == nullptr)
            {
              return;
            }

            if (sourceUnitIt->second->getFactionNumber() != originUnit->getFactionNumber())
            {
              return;
            }
          }
        }
        else
        {
          sourceIsNewUnit = false;
          destinationIsNewUnit = giveCommand.receiverIsNewUnit;
        }

        std::map<std::wstring, int>* sourceCounts = nullptr;
        if (giveCommand.isTake && giveCommand.receiverIsNewUnit)
        {
          const auto sourceCountsNewIt = simulation.itemCountsByNewUnit.find(sourceUnitNumber);
          if (sourceCountsNewIt == simulation.itemCountsByNewUnit.end())
          {
            return;
          }
          sourceCounts = &sourceCountsNewIt->second;
        }
        else
        {
          const auto sourceCountsIt = simulation.itemCountsByUnit.find(sourceUnitNumber);
          if (sourceCountsIt == simulation.itemCountsByUnit.end())
          {
            return;
          }
          sourceCounts = &sourceCountsIt->second;
        }

        std::map<std::wstring, int>* destinationCounts = nullptr;
        if (!giveCommand.isTake && giveCommand.receiverIsNewUnit)
        {
          const auto destinationCountsNewIt = simulation.itemCountsByNewUnit.find(destinationUnitNumber);
          if (destinationCountsNewIt == simulation.itemCountsByNewUnit.end())
          {
            return;
          }
          destinationCounts = &destinationCountsNewIt->second;
        }
        else
        {
          const auto destinationCountsIt = simulation.itemCountsByUnit.find(destinationUnitNumber);
          if (destinationCountsIt == simulation.itemCountsByUnit.end())
          {
            return;
          }
          destinationCounts = &destinationCountsIt->second;
        }

        std::map<std::wstring, int>& sourceCountsRef = *sourceCounts;
        std::map<std::wstring, int>& destinationCountsRef = *destinationCounts;

        std::map<std::wstring, int>* sourceSkills = nullptr;
        if (sourceIsNewUnit)
        {
          const auto sourceSkillsIt = simulation.skillsByNewUnit.find(sourceUnitNumber);
          if (sourceSkillsIt == simulation.skillsByNewUnit.end())
          {
            return;
          }
          sourceSkills = &sourceSkillsIt->second;
        }
        else
        {
          const auto sourceSkillsIt = simulation.skillsByUnit.find(sourceUnitNumber);
          if (sourceSkillsIt == simulation.skillsByUnit.end())
          {
            return;
          }
          sourceSkills = &sourceSkillsIt->second;
        }

        std::map<std::wstring, int>* destinationSkills = nullptr;
        if (destinationIsNewUnit)
        {
          const auto destinationSkillsIt = simulation.skillsByNewUnit.find(destinationUnitNumber);
          if (destinationSkillsIt == simulation.skillsByNewUnit.end())
          {
            return;
          }
          destinationSkills = &destinationSkillsIt->second;
        }
        else
        {
          const auto destinationSkillsIt = simulation.skillsByUnit.find(destinationUnitNumber);
          if (destinationSkillsIt == simulation.skillsByUnit.end())
          {
            return;
          }
          destinationSkills = &destinationSkillsIt->second;
        }

        const auto availableIt = sourceCountsRef.find(giveCommand.itemToken);
        const int availableAmount = (availableIt == sourceCountsRef.end()) ? 0 : availableIt->second;
        if (availableAmount <= 0)
        {
          return;
        }

        int transferAmount = 0;
        switch (giveCommand.mode)
        {
          case GiveMode::Quantity:
            if (availableAmount < giveCommand.quantity)
            {
              return;
            }
            transferAmount = giveCommand.quantity;
            break;

          case GiveMode::All:
            transferAmount = availableAmount;
            break;

          case GiveMode::AllExcept:
            transferAmount = std::max(0, availableAmount - giveCommand.exceptQuantity);
            break;
        }

        if (transferAmount <= 0)
        {
          return;
        }

        const Item* transferredItem = appData.itemRepository().findByIdentifierToken(giveCommand.itemToken);
        const bool transferIsManItem = transferredItem && transferredItem->isMan();
        const int destinationManCountBeforeTransfer = transferIsManItem
          ? appData.itemRepository().calculateManItemCount(destinationCountsRef)
          : 0;

        const int remaining = availableAmount - transferAmount;
        if (remaining > 0)
        {
          sourceCountsRef[giveCommand.itemToken] = remaining;
        }
        else
        {
          sourceCountsRef.erase(giveCommand.itemToken);
        }

        destinationCountsRef[giveCommand.itemToken] += transferAmount;

        if (transferIsManItem && sourceSkills && destinationSkills)
        {
          const int destinationManCountAfterTransfer = destinationManCountBeforeTransfer + transferAmount;
          if (destinationManCountAfterTransfer > 0)
          {
            std::set<std::wstring> skillTokens;
            for (const auto& [skillToken, _days] : *sourceSkills)
            {
              skillTokens.insert(skillToken);
            }
            for (const auto& [skillToken, _days] : *destinationSkills)
            {
              skillTokens.insert(skillToken);
            }

            for (const std::wstring& skillToken : skillTokens)
            {
              const auto sourceSkillIt = sourceSkills->find(skillToken);
              const int sourceSkillDays = (sourceSkillIt == sourceSkills->end()) ? 0 : sourceSkillIt->second;

              const auto destinationSkillIt = destinationSkills->find(skillToken);
              const int destinationSkillDays =
                (destinationSkillIt == destinationSkills->end()) ? 0 : destinationSkillIt->second;

              const long long weightedSkillDays =
                static_cast<long long>(destinationSkillDays) * static_cast<long long>(destinationManCountBeforeTransfer) +
                static_cast<long long>(sourceSkillDays) * static_cast<long long>(transferAmount);
              int blendedSkillDays = static_cast<int>(weightedSkillDays / destinationManCountAfterTransfer);

              const std::optional<int> destinationLevelLimit =
                resolveUnitSkillLevelLimit(appData, destinationCountsRef, skillToken);
              if (destinationLevelLimit.has_value())
              {
                const int cappedDays = maxTrainingDaysForLevelLimit(*destinationLevelLimit);
                blendedSkillDays = (std::min)(blendedSkillDays, cappedDays);
              }

              if (blendedSkillDays > 0)
              {
                (*destinationSkills)[skillToken] = blendedSkillDays;
              }
              else
              {
                destinationSkills->erase(skillToken);
              }
            }
          }
        }

        return;
      }

      case SimulatedCommandKind::Entertain:
      {
        if (simulation.remainingEntertainment <= 0)
        {
          return;
        }

        std::map<int, std::map<std::wstring, int>>& originItemCountsMap =
          isNewUnit ? simulation.itemCountsByNewUnit : simulation.itemCountsByUnit;
        std::map<int, std::map<std::wstring, int>>& originSkillsMap =
          isNewUnit ? simulation.skillsByNewUnit : simulation.skillsByUnit;

        auto originCountsIt = originItemCountsMap.find(originUnitNumber);
        if (originCountsIt == originItemCountsMap.end())
        {
          return;
        }

        auto originSkillsIt = originSkillsMap.find(originUnitNumber);
        if (originSkillsIt == originSkillsMap.end())
        {
          return;
        }

        const int manCount =  appData.itemRepository().calculateManItemCount(originCountsIt->second);
        if (manCount <= 0)
        {
          return;
        }

        int entertainDays = 0;
        for (const auto& [skillToken, days] : originSkillsIt->second)
        {
          if (normalizeItemToken(skillToken) == L"ENTE")
          {
            entertainDays = days;
            break;
          }
        }

        const int entertainSkillLevel =  Skill::trainingDaysToLevel(entertainDays);
        if (entertainSkillLevel <= 0)
        {
          return;
        }

        const int silverCapacity = manCount * entertainSkillLevel * 20;
        const int silverAmount = std::min(silverCapacity, simulation.remainingEntertainment);
        if (silverAmount <= 0)
        {
          return;
        }

        originCountsIt->second[L"SILV"] += silverAmount;
        simulation.remainingEntertainment -= silverAmount;
        return;
      }

      case SimulatedCommandKind::Work:
      {
        if (simulation.remainingWorkWages <= 0 || !region)
        {
          return;
        }

        std::map<int, std::map<std::wstring, int>>& originItemCountsMap =
          isNewUnit ? simulation.itemCountsByNewUnit : simulation.itemCountsByUnit;

        auto originCountsIt = originItemCountsMap.find(originUnitNumber);
        if (originCountsIt == originItemCountsMap.end())
        {
          return;
        }

        const int manCount = appData.itemRepository().calculateManItemCount(originCountsIt->second);
        if (manCount <= 0)
        {
          return;
        }

        const double wagesPerMan = std::max(0.0, region->getWages());
        if (wagesPerMan <= 0.0)
        {
          return;
        }

        const int silverCapacity = static_cast<int>(std::floor(static_cast<double>(manCount) * wagesPerMan));
        const int silverAmount = std::min(silverCapacity, simulation.remainingWorkWages);
        if (silverAmount <= 0)
        {
          return;
        }

        originCountsIt->second[L"SILV"] += silverAmount;
        simulation.remainingWorkWages -= silverAmount;
        return;
      }

      case SimulatedCommandKind::Produce:
      {
        ProduceCommand produceCommand = parsedCommand.produce;

        if (!tryResolveItemToken(appData, produceCommand.itemOperand, produceCommand.itemOperandWasQuoted, produceCommand.itemToken))
        {
          return;
        }

        auto originCountsIt = simulation.itemCountsByUnit.find(originUnitNumber);
        if (originCountsIt == simulation.itemCountsByUnit.end())
        {
          return;
        }

        const Item* producedItem = appData.itemRepository().findByIdentifierToken(produceCommand.itemToken);
        if (!producedItem)
        {
          return;
        }

        const int unitProductionSkill = getBestQualifiedProductionSkillLevel(*originUnit, *producedItem);
        if (unitProductionSkill <= 0)
        {
          return;
        }

        std::map<std::wstring, int>& originCounts = originCountsIt->second;
        const auto& productionRequirements = producedItem->getResources();
        const std::wstring producedTokenNormalized = normalizeItemToken(produceCommand.itemToken);
        bool hasOnlySelfRequirement = !productionRequirements.empty();
        for (const auto& [requirementToken, requirementAmount] : productionRequirements)
        {
          if (requirementAmount <= 0)
          {
            continue;
          }

          if (normalizeItemToken(requirementToken) != producedTokenNormalized)
          {
            hasOnlySelfRequirement = false;
            break;
          }
        }

        const bool usesUnitIngredients = !productionRequirements.empty() && !hasOnlySelfRequirement;

        const int manCount = appData.itemRepository().calculateManItemCount(originCountsIt->second);
        if (manCount <= 0)
        {
          return;
        }

        int producedAmount = manCount * unitProductionSkill;
        if (produceCommand.hasRequestedAmount)
        {
          producedAmount = std::min(producedAmount, produceCommand.requestedAmount);
        }

        // Apply production help bonuses from helper items with 1:1 man-to-tool constraint
        {
          const ItemRepository& itemRepo = appData.itemRepository();
          
          // Iterate through all items in the unit's inventory to find helpers
          for (const auto& [helperToken, helperCount] : originCounts)
          {
            if (helperCount <= 0)
            {
              continue;
            }
            
            const Item* helperItem = itemRepo.findByIdentifierToken(helperToken);
            if (!helperItem)
            {
              continue;
            }
            
            const auto& productionHelp = helperItem->getProductionHelp();
            
            // Check if this helper item provides a bonus for the produced item
            for (const auto& [helpToken, helpAmount] : productionHelp)
            {
              if (normalizeItemToken(helpToken) == producedTokenNormalized)
              {
                // Apply bonus with 1:1 man-to-tool constraint
                const int cappedHelperCount = std::min(manCount, helperCount);
                producedAmount += cappedHelperCount * helpAmount;
                break; // Only one bonus per helper item per produced item
              }
            }
          }
        }

        if (usesUnitIngredients)
        {
          bool hasValidRequirement = false;
          bool firstRequirement = true;
          int ingredientProductionCap = 0;

          for (const auto& [requirementToken, requirementAmount] : productionRequirements)
          {
            if (requirementAmount <= 0)
            {
              continue;
            }

            const std::wstring normalizedRequirementToken = normalizeItemToken(requirementToken);
            if (normalizedRequirementToken.empty())
            {
              continue;
            }

            hasValidRequirement = true;
            const auto availableIt = originCounts.find(normalizedRequirementToken);
            const int availableAmount = (availableIt == originCounts.end()) ? 0 : availableIt->second;
            const int capForThisRequirement = availableAmount / requirementAmount;

            if (firstRequirement)
            {
              ingredientProductionCap = capForThisRequirement;
              firstRequirement = false;
            }
            else
            {
              ingredientProductionCap = std::min(ingredientProductionCap, capForThisRequirement);
            }
          }

          if (!hasValidRequirement)
          {
            return;
          }

          producedAmount = std::min(producedAmount, ingredientProductionCap);
        }
        else
        {
          const auto availableResourceIt = simulation.remainingResources.find(produceCommand.itemToken);
          const int availableResourceAmount =
            (availableResourceIt == simulation.remainingResources.end()) ? 0 : availableResourceIt->second;
          producedAmount = std::min(producedAmount, availableResourceAmount);
        }

        if (producedAmount <= 0)
        {
          return;
        }

        originCounts[produceCommand.itemToken] += producedAmount;

        if (usesUnitIngredients)
        {
          for (const auto& [requirementToken, requirementAmount] : productionRequirements)
          {
            if (requirementAmount <= 0)
            {
              continue;
            }

            const std::wstring normalizedRequirementToken = normalizeItemToken(requirementToken);
            if (normalizedRequirementToken.empty())
            {
              continue;
            }

            const int consumedAmount = requirementAmount * producedAmount;
            auto availableIt = originCounts.find(normalizedRequirementToken);
            if (availableIt == originCounts.end())
            {
              continue;
            }

            const int remainingAmount = availableIt->second - consumedAmount;
            if (remainingAmount > 0)
            {
              availableIt->second = remainingAmount;
            }
            else
            {
              originCounts.erase(availableIt);
            }
          }
        }
        else
        {
          const auto availableResourceIt = simulation.remainingResources.find(produceCommand.itemToken);
          const int availableResourceAmount =
            (availableResourceIt == simulation.remainingResources.end()) ? 0 : availableResourceIt->second;
          const int remainingResource = availableResourceAmount - producedAmount;
          if (remainingResource > 0)
          {
            simulation.remainingResources[produceCommand.itemToken] = remainingResource;
          }
          else
          {
            simulation.remainingResources.erase(produceCommand.itemToken);
          }
        }

        return;
      }

      case SimulatedCommandKind::Study:
      {
        // STUDY command: remove silver equal to manCount * studyCost for the studied skill.
        // Also add 30 days to the skill being studied.
        std::map<int, std::map<std::wstring, int>>& originItemCountsMap =
          isNewUnit ? simulation.itemCountsByNewUnit : simulation.itemCountsByUnit;
        std::map<int, std::map<std::wstring, int>>& originSkillsMap =
          isNewUnit ? simulation.skillsByNewUnit : simulation.skillsByUnit;

        auto originCountsIt = originItemCountsMap.find(originUnitNumber);
        if (originCountsIt == originItemCountsMap.end())
        {
          return;
        }

        std::map<std::wstring, int>& originCounts = originCountsIt->second;
        int manCount = appData.itemRepository().calculateManItemCount(originCounts);
        if (manCount <= 0)
        {
          // Some datasets may miss man/race item metadata even for valid study-capable units.
          // If the unit has inventory at all, fall back to one studying participant.
          if (!originCounts.empty())
          {
            manCount = 1;
          }
          else
          {
            return;
          }
        }

        if (parsedCommand.study.skillToken.empty())
        {
          return;
        }

        const std::wstring studyOperand = StringUtils::trimWhitespace(parsedCommand.study.skillToken);
        const Skill* studiedSkill = findSkillByTokenOrNameNormalized(appData, studyOperand);

        if (!studiedSkill)
        {
          return;
        }

        auto skillsIt = originSkillsMap.find(originUnitNumber);
        const std::map<std::wstring, int>* fallbackSkillsPtr = nullptr;
        if (isNewUnit)
        {
          const auto nearbyIt = simulation.nearbyNewUnits.find(originUnitNumber);
          if (nearbyIt != simulation.nearbyNewUnits.end() && nearbyIt->second)
          {
            fallbackSkillsPtr = &nearbyIt->second->getSkills();
          }
        }
        else if (originUnit)
        {
          fallbackSkillsPtr = &originUnit->getSkills();
        }

        const std::map<std::wstring, int> emptySkills;
        const std::map<std::wstring, int>& unitSkillsForPrereq =
          (skillsIt != originSkillsMap.end()) ? skillsIt->second
          : (fallbackSkillsPtr ? *fallbackSkillsPtr : emptySkills);
        if (!hasStudyPrerequisites(unitSkillsForPrereq, *studiedSkill))
        {
          return;
        }

        const int studyCost = (std::max)(0, resolveStudyCostPerManMonth(*studiedSkill));
        if (studyCost > 0)
        {
          const int totalCost = manCount * studyCost;

          auto silvIt = originCounts.find(L"SILV");
          int silverAvailable = (silvIt == originCounts.end()) ? 0 : silvIt->second;
          const int silverAfterStudy = silverAvailable - totalCost;
          if (silverAfterStudy > 0)
          {
            originCounts[L"SILV"] = silverAfterStudy;
          }
          else if (silverAfterStudy == 0)
          {
            originCounts.erase(L"SILV");
          }
          else
          {
            return;
          }
        }

        // Add skill advancement: 30 points to the studied skill
        // Ensure skills are initialized in the simulation map if not already present
        if (skillsIt == originSkillsMap.end())
        {
          originSkillsMap[originUnitNumber] = fallbackSkillsPtr ? *fallbackSkillsPtr : emptySkills;
          skillsIt = originSkillsMap.find(originUnitNumber);
        }

        if (skillsIt != originSkillsMap.end())
        {
          std::map<std::wstring, int>& unitSkills = skillsIt->second;
          const std::wstring studiedSkillToken = studiedSkill->getIdentifierToken();
          const std::wstring normalizedStudiedSkillToken = normalizeItemToken(studiedSkillToken);
          std::wstring targetSkillToken = studiedSkillToken;
          for (const auto& [existingSkillToken, existingDays] : unitSkills)
          {
            (void)existingDays;
            if (normalizeItemToken(existingSkillToken) == normalizedStudiedSkillToken)
            {
              targetSkillToken = existingSkillToken;
              break;
            }
          }

          const int currentDays = unitSkills.count(targetSkillToken) > 0 ? unitSkills[targetSkillToken] : 0;
          const int proposedDays = currentDays + 30;

          const std::optional<int> unitLevelLimit = resolveUnitSkillLevelLimit(appData, originCounts, targetSkillToken);
          if (unitLevelLimit.has_value())
          {
            const int cappedDays = maxTrainingDaysForLevelLimit(*unitLevelLimit);
            if (proposedDays > cappedDays)
            {
              unitSkills[targetSkillToken] = (std::max)(currentDays, (std::min)(proposedDays, cappedDays));
            }
            else
            {
              unitSkills[targetSkillToken] = proposedDays;
            }
          }
          else
          {
            unitSkills[targetSkillToken] = proposedDays;
          }
        }

        return;
      }

      case SimulatedCommandKind::Teach:
      {
        // TEACH command: provide bonus to students studying a skill that the teacher knows
        TeachCommand teachCommand = parsedCommand.teach;
        
        auto originCountsIt = simulation.itemCountsByUnit.find(originUnitNumber);
        if (originCountsIt == simulation.itemCountsByUnit.end())
        {
          return;
        }

        // Check if teacher can teach: must have isMan items
        int teacherManCount = appData.itemRepository().calculateManItemCount(originCountsIt->second);
        if (teacherManCount <= 0)
        {
          return;
        }

        // Check onlyLeaderCanTeach setting
        if (appData.getOnlyLeaderCanTeach())
        {
          bool hasLeadItem = false;
          for (const auto& [itemToken, amount] : originCountsIt->second)
          {
            if (amount > 0 && normalizeItemToken(itemToken) == L"LEAD")
            {
              const Item* itemDef = appData.itemRepository().findByIdentifierToken(itemToken);
              if (itemDef && itemDef->isMan())
              {
                hasLeadItem = true;
                break;
              }
            }
          }
          if (!hasLeadItem)
          {
            return; // Teacher doesn't have LEAD items, can't teach
          }
        }

        // Get teacher's skills from simulation
        auto teacherSkillsIt = simulation.skillsByUnit.find(originUnitNumber);
        if (teacherSkillsIt == simulation.skillsByUnit.end())
        {
          return;
        }

        const std::map<std::wstring, int>& teacherSkills = teacherSkillsIt->second;
        
        // Collect valid students and their studied skills
        int totalStudentManCount = 0;
        std::map<int, std::pair<std::wstring, int>> studentSkillsToTeach; // studentNumber -> (skillToken, studentManCount)

        for (int studentUnitNumber : teachCommand.studentUnitNumbers)
        {
          const auto studentUnitIt = simulation.nearbyUnits.find(studentUnitNumber);
          if (studentUnitIt == simulation.nearbyUnits.end() || studentUnitIt->second == nullptr)
          {
            continue; // Student unit not in region
          }

          // Find what skill the student is studying from repository-backed orders
          std::wstring studiedSkillToken;
          const std::vector<Order>* studentOrders =
            appData.orderRepository().getOrdersForUnit(studentUnitNumber, false);
          if (!studentOrders)
          {
            continue; // Student has no tracked orders
          }

          for (const Order& order : *studentOrders)
          {
            StudyCommand studyCmd;
            if (tryParseStudyCommand(order.getFullOrderText(), studyCmd))
            {
              const Skill* skill = findSkillByTokenOrNameNormalized(appData, studyCmd.skillToken);
              if (skill)
              {
                studiedSkillToken = skill->getIdentifierToken();
              }
              break;
            }
          }

          if (studiedSkillToken.empty())
          {
            continue; // Student is not studying
          }

          // Check if teacher knows this skill at a level higher than student
          auto teacherSkillIt = teacherSkills.find(studiedSkillToken);
          if (teacherSkillIt == teacherSkills.end())
          {
            continue; // Teacher doesn't know this skill
          }

          int teacherSkillLevel = Skill::trainingDaysToLevel(teacherSkillIt->second);
          auto studentSkillsIt = simulation.skillsByUnit.find(studentUnitNumber);
          int studentSkillLevel = 0;
          if (studentSkillsIt != simulation.skillsByUnit.end())
          {
            auto studentSkillIt = studentSkillsIt->second.find(studiedSkillToken);
            if (studentSkillIt != studentSkillsIt->second.end())
            {
              studentSkillLevel = Skill::trainingDaysToLevel(studentSkillIt->second);
            }
          }

          // Only teach if teacher's level is higher than student's
          if (teacherSkillLevel <= studentSkillLevel)
          {
            continue;
          }

          // Get student's man count
          auto studentCountsIt = simulation.itemCountsByUnit.find(studentUnitNumber);
          if (studentCountsIt == simulation.itemCountsByUnit.end())
          {
            continue;
          }

          int studentManCount = appData.itemRepository().calculateManItemCount(studentCountsIt->second);
          if (studentManCount <= 0)
          {
            studentManCount = 1; // Fallback
          }

          studentSkillsToTeach[studentUnitNumber] = { studiedSkillToken, studentManCount };
          totalStudentManCount += studentManCount;
        }

        if (studentSkillsToTeach.empty())
        {
          return; // No valid students to teach
        }

        // Calculate teaching bonus
        // Teacher can teach at most 10 isMan items per skill, with full 30 day bonus per student's isMan
        // If exceeded, distribute teaching capacity equally among all students' isMan items
        const int teachingCapacity = teacherManCount * 10;
        const int bonusDaysPerManIfUnlimited = 30;

        int actualBonusPerStudent = bonusDaysPerManIfUnlimited;
        if (totalStudentManCount > teachingCapacity)
        {
          // Distribute bonus pool: teacher capacity (10 per isMan) times 30 days, divided by all student isMan
          actualBonusPerStudent = (teacherManCount * 10 * bonusDaysPerManIfUnlimited) / totalStudentManCount;
        }

        // Apply bonuses to each student
        for (const auto& [studentUnitNumber, skillData] : studentSkillsToTeach)
        {
          const std::wstring& skillToken = skillData.first;

          auto studentSkillsIt = simulation.skillsByUnit.find(studentUnitNumber);
          if (studentSkillsIt == simulation.skillsByUnit.end())
          {
            continue;
          }

          std::map<std::wstring, int>& studentSkills = studentSkillsIt->second;
          const int currentDays = studentSkills.count(skillToken) > 0 ? studentSkills[skillToken] : 0;
          const int proposedDays = currentDays + actualBonusPerStudent; // TEACH bonus only; STUDY already added 30 days

          // Apply skill limit
          auto studentCountsIt = simulation.itemCountsByUnit.find(studentUnitNumber);
          if (studentCountsIt != simulation.itemCountsByUnit.end())
          {
            const std::optional<int> studentLevelLimit = resolveUnitSkillLevelLimit(appData, studentCountsIt->second, skillToken);
            if (studentLevelLimit.has_value())
            {
              const int cappedDays = maxTrainingDaysForLevelLimit(*studentLevelLimit);
              if (proposedDays > cappedDays)
              {
                studentSkills[skillToken] = (std::max)(currentDays, (std::min)(proposedDays, cappedDays));
              }
              else
              {
                studentSkills[skillToken] = proposedDays;
              }
            }
            else
            {
              studentSkills[skillToken] = proposedDays;
            }
          }
          else
          {
            studentSkills[skillToken] = proposedDays;
          }
        }

        return;
      }

      case SimulatedCommandKind::NameUnit:
      {
        const std::wstring resolvedName = StringUtils::trimWhitespace(parsedCommand.nameUnit.newName);
        if (resolvedName.empty())
        {
          return;
        }

        if (isNewUnit)
        {
          simulation.unitNamesByNewUnit[originUnitNumber] = resolvedName;
        }
        else
        {
          simulation.unitNamesByUnit[originUnitNumber] = resolvedName;
        }

        return;
      }

      case SimulatedCommandKind::Move:
      {
        // MOVE command has no simulation effect - it's purely for display on the map
        return;
      }

      case SimulatedCommandKind::Sail:
      {
        // SAIL is display-only for now and valid only for ship owners.
        if (!isUnitShipOwnerAtCurrentLocation(appData, *originUnit))
        {
          return;
        }

        return;
      }

      case SimulatedCommandKind::Leave:
      {
        // LEAVE command has no simulation effect in this phase
        return;
      }

      case SimulatedCommandKind::Enter:
      {
        // ENTER command has no simulation effect in this phase
        return;
      }

      default:
        return;
    }
  }

  /**
  * @brief Runs full region command simulation and returns resulting state snapshots.
  */
  RegionCommandSimulation simulateRegionCommands(const AppData& appData,
                                                int regionX,
                                                int regionY,
                                                int regionZ)
  {
    RegionCommandSimulation simulation;
    std::vector<ScheduledCommand> scheduledCommands;

    const UnitRepository& unitRepository = appData.unitRepository();
    for (std::size_t index = 0; index < unitRepository.size(); ++index)
    {
      const Unit& candidate = unitRepository.at(index);
      const int unitNumber = candidate.getUnitNumber();
      if (candidate.getXCoordinate() == regionX &&
          candidate.getYCoordinate() == regionY &&
          candidate.getZCoordinate() == regionZ)
      {
        simulation.regionUnits[unitNumber] = &candidate;
      }

      if (candidate.getZCoordinate() != regionZ)
      {
        continue;
      }

      if (doubledHexDistance(candidate.getXCoordinate(), candidate.getYCoordinate(), regionX, regionY) <= 2)
      {
        simulation.nearbyUnits[unitNumber] = &candidate;
        simulation.itemCountsByUnit[unitNumber] = normalizeItemCountsMap(candidate.getItems());
        simulation.skillsByUnit[unitNumber] = candidate.getSkills();
        simulation.unitNamesByUnit[unitNumber] = candidate.getUnitName();
      }
    }

    const UnitNewRepository& unitNewRepository = appData.unitNewRepository();
    for (std::size_t index = 0; index < unitNewRepository.size(); ++index)
    {
      const UnitNew& candidate = unitNewRepository.at(index);
      const int unitNumber = candidate.getUnitNumber();
      if (candidate.getXCoordinate() == regionX &&
          candidate.getYCoordinate() == regionY &&
          candidate.getZCoordinate() == regionZ)
      {
        simulation.regionNewUnits[unitNumber] = &candidate;
      }

      if (candidate.getZCoordinate() != regionZ)
      {
        continue;
      }

      if (doubledHexDistance(candidate.getXCoordinate(), candidate.getYCoordinate(), regionX, regionY) <= 2)
      {
        simulation.nearbyNewUnits[unitNumber] = &candidate;
        simulation.itemCountsByNewUnit[unitNumber] = normalizeItemCountsMap(candidate.getItems());
        simulation.skillsByNewUnit[unitNumber] = candidate.getSkills();
        simulation.unitNamesByNewUnit[unitNumber] = candidate.getUnitName();
      }
    }

    const Region* region = appData.regionRepository().findByCoordinates(regionX, regionY, regionZ);
    if (!region)
    {
      // Some datasets may have region z-level mismatches while units still carry z.
      region = appData.regionRepository().findByCoordinates(regionX, regionY);
    }

    if (region)
    {
      simulation.remainingTaxableIncome = std::max(0, region->getTaxableIncome());
      simulation.remainingEntertainment = std::max(0, region->getEntertainment());
      simulation.remainingWorkWages = std::max(0, region->getWagesMax());
      for (const auto& [resourceToken, amount] : region->getResources())
      {
        if (amount <= 0)
        {
          continue;
        }

        const std::wstring normalizedToken = normalizeItemToken(resourceToken);
        if (normalizedToken.empty())
        {
          continue;
        }

        simulation.remainingResources[normalizedToken] += amount;
      }

      for (const auto& [itemToken, amountPrice] : region->getForSale())
      {
        const std::wstring normalizedToken = normalizeItemToken(itemToken);
        if (!normalizedToken.empty())
        {
          simulation.remainingForSale[normalizedToken] = amountPrice;
        }
      }

      for (const auto& [itemToken, amountPrice] : region->getWanted())
      {
        const std::wstring normalizedToken = normalizeItemToken(itemToken);
        if (!normalizedToken.empty())
        {
          simulation.remainingWanted[normalizedToken] = amountPrice;
        }
      }
    }

    for (const auto& [originUnitNumber, originUnit] : simulation.nearbyUnits)
    {
      if (!originUnit)
      {
        continue;
      }

      const bool originIsLocal = simulation.regionUnits.find(originUnitNumber) != simulation.regionUnits.end();
      const std::vector<Order>* storedOrders = appData.orderRepository().getOrdersForUnit(originUnitNumber, false);
      if (!storedOrders)
      {
        continue;
      }

      for (const Order& storedOrder : *storedOrders)
      {
        ParsedOrderCommand parsedCommand;
        if (!tryParseSupportedOrderCommand(storedOrder.getFullOrderText(), parsedCommand))
        {
          continue;
        }

        scheduledCommands.push_back({ originUnitNumber, originUnit, originIsLocal, parsedCommand, false });
      }
    }

    for (const auto& [originUnitNumber, originNewUnit] : simulation.nearbyNewUnits)
    {
      if (!originNewUnit)
      {
        continue;
      }

      const bool originIsLocal = simulation.regionNewUnits.find(originUnitNumber) != simulation.regionNewUnits.end();
      const std::vector<Order>* storedOrders = appData.orderRepository().getOrdersForUnit(originUnitNumber, true);
      if (!storedOrders)
      {
        continue;
      }

      for (const Order& storedOrder : *storedOrders)
      {
        ParsedOrderCommand parsedCommand;
        if (!tryParseSupportedOrderCommand(storedOrder.getFullOrderText(), parsedCommand))
        {
          continue;
        }

        scheduledCommands.push_back({ originUnitNumber, nullptr, originIsLocal, parsedCommand, true });
      }
    }

    constexpr std::array<CommandPhase, 9> kCommandPhases = {
      CommandPhase::GiveTake,
      CommandPhase::Tax,
      CommandPhase::Sell,
      CommandPhase::Buy,
      CommandPhase::Produce,
      CommandPhase::Entertain,
      CommandPhase::Name,
      CommandPhase::Work,
      CommandPhase::Transport,
    };

    for (const CommandPhase phase : kCommandPhases)
    {
      for (const ScheduledCommand& scheduledCommand : scheduledCommands)
      {
        if (getCommandPhase(scheduledCommand.parsedCommand) != phase)
        {
          continue;
        }

        executeScheduledCommand(appData, region, simulation, scheduledCommand);
      }
    }

    return simulation;
  }
}

/**
* @brief Calculates post-command item counts for a specific unit.
*/
std::map<std::wstring, int> Commands::calculateAfterCommandItemCountsForUnit(const AppData& appData,
                                                                              const Unit& unit)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unit.getXCoordinate(),
    unit.getYCoordinate(),
    unit.getZCoordinate());

  const auto targetIt = simulation.itemCountsByUnit.find(unit.getUnitNumber());
  if (targetIt == simulation.itemCountsByUnit.end())
  {
    return unit.getItems();
  }

  return targetIt->second;
}

/**
* @brief Calculates post-command item counts for a specific UnitNew.
*/
std::map<std::wstring, int> Commands::calculateAfterCommandItemCountsForUnitNew(const AppData& appData,
                                                                                 const UnitNew& unitNew)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unitNew.getXCoordinate(),
    unitNew.getYCoordinate(),
    unitNew.getZCoordinate());

  const auto targetIt = simulation.itemCountsByNewUnit.find(unitNew.getUnitNumber());
  if (targetIt == simulation.itemCountsByNewUnit.end())
  {
    return unitNew.getItems();
  }

  return targetIt->second;
}

/**
* @brief Calculates unit skill days after command simulation.
*/
std::map<std::wstring, int> Commands::calculateAfterCommandSkillDaysForUnit(const AppData& appData,
                                                                            const Unit& unit)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unit.getXCoordinate(),
    unit.getYCoordinate(),
    unit.getZCoordinate());

  const auto targetIt = simulation.skillsByUnit.find(unit.getUnitNumber());
  if (targetIt == simulation.skillsByUnit.end())
  {
    return unit.getSkills();
  }

  return targetIt->second;
}

std::map<std::wstring, int> Commands::calculateAfterCommandSkillDaysForUnitNew(const AppData& appData,
                                                                               const UnitNew& unitNew)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unitNew.getXCoordinate(),
    unitNew.getYCoordinate(),
    unitNew.getZCoordinate());

  const auto targetIt = simulation.skillsByNewUnit.find(unitNew.getUnitNumber());
  if (targetIt == simulation.skillsByNewUnit.end())
  {
    return unitNew.getSkills();
  }

  return targetIt->second;
}

std::wstring Commands::calculateAfterCommandUnitNameForUnit(const AppData& appData,
                                                            const Unit& unit)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unit.getXCoordinate(),
    unit.getYCoordinate(),
    unit.getZCoordinate());

  const auto targetIt = simulation.unitNamesByUnit.find(unit.getUnitNumber());
  if (targetIt == simulation.unitNamesByUnit.end())
  {
    return unit.getUnitName();
  }

  return targetIt->second;
}

std::wstring Commands::calculateAfterCommandUnitNameForUnitNew(const AppData& appData,
                                                               const UnitNew& unitNew)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    unitNew.getXCoordinate(),
    unitNew.getYCoordinate(),
    unitNew.getZCoordinate());

  const auto targetIt = simulation.unitNamesByNewUnit.find(unitNew.getUnitNumber());
  if (targetIt == simulation.unitNamesByNewUnit.end())
  {
    return unitNew.getUnitName();
  }

  return targetIt->second;
}

/**
* @brief Calculates remaining regional resources after command simulation.
*/
std::map<std::wstring, int> Commands::calculateAfterCommandRegionResources(const AppData& appData,
                                                                            const Region& region)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate());

  return simulation.remainingResources;
}

/**
* @brief Calculates remaining regional for-sale market entries after simulation.
*/
std::map<std::wstring, std::pair<int, int>> Commands::calculateAfterCommandRegionForSale(const AppData& appData,
                                                                                          const Region& region)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate());

  return simulation.remainingForSale;
}

/**
* @brief Calculates remaining regional wanted market entries after simulation.
*/
std::map<std::wstring, std::pair<int, int>> Commands::calculateAfterCommandRegionWanted(const AppData& appData,
                                                                                         const Region& region)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate());

  return simulation.remainingWanted;
}

/**
* @brief Calculates remaining tax/entertain/work economy pools after simulation.
*/
Commands::RegionEconomyAfterCommands Commands::calculateAfterCommandRegionEconomy(const AppData& appData,
                                                                                  const Region& region)
{
  const RegionCommandSimulation simulation = simulateRegionCommands(
    appData,
    region.getXCoordinate(),
    region.getYCoordinate(),
    region.getZCoordinate());

  RegionEconomyAfterCommands economy {};
  economy.remainingTaxableIncome = simulation.remainingTaxableIncome;
  economy.remainingEntertainment = simulation.remainingEntertainment;
  economy.remainingWorkWages = simulation.remainingWorkWages;
  return economy;
}

/**
* @brief Returns month-long order keywords as CSV text.
*/
std::wstring Commands::getFullMonthOrderKeywordsCsv()
{
  return joinCsvKeywordList(gFullMonthOrderKeywords);
}

/**
* @brief Updates month-long command keywords from CSV input.
*/
void Commands::setFullMonthOrderKeywordsCsv(const std::wstring& csvKeywords)
{
  std::vector<std::wstring> parsedKeywords = parseCsvKeywordList(csvKeywords);
  if (parsedKeywords.empty())
  {
    parsedKeywords = kDefaultFullMonthOrderKeywords;
  }

  gFullMonthOrderKeywords = std::move(parsedKeywords);
}

/**
* @brief Returns the normalized in-memory month-long keyword list.
*/
std::vector<std::wstring> Commands::getFullMonthOrderKeywords()
{
  return gFullMonthOrderKeywords;
}
