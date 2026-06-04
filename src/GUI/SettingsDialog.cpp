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
 * File: SettingsDialog.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/SettingsDialog.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Function/StringUtils.hpp"

#include <commdlg.h>
#include <shlobj.h>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>

namespace
{
std::vector<std::wstring> splitCsv(const std::wstring& csv)
{
  std::vector<std::wstring> values;
  std::wstring current;

  for (wchar_t ch : csv)
  {
    if (ch == L',')
    {
      std::wstring item = StringUtils::trimWhitespace(current);
      if (!item.empty())
      {
        values.push_back(item);
      }
      current.clear();
      continue;
    }

    current.push_back(ch);
  }

  std::wstring item = StringUtils::trimWhitespace(current);
  if (!item.empty())
  {
    values.push_back(item);
  }

  return values;
}

void loadListFromCsv(HWND listHandle, const std::wstring& csv)
{
  if (!listHandle)
  {
    return;
  }

  SendMessageW(listHandle, LB_RESETCONTENT, 0, 0);
  const std::vector<std::wstring> values = splitCsv(csv);
  for (const std::wstring& value : values)
  {
    SendMessageW(listHandle, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(value.c_str()));
  }
}

std::wstring buildCsvFromList(HWND listHandle)
{
  if (!listHandle)
  {
    return L"";
  }

  const LRESULT count = SendMessageW(listHandle, LB_GETCOUNT, 0, 0);
  if (count <= 0)
  {
    return L"";
  }

  std::wstring csv;
  for (int index = 0; index < static_cast<int>(count); ++index)
  {
    const LRESULT length = SendMessageW(listHandle, LB_GETTEXTLEN, static_cast<WPARAM>(index), 0);
    if (length <= 0)
    {
      continue;
    }

    std::wstring text(static_cast<std::size_t>(length), L'\0');
    SendMessageW(listHandle, LB_GETTEXT, static_cast<WPARAM>(index), reinterpret_cast<LPARAM>(text.data()));

    if (!csv.empty())
    {
      csv += L", ";
    }
    csv += text;
  }

  return csv;
}

bool listContainsItem(HWND listHandle, const std::wstring& item)
{
  if (!listHandle || item.empty())
  {
    return false;
  }

  const LRESULT count = SendMessageW(listHandle, LB_GETCOUNT, 0, 0);
  for (int index = 0; index < static_cast<int>(count); ++index)
  {
    const LRESULT length = SendMessageW(listHandle, LB_GETTEXTLEN, static_cast<WPARAM>(index), 0);
    if (length <= 0)
    {
      continue;
    }

    std::wstring existing(static_cast<std::size_t>(length), L'\0');
    SendMessageW(listHandle, LB_GETTEXT, static_cast<WPARAM>(index), reinterpret_cast<LPARAM>(existing.data()));

    if (_wcsicmp(existing.c_str(), item.c_str()) == 0)
    {
      return true;
    }
  }

  return false;
}

std::wstring getOpenDataFilePath(HWND hwnd, const std::wstring& initialFilePath)
{
  wchar_t szFile[MAX_PATH] = {};
  if (!initialFilePath.empty())
  {
    wcsncpy_s(szFile, sizeof(szFile) / sizeof(szFile[0]), initialFilePath.c_str(), _TRUNCATE);
  }

  std::filesystem::path initialPath(initialFilePath);
  std::wstring initialDir;
  if (initialPath.has_parent_path())
  {
    initialDir = initialPath.parent_path().wstring();
  }

  OPENFILENAMEW ofn {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
  ofn.lpstrInitialDir = initialDir.empty() ? nullptr : initialDir.c_str();
  ofn.lpstrFilter = L"Data Files (*.txt;*.dat)\0*.txt;*.dat\0All Files (*.*)\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrDefExt = L"txt";
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

  if (GetOpenFileNameW(&ofn))
  {
    return szFile;
  }

  return L"";
}

std::wstring getReportFolderPath(HWND hwnd)
{
  wchar_t selectedFolder[MAX_PATH] = {};
  BROWSEINFOW browseInfo{};
  browseInfo.hwndOwner = hwnd;
  browseInfo.lpszTitle = L"Select report folder";
  browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;

  PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&browseInfo);
  if (!pidl)
  {
    return L"";
  }

  std::wstring folder;
  if (SHGetPathFromIDListW(pidl, selectedFolder))
  {
    folder = selectedFolder;
  }

  CoTaskMemFree(pidl);
  return folder;
}

} // namespace

SettingsDialog::SettingsDialog() = default;

SettingsDialog::~SettingsDialog()
{
  if (hwnd_ && IsWindow(hwnd_))
  {
    DestroyWindow(hwnd_);
  }
}

LRESULT CALLBACK SettingsDialog::windowProcedureStatic(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  if (msg == WM_CREATE)
  {
    CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lp);
    SettingsDialog* pThis = reinterpret_cast<SettingsDialog*>(pCreate->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    return 0;
  }

  SettingsDialog* pThis = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (pThis)
  {
    return pThis->windowProcedure(hwnd, msg, wp, lp);
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

bool SettingsDialog::applySettingsFromControls(HWND hwnd, bool closeOnSuccess)
{
  if (!appData_)
  {
    result_ = IDCANCEL;
    if (closeOnSuccess)
    {
      DestroyWindow(hwnd);
    }
    return false;
  }

  wchar_t thresholdBuffer[64] = {};
  GetWindowTextW(shipThresholdEdit_, thresholdBuffer, static_cast<int>(std::size(thresholdBuffer)));

  int shipThreshold = 0;
  try
  {
    shipThreshold = std::stoi(std::wstring(thresholdBuffer));
  }
  catch (const std::exception&)
  {
    MessageBoxW(hwnd,
                L"Ship threshold must be a valid integer.",
                L"Invalid Settings",
                MB_ICONERROR | MB_OK);
    return false;
  }

  //const std::wstring flyingShipsCsv = buildCsvFromList(flyingShipsList_);
  const std::wstring fullMonthOrdersCsv = buildCsvFromList(fullMonthOrdersList_);
  const std::wstring magicTriggersCsv = buildCsvFromList(magicTriggersList_);

  appData_->setShipStructureIdThreshold(shipThreshold);
  //appData_->setFlyingShipsCsv(flyingShipsCsv);
  appData_->setMagicSkillTriggersCsv(magicTriggersCsv);
  appData_->setOnlyLeaderCanTeach(SendMessageW(onlyLeaderCanTeachCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED);
  appData_->setLeaderMages(SendMessageW(leaderMagesCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED);
  Commands::setFullMonthOrderKeywordsCsv(fullMonthOrdersCsv);

  std::wstring dataFilePath;
  if (dataFilePathEdit_)
  {
    wchar_t dataPathBuffer[MAX_PATH] = {};
    GetWindowTextW(dataFilePathEdit_, dataPathBuffer, static_cast<int>(std::size(dataPathBuffer)));
    dataFilePath = StringUtils::trimWhitespace(dataPathBuffer);
  }

  std::wstring reportFolderPath;
  if (reportFolderPathEdit_)
  {
    wchar_t folderPathBuffer[MAX_PATH] = {};
    GetWindowTextW(reportFolderPathEdit_, folderPathBuffer, static_cast<int>(std::size(folderPathBuffer)));
    reportFolderPath = StringUtils::trimWhitespace(folderPathBuffer);
  }

  if (appConfig_)
  {
    const LRESULT onlyLeaderChecked = SendMessageW(onlyLeaderCanTeachCheck_, BM_GETCHECK, 0, 0);
    const LRESULT leaderMagesChecked = SendMessageW(leaderMagesCheck_, BM_GETCHECK, 0, 0);
    appConfig_->setOnlyLeaderCanTeach(onlyLeaderChecked == BST_CHECKED);
    appConfig_->setLeaderMages(leaderMagesChecked == BST_CHECKED);
    //appConfig_->setFlyingShipsCsv(flyingShipsCsv);
    appConfig_->setFullMonthOrdersCsv(fullMonthOrdersCsv);
    appConfig_->setMagicSkillTriggersCsv(magicTriggersCsv);
    appConfig_->setDataFilePath(dataFilePath);
    appConfig_->setReportImportFolder(reportFolderPath);
    appConfig_->save();
  }

  if (closeOnSuccess)
  {
    result_ = IDOK;
    DestroyWindow(hwnd);
  }

  return true;
}

void SettingsDialog::addListItem(HWND listHandle, HWND inputHandle, const wchar_t* listNameForWarning)
{
  if (!listHandle || !inputHandle)
  {
    return;
  }

  wchar_t buffer[512] = {};
  GetWindowTextW(inputHandle, buffer, static_cast<int>(std::size(buffer)));
  const std::wstring item = StringUtils::trimWhitespace(buffer);
  if (item.empty())
  {
    return;
  }

  if (listContainsItem(listHandle, item))
  {
    std::wstring message = L"This entry already exists";
    if (listNameForWarning && *listNameForWarning)
    {
      message += L" in ";
      message += listNameForWarning;
    }
    message += L".";

    MessageBoxW(
      hwnd_,
      message.c_str(),
      L"Duplicate Entry",
      MB_OK | MB_ICONWARNING
    );
    return;
  }

  SendMessageW(listHandle, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
  SetWindowTextW(inputHandle, L"");
}

void SettingsDialog::removeSelectedListItem(HWND listHandle)
{
  if (!listHandle)
  {
    return;
  }

  const LRESULT selectedIndex = SendMessageW(listHandle, LB_GETCURSEL, 0, 0);
  if (selectedIndex == LB_ERR)
  {
    return;
  }

  SendMessageW(listHandle, LB_DELETESTRING, static_cast<WPARAM>(selectedIndex), 0);
}

LRESULT SettingsDialog::windowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_COMMAND:
    {
      const int controlId = LOWORD(wp);
      switch (controlId)
      {
        case IDC_OK:
          applySettingsFromControls(hwnd, true);
          return 0;

        case IDC_SAVE:
          applySettingsFromControls(hwnd, false);
          return 0;

        case IDC_CANCEL:
          result_ = IDCANCEL;
          DestroyWindow(hwnd);
          return 0;

        //case IDC_FLYING_SHIPS_ADD:
        //  addListItem(flyingShipsList_, flyingShipsInput_, L"Flying ships");
        //  return 0;

        //case IDC_FLYING_SHIPS_REMOVE:
        //  removeSelectedListItem(flyingShipsList_);
        //  return 0;

        case IDC_FULL_MONTH_ORDERS_ADD:
          addListItem(fullMonthOrdersList_, fullMonthOrdersInput_, L"Full-month orders");
          return 0;

        case IDC_FULL_MONTH_ORDERS_REMOVE:
          removeSelectedListItem(fullMonthOrdersList_);
          return 0;

        case IDC_MAGIC_TRIGGERS_ADD:
          addListItem(magicTriggersList_, magicTriggersInput_, L"Magic skill triggers");
          return 0;

        case IDC_MAGIC_TRIGGERS_REMOVE:
          removeSelectedListItem(magicTriggersList_);
          return 0;

        case IDC_DATA_FILE_PATH_BROWSE:
        {
          std::wstring currentPath;
          if (dataFilePathEdit_)
          {
            wchar_t pathBuffer[MAX_PATH] = {};
            GetWindowTextW(dataFilePathEdit_, pathBuffer, static_cast<int>(std::size(pathBuffer)));
            currentPath = StringUtils::trimWhitespace(pathBuffer);
          }

          const std::wstring selectedFile = getOpenDataFilePath(hwnd, currentPath);
          if (!selectedFile.empty() && dataFilePathEdit_)
          {
            SetWindowTextW(dataFilePathEdit_, selectedFile.c_str());
          }
          return 0;
        }

        case IDC_REPORT_FOLDER_PATH_BROWSE:
        {
          const std::wstring selectedFolder = getReportFolderPath(hwnd);
          if (!selectedFolder.empty() && reportFolderPathEdit_)
          {
            SetWindowTextW(reportFolderPathEdit_, selectedFolder.c_str());
          }
          return 0;
        }
      }
      break;
    }

    case WM_CLOSE:
      result_ = IDCANCEL;
      DestroyWindow(hwnd);
      return 0;
  }

  return DefWindowProcW(hwnd, msg, wp, lp);
}

int SettingsDialog::showDialog(HWND parentHwnd, AppData& appData, AppConfig& appConfig)
{
  appData_ = &appData;
  appConfig_ = &appConfig;

  WNDCLASSEXW wc {};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = windowProcedureStatic;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  wc.lpszClassName = L"SettingsDialogClass";

  static bool classRegistered = false;
  if (!classRegistered)
  {
    if (!RegisterClassExW(&wc))
    {
      return -1;
    }
    classRegistered = true;
  }

  hwnd_ = CreateWindowExW(
    WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
    L"SettingsDialogClass",
    L"Settings",
    WS_DLGFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    760,
    620,
    parentHwnd,
    nullptr,
    GetModuleHandleW(nullptr),
    this
  );

  if (!hwnd_)
  {
    return -1;
  }

  if (parentHwnd && IsWindow(parentHwnd))
  {
    RECT parentRc;
    RECT dialogRc;
    GetWindowRect(parentHwnd, &parentRc);
    GetWindowRect(hwnd_, &dialogRc);

    const int dialogWidth = dialogRc.right - dialogRc.left;
    const int dialogHeight = dialogRc.bottom - dialogRc.top;
    const int parentWidth = parentRc.right - parentRc.left;
    const int parentHeight = parentRc.bottom - parentRc.top;

    const int x = parentRc.left + (parentWidth - dialogWidth) / 2;
    const int y = parentRc.top + (parentHeight - dialogHeight) / 2;

    SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }

  CreateWindowExW(
    0,
    L"STATIC",
    L"Ship ID threshold:",
    WS_CHILD | WS_VISIBLE,
    20,
    20,
    140,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );

  shipThresholdEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    170,
    16,
    100,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_SHIP_THRESHOLD_EDIT)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  onlyLeaderCanTeachCheck_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Only leader can teach",
    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
    300,
    18,
    220,
    22,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_ONLY_LEADER_CAN_TEACH_CHECK)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  leaderMagesCheck_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Only leader Mages",
    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
    530,
    18,
    180,
    22,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_LEADER_MAGES_CHECK)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"STATIC",
    L"Data file path:",
    WS_CHILD | WS_VISIBLE,
    20,
    50,
    100,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );

  dataFilePathEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    130,
    46,
    520,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_DATA_FILE_PATH_INPUT)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Browse...",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    660,
    46,
    80,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_DATA_FILE_PATH_BROWSE)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"STATIC",
    L"Report folder:",
    WS_CHILD | WS_VISIBLE,
    20,
    84,
    100,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );

  reportFolderPathEdit_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    130,
    80,
    520,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_REPORT_FOLDER_PATH_INPUT)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Browse...",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    660,
    80,
    80,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_REPORT_FOLDER_PATH_BROWSE)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  /*
  CreateWindowExW(
    0,
    L"STATIC",
    L"Flying ship types",
    WS_CHILD | WS_VISIBLE,
    20,
    114,
    180,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );
  */

  /*
  flyingShipsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"LISTBOX",
    L"",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
    20,
    138,
    220,
    170,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FLYING_SHIPS_LIST)),
    GetModuleHandleW(nullptr),
    nullptr
  );
  */

  /*
  flyingShipsInput_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    20,
    318,
    220,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FLYING_SHIPS_INPUT)),
    GetModuleHandleW(nullptr),
    nullptr
  );
  */

  /*
  CreateWindowExW(
    0,
    L"BUTTON",
    L"Add",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    20,
    348,
    70,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FLYING_SHIPS_ADD)),
    GetModuleHandleW(nullptr),
    nullptr
  );
  */

  /*
  CreateWindowExW(
    0,
    L"BUTTON",
    L"Remove",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    100,
    348,
    80,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FLYING_SHIPS_REMOVE)),
    GetModuleHandleW(nullptr),
    nullptr
  );
  */

  CreateWindowExW(
    0,
    L"STATIC",
    L"Full-month orders",
    WS_CHILD | WS_VISIBLE,
    265,
    114,
    180,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );

  fullMonthOrdersList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"LISTBOX",
    L"",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
    265,
    138,
    220,
    170,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FULL_MONTH_ORDERS_LIST)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  fullMonthOrdersInput_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    265,
    318,
    220,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FULL_MONTH_ORDERS_INPUT)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Add",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    265,
    348,
    70,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FULL_MONTH_ORDERS_ADD)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Remove",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    345,
    348,
    80,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_FULL_MONTH_ORDERS_REMOVE)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"STATIC",
    L"Magic skill triggers",
    WS_CHILD | WS_VISIBLE,
    20,
    114,
    180,
    20,
    hwnd_,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );

  magicTriggersList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"LISTBOX",
    L"",
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
    20,
    138,
    220,
    170,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_MAGIC_TRIGGERS_LIST)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  magicTriggersInput_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
    20,
    318,
    220,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_MAGIC_TRIGGERS_INPUT)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Add",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    20,
    348,
    70,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_MAGIC_TRIGGERS_ADD)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Remove",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    100,
    348,
    80,
    24,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_MAGIC_TRIGGERS_REMOVE)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Save",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    350,
    540,
    90,
    28,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_SAVE)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"OK",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    450,
    540,
    90,
    28,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_OK)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  CreateWindowExW(
    0,
    L"BUTTON",
    L"Cancel",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    550,
    540,
    90,
    28,
    hwnd_,
    reinterpret_cast<HMENU>(static_cast<intptr_t>(IDC_CANCEL)),
    GetModuleHandleW(nullptr),
    nullptr
  );

  if (shipThresholdEdit_)
  {
    SetWindowTextW(shipThresholdEdit_, std::to_wstring(appData_->getShipStructureIdThreshold()).c_str());
  }
  if (onlyLeaderCanTeachCheck_)
  {
    SendMessageW(onlyLeaderCanTeachCheck_,
                 BM_SETCHECK,
                 appData_->getOnlyLeaderCanTeach() ? BST_CHECKED : BST_UNCHECKED,
                 0);
  }
  if (dataFilePathEdit_)
  {
    SetWindowTextW(dataFilePathEdit_, appConfig_->getDataFilePath().c_str());
  }
  if (reportFolderPathEdit_)
  {
    SetWindowTextW(reportFolderPathEdit_, appConfig_->getReportImportFolder().c_str());
  }
  if (leaderMagesCheck_)
  {
    SendMessageW(leaderMagesCheck_,
                 BM_SETCHECK,
                 appData_->getLeaderMages() ? BST_CHECKED : BST_UNCHECKED,
                 0);
  }

  //loadListFromCsv(flyingShipsList_, appData_->getFlyingShipsCsv());
  loadListFromCsv(fullMonthOrdersList_, Commands::getFullMonthOrderKeywordsCsv());
  loadListFromCsv(magicTriggersList_, appData_->getMagicSkillTriggersCsv());

  ShowWindow(hwnd_, SW_SHOW);
  UpdateWindow(hwnd_);

  MSG msg {};
  while (IsWindow(hwnd_) && GetMessageW(&msg, nullptr, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  const int resultValue = result_;
  appData_ = nullptr;
  appConfig_ = nullptr;
  hwnd_ = nullptr;
  shipThresholdEdit_ = nullptr;
  //flyingShipsList_ = nullptr;
  fullMonthOrdersList_ = nullptr;
  magicTriggersList_ = nullptr;
  //flyingShipsInput_ = nullptr;
  fullMonthOrdersInput_ = nullptr;
  magicTriggersInput_ = nullptr;
  onlyLeaderCanTeachCheck_ = nullptr;
  leaderMagesCheck_ = nullptr;
  result_ = -1;

  return resultValue;
}
