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
 * File: LocalEngineTabContent.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/LocalEngineTabContent.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/LocalEngineFactionUtils.hpp"
#include "Function/LocalEngineRunUtils.hpp"
#include "Function/LocalEngineSettingsUtils.hpp"

#include <commctrl.h>
#include <commdlg.h>

#include <algorithm>
#include <filesystem>

namespace
{
constexpr UINT_PTR kTerminalSubclassId = 1;

constexpr int kLocalCheckId = 10001;
constexpr int kBrowseEngineButtonId = 10002;
constexpr int kNewButtonId = 10003;
constexpr int kBrowseOrdersButtonId = 10004;
constexpr int kAddFactionButtonId = 10005;
constexpr int kRunButtonId = 10006;
constexpr int kFactionNameEditId = 10007;
constexpr int kPlayerNumberEditId = 10008;
constexpr int kOrdersEditId = 10009;
constexpr int kMainFactionEditId = 10010;

std::wstring directoryFromFilePath(const std::wstring& filePath)
{
  if (filePath.empty())
  {
    wchar_t currentDir[MAX_PATH] = {};
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    return currentDir;
  }

  std::error_code ec;
  if (std::filesystem::is_directory(filePath, ec))
  {
    return filePath;
  }

  const std::filesystem::path path(filePath);
  if (path.has_parent_path())
  {
    return path.parent_path().wstring();
  }

  wchar_t currentDir[MAX_PATH] = {};
  GetCurrentDirectoryW(MAX_PATH, currentDir);
  return currentDir;
}

std::wstring getFileDialogPath(HWND hwnd,
                               bool isSaveDialog,
                               const std::wstring& configuredSaveFilePath,
                               const wchar_t* defaultExtension,
                               const wchar_t* filterString)
{
  wchar_t szFile[MAX_PATH] = {};
  wchar_t szDir[MAX_PATH] = {};

  std::error_code isDirEc;
  const bool configuredPathIsDirectory = !configuredSaveFilePath.empty()
    && std::filesystem::is_directory(configuredSaveFilePath, isDirEc);

  if (!configuredSaveFilePath.empty() && !configuredPathIsDirectory)
  {
    wcsncpy_s(szFile,
              sizeof(szFile) / sizeof(szFile[0]),
              configuredSaveFilePath.c_str(),
              _TRUNCATE);
  }

  const std::wstring initialDir = directoryFromFilePath(configuredSaveFilePath);
  wcsncpy_s(szDir,
            sizeof(szDir) / sizeof(szDir[0]),
            initialDir.c_str(),
            _TRUNCATE);

  OPENFILENAMEW ofn {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
  ofn.lpstrInitialDir = szDir;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
  ofn.lpstrFilter = filterString;
  ofn.nFilterIndex = 1;
  ofn.lpstrDefExt = defaultExtension;

  std::wstring result;
  if (isSaveDialog)
  {
    ofn.Flags |= OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameW(&ofn))
    {
      result = szFile;
    }
  }
  else
  {
    ofn.Flags |= OFN_FILEMUSTEXIST;
    if (GetOpenFileNameW(&ofn))
    {
      result = szFile;
    }
  }

  return result;
}

std::wstring readWindowText(HWND control, int bufferSize)
{
  std::wstring value(static_cast<std::size_t>(bufferSize), L'\0');
  const int length = GetWindowTextW(control, value.data(), bufferSize);
  value.resize(static_cast<std::size_t>((std::max)(0, length)));
  return value;
}
}

LocalEngineTabContent::~LocalEngineTabContent()
{
  stopProcess();
}

bool LocalEngineTabContent::create(HWND ownerWindow,
                                   HINSTANCE instance,
                                   HWND panel,
                                   AppData& appData,
                                   AppConfig& appConfig,
                                   Callbacks callbacks)
{
  ownerWindow_ = ownerWindow;
  instance_ = instance;
  panel_ = panel;
  appData_ = &appData;
  appConfig_ = &appConfig;
  callbacks_ = std::move(callbacks);

  if (!panel_)
  {
    return false;
  }

  localCheck_ = CreateWindowExW(0, L"BUTTON", L"Local", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
    12, 8, 120, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLocalCheckId)), instance_, nullptr);

  pathLabel_ = CreateWindowExW(0, L"STATIC", L"Engine", WS_CHILD | WS_VISIBLE | SS_LEFT,
    12, 48, 60, 24, panel_, nullptr, instance_, nullptr);

  pathEdit_ = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
    80, 44, 260, 24, panel_, nullptr, instance_, nullptr);

  browseButton_ = CreateWindowExW(0, L"BUTTON", L"Browse", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    350, 44, 90, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBrowseEngineButtonId)), instance_, nullptr);

  newButton_ = CreateWindowExW(0, L"BUTTON", L"New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    12, 80, 90, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kNewButtonId)), instance_, nullptr);

  factionNameLabel_ = CreateWindowExW(0, L"STATIC", L"Faction Name:", WS_CHILD | WS_VISIBLE | SS_LEFT,
    12, 116, 90, 24, panel_, nullptr, instance_, nullptr);

  factionNameEdit_ = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
    105, 112, 180, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFactionNameEditId)), instance_, nullptr);

  addFactionButton_ = CreateWindowExW(0, L"BUTTON", L"Add Faction", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    295, 112, 110, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAddFactionButtonId)), instance_, nullptr);

  playerNumberLabel_ = CreateWindowExW(0, L"STATIC", L"Player Number:", WS_CHILD | WS_VISIBLE | SS_LEFT,
    12, 152, 90, 24, panel_, nullptr, instance_, nullptr);

  playerNumberEdit_ = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_NUMBER,
    105, 152, 40, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPlayerNumberEditId)), instance_, nullptr);
  SendMessageW(playerNumberEdit_, EM_SETLIMITTEXT, 3, 0);

  ordersLabel_ = CreateWindowExW(0, L"STATIC", L"Orders", WS_CHILD | WS_VISIBLE | SS_LEFT,
    160, 152, 50, 24, panel_, nullptr, instance_, nullptr);

  ordersEdit_ = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
    215, 152, 145, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kOrdersEditId)), instance_, nullptr);

  ordersBrowseButton_ = CreateWindowExW(0, L"BUTTON", L"Browse", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    365, 152, 80, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBrowseOrdersButtonId)), instance_, nullptr);

  mainFactionLabel_ = CreateWindowExW(0, L"STATIC", L"Main Faction", WS_CHILD | WS_VISIBLE | SS_LEFT,
    12, 188, 90, 24, panel_, nullptr, instance_, nullptr);

  mainFactionEdit_ = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_NUMBER,
    105, 188, 45, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMainFactionEditId)), instance_, nullptr);
  SendMessageW(mainFactionEdit_, EM_SETLIMITTEXT, 3, 0);

  runButton_ = CreateWindowExW(0, L"BUTTON", L"Run", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    160, 188, 80, 24, panel_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRunButtonId)), instance_, nullptr);

  currentTurnLabel_ = CreateWindowExW(0, L"STATIC", L"Current turn: 0", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
    250, 188, 150, 24, panel_, nullptr, instance_, nullptr);

  terminalEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
    460, 8, 200, 200, panel_, nullptr, instance_, nullptr);
  SendMessageW(terminalEdit_, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(ANSI_FIXED_FONT)), MAKELPARAM(TRUE, 0));
  SetWindowSubclass(terminalEdit_, &LocalEngineTabContent::terminalSubclassProc,
                    kTerminalSubclassId, reinterpret_cast<DWORD_PTR>(this));

  if (!localCheck_ || !pathLabel_ || !pathEdit_ || !browseButton_ || !newButton_
    || !factionNameLabel_ || !factionNameEdit_ || !addFactionButton_
    || !mainFactionLabel_ || !mainFactionEdit_ || !runButton_ || !currentTurnLabel_
    || !playerNumberLabel_ || !playerNumberEdit_ || !ordersLabel_ || !ordersEdit_ || !ordersBrowseButton_
    || !terminalEdit_)
  {
    return false;
  }

  SendMessageW(localCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
  SetWindowTextW(pathEdit_, appConfig_->getLocalEnginePath().c_str());
  updateControls();
  resize(RECT{});
  return true;
}

void LocalEngineTabContent::resize(const RECT& displayRect)
{
  (void)displayRect;
  if (!terminalEdit_ || !panel_)
  {
    return;
  }

  RECT panelRect {};
  GetClientRect(panel_, &panelRect);

  constexpr int margin = 8;
  const int panelWidth = static_cast<int>(panelRect.right - panelRect.left);
  const int panelHeight = static_cast<int>(panelRect.bottom - panelRect.top);
  const int terminalLeft = (std::max)(panelWidth / 2, 460);
  const int terminalWidth = (std::max)(0, panelWidth - terminalLeft - margin);
  const int terminalHeight = (std::max)(0, panelHeight - 2 * margin);

  SetWindowPos(terminalEdit_, nullptr,
    terminalLeft, margin,
    terminalWidth, terminalHeight,
    SWP_NOZORDER | SWP_NOACTIVATE);
}

bool LocalEngineTabContent::handleCommand(int id, int code)
{
  if (id == kLocalCheckId && code == BN_CLICKED)
  {
    if (SendMessageW(localCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      const int confirmResult = MessageBoxW(ownerWindow_, L"Prepare local?", L"Local Engine", MB_OKCANCEL | MB_ICONQUESTION);
      if (confirmResult != IDOK)
      {
        SendMessageW(localCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
        updateControls();
        return true;
      }

      prepareLocal();
      loadSettings();
    }

    updateControls();
    return true;
  }

  if (id == kBrowseEngineButtonId && code == BN_CLICKED)
  {
    browseEnginePath();
    return true;
  }

  if (id == kNewButtonId && code == BN_CLICKED)
  {
    clearLocalFolder();
    return true;
  }

  if (id == kAddFactionButtonId && code == BN_CLICKED)
  {
    addFaction();
    return true;
  }

  if (id == kRunButtonId && code == BN_CLICKED)
  {
    runEngine();
    return true;
  }

  if (code == EN_CHANGE && (id == kFactionNameEditId || id == kPlayerNumberEditId || id == kOrdersEditId || id == kMainFactionEditId))
  {
    saveSettings();
    return true;
  }

  if (id == kBrowseOrdersButtonId && code == BN_CLICKED)
  {
    browseOrdersPath();
    return true;
  }

  return false;
}

void LocalEngineTabContent::appendProcessOutput(const std::wstring& text)
{
  if (!terminalEdit_)
  {
    return;
  }

  const int currentLength = GetWindowTextLengthW(terminalEdit_);
  SendMessageW(terminalEdit_, EM_SETSEL, currentLength, currentLength);
  SendMessageW(terminalEdit_, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));

  const int newLength = GetWindowTextLengthW(terminalEdit_);
  inputLineStart_ = newLength;
  SendMessageW(terminalEdit_, EM_SETSEL, newLength, newLength);
  SendMessageW(terminalEdit_, EM_SCROLLCARET, 0, 0);
}

void LocalEngineTabContent::handleProcessExited()
{
  processActive_ = false;

  if (readerThread_.joinable())
  {
    readerThread_.join();
  }

  if (stdoutRead_)
  {
    CloseHandle(stdoutRead_);
    stdoutRead_ = nullptr;
  }
  if (stdinWrite_)
  {
    CloseHandle(stdinWrite_);
    stdinWrite_ = nullptr;
  }
  if (processInfo_.hProcess)
  {
    CloseHandle(processInfo_.hProcess);
  }
  if (processInfo_.hThread)
  {
    CloseHandle(processInfo_.hThread);
  }
  processInfo_ = PROCESS_INFORMATION{};

  appendProcessOutput(L"\r\n[process exited]\r\n");
  if (terminalEdit_)
  {
    SendMessageW(terminalEdit_, EM_SETREADONLY, TRUE, 0);
  }

  if (runPending_)
  {
    completeRun();
  }
}

void LocalEngineTabContent::stopProcess()
{
  if (processInfo_.hProcess)
  {
    TerminateProcess(processInfo_.hProcess, 0);
  }

  processActive_ = false;

  if (readerThread_.joinable())
  {
    readerThread_.join();
  }

  if (stdoutRead_)
  {
    CloseHandle(stdoutRead_);
    stdoutRead_ = nullptr;
  }
  if (stdinWrite_)
  {
    CloseHandle(stdinWrite_);
    stdinWrite_ = nullptr;
  }
  if (processInfo_.hProcess)
  {
    CloseHandle(processInfo_.hProcess);
  }
  if (processInfo_.hThread)
  {
    CloseHandle(processInfo_.hThread);
  }
  processInfo_ = PROCESS_INFORMATION{};

  if (terminalEdit_)
  {
    SendMessageW(terminalEdit_, EM_SETREADONLY, TRUE, 0);
  }
}

bool LocalEngineTabContent::isLocalEnabled() const
{
  return localCheck_ && SendMessageW(localCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void LocalEngineTabContent::setMainFactionNumber(int factionNumber)
{
  if (!mainFactionEdit_)
  {
    return;
  }

  wchar_t currentMainFaction[8] = {};
  SendMessageW(mainFactionEdit_, WM_GETTEXT, 8, reinterpret_cast<LPARAM>(currentMainFaction));
  const std::wstring expectedMainFaction = std::to_wstring(factionNumber);
  if (std::wstring(currentMainFaction) != expectedMainFaction)
  {
    SetWindowTextW(mainFactionEdit_, expectedMainFaction.c_str());
  }
}

void LocalEngineTabContent::setOrdersPath(const std::wstring& ordersPath)
{
  if (ordersEdit_)
  {
    SetWindowTextW(ordersEdit_, ordersPath.c_str());
  }
}

bool LocalEngineTabContent::copyOrdersToLocalEngineFolder(const std::wstring& sourcePath,
                                                          int factionNumber,
                                                          std::wstring& copiedPath,
                                                          std::wstring& errorMessage) const
{
  wchar_t selectedEnginePath[MAX_PATH] = {};
  if (pathEdit_)
  {
    SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  }

  const std::filesystem::path selectedEngine(selectedEnginePath);
  const std::filesystem::path localEngineFolder = selectedEngine.has_parent_path()
    ? selectedEngine.parent_path()
    : std::filesystem::path(defaultFolder_);
  if (localEngineFolder.empty() || !std::filesystem::is_directory(localEngineFolder))
  {
    errorMessage = L"The selected local engine folder is not available.";
    return false;
  }

  const std::filesystem::path destination = localEngineFolder / (L"orders." + std::to_wstring(factionNumber));
  std::error_code copyError;
  std::filesystem::copy_file(sourcePath,
                             destination,
                             std::filesystem::copy_options::overwrite_existing,
                             copyError);
  if (copyError)
  {
    errorMessage = L"Failed to copy orders into the local engine folder.";
    return false;
  }

  copiedPath = destination.wstring();
  errorMessage.clear();
  return true;
}

void LocalEngineTabContent::prepareLocal()
{
  const std::filesystem::path localEngineDir = std::filesystem::path(AppConfig::getExecutableDirectory()) / L"LocalEngine";
  std::error_code ec;
  std::filesystem::create_directory(localEngineDir, ec);
  defaultFolder_ = localEngineDir.wstring();
}

void LocalEngineTabContent::updateControls()
{
  if (!panel_ || !localCheck_)
  {
    return;
  }

  const bool enabled = SendMessageW(localCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
  for (HWND control = GetWindow(panel_, GW_CHILD);
       control != nullptr;
       control = GetWindow(control, GW_HWNDNEXT))
  {
    if (control != localCheck_)
    {
      EnableWindow(control, enabled);
    }
  }
}

void LocalEngineTabContent::browseEnginePath()
{
  if (!pathEdit_ || !appConfig_)
  {
    return;
  }

  std::wstring initialPath = defaultFolder_;
  wchar_t currentPath[MAX_PATH] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(currentPath));
  if (currentPath[0] != L'\0')
  {
    initialPath = currentPath;
  }

  const std::wstring filePath = getFileDialogPath(ownerWindow_, false, initialPath, L"exe",
    L"Executable Files (*.exe;*.bat;*.cmd;*.sh;*.py)\0*.exe;*.bat;*.cmd;*.sh;*.py\0All Files (*)\0*.*\0");
  if (!filePath.empty())
  {
    SetWindowTextW(pathEdit_, filePath.c_str());
    appConfig_->setLocalEnginePath(filePath);
    appConfig_->save();
  }
}

void LocalEngineTabContent::browseOrdersPath()
{
  if (!ordersEdit_)
  {
    return;
  }

  std::wstring initialPath = defaultFolder_;
  wchar_t currentPath[MAX_PATH] = {};
  SendMessageW(ordersEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(currentPath));
  if (currentPath[0] != L'\0')
  {
    initialPath = currentPath;
  }

  const std::wstring filePath = getFileDialogPath(ownerWindow_, false, initialPath, L"ord",
    L"Order Files (*.ord)\0*.ord\0All Files (*)\0*.*\0");
  if (filePath.empty())
  {
    return;
  }

  if (defaultFolder_.empty())
  {
    SetWindowTextW(ordersEdit_, filePath.c_str());
    return;
  }

  wchar_t playerNumberText[8] = {};
  if (playerNumberEdit_)
  {
    SendMessageW(playerNumberEdit_, WM_GETTEXT, 8, reinterpret_cast<LPARAM>(playerNumberText));
  }

  if (playerNumberText[0] == L'\0')
  {
    MessageBoxW(ownerWindow_, L"Enter a player number before selecting orders.", L"Local Engine", MB_ICONWARNING | MB_OK);
    return;
  }

  const std::filesystem::path destination = std::filesystem::path(defaultFolder_) / (L"orders." + std::wstring(playerNumberText));

  std::error_code copyEc;
  std::filesystem::copy_file(filePath, destination, std::filesystem::copy_options::overwrite_existing, copyEc);
  if (copyEc)
  {
    MessageBoxW(ownerWindow_, L"Failed to copy orders file into the local engine folder.", L"Local Engine", MB_ICONERROR | MB_OK);
    return;
  }

  SetWindowTextW(ordersEdit_, destination.wstring().c_str());
  saveSettings();
}

void LocalEngineTabContent::clearLocalFolder()
{
  if (!pathEdit_ || defaultFolder_.empty())
  {
    return;
  }

  const int confirmResult = MessageBoxW(ownerWindow_, L"Clear local engine folder?", L"Local Engine", MB_OKCANCEL | MB_ICONQUESTION);
  if (confirmResult != IDOK)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));

  std::error_code ec;
  const std::filesystem::path selectedEngine = std::filesystem::weakly_canonical(std::filesystem::path(selectedEnginePath), ec);

  for (const auto& entry : std::filesystem::directory_iterator(defaultFolder_, ec))
  {
    const std::filesystem::path entryPath = std::filesystem::weakly_canonical(entry.path(), ec);
    if (!selectedEnginePath[0] || entryPath != selectedEngine)
    {
      std::error_code removeEc;
      std::filesystem::remove_all(entry.path(), removeEc);
    }
  }

  if (selectedEnginePath[0])
  {
    launchProcess(selectedEnginePath);
    resetSettings();
  }
}

void LocalEngineTabContent::addFaction()
{
  if (!pathEdit_ || !factionNameEdit_)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  wchar_t factionName[512] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  SendMessageW(factionNameEdit_, WM_GETTEXT, 512, reinterpret_cast<LPARAM>(factionName));

  if (selectedEnginePath[0] == L'\0')
  {
    MessageBoxW(ownerWindow_, L"Select an engine before adding a faction.", L"Local Engine", MB_ICONWARNING | MB_OK);
    return;
  }

  if (factionName[0] == L'\0')
  {
    MessageBoxW(ownerWindow_, L"Enter a faction name before adding a faction.", L"Local Engine", MB_ICONWARNING | MB_OK);
    return;
  }

  const auto result = LocalEngineFactionUtils::addFaction(std::filesystem::path(selectedEnginePath), factionName);
  switch (result)
  {
    case LocalEngineFactionUtils::AddFactionResult::Added:
      SetWindowTextW(factionNameEdit_, L"");
      saveSettings();
      return;

    case LocalEngineFactionUtils::AddFactionResult::DuplicateName:
      MessageBoxW(ownerWindow_, L"A faction with this name already exists.", L"Local Engine", MB_ICONWARNING | MB_OK);
      return;

    case LocalEngineFactionUtils::AddFactionResult::PlayersFileMissing:
      MessageBoxW(ownerWindow_, L"players.out was not found beside the selected engine.", L"Local Engine", MB_ICONWARNING | MB_OK);
      return;

    case LocalEngineFactionUtils::AddFactionResult::EmptyFactionName:
      MessageBoxW(ownerWindow_, L"Enter a faction name before adding a faction.", L"Local Engine", MB_ICONWARNING | MB_OK);
      return;

    case LocalEngineFactionUtils::AddFactionResult::ReadError:
      MessageBoxW(ownerWindow_, L"Unable to read players.out beside the selected engine.", L"Local Engine", MB_ICONERROR | MB_OK);
      return;

    case LocalEngineFactionUtils::AddFactionResult::WriteError:
      MessageBoxW(ownerWindow_, L"Unable to update players.out beside the selected engine.", L"Local Engine", MB_ICONERROR | MB_OK);
      return;
  }
}

void LocalEngineTabContent::runEngine()
{
  if (!pathEdit_ || !mainFactionEdit_ || processActive_)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  if (selectedEnginePath[0] == L'\0')
  {
    MessageBoxW(ownerWindow_, L"Select an engine before running it.", L"Local Engine", MB_ICONWARNING | MB_OK);
    return;
  }

  const auto preparation = LocalEngineRunUtils::prepareRun(selectedEnginePath);
  if (preparation != LocalEngineRunUtils::RunStatus::Success)
  {
    MessageBoxW(ownerWindow_, L"Unable to prepare the local engine files.", L"Local Engine", MB_ICONERROR | MB_OK);
    return;
  }

  runPending_ = true;
  launchProcess(selectedEnginePath, L"run");
  if (!processActive_)
  {
    runPending_ = false;
  }
}

void LocalEngineTabContent::completeRun()
{
  runPending_ = false;

  if (!appData_ || !appConfig_)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  wchar_t mainFactionText[8] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  SendMessageW(mainFactionEdit_, WM_GETTEXT, 8, reinterpret_cast<LPARAM>(mainFactionText));

  const std::filesystem::path fallbackReportsFolder =
    std::filesystem::path(AppConfig::getExecutableDirectory()) / L"Reports";
  const auto result = LocalEngineRunUtils::finalizeRun(
    selectedEnginePath,
    mainFactionText,
    appConfig_->getReportImportFolder(),
    fallbackReportsFolder);
  if (result.status != LocalEngineRunUtils::RunStatus::Success)
  {
    MessageBoxW(ownerWindow_, L"Unable to create the local engine report.", L"Local Engine", MB_ICONERROR | MB_OK);
    return;
  }

  if (mainFactionText[0] == L'\0')
  {
    const std::wstring selectedFaction = std::to_wstring(result.mainFaction);
    SetWindowTextW(mainFactionEdit_, selectedFaction.c_str());
  }

  updateTurnLabel(result.currentTurn);
  saveSettings();

  appConfig_->setReportImportFolder(result.reportPath.parent_path().wstring());
  appConfig_->save();
  if (!AppDataUtils::importReportFromFile(*appData_, appConfig_, result.reportPath.wstring(), true, true, false))
  {
    const std::wstring message = L"Failed to load report:\n\n" + appData_->reportRepository().getLastError();
    MessageBoxW(ownerWindow_, message.c_str(), L"Local Engine", MB_ICONERROR | MB_OK);
    return;
  }

  if (callbacks_.refreshAllTabs)
  {
    callbacks_.refreshAllTabs();
  }
  if (callbacks_.showMapTab)
  {
    callbacks_.showMapTab();
  }
}

void LocalEngineTabContent::loadSettings()
{
  if (!pathEdit_)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  if (selectedEnginePath[0] == L'\0')
  {
    return;
  }

  LocalEngineSettingsUtils::Settings settings;
  if (!LocalEngineSettingsUtils::load(selectedEnginePath, settings))
  {
    return;
  }

  SetWindowTextW(factionNameEdit_, settings.factionName.c_str());
  SetWindowTextW(mainFactionEdit_, settings.mainFaction.c_str());
  SetWindowTextW(playerNumberEdit_, settings.playerNumber.c_str());
  SetWindowTextW(ordersEdit_, settings.ordersPath.c_str());
  updateTurnLabel(settings.currentTurn);
}

void LocalEngineTabContent::saveSettings()
{
  if (!pathEdit_ || !factionNameEdit_ || !mainFactionEdit_ || !playerNumberEdit_ || !ordersEdit_)
  {
    return;
  }

  wchar_t selectedEnginePath[MAX_PATH] = {};
  SendMessageW(pathEdit_, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(selectedEnginePath));
  if (selectedEnginePath[0] == L'\0')
  {
    return;
  }

  LocalEngineSettingsUtils::Settings settings;
  settings.factionName = readWindowText(factionNameEdit_, 512);
  settings.mainFaction = readWindowText(mainFactionEdit_, 8);
  settings.playerNumber = readWindowText(playerNumberEdit_, 8);
  settings.ordersPath = readWindowText(ordersEdit_, MAX_PATH);
  settings.currentTurn = currentTurn_;
  LocalEngineSettingsUtils::save(selectedEnginePath, settings);
}

void LocalEngineTabContent::resetSettings()
{
  SetWindowTextW(factionNameEdit_, L"");
  SetWindowTextW(mainFactionEdit_, L"");
  SetWindowTextW(playerNumberEdit_, L"");
  SetWindowTextW(ordersEdit_, L"");
  updateTurnLabel(0);
  saveSettings();
}

void LocalEngineTabContent::updateTurnLabel(int turnNumber)
{
  currentTurn_ = turnNumber;
  if (currentTurnLabel_)
  {
    const std::wstring label = L"Current turn: " + std::to_wstring(turnNumber);
    SetWindowTextW(currentTurnLabel_, label.c_str());
  }
}

void LocalEngineTabContent::launchProcess(const std::wstring& enginePath, const std::wstring& action)
{
  stopProcess();

  if (!terminalEdit_)
  {
    return;
  }

  SECURITY_ATTRIBUTES sa {};
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  HANDLE stdoutReadForParent = nullptr;
  HANDLE stdoutWriteForChild = nullptr;
  HANDLE stdinReadForChild = nullptr;
  HANDLE stdinWriteForParent = nullptr;

  if (!CreatePipe(&stdoutReadForParent, &stdoutWriteForChild, &sa, 0) ||
      !CreatePipe(&stdinReadForChild, &stdinWriteForParent, &sa, 0))
  {
    appendProcessOutput(L"[failed to create pipes]\r\n");
    if (stdoutReadForParent) { CloseHandle(stdoutReadForParent); }
    if (stdoutWriteForChild) { CloseHandle(stdoutWriteForChild); }
    if (stdinReadForChild) { CloseHandle(stdinReadForChild); }
    if (stdinWriteForParent) { CloseHandle(stdinWriteForParent); }
    return;
  }

  SetHandleInformation(stdoutReadForParent, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(stdinWriteForParent, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFOW startupInfo {};
  startupInfo.cb = sizeof(startupInfo);
  startupInfo.dwFlags = STARTF_USESTDHANDLES;
  startupInfo.hStdInput = stdinReadForChild;
  startupInfo.hStdOutput = stdoutWriteForChild;
  startupInfo.hStdError = stdoutWriteForChild;

  const std::filesystem::path enginePathObj(enginePath);
  std::wstring commandLine = L"\"" + enginePath + L"\" " + action;
  const std::wstring workingDirectory = enginePathObj.has_parent_path()
    ? enginePathObj.parent_path().wstring()
    : defaultFolder_;

  PROCESS_INFORMATION processInfo {};
  const BOOL created = CreateProcessW(
    nullptr,
    commandLine.data(),
    nullptr,
    nullptr,
    TRUE,
    CREATE_NO_WINDOW,
    nullptr,
    workingDirectory.c_str(),
    &startupInfo,
    &processInfo);

  CloseHandle(stdoutWriteForChild);
  CloseHandle(stdinReadForChild);

  if (!created)
  {
    appendProcessOutput(L"[failed to start local engine]\r\n");
    CloseHandle(stdoutReadForParent);
    CloseHandle(stdinWriteForParent);
    return;
  }

  processInfo_ = processInfo;
  stdoutRead_ = stdoutReadForParent;
  stdinWrite_ = stdinWriteForParent;
  processActive_ = true;

  SendMessageW(terminalEdit_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(L""));
  inputLineStart_ = 0;
  SendMessageW(terminalEdit_, EM_SETREADONLY, FALSE, 0);

  HWND ownerWnd = ownerWindow_;
  HANDLE readHandle = stdoutRead_;
  readerThread_ = std::thread([ownerWnd, readHandle]()
  {
    char buffer[4096];
    DWORD bytesRead = 0;
    while (ReadFile(readHandle, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0)
    {
      const int wideLength = MultiByteToWideChar(CP_ACP, 0, buffer, static_cast<int>(bytesRead), nullptr, 0);
      if (wideLength > 0)
      {
        auto* text = new std::wstring(static_cast<size_t>(wideLength), L'\0');
        MultiByteToWideChar(CP_ACP, 0, buffer, static_cast<int>(bytesRead), text->data(), wideLength);
        PostMessageW(ownerWnd, LocalEngineTabContent::kProcessOutputMessage, 0, reinterpret_cast<LPARAM>(text));
      }
    }
    PostMessageW(ownerWnd, LocalEngineTabContent::kProcessExitedMessage, 0, 0);
  });
}

void LocalEngineTabContent::handleTerminalEnter()
{
  if (!terminalEdit_ || !processActive_ || !stdinWrite_)
  {
    return;
  }

  const int textLength = GetWindowTextLengthW(terminalEdit_);
  if (textLength < inputLineStart_)
  {
    return;
  }

  std::wstring fullText(static_cast<size_t>(textLength), L'\0');
  GetWindowTextW(terminalEdit_, fullText.data(), textLength + 1);

  std::wstring enteredLine = fullText.substr(static_cast<size_t>(inputLineStart_));
  while (!enteredLine.empty() && (enteredLine.back() == L'\r' || enteredLine.back() == L'\n'))
  {
    enteredLine.pop_back();
  }

  const std::wstring lineWithNewline = enteredLine + L"\r\n";
  const int narrowLength = WideCharToMultiByte(CP_ACP, 0, lineWithNewline.c_str(), static_cast<int>(lineWithNewline.size()),
                                              nullptr, 0, nullptr, nullptr);
  if (narrowLength > 0)
  {
    std::string narrowLine(static_cast<size_t>(narrowLength), '\0');
    WideCharToMultiByte(CP_ACP, 0, lineWithNewline.c_str(), static_cast<int>(lineWithNewline.size()),
                      narrowLine.data(), narrowLength, nullptr, nullptr);
    DWORD bytesWritten = 0;
    WriteFile(stdinWrite_, narrowLine.data(), static_cast<DWORD>(narrowLine.size()), &bytesWritten, nullptr);
  }

  inputLineStart_ = GetWindowTextLengthW(terminalEdit_);
}

LRESULT CALLBACK LocalEngineTabContent::terminalSubclassProc(HWND hwnd,
                                                             UINT msg,
                                                             WPARAM wp,
                                                             LPARAM lp,
                                                             UINT_PTR /*subclassId*/,
                                                             DWORD_PTR refData)
{
  auto* tabContent = reinterpret_cast<LocalEngineTabContent*>(refData);

  if (msg == WM_CHAR && tabContent && tabContent->processActive_)
  {
    DWORD selStart = 0;
    DWORD selEnd = 0;
    SendMessageW(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&selStart), reinterpret_cast<LPARAM>(&selEnd));

    if (wp == VK_BACK && selStart <= static_cast<DWORD>(tabContent->inputLineStart_))
    {
      return 0;
    }

    if (selStart < static_cast<DWORD>(tabContent->inputLineStart_))
    {
      const int endPos = GetWindowTextLengthW(hwnd);
      SendMessageW(hwnd, EM_SETSEL, endPos, endPos);
    }

    if (wp == VK_RETURN)
    {
      const LRESULT result = DefSubclassProc(hwnd, msg, wp, lp);
      tabContent->handleTerminalEnter();
      return result;
    }
  }
  else if (msg == WM_NCDESTROY)
  {
    RemoveWindowSubclass(hwnd, &LocalEngineTabContent::terminalSubclassProc, kTerminalSubclassId);
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
}