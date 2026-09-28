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
 * File: LocalEngineTabContent.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>

class AppConfig;
class AppData;

class LocalEngineTabContent
{
public:
  struct Callbacks
  {
    std::function<void()> refreshAllTabs;
    std::function<void()> showMapTab;
  };

  static constexpr UINT kProcessOutputMessage = WM_APP + 3;
  static constexpr UINT kProcessExitedMessage = WM_APP + 4;

  LocalEngineTabContent() = default;
  ~LocalEngineTabContent();

  LocalEngineTabContent(const LocalEngineTabContent&) = delete;
  LocalEngineTabContent& operator=(const LocalEngineTabContent&) = delete;
  LocalEngineTabContent(LocalEngineTabContent&&) = delete;
  LocalEngineTabContent& operator=(LocalEngineTabContent&&) = delete;

  bool create(HWND ownerWindow,
              HINSTANCE instance,
              HWND panel,
              AppData& appData,
              AppConfig& appConfig,
              Callbacks callbacks);
  void resize(const RECT& displayRect);
  bool handleCommand(int id, int code);
  void appendProcessOutput(const std::wstring& text);
  void handleProcessExited();
  void stopProcess();

  bool isLocalEnabled() const;
  void setMainFactionNumber(int factionNumber);
  void setOrdersPath(const std::wstring& ordersPath);
  bool copyOrdersToLocalEngineFolder(const std::wstring& sourcePath,
                                     int factionNumber,
                                     std::wstring& copiedPath,
                                     std::wstring& errorMessage) const;

private:
  HWND ownerWindow_ { nullptr };
  HINSTANCE instance_ { nullptr };
  HWND panel_ { nullptr };
  AppData* appData_ { nullptr };
  AppConfig* appConfig_ { nullptr };
  Callbacks callbacks_;

  HWND pathLabel_ { nullptr };
  HWND pathEdit_ { nullptr };
  HWND browseButton_ { nullptr };
  HWND newButton_ { nullptr };
  HWND factionNameLabel_ { nullptr };
  HWND factionNameEdit_ { nullptr };
  HWND addFactionButton_ { nullptr };
  HWND mainFactionLabel_ { nullptr };
  HWND mainFactionEdit_ { nullptr };
  HWND runButton_ { nullptr };
  HWND currentTurnLabel_ { nullptr };
  HWND localCheck_ { nullptr };
  HWND terminalEdit_ { nullptr };
  HWND playerNumberLabel_ { nullptr };
  HWND playerNumberEdit_ { nullptr };
  HWND ordersLabel_ { nullptr };
  HWND ordersEdit_ { nullptr };
  HWND ordersBrowseButton_ { nullptr };
  std::wstring defaultFolder_;

  PROCESS_INFORMATION processInfo_ {};
  HANDLE stdinWrite_ { nullptr };
  HANDLE stdoutRead_ { nullptr };
  std::thread readerThread_;
  std::atomic<bool> processActive_ { false };
  int inputLineStart_ { 0 };
  bool runPending_ { false };
  int currentTurn_ { 0 };

  void prepareLocal();
  void updateControls();
  void browseEnginePath();
  void browseOrdersPath();
  void clearLocalFolder();
  void addFaction();
  void runEngine();
  void completeRun();
  void loadSettings();
  void saveSettings();
  void resetSettings();
  void updateTurnLabel(int turnNumber);
  void launchProcess(const std::wstring& enginePath, const std::wstring& action = L"new");
  void handleTerminalEnter();

  static LRESULT CALLBACK terminalSubclassProc(HWND hwnd,
                                               UINT msg,
                                               WPARAM wp,
                                               LPARAM lp,
                                               UINT_PTR subclassId,
                                               DWORD_PTR refData);
};