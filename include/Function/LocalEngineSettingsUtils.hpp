#pragma once

#include <filesystem>
#include <string>

namespace LocalEngineSettingsUtils
{
struct Settings
{
  std::wstring factionName;
  std::wstring mainFaction;
  std::wstring playerNumber;
  std::wstring ordersPath;
  int currentTurn { 0 };
};

std::filesystem::path settingsPath(const std::filesystem::path& enginePath);
bool load(const std::filesystem::path& enginePath, Settings& settings);
bool save(const std::filesystem::path& enginePath, const Settings& settings);
}
