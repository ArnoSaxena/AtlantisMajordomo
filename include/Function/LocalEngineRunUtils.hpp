#pragma once

#include <filesystem>
#include <string>

namespace LocalEngineRunUtils
{
enum class RunStatus
{
  Success,
  EngineFolderError,
  PreparationError,
  ReportNotFound,
  InvalidReportDate,
  ReportReadError,
  ReportWriteError,
};

struct FinalizeResult
{
  RunStatus status { RunStatus::ReportReadError };
  std::filesystem::path reportPath;
  int mainFaction { 0 };
  int currentTurn { 0 };
};

RunStatus prepareRun(const std::filesystem::path& enginePath);
FinalizeResult finalizeRun(const std::filesystem::path& enginePath,
                           const std::wstring& mainFactionText,
                           const std::filesystem::path& configuredReportsFolder,
                           const std::filesystem::path& fallbackReportsFolder);
}
