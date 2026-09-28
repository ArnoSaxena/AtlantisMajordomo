#include "Function/LocalEngineRunUtils.hpp"

#include "Function/MonthUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <limits>
#include <vector>

namespace
{
std::wstring trim(const std::wstring& value)
{
  return StringUtils::trimWhitespace(value);
}

bool parsePositiveNumber(const std::wstring& value, int& number)
{
  if (value.empty()) return false;
  for (wchar_t character : value)
  {
    if (character < L'0' || character > L'9') return false;
  }
  try
  {
    const unsigned long parsed = std::stoul(value);
    if (parsed > static_cast<unsigned long>(std::numeric_limits<int>::max())) return false;
    number = static_cast<int>(parsed);
    return number > 0;
  }
  catch (...)
  {
    return false;
  }
}

bool parseNonNegativeNumber(const std::wstring& value, int& number)
{
  if (value.empty()) return false;
  for (wchar_t character : value)
  {
    if (character < L'0' || character > L'9') return false;
  }
  try
  {
    const unsigned long parsed = std::stoul(value);
    if (parsed > static_cast<unsigned long>(std::numeric_limits<int>::max())) return false;
    number = static_cast<int>(parsed);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool numberedFileFromPath(const std::filesystem::path& path,
                          const std::wstring& prefix,
                          int& number)
{
  const std::wstring name = path.filename().wstring();
  if (name.rfind(prefix, 0) != 0) return false;
  return parsePositiveNumber(name.substr(prefix.size()), number);
}

bool reportNumberFromPath(const std::filesystem::path& path, int& number)
{
  return numberedFileFromPath(path, L"report.", number);
}

bool templateNumberFromPath(const std::filesystem::path& path, int& number)
{
  return numberedFileFromPath(path, L"template.", number);
}

bool removeMatchingFiles(const std::filesystem::path& folder,
                         const std::function<bool(const std::filesystem::path&)>& predicate)
{
  std::error_code error;
  for (const auto& entry : std::filesystem::directory_iterator(folder, error))
  {
    if (error) return false;
    if (entry.is_regular_file(error) && predicate(entry.path()))
    {
      std::filesystem::remove(entry.path(), error);
      if (error) return false;
    }
  }
  return !error;
}

bool hasOrderFile(const std::filesystem::path& folder, int factionNumber)
{
  std::error_code error;
  const std::filesystem::path singular = folder / (L"order." + std::to_wstring(factionNumber));
  if (std::filesystem::is_regular_file(singular, error)) return true;
  error.clear();
  const std::filesystem::path plural = folder / (L"orders." + std::to_wstring(factionNumber));
  return std::filesystem::is_regular_file(plural, error);
}

int readTurnNumberFromFile(const std::filesystem::path& path)
{
  std::wifstream input(path);
  if (!input.is_open()) return 0;

  std::wstring line;
  while (std::getline(input, line))
  {
    const std::wstring trimmedLine = trim(line);
    if (trimmedLine.rfind(L"TurnNumber:", 0) == 0)
    {
      int turn = 0;
      if (parseNonNegativeNumber(trim(trimmedLine.substr(11)), turn)) return turn;
    }
  }
  return 0;
}

bool updatePlayerOrderTurns(const std::filesystem::path& folder)
{
  const std::filesystem::path playerFile = folder / L"player.in";
  const int playersTurn = readTurnNumberFromFile(folder / L"players.in");
  std::error_code existenceError;
  if (!std::filesystem::exists(playerFile, existenceError))
  {
    return !existenceError;
  }

  std::wifstream input(playerFile);
  if (!input.is_open()) return false;

  std::vector<std::wstring> lines;
  std::wstring line;
  int currentTurn = 0;
  int playerTurn = 0;
  bool foundTurn = false;
  while (std::getline(input, line))
  {
    lines.push_back(line);
    const std::wstring trimmedLine = trim(line);
    if (trimmedLine.rfind(L"TurnNumber:", 0) == 0)
    {
      foundTurn = parseNonNegativeNumber(trim(trimmedLine.substr(11)), playerTurn);
    }
  }
  if (input.bad() || !foundTurn) return false;

  currentTurn = (std::max)(playerTurn, playersTurn);

  bool updateCurrentFaction = false;
  for (std::wstring& currentLine : lines)
  {
    const std::wstring trimmedLine = trim(currentLine);
    if (trimmedLine.rfind(L"TurnNumber:", 0) == 0)
    {
      const std::size_t valueStart = currentLine.find(L"TurnNumber:") + 11;
      currentLine = currentLine.substr(0, valueStart) + L" " + std::to_wstring(currentTurn);
      continue;
    }

    if (trimmedLine.rfind(L"Faction:", 0) == 0)
    {
      int factionNumber = 0;
      updateCurrentFaction = parsePositiveNumber(trim(trimmedLine.substr(8)), factionNumber)
        && hasOrderFile(folder, factionNumber);
      continue;
    }

    if (updateCurrentFaction && trimmedLine.rfind(L"LastOrders:", 0) == 0)
    {
      int existingLastOrders = 0;
      const bool hasExistingLastOrders = parseNonNegativeNumber(trim(trimmedLine.substr(11)), existingLastOrders);
      const int updatedLastOrders = hasExistingLastOrders
        ? (std::max)(existingLastOrders, currentTurn)
        : currentTurn;
      const std::size_t valueStart = currentLine.find(L"LastOrders:") + 11;
      currentLine = currentLine.substr(0, valueStart) + L" " + std::to_wstring(updatedLastOrders);
    }
  }

  const std::filesystem::path temporaryFile = folder / L"player.in.tmp";
  std::wofstream output(temporaryFile, std::ios::trunc);
  if (!output.is_open()) return false;
  for (const std::wstring& outputLine : lines)
  {
    output << outputLine << L'\n';
  }
  output.close();
  if (!output) return false;

  std::error_code error;
  std::filesystem::remove(playerFile, error);
  if (error) return false;
  std::filesystem::rename(temporaryFile, playerFile, error);
  return !error;
}

int readCurrentTurn(const std::filesystem::path& folder)
{
  return readTurnNumberFromFile(folder / L"players.out");
}
}

namespace LocalEngineRunUtils
{
RunStatus prepareRun(const std::filesystem::path& enginePath)
{
  const std::filesystem::path folder = enginePath.parent_path();
  std::error_code error;
  if (folder.empty() || !std::filesystem::is_directory(folder, error) || error)
  {
    return RunStatus::EngineFolderError;
  }

  if (!removeMatchingFiles(folder, [](const std::filesystem::path& path)
  {
    return path.extension() == L".in";
  }))
  {
    return RunStatus::PreparationError;
  }

  for (const auto& entry : std::filesystem::directory_iterator(folder, error))
  {
    if (error) return RunStatus::PreparationError;
    if (!entry.is_regular_file(error) || entry.path().extension() != L".out") continue;

    std::filesystem::path destination = entry.path();
    destination.replace_extension(L".in");
    std::filesystem::rename(entry.path(), destination, error);
    if (error) return RunStatus::PreparationError;
  }

  if (!updatePlayerOrderTurns(folder))
  {
    return RunStatus::PreparationError;
  }

  if (!removeMatchingFiles(folder, [](const std::filesystem::path& path)
  {
    int reportNumber = 0;
    int templateNumber = 0;
    return reportNumberFromPath(path, reportNumber)
      || templateNumberFromPath(path, templateNumber);
  }))
  {
    return RunStatus::PreparationError;
  }

  return RunStatus::Success;
}

FinalizeResult finalizeRun(const std::filesystem::path& enginePath,
                           const std::wstring& mainFactionText,
                           const std::filesystem::path& configuredReportsFolder,
                           const std::filesystem::path& fallbackReportsFolder)
{
  const std::filesystem::path engineFolder = enginePath.parent_path();
  std::error_code error;
  int mainFaction = 0;
  if (!trim(mainFactionText).empty() && !parsePositiveNumber(trim(mainFactionText), mainFaction))
  {
    return { RunStatus::ReportNotFound, {} };
  }

  if (mainFaction == 0)
  {
    int smallest = std::numeric_limits<int>::max();
    for (const auto& entry : std::filesystem::directory_iterator(engineFolder, error))
    {
      if (error) return { RunStatus::ReportReadError, {} };
      int candidate = 0;
      if (entry.is_regular_file(error) && reportNumberFromPath(entry.path(), candidate) && candidate > 2)
      {
        smallest = (std::min)(smallest, candidate);
      }
    }
    if (smallest == std::numeric_limits<int>::max()) return { RunStatus::ReportNotFound, {} };
    mainFaction = smallest;
  }

  const std::filesystem::path sourceReport = engineFolder / (L"report." + std::to_wstring(mainFaction));
  if (!std::filesystem::is_regular_file(sourceReport, error))
  {
    return { RunStatus::ReportNotFound, {} };
  }

  std::wifstream reportFile(sourceReport);
  if (!reportFile.is_open()) return { RunStatus::ReportReadError, {} };

  std::wstring line;
  int reportMarkerLine = 0;
  std::wstring dateLine;
  while (std::getline(reportFile, line))
  {
    if (trim(line) == L"Atlantis Report For:")
    {
      reportMarkerLine = 1;
      continue;
    }
    if (reportMarkerLine > 0)
    {
      ++reportMarkerLine;
      if (reportMarkerLine == 3)
      {
        dateLine = trim(line);
        break;
      }
    }
  }

  const std::size_t comma = dateLine.find(L',');
  const std::size_t yearMarker = dateLine.find(L"Year", comma == std::wstring::npos ? 0 : comma + 1);
  if (comma == std::wstring::npos || yearMarker == std::wstring::npos)
  {
    return { RunStatus::InvalidReportDate, {} };
  }

  int month = 0;
  if (!MonthUtils::tryParseMonthName(trim(dateLine.substr(0, comma)), month))
  {
    return { RunStatus::InvalidReportDate, {} };
  }
  int year = 0;
  if (!parsePositiveNumber(trim(dateLine.substr(yearMarker + 4)), year))
  {
    return { RunStatus::InvalidReportDate, {} };
  }

  std::filesystem::path reportsFolder = configuredReportsFolder;
  if (reportsFolder.empty() || !std::filesystem::is_directory(reportsFolder, error))
  {
    reportsFolder = fallbackReportsFolder;
    error.clear();
    std::filesystem::create_directories(reportsFolder, error);
    if (error) return { RunStatus::ReportWriteError, {} };
  }

  const std::filesystem::path outputPath = reportsFolder /
    (L"report_" + std::to_wstring(mainFaction) + L"_" +
     (year < 100 ? (year < 10 ? L"00" : L"0") : L"") + std::to_wstring(year) + L"_" +
     (month < 10 ? L"0" : L"") + std::to_wstring(month) + L".txt");

  std::ifstream source(sourceReport, std::ios::binary);
  std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
  if (!source.is_open() || !output.is_open()) return { RunStatus::ReportWriteError, {} };
  output << source.rdbuf() << "\n\n";

  const std::filesystem::path templatePath = engineFolder / (L"template." + std::to_wstring(mainFaction));
  std::ifstream templateFile(templatePath, std::ios::binary);
  if (templateFile.is_open()) output << templateFile.rdbuf();
  if (!output) return { RunStatus::ReportWriteError, {} };

  return { RunStatus::Success, outputPath, mainFaction, readCurrentTurn(engineFolder) };
}
}
