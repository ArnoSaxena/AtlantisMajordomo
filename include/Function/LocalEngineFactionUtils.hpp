/*
 * Atlantis Majordomo
 *
 * Shared local-engine faction file operations.
 */
#pragma once

#include <filesystem>
#include <string>

namespace LocalEngineFactionUtils
{
enum class AddFactionResult
{
  Added,
  EmptyFactionName,
  PlayersFileMissing,
  ReadError,
  DuplicateName,
  WriteError,
};

AddFactionResult addFaction(const std::filesystem::path& enginePath,
                            const std::wstring& factionName);
}
