#include "Function/LocalEngineSettingsUtils.hpp"

#include "Function/JsonUtils.hpp"

#include <fstream>
#include <sstream>

namespace LocalEngineSettingsUtils
{
std::filesystem::path settingsPath(const std::filesystem::path& enginePath)
{
  return enginePath.parent_path() / L"local_engine_settings.json";
}

bool load(const std::filesystem::path& enginePath, Settings& settings)
{
  std::wifstream input(settingsPath(enginePath));
  if (!input.is_open()) return false;

  std::wstringstream content;
  content << input.rdbuf();
  const std::wstring json = content.str();
  std::wstring value;
  if (JsonUtils::extractJsonStringField(json, L"factionName", value)) settings.factionName = value;
  if (JsonUtils::extractJsonStringField(json, L"mainFaction", value)) settings.mainFaction = value;
  if (JsonUtils::extractJsonStringField(json, L"playerNumber", value)) settings.playerNumber = value;
  if (JsonUtils::extractJsonStringField(json, L"ordersPath", value)) settings.ordersPath = value;
  if (JsonUtils::extractJsonFieldValue(json, L"currentTurn", value))
  {
    try
    {
      settings.currentTurn = std::stoi(value);
    }
    catch (...)
    {
      return false;
    }
  }
  return true;
}

bool save(const std::filesystem::path& enginePath, const Settings& settings)
{
  std::wofstream output(settingsPath(enginePath), std::ios::trunc);
  if (!output.is_open()) return false;

  output << L"{\n"
          << L"  \"factionName\": \"" << JsonUtils::escapeJsonString(settings.factionName) << L"\",\n"
          << L"  \"mainFaction\": \"" << JsonUtils::escapeJsonString(settings.mainFaction) << L"\",\n"
          << L"  \"playerNumber\": \"" << JsonUtils::escapeJsonString(settings.playerNumber) << L"\",\n"
          << L"  \"ordersPath\": \"" << JsonUtils::escapeJsonString(settings.ordersPath) << L"\",\n"
          << L"  \"currentTurn\": " << settings.currentTurn << L"\n"
          << L"}\n";
  return static_cast<bool>(output);
}
}
