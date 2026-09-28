/*
 * Atlantis Majordomo
 *
 * Shared local-engine faction file operations.
 */
#include "Function/LocalEngineFactionUtils.hpp"

#include <fstream>

namespace
{
std::wstring trimWhitespace(const std::wstring& value)
{
  const size_t firstNonWhitespace = value.find_first_not_of(L" \t\r");
  const size_t lastNonWhitespace = value.find_last_not_of(L" \t\r");
  if (firstNonWhitespace == std::wstring::npos)
  {
    return L"";
  }

  return value.substr(firstNonWhitespace, lastNonWhitespace - firstNonWhitespace + 1);
}
}

namespace LocalEngineFactionUtils
{
AddFactionResult addFaction(const std::filesystem::path& enginePath,
                            const std::wstring& factionName)
{
  if (factionName.empty())
  {
    return AddFactionResult::EmptyFactionName;
  }

  const std::filesystem::path playersPath = enginePath.parent_path() / L"players.out";
  std::error_code fileError;
  const bool playersFileExists = std::filesystem::exists(playersPath, fileError);
  if (fileError)
  {
    return AddFactionResult::ReadError;
  }
  if (!playersFileExists)
  {
    return AddFactionResult::PlayersFileMissing;
  }

  const auto existingFileSize = std::filesystem::file_size(playersPath, fileError);
  if (fileError)
  {
    return AddFactionResult::ReadError;
  }

  std::wifstream existingPlayersFile(playersPath);
  if (!existingPlayersFile.is_open())
  {
    return AddFactionResult::ReadError;
  }

  std::wstring line;
  while (std::getline(existingPlayersFile, line))
  {
    if (line.rfind(L"Name:", 0) != 0)
    {
      continue;
    }

    const std::wstring existingFactionName = trimWhitespace(line.substr(5));
    if (existingFactionName == factionName)
    {
      return AddFactionResult::DuplicateName;
    }
  }
  if (existingPlayersFile.bad())
  {
    return AddFactionResult::ReadError;
  }

  std::wofstream playersFile(playersPath, std::ios::out | std::ios::app);
  if (!playersFile.is_open())
  {
    return AddFactionResult::WriteError;
  }

  if (existingFileSize > 0)
  {
    playersFile << L'\n';
  }
  playersFile << L"Faction: new\n"
              << L"Name: " << factionName << L"\n"
              << L"Email: NoAddress\n";
  if (!playersFile)
  {
    return AddFactionResult::WriteError;
  }

  return AddFactionResult::Added;
}
}
