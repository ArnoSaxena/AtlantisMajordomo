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
 * File: MainWindow.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/MainWindow.hpp"

#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/DataSerializer.hpp"
#include "Data/Faction.hpp"
#include "Data/Unit.hpp"
#include "GUI/EventsTabContent.hpp"
#include "GUI/BattlesTabContent.hpp"
#include "GUI/ControlIds.hpp"
#include "GUI/FactionsTabContent.hpp"
#include "GUI/ItemsTabContent.hpp"
#include "GUI/MapTabContent.hpp"
#include "GUI/ResourceIds.hpp"
#include "GUI/ReportsTabContent.hpp"
#include "GUI/SkillsTabContent.hpp"
#include "GUI/SettingsDialog.hpp"
#include "GUI/TabView.hpp"

#include <commctrl.h>
#include <commdlg.h>
#include <shlwapi.h>

#include <filesystem>
#include <algorithm>
#include <array>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <vector>

#pragma comment(lib, "shlwapi.lib")

// Menu command IDs
constexpr int IDM_FILE_NEW         = 1001;
constexpr int IDM_FILE_OPEN        = 1002;
constexpr int IDM_FILE_SAVE        = 1003;
constexpr int IDM_FILE_LOAD_REPORT = 1004;
constexpr int IDM_FILE_IMPORT_DATA = 1006;
constexpr int IDM_FILE_EXPORT_ORDERS = 1007;
constexpr int IDM_FILE_EXIT        = 1005;
constexpr int IDM_SETTINGS_OPTIONS = 2001;
constexpr int IDM_HELP_ABOUT       = 9001;
constexpr int IDM_HELP_DESCRIPTION = 9002;
constexpr int IDM_TAB_CTX_LOAD_REPORT = 4001;

static constexpr wchar_t kClassName[] = L"WindowsAppMainWnd";

enum class InternalTabId
{
  Reports,
  Map,
  Events,
  Items,
  Skills,
  Factions,
  Battles,
};

struct TabDefinition
{
  InternalTabId id;
  int order;
  const wchar_t* label;
};

constexpr std::array<TabDefinition, 7> kTabDefinitions = {{
  { InternalTabId::Reports, 10, L"Loaded Reports" },
  { InternalTabId::Map,     20, L"Map" },
  { InternalTabId::Events,  30, L"Events" },
  { InternalTabId::Items,   40, L"Items" },
  { InternalTabId::Skills,  50, L"Skills" },
  { InternalTabId::Factions, 60, L"Factions" },
  { InternalTabId::Battles, 70, L"Battles" },
}};

static RECT getTabDisplayRect(HWND tabControl, const RECT& tabBounds)
{
  RECT displayRc = tabBounds;
  TabCtrl_AdjustRect(tabControl, FALSE, &displayRc);
  return displayRc;
}

namespace
{
  struct MainFactionExportContext
  {
    int factionNumber { 0 };
    std::wstring password;
    int nextMonth { 0 };
    int nextYear { 0 };
  };

  bool isLaterPeriod(int year, int month, int referenceYear, int referenceMonth)
  {
    return (year > referenceYear) || (year == referenceYear && month > referenceMonth);
  }

  bool tryBuildMainFactionExportContext(const AppData& appData,
                                        MainFactionExportContext& exportContext,
                                        std::wstring& errorMessage)
  {
    const auto& factionRepository = appData.factionRepository();
    const Faction* mainFaction = nullptr;
    int mainFactionCount = 0;

    for (std::size_t index = 0; index < factionRepository.size(); ++index)
    {
      const Faction& faction = factionRepository.at(index);
      if (faction.isMainFaction())
      {
        ++mainFactionCount;
        mainFaction = &faction;
      }
    }

    if (mainFactionCount == 0 || mainFaction == nullptr)
    {
      errorMessage = L"Cannot export orders: no main faction is selected.";
      return false;
    }

    if (mainFactionCount > 1)
    {
      errorMessage = L"Cannot export orders: more than one main faction is selected.";
      return false;
    }

    int latestYear = 0;
    int latestMonth = 0;
    bool hasPeriod = false;

    for (std::size_t index = 0; index < factionRepository.size(); ++index)
    {
      const Faction& faction = factionRepository.at(index);
      const int month = faction.getMonth();
      const int year = faction.getYear();
      if (month < 1 || month > 12)
      {
        continue;
      }

      if (!hasPeriod || isLaterPeriod(year, month, latestYear, latestMonth))
      {
        latestYear = year;
        latestMonth = month;
        hasPeriod = true;
      }
    }

    if (!hasPeriod)
    {
      errorMessage = L"Cannot export orders: no valid month/year was found in loaded faction data.";
      return false;
    }

    ++latestMonth;
    if (latestMonth > 12)
    {
      latestMonth = 1;
      ++latestYear;
    }

    exportContext.factionNumber = mainFaction->getFactionNumber();
    exportContext.password = mainFaction->getPassword();
    exportContext.nextMonth = latestMonth;
    exportContext.nextYear = latestYear;
    errorMessage.clear();
    return true;
  }

  std::wstring formatTwoDigits(int value)
  {
    if (value < 10)
    {
      return L"0" + std::to_wstring(value);
    }

    return std::to_wstring(value);
  }

  std::wstring buildSuggestedOrdersFilename(const MainFactionExportContext& exportContext)
  {
    return L"orders_"
      + std::to_wstring(exportContext.factionNumber)
      + L"_"
      + std::to_wstring(exportContext.nextYear)
      + L"_"
      + formatTwoDigits(exportContext.nextMonth)
      + L".ord";
  }

  std::wstring buildOrdersExportContent(const AppData& appData,
                                        int mainFactionNumber,
                                        const std::wstring& factionPassword)
  {
    std::vector<const Unit*> mainFactionUnits;
    const auto& unitRepository = appData.unitRepository();
    for (std::size_t index = 0; index < unitRepository.size(); ++index)
    {
      const Unit& unit = unitRepository.at(index);
      if (unit.getFactionNumber() == mainFactionNumber)
      {
        mainFactionUnits.push_back(&unit);
      }
    }

    std::sort(mainFactionUnits.begin(),
              mainFactionUnits.end(),
              [](const Unit* lhs, const Unit* rhs)
              {
                return lhs->getUnitNumber() < rhs->getUnitNumber();
              });

    std::wostringstream output;
    output << L"#atlantis " << mainFactionNumber << L" \"" << factionPassword << L"\"\n";

    for (const Unit* unit : mainFactionUnits)
    {
      output << L"\n";
      output << L"unit " << unit->getUnitNumber() << L"\n";
      for (const std::wstring& orderLine : unit->getOrders())
      {
        output << orderLine << L"\n";
      }
    }

    output << L"\n";
    output << L"#end\n";
    return output.str();
  }

  static constexpr wchar_t kMapHelpDescriptionText[] =
    L"Map View Legend:\r\n"
    L"- Settlement marker in the center of the region:\r\n"
    L"  - City: big black dot .\r\n"
    L"  - Town: black ring with small black dot in center.\r\n"
    L"  - Village: black ring.\r\n"
    L"- Hex fill color: the region terrain type.\r\n"
    L"- Grey line from hex centre to border: road connection in the respective direction.\r\n"
    L"- Gray dot at top left: indicates a structure in the region.\r\n"
    L"- Orange dot at top left: one of the present structures is a caravanserai.\r\n"
    L"- Black outline on the top left dot: one of the present structures is a shaft.\r\n"
    L"- Light blue dot at bottom left: a or more ships are present in the region.\r\n"
    L"- Black outline on the ship dot: one or more of the present ships are airborne.\r\n"
    L"- Red x on at the bottom: a battle has taken place in the region in the last turn.\r\n"
    L"\r\n"
    L"Unit List Legend:\r\n"
    L"- Green background of unit number and name: the unit on guard.\r\n"
    L"- Green structure name: the unit is owner of the structure.\r\n";

  LRESULT CALLBACK mapDescriptionWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
  {
    switch (msg)
    {
      case WM_CREATE:
      {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        const int editMargin = 10;
        const int okHeight = 28;
        const int okWidth = 80;
        const int editHeight = clientRect.bottom - clientRect.top - okHeight - (editMargin * 3);
        const int editWidth = clientRect.right - clientRect.left - (editMargin * 2);

        CreateWindowExW(
          WS_EX_CLIENTEDGE,
          L"EDIT",
          kMapHelpDescriptionText,
          WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
          editMargin,
          editMargin,
          editWidth,
          editHeight,
          hwnd,
          reinterpret_cast<HMENU>(1001),
          reinterpret_cast<LPCREATESTRUCT>(lp)->hInstance,
          nullptr);

        HWND okButton = CreateWindowExW(
          0,
          L"BUTTON",
          L"OK",
          WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
          (clientRect.right - clientRect.left - okWidth) / 2,
          clientRect.bottom - okHeight - editMargin,
          okWidth,
          okHeight,
          hwnd,
          reinterpret_cast<HMENU>(IDOK),
          reinterpret_cast<LPCREATESTRUCT>(lp)->hInstance,
          nullptr);

        if (okButton)
        {
          SetFocus(okButton);
        }
        return 0;
      }

      case WM_COMMAND:
        if (LOWORD(wp) == IDOK)
        {
          DestroyWindow(hwnd);
        }
        return 0;

      case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

      case WM_DESTROY:
      {
        HWND parent = reinterpret_cast<HWND>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (parent)
        {
          EnableWindow(parent, TRUE);
          SetActiveWindow(parent);
        }
        return 0;
      }
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
  }

  bool registerMapDescriptionDialogClass(HINSTANCE instance)
  {
    static bool registered = false;
    if (registered)
    {
      return true;
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = mapDescriptionWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"WindowsAppMapDescriptionDialog";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    const ATOM result = RegisterClassW(&wc);
    if (result == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
      return false;
    }

    registered = true;
    return true;
  }

  void showMapDescriptionDialog(HWND parent)
  {
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    if (!registerMapDescriptionDialogClass(instance))
    {
      MessageBoxW(parent, L"Unable to open the map description window.", L"Description", MB_OK | MB_ICONERROR);
      return;
    }

    const int width = 540;
    const int height = 420;
    RECT parentRect;
    GetWindowRect(parent, &parentRect);
    const int x = parentRect.left + ((parentRect.right - parentRect.left) - width) / 2;
    const int y = parentRect.top + ((parentRect.bottom - parentRect.top) - height) / 2;

    HWND dialog = CreateWindowExW(
      0,
      L"WindowsAppMapDescriptionDialog",
      L"Description",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      x,
      y,
      width,
      height,
      parent,
      nullptr,
      instance,
      nullptr);

    if (!dialog)
    {
      MessageBoxW(parent, L"Unable to open the map description window.", L"Description", MB_OK | MB_ICONERROR);
      return;
    }

    SetWindowLongPtr(dialog, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(parent));
    EnableWindow(parent, FALSE);
    ShowWindow(dialog, SW_SHOW);
    UpdateWindow(dialog);

    MSG msg;
    while (IsWindow(dialog) && GetMessageW(&msg, nullptr, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (IsWindow(parent))
    {
      EnableWindow(parent, TRUE);
      SetActiveWindow(parent);
    }
  }

  std::wstring buildAboutDialogText()
  {
    return std::wstring(MainWindow::kAboutAppName)
           + L"\r\n\r\n"
           + MainWindow::kAboutDescription
           + L"\r\n\r\nVersion "
           + MainWindow::kAboutVersion;
  }

  LRESULT CALLBACK aboutWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
  {
    switch (msg)
    {
      case WM_CREATE:
      {
        auto* createStruct = reinterpret_cast<LPCREATESTRUCTW>(lp);

        RECT clientRect {};
        GetClientRect(hwnd, &clientRect);

        const int margin = 16;
        const int buttonWidth = 90;
        const int buttonHeight = 28;
        const int iconSize = std::max(32, GetSystemMetrics(SM_CXICON) * 2);
        const int iconY = margin;
        const int iconX = margin;
        const int textX = iconX + iconSize + 16;
        const int textY = margin + 2;
        const int textWidth = clientRect.right - textX - margin;
        const int textHeight = clientRect.bottom - textY - buttonHeight - (margin * 2);

        HICON largeIcon = reinterpret_cast<HICON>(
          LoadImageW(createStruct->hInstance,
                     MAKEINTRESOURCEW(IDI_APP_ICON),
                     IMAGE_ICON,
                     iconSize,
                     iconSize,
                     LR_DEFAULTCOLOR));
        if (largeIcon == nullptr)
        {
          largeIcon = LoadIconW(nullptr, IDI_APPLICATION);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(largeIcon));

        HWND iconControl = CreateWindowExW(
          0,
          L"STATIC",
          nullptr,
          WS_CHILD | WS_VISIBLE | SS_ICON,
          iconX,
          iconY,
          iconSize,
          iconSize,
          hwnd,
          nullptr,
          createStruct->hInstance,
          nullptr);
        if (iconControl != nullptr)
        {
          SendMessageW(iconControl, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(largeIcon));
        }

        const std::wstring aboutText = buildAboutDialogText();
        HWND textControl = CreateWindowExW(
          0,
          L"STATIC",
          aboutText.c_str(),
          WS_CHILD | WS_VISIBLE | SS_LEFT,
          textX,
          textY,
          textWidth,
          textHeight,
          hwnd,
          nullptr,
          createStruct->hInstance,
          nullptr);

        HWND okButton = CreateWindowExW(
          0,
          L"BUTTON",
          L"OK",
          WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
          (clientRect.right - buttonWidth) / 2,
          clientRect.bottom - margin - buttonHeight,
          buttonWidth,
          buttonHeight,
          hwnd,
          reinterpret_cast<HMENU>(IDOK),
          createStruct->hInstance,
          nullptr);

        HFONT guiFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        if (textControl != nullptr)
        {
          SendMessageW(textControl, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
        }
        if (okButton != nullptr)
        {
          SendMessageW(okButton, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
          SetFocus(okButton);
        }

        return 0;
      }

      case WM_COMMAND:
        if (LOWORD(wp) == IDOK)
        {
          DestroyWindow(hwnd);
        }
        return 0;

      case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

      case WM_DESTROY:
      {
        HICON iconHandle = reinterpret_cast<HICON>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (iconHandle != nullptr)
        {
          DestroyIcon(iconHandle);
        }
        return 0;
      }
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
  }

  bool registerAboutDialogClass(HINSTANCE instance)
  {
    static bool registered = false;
    if (registered)
    {
      return true;
    }

    WNDCLASSEXW wc {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = aboutWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"WindowsAppAboutDialog";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    //wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));

    const ATOM result = RegisterClassExW(&wc);
    if (result == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
      return false;
    }

    registered = true;
    return true;
  }

  void showAboutDialog(HWND parent, HINSTANCE instance)
  {
    if (!registerAboutDialogClass(instance))
    {
      MessageBoxW(parent, L"Unable to open the About window.", L"About", MB_OK | MB_ICONERROR);
      return;
    }

    const int width = 640;
    const int height = 280;
    RECT parentRect {};
    GetWindowRect(parent, &parentRect);
    const int x = parentRect.left + ((parentRect.right - parentRect.left) - width) / 2;
    const int y = parentRect.top + ((parentRect.bottom - parentRect.top) - height) / 2;

    HWND dialog = CreateWindowExW(
      WS_EX_DLGMODALFRAME,
      L"WindowsAppAboutDialog",
      L"About",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      x,
      y,
      width,
      height,
      parent,
      nullptr,
      instance,
      nullptr);

    if (dialog == nullptr)
    {
      MessageBoxW(parent, L"Unable to open the About window.", L"About", MB_OK | MB_ICONERROR);
      return;
    }

    EnableWindow(parent, FALSE);
    ShowWindow(dialog, SW_SHOW);
    UpdateWindow(dialog);

    MSG msg {};
    while (IsWindow(dialog) && GetMessageW(&msg, nullptr, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (IsWindow(parent))
    {
      EnableWindow(parent, TRUE);
      SetActiveWindow(parent);
    }
  }

  bool saveTextFile(const std::wstring& filePath,
                    const std::wstring& content,
                    std::wstring& errorMessage)
  {
    std::wofstream file(filePath);
    if (!file.is_open())
    {
      errorMessage = L"Failed to open file for writing: " + filePath;
      return false;
    }

    file << content;
    if (!file.good())
    {
      errorMessage = L"Failed while writing file: " + filePath;
      return false;
    }

    errorMessage.clear();
    return true;
  }

  std::wstring directoryFromFilePath(const std::wstring& filePath)
  {
    if (filePath.empty())
    {
      wchar_t currentDir[MAX_PATH] = {};
      GetCurrentDirectoryW(MAX_PATH, currentDir);
      return currentDir;
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
                                const wchar_t* defaultExtension = L"dat",
                                const wchar_t* filterString = nullptr)
  {
    wchar_t szFile[MAX_PATH] = {};
    wchar_t szDir[MAX_PATH] = {};

    if (!configuredSaveFilePath.empty())
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

    const wchar_t defaultFilterString[] = L"Data Files (*.dat)\0*.dat\0All Files (*)\0*.*\0";
    ofn.lpstrFilter = (filterString != nullptr) ? filterString : defaultFilterString;
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
}

bool MainWindow::create(HINSTANCE instance, AppData& appData)
{
  instance_ = instance;
  appData_  = &appData;
  tabView_  = std::make_unique<TabView>();

  // Load (or create) a config file next to the executable.
  appConfig_.load();
  appData_->setFlyingShipsCsv(appConfig_.getFlyingShipsCsv());
  appData_->setMagicSkillTriggersCsv(appConfig_.getMagicSkillTriggersCsv());
  appData_->setOnlyLeaderCanTeach(appConfig_.getOnlyLeaderCanTeach());
  appData_->setLeaderMages(appConfig_.getLeaderMages());
  Commands::setFullMonthOrderKeywordsCsv(appConfig_.getFullMonthOrdersCsv());

  WNDCLASSEXW wc {};
  wc.cbSize        = sizeof(wc);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = wndProc;
  wc.hInstance     = instance;
  wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
  //wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  wc.lpszClassName = kClassName;
  HICON appIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON));
  if (appIcon == nullptr)
  {
    appIcon = LoadIconW(nullptr, IDI_APPLICATION);
  }
  wc.hIcon         = appIcon;
  wc.hIconSm       = appIcon;

  if (!RegisterClassExW(&wc))
  {
    return false;
  }

  hwnd_ = CreateWindowExW(
    0,
    kClassName,
    L"Atlantis Majordomo",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    appConfig_.getMainWindowWidth(),
    appConfig_.getMainWindowHeight(),
    nullptr, nullptr, instance, this
  );

  return hwnd_ != nullptr;
}

int MainWindow::run(int showCmd)
{
  ShowWindow(hwnd_, showCmd);
  UpdateWindow(hwnd_);

  MSG msg {};
  while (GetMessageW(&msg, nullptr, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return static_cast<int>(msg.wParam);
}

TabView& MainWindow::getTabView()
{
  return *tabView_;
}

LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  if (msg == WM_NCCREATE)
  {
    auto* cs   = reinterpret_cast<CREATESTRUCTW*>(lp);
    auto* self = static_cast<MainWindow*>(cs->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    self->hwnd_ = hwnd;
  }

  auto* self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (self)
  {
    return self->handleMessage(msg, wp, lp);
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT MainWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_CREATE:
    {
      createMenu();
      tabView_->create(hwnd_, GUI::ControlIds::kMainTabView);
      initializeTabs();

      reportsTabContent_ = std::make_unique<ReportsTabContent>();
      itemsTabContent_ = std::make_unique<ItemsTabContent>();
      skillsTabContent_ = std::make_unique<SkillsTabContent>();
      eventsTabContent_ = std::make_unique<EventsTabContent>();
      mapTabContent_ = std::make_unique<MapTabContent>();
      factionsTabContent_ = std::make_unique<FactionsTabContent>();
      battlesTabContent_ = std::make_unique<BattlesTabContent>();

        if (!reportsTabContent_->create(hwnd_, instance_, *appData_, appConfig_)
          || !mapTabContent_->create(hwnd_, instance_, *appData_, appConfig_)
            || !itemsTabContent_->create(hwnd_, instance_, *appData_)
            || !skillsTabContent_->create(hwnd_, instance_, *appData_)
          || !eventsTabContent_->create(hwnd_, instance_, *appData_)
          || !factionsTabContent_->create(hwnd_, instance_, *appData_)
          || !battlesTabContent_->create(hwnd_, instance_, *appData_))
      {
        return -1;
      }

      // Auto-load the configured data file if it exists (parse it like File/Import Data)
      const std::wstring configuredDataFile = appConfig_.getDataFilePath();
      if (!configuredDataFile.empty())
      {
        std::filesystem::path dataPath(configuredDataFile);
        if (std::filesystem::exists(dataPath) && std::filesystem::is_regular_file(dataPath))
        {
          auto& reportRepo = appData_->reportRepository();
          if (!reportRepo.addFromFile(configuredDataFile,
                                      appData_->factionRepository(),
                                      appData_->regionRepository(),
                                      appData_->unitRepository(),
                                      appData_->battleRepository(),
                                      appData_->eventRepository(),
                                      appData_->itemRepository(),
                                      appData_->skillRepository(),
                                      appData_->structureRepository(),
                                      appData_->structInfoRepository(),
                                      appData_->orderRepository(),
                                      appData_->getShipStructureIdThreshold(),
                                      appData_->getFlyingShipTypeTokens(),
                                      appData_->getMagicSkillTriggerPhrases(),
                                      false))
          {
            const std::wstring message = L"Failed to auto-load the configured data file:\n\n" + reportRepo.getLastError();
            MessageBoxW(hwnd_, message.c_str(), L"Startup Load Error", MB_OK | MB_ICONWARNING);
          }
        }
      }

      const std::wstring configuredReportFolder = appConfig_.getReportImportFolder();
      if (!configuredReportFolder.empty())
      {
        std::filesystem::path reportFolderPath(configuredReportFolder);
        if (std::filesystem::exists(reportFolderPath) && std::filesystem::is_directory(reportFolderPath))
        {
          auto& reportRepo = appData_->reportRepository();
          std::vector<std::wstring> failedReports;

          const std::array<std::wstring, 4> allowedExtensions = { L".rep", L".txt", L".html", L".htm" };
          for (const auto& entry : std::filesystem::directory_iterator(reportFolderPath))
          {
            if (!entry.is_regular_file())
            {
              continue;
            }

            const std::wstring extension = entry.path().extension().wstring();
            std::wstring lowerExtension = extension;
            std::transform(lowerExtension.begin(), lowerExtension.end(), lowerExtension.begin(), std::towlower);
            if (std::find(allowedExtensions.begin(), allowedExtensions.end(), lowerExtension) == allowedExtensions.end())
            {
              continue;
            }

            if (!reportRepo.addFromFile(entry.path().wstring(),
                                        appData_->factionRepository(),
                                        appData_->regionRepository(),
                                        appData_->unitRepository(),
                                        appData_->battleRepository(),
                                        appData_->eventRepository(),
                                        appData_->itemRepository(),
                                        appData_->skillRepository(),
                                        appData_->structureRepository(),
                                        appData_->structInfoRepository(),
                                        appData_->orderRepository(),
                                        appData_->getShipStructureIdThreshold(),
                                        appData_->getFlyingShipTypeTokens(),
                                        appData_->getMagicSkillTriggerPhrases(),
                                        true))
            {
              failedReports.push_back(entry.path().filename().wstring() + L": " + reportRepo.getLastError());
            }
          }

          if (!failedReports.empty())
          {
            std::wstring message = L"Some reports failed to auto-load from the configured report folder:\n\n";
            for (std::size_t i = 0; i < failedReports.size() && i < 10; ++i)
            {
              message += failedReports[i] + L"\n";
            }
            if (failedReports.size() > 10)
            {
              message += L"...and more\n";
            }
            MessageBoxW(hwnd_, message.c_str(), L"Startup Load Error", MB_OK | MB_ICONWARNING);
          }
        }
      }

      refreshAllTabContents();

      return 0;
    }

    case WM_SIZE:
    {
      if (wp != SIZE_MINIMIZED)
      {
        persistWindowSizeToConfig();
      }

      RECT rc {};
      GetClientRect(hwnd_, &rc);

      constexpr int kMargin = 8;
      rc.left   += kMargin;
      rc.top    += kMargin;
      rc.right  -= kMargin;
      rc.bottom -= kMargin;
      tabView_->resize(rc);

      HWND tabCtrl = tabView_->getTabControl();
      if (tabCtrl)
      {
        RECT tabWndRc {};
        GetWindowRect(tabCtrl, &tabWndRc);
        MapWindowPoints(HWND_DESKTOP, hwnd_, reinterpret_cast<LPPOINT>(&tabWndRc), 2);
        RECT displayRc = getTabDisplayRect(tabCtrl, tabWndRc);

        if (reportsTabContent_)
        {
          reportsTabContent_->resize(displayRc);
        }
        if (mapTabContent_)
        {
          mapTabContent_->resize(displayRc);
        }
        if (eventsTabContent_)
        {
          eventsTabContent_->resize(displayRc);
        }
        if (itemsTabContent_)
        {
          itemsTabContent_->resize(displayRc);
        }
        if (skillsTabContent_)
        {
          skillsTabContent_->resize(displayRc);
        }
        if (factionsTabContent_)
        {
          factionsTabContent_->resize(displayRc);
        }
        if (battlesTabContent_)
        {
          battlesTabContent_->resize(displayRc);
        }
      }

      updateReportsTabVisibility();
      return 0;
    }

    case WM_NOTIFY:
    {
      const auto* hdr = reinterpret_cast<NMHDR*>(lp);
      if (hdr->code == TCN_SELCHANGE && hdr->idFrom == static_cast<UINT>(GUI::ControlIds::kMainTabView))
      {
        tabView_->onSelectionChange();
        updateReportsTabVisibility();
      }

      if (hdr->code == NM_RCLICK && hdr->idFrom == static_cast<UINT>(GUI::ControlIds::kMainTabView))
      {
        HWND tabCtrl = tabView_->getTabControl();
        if (tabCtrl)
        {
          POINT cursorPos {};
          GetCursorPos(&cursorPos);

          POINT clientPos = cursorPos;
          ScreenToClient(tabCtrl, &clientPos);

          TCHITTESTINFO hitInfo {};
          hitInfo.pt = clientPos;
          const int tabIndex = TabCtrl_HitTest(tabCtrl, &hitInfo);
          if (tabIndex == reportsTabIndex_ && (hitInfo.flags & TCHT_ONITEM))
          {
            showLoadedReportsTabContextMenu(cursorPos);
            return 0;
          }
          if (tabIndex == mapTabIndex_ && (hitInfo.flags & TCHT_ONITEM) && mapTabContent_)
          {
            mapTabContent_->showZSelectionContextMenu(hwnd_, cursorPos);
            return 0;
          }
        }
      }

      if (reportsTabContent_ && reportsTabContent_->handleNotify(hdr))
      {
        return 0;
      }

      if (eventsTabContent_ && eventsTabContent_->handleNotify(hdr))
      {
        return eventsTabContent_->getNotifyResult();
      }

      if (itemsTabContent_ && itemsTabContent_->handleNotify(hdr))
      {
        return itemsTabContent_->getNotifyResult();
      }

      if (skillsTabContent_ && skillsTabContent_->handleNotify(hdr))
      {
        return skillsTabContent_->getNotifyResult();
      }

      if (mapTabContent_ && mapTabContent_->handleNotify(hdr))
      {
        return mapTabContent_->getNotifyResult();
      }

      if (factionsTabContent_ && factionsTabContent_->handleNotify(hdr))
      {
        return factionsTabContent_->getNotifyResult();
      }

      if (battlesTabContent_ && battlesTabContent_->handleNotify(hdr))
      {
        return 0;
      }

      return 0;
    }

    case WM_VSCROLL:
    {
      if (itemsTabContent_ && itemsTabContent_->handleVScroll(wp, lp))
      {
        return 0;
      }

      if (skillsTabContent_ && skillsTabContent_->handleVScroll(wp, lp))
      {
        return 0;
      }

      if (factionsTabContent_ && factionsTabContent_->handleVScroll(wp, lp))
      {
        return 0;
      }

      break;
    }

    case WM_DRAWITEM:
    {
      const auto* drawItem = reinterpret_cast<const DRAWITEMSTRUCT*>(lp);
      if (mapTabContent_ && mapTabContent_->handleDrawItem(drawItem))
      {
        return TRUE;
      }
      break;
    }

    case WM_COMMAND:
    {
      const int id   = LOWORD(wp);
      const int code = static_cast<int>(HIWORD(wp));

      if (reportsTabContent_ && reportsTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (eventsTabContent_ && eventsTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (itemsTabContent_ && itemsTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (skillsTabContent_ && skillsTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (mapTabContent_ && mapTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (factionsTabContent_ && factionsTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      if (battlesTabContent_ && battlesTabContent_->handleCommand(id, code))
      {
        return 0;
      }

      switch (id)
      {
        case IDM_FILE_NEW:
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }
          appData_->clear();
          refreshAllTabContents();
          MessageBoxW(hwnd_, L"New report created (not yet implemented).", L"File", MB_ICONINFORMATION | MB_OK);
          return 0;

        case IDM_FILE_OPEN:
        {
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }
          std::wstring filePath = getFileDialogPath(hwnd_, false, appConfig_.getSaveFilePath());
          if (!filePath.empty())
          {
            appConfig_.setSaveFilePath(filePath);
            appConfig_.save();

            if (DataSerializer::loadFromFile(*appData_, filePath))
            {
              refreshAllTabContents();
              MessageBoxW(hwnd_, L"Dataset loaded successfully.", L"Success", MB_ICONINFORMATION | MB_OK);
            }
            else
            {
              std::wstring errorMsg = L"Failed to load dataset.\n" + DataSerializer::getLastError();
              MessageBoxW(hwnd_, errorMsg.c_str(), L"Error", MB_ICONERROR | MB_OK);
            }
          }
          return 0;
        }

        case IDM_FILE_SAVE:
        {
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }
          std::wstring filePath = getFileDialogPath(hwnd_, true, appConfig_.getSaveFilePath());
          if (!filePath.empty())
          {
            appConfig_.setSaveFilePath(filePath);
            appConfig_.save();

            if (DataSerializer::saveToFile(*appData_, filePath))
            {
              MessageBoxW(hwnd_, L"Dataset saved successfully.", L"Success", MB_ICONINFORMATION | MB_OK);
            }
            else
            {
              std::wstring errorMsg = L"Failed to save dataset.\n" + DataSerializer::getLastError();
              MessageBoxW(hwnd_, errorMsg.c_str(), L"Error", MB_ICONERROR | MB_OK);
            }
          }
          return 0;
        }

        case IDM_FILE_LOAD_REPORT:
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }
          if (reportsTabContent_)
          {
            reportsTabContent_->loadReport();
          }
          refreshAllTabContents();
          return 0;

        case IDM_FILE_IMPORT_DATA:
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }
          if (reportsTabContent_)
          {
            reportsTabContent_->loadReport(false, false, true);
          }
          refreshAllTabContents();
          return 0;

        case IDM_FILE_EXPORT_ORDERS:
        {
          if (mapTabContent_)
          {
            mapTabContent_->commitPendingEdits();
          }

          MainFactionExportContext exportContext;
          std::wstring contextError;
          if (!tryBuildMainFactionExportContext(*appData_, exportContext, contextError))
          {
            MessageBoxW(hwnd_, contextError.c_str(), L"Export Orders", MB_ICONWARNING | MB_OK);
            return 0;
          }

          std::wstring initialFolder = appConfig_.getExportOrdersFolder();
          if (initialFolder.empty())
          {
            initialFolder = directoryFromFilePath(appConfig_.getSaveFilePath());
          }

          std::filesystem::path initialFilePath(initialFolder);
          initialFilePath /= buildSuggestedOrdersFilename(exportContext);

          static constexpr wchar_t kOrdersFilter[] = L"Order Files (*.ord)\0*.ord\0All Files (*)\0*.*\0";
          std::wstring outputFilePath = getFileDialogPath(hwnd_,
                                                          true,
                                                          initialFilePath.wstring(),
                                                          L"ord",
                                                          kOrdersFilter);
          if (outputFilePath.empty())
          {
            return 0;
          }

          appConfig_.setExportOrdersFolder(directoryFromFilePath(outputFilePath));
          appConfig_.save();

          const std::wstring content = buildOrdersExportContent(*appData_,
                                                                exportContext.factionNumber,
                                                                exportContext.password);

          std::wstring saveError;
          if (!saveTextFile(outputFilePath, content, saveError))
          {
            MessageBoxW(hwnd_, saveError.c_str(), L"Export Orders", MB_ICONERROR | MB_OK);
            return 0;
          }

          MessageBoxW(hwnd_, L"Orders exported successfully.", L"Export Orders", MB_ICONINFORMATION | MB_OK);
          return 0;
        }

        case IDM_FILE_EXIT:
          PostMessageW(hwnd_, WM_CLOSE, 0, 0);
          return 0;

        case IDM_SETTINGS_OPTIONS:
        {
          SettingsDialog settingsDialog;
          settingsDialog.showDialog(hwnd_, *appData_, appConfig_);
          return 0;
        }

        case IDM_HELP_DESCRIPTION:
        {
          showMapDescriptionDialog(hwnd_);
          return 0;
        }

        case IDM_HELP_ABOUT:
        {
          showAboutDialog(hwnd_, instance_);
          return 0;
        }
      }

      return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
      HWND tabCtrl = tabView_ ? tabView_->getTabControl() : nullptr;
      const int selectedTab = tabCtrl ? TabCtrl_GetCurSel(tabCtrl) : -1;

      if (selectedTab == reportsTabIndex_ && reportsTabContent_ && reportsTabContent_->handleMouseMessage(msg, wp, lp))
      {
        return 0;
      }

      if (selectedTab == mapTabIndex_ && mapTabContent_ && mapTabContent_->handleMouseMessage(msg, wp, lp))
      {
        return 0;
      }

      break;
    }

    case WM_SETCURSOR:
    {
      if (LOWORD(lp) == HTCLIENT)
      {
        HWND tabCtrl = tabView_ ? tabView_->getTabControl() : nullptr;
        const int selectedTab = tabCtrl ? TabCtrl_GetCurSel(tabCtrl) : -1;

        POINT cursorPos {};
        GetCursorPos(&cursorPos);
        ScreenToClient(hwnd_, &cursorPos);
        const LPARAM pointParam = MAKELPARAM(cursorPos.x, cursorPos.y);

        if (selectedTab == reportsTabIndex_ && reportsTabContent_ && reportsTabContent_->handleMouseMessage(WM_MOUSEMOVE, 0, pointParam))
        {
          return TRUE;
        }

        if (selectedTab == mapTabIndex_ && mapTabContent_ && mapTabContent_->handleMouseMessage(WM_MOUSEMOVE, 0, pointParam))
        {
          return TRUE;
        }
      }

      break;
    }

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcW(hwnd_, msg, wp, lp);
}

void MainWindow::initializeTabs()
{
  auto orderedTabs = kTabDefinitions;
  std::stable_sort(orderedTabs.begin(),
                  orderedTabs.end(),
                  [](const TabDefinition& lhs, const TabDefinition& rhs)
                  {
                    return lhs.order < rhs.order;
                  });

  reportsTabIndex_ = -1;
  mapTabIndex_ = -1;
  eventsTabIndex_ = -1;
  itemsTabIndex_ = -1;
  skillsTabIndex_ = -1;
  factionsTabIndex_ = -1;
  battlesTabIndex_ = -1;

  for (int index = 0; index < static_cast<int>(orderedTabs.size()); ++index)
  {
    const auto& tab = orderedTabs[static_cast<std::size_t>(index)];
    tabView_->addTab(tab.label);

    switch (tab.id)
    {
      case InternalTabId::Reports:
        reportsTabIndex_ = index;
        break;
      case InternalTabId::Map:
        mapTabIndex_ = index;
        break;
      case InternalTabId::Events:
        eventsTabIndex_ = index;
        break;
      case InternalTabId::Items:
        itemsTabIndex_ = index;
        break;
      case InternalTabId::Skills:
        skillsTabIndex_ = index;
        break;
      case InternalTabId::Factions:
        factionsTabIndex_ = index;
        break;
      case InternalTabId::Battles:
        battlesTabIndex_ = index;
        break;
    }
  }
}

void MainWindow::createMenu()
{
  HMENU hMenuBar = CreateMenu();
  HMENU hFileMenu = CreateMenu();

  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_NEW,         L"&New");
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_OPEN,        L"&Open...");
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_SAVE,        L"&Save");
  AppendMenuW(hFileMenu, MFT_SEPARATOR, 0,                 nullptr);
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_LOAD_REPORT, L"&Load Report");
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_IMPORT_DATA, L"&Import Data...");
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_EXPORT_ORDERS, L"E&xport Orders...");
  AppendMenuW(hFileMenu, MFT_SEPARATOR, 0,                 nullptr);
  AppendMenuW(hFileMenu, MFT_STRING, IDM_FILE_EXIT,        L"E&xit");

  AppendMenuW(hMenuBar, MFT_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"&File");

  HMENU hSettingsMenu = CreateMenu();
  AppendMenuW(hSettingsMenu, MFT_STRING, IDM_SETTINGS_OPTIONS, L"&Options");

  AppendMenuW(hMenuBar, MFT_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSettingsMenu), L"&Settings");

  HMENU hHelpMenu = CreateMenu();
  AppendMenuW(hHelpMenu, MFT_STRING, IDM_HELP_DESCRIPTION, L"&Description");
  AppendMenuW(hHelpMenu, MFT_STRING, IDM_HELP_ABOUT, L"&About");

  AppendMenuW(hMenuBar, MFT_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), L"&Help");

  SetMenu(hwnd_, hMenuBar);
}

void MainWindow::refreshAllTabContents()
{
  if (reportsTabContent_)
  {
    reportsTabContent_->refresh();
  }
  if (mapTabContent_)
  {
    mapTabContent_->refresh();
  }
  if (eventsTabContent_)
  {
    eventsTabContent_->refresh();
  }
  if (itemsTabContent_)
  {
    itemsTabContent_->refresh();
  }
  if (skillsTabContent_)
  {
    skillsTabContent_->refresh();
  }
  if (factionsTabContent_)
  {
    factionsTabContent_->refresh();
  }
  if (battlesTabContent_)
  {
    battlesTabContent_->refresh();
  }

  // Always re-apply visibility after bulk refresh to prevent controls from
  // non-selected tabs remaining visible.
  updateReportsTabVisibility();
}

void MainWindow::updateReportsTabVisibility()
{
  if (!tabView_ 
    || !reportsTabContent_ 
    || !mapTabContent_ 
    || !eventsTabContent_ 
    || !itemsTabContent_ 
    || !skillsTabContent_ 
    || !factionsTabContent_ 
    || !battlesTabContent_)
  {
    return;
  }

  HWND tabCtrl = tabView_->getTabControl();
  if (!tabCtrl)
  {
    return;
  }

  const int selected = TabCtrl_GetCurSel(tabCtrl);
  reportsTabContent_->setVisible(selected == reportsTabIndex_);
  mapTabContent_->setVisible(selected == mapTabIndex_);
  eventsTabContent_->setVisible(selected == eventsTabIndex_);
  itemsTabContent_->setVisible(selected == itemsTabIndex_);
  skillsTabContent_->setVisible(selected == skillsTabIndex_);
  factionsTabContent_->setVisible(selected == factionsTabIndex_);
  battlesTabContent_->setVisible(selected == battlesTabIndex_);
}

void MainWindow::showLoadedReportsTabContextMenu(POINT screenPoint)
{
  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  AppendMenuW(menu, MFT_STRING, IDM_TAB_CTX_LOAD_REPORT, L"Load Report");

  const UINT cmd = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    hwnd_,
    nullptr
  );

  DestroyMenu(menu);

  if (cmd == IDM_TAB_CTX_LOAD_REPORT && reportsTabContent_)
  {
    reportsTabContent_->loadReport();
    refreshAllTabContents();
  }
}

MainWindow::MainWindow() = default;
MainWindow::~MainWindow() = default;

void MainWindow::persistWindowSizeToConfig()
{
  if (!hwnd_)
  {
    return;
  }

  RECT windowRect {};
  if (!GetWindowRect(hwnd_, &windowRect))
  {
    return;
  }

  const int width = windowRect.right - windowRect.left;
  const int height = windowRect.bottom - windowRect.top;
  if (width <= 0 || height <= 0)
  {
    return;
  }

  appConfig_.setMainWindowWidth(width);
  appConfig_.setMainWindowHeight(height);
  appConfig_.save();
}
