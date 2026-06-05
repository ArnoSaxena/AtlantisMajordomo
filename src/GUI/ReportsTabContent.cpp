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
 * File: ReportsTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/ReportsTabContent.hpp"

#include "Data/AppData.hpp"
#include "AppConfig.hpp"
#include "Function/MonthUtils.hpp"

#include <algorithm>
#include <commdlg.h>
#include <filesystem>
#include <windowsx.h>

namespace
{
constexpr int kMargin = 8;
constexpr int kPaneGap = 12;
constexpr int kMinPaneWidth = 180;

template <typename T>
T clampValue(T value, T low, T high)
{
  return (std::max)(low, (std::min)(value, high));
}
}

bool ReportsTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData, AppConfig& appConfig)
{
  parentWindow_ = parentWindow;
  instance_ = instance;
  appData_ = &appData;
  appConfig_ = &appConfig;

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  reportsList_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | WS_TABSTOP | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    parentWindow_,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kListControlId)),
    instance_,
    nullptr
  );

  if (!reportsList_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(reportsList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  LVCOLUMNW column {};
  column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  column.pszText = const_cast<wchar_t*>(L"#");
  column.cx = 56;
  column.iSubItem = 0;
  ListView_InsertColumn(reportsList_, 0, &column);

  column.pszText = const_cast<wchar_t*>(L"Report File");
  column.cx = 300;
  column.iSubItem = 1;
  ListView_InsertColumn(reportsList_, 1, &column);

  removeReportButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Clear",
    WS_CHILD | BS_PUSHBUTTON,
    0, 0, 140, 30,
    parentWindow_,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRemoveButtonId)),
    instance_,
    nullptr
  );

  if (!removeReportButton_)
  {
    return false;
  }

  rightPane_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    L"STATIC",
    L"",
    WS_CHILD,
    0, 0, 100, 100,
    parentWindow_,
    nullptr,
    instance_,
    nullptr
  );

  if (!rightPane_)
  {
    return false;
  }

  factionLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parentWindow_,
    nullptr,
    instance_,
    nullptr
  );

  if (!factionLabel_)
  {
    return false;
  }

  monthLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parentWindow_,
    nullptr,
    instance_,
    nullptr
  );

  if (!monthLabel_)
  {
    return false;
  }

  foundRegionsLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parentWindow_,
    nullptr,
    instance_,
    nullptr
  );

  if (!foundRegionsLabel_)
  {
    return false;
  }

  visitedRegionsLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"",
    WS_CHILD | SS_LEFT,
    0, 0, 100, 20,
    parentWindow_,
    nullptr,
    instance_,
    nullptr
  );

  return visitedRegionsLabel_ != nullptr;
}

void ReportsTabContent::resize(const RECT& displayRect)
{
  lastDisplayRect_ = displayRect;

  if (!reportsList_ || !removeReportButton_ || !rightPane_ || !factionLabel_ || !monthLabel_ || !foundRegionsLabel_ || !visitedRegionsLabel_)
  {
    return;
  }

  constexpr int kButtonHeight = 30;
  constexpr int kButtonWidth = 140;

  const int areaLeft = displayRect.left + kMargin;
  const int areaTop = displayRect.top + kMargin;
  const int contentWidth = static_cast<int>(displayRect.right - displayRect.left) - 2 * kMargin;
  const int contentHeight = static_cast<int>(displayRect.bottom - displayRect.top) - 2 * kMargin;

  const int paneAreaWidth = (std::max)(0, contentWidth - kPaneGap);
  const int minPaneWidth = (std::min)(kMinPaneWidth, paneAreaWidth / 2);
  const int leftPaneWidth = clampValue(
    static_cast<int>(horizontalSplitRatio_ * static_cast<float>(paneAreaWidth)),
    minPaneWidth,
    (std::max)(minPaneWidth, paneAreaWidth - minPaneWidth));
  const int rightPaneWidth = paneAreaWidth - leftPaneWidth;
  const int leftPaneLeft = areaLeft;
  const int rightPaneLeft = leftPaneLeft + leftPaneWidth + kPaneGap;

  const int tableHeight = (std::max)(80, contentHeight - kButtonHeight - kMargin);

  SetWindowPos(
    reportsList_,
    HWND_TOP,
    leftPaneLeft,
    areaTop,
    leftPaneWidth,
    tableHeight,
    SWP_NOACTIVATE
  );

  const int listClientW = (std::max)(0, leftPaneWidth - 6);
  constexpr int kIndexColumnWidth = 56;
  ListView_SetColumnWidth(reportsList_, 0, kIndexColumnWidth);
  ListView_SetColumnWidth(reportsList_, 1, (std::max)(100, listClientW - kIndexColumnWidth));

  SetWindowPos(
    removeReportButton_,
    HWND_TOP,
    leftPaneLeft + (std::max)(0, leftPaneWidth - kButtonWidth),
    areaTop + tableHeight + kMargin,
    kButtonWidth,
    kButtonHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    rightPane_,
    HWND_TOP,
    rightPaneLeft,
    areaTop,
    rightPaneWidth,
    contentHeight,
    SWP_NOACTIVATE
  );

  constexpr int kLabelHeight = 20;
  constexpr int kLabelMargin = 8;
  const int labelLeft  = rightPaneLeft + kLabelMargin;
  const int labelWidth = (std::max)(0, rightPaneWidth - 2 * kLabelMargin);

  SetWindowPos(
    factionLabel_,
    HWND_TOP,
    labelLeft,
    areaTop + kLabelMargin,
    labelWidth,
    kLabelHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    monthLabel_,
    HWND_TOP,
    labelLeft,
    areaTop + kLabelMargin + kLabelHeight + kLabelMargin,
    labelWidth,
    kLabelHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    foundRegionsLabel_,
    HWND_TOP,
    labelLeft,
    areaTop + kLabelMargin + 2 * (kLabelHeight + kLabelMargin),
    labelWidth,
    kLabelHeight,
    SWP_NOACTIVATE
  );

  SetWindowPos(
    visitedRegionsLabel_,
    HWND_TOP,
    labelLeft,
    areaTop + kLabelMargin + 3 * (kLabelHeight + kLabelMargin),
    labelWidth,
    kLabelHeight,
    SWP_NOACTIVATE
  );
}

bool ReportsTabContent::handleMouseMessage(UINT msg, WPARAM wp, LPARAM lp)
{
  (void)wp;

  if (!reportsList_)
  {
    return false;
  }

  const POINT point {
    GET_X_LPARAM(lp),
    GET_Y_LPARAM(lp)
  };

  switch (msg)
  {
    case WM_LBUTTONDOWN:
    {
      const RECT splitterRect = getHorizontalSplitterRect();
      if (PtInRect(&splitterRect, point))
      {
        draggingHorizontalSplit_ = true;
        SetCapture(parentWindow_);
        return true;
      }
      return false;
    }

    case WM_MOUSEMOVE:
    {
      if (draggingHorizontalSplit_)
      {
        const int contentWidth = (std::max)(0, static_cast<int>(lastDisplayRect_.right - lastDisplayRect_.left) - 2 * kMargin);
        const int paneAreaWidth = (std::max)(1, contentWidth - kPaneGap);
        const int areaLeft = lastDisplayRect_.left + kMargin;
        const int minPaneWidth = (std::min)(kMinPaneWidth, paneAreaWidth / 2);

        int proposedLeft = point.x - areaLeft - (kPaneGap / 2);
        proposedLeft = clampValue(proposedLeft, minPaneWidth, (std::max)(minPaneWidth, paneAreaWidth - minPaneWidth));
        horizontalSplitRatio_ = static_cast<float>(proposedLeft) / static_cast<float>(paneAreaWidth);
        resize(lastDisplayRect_);
        return true;
      }

      const RECT splitterRect = getHorizontalSplitterRect();
      if (PtInRect(&splitterRect, point))
      {
        SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
        return true;
      }
      return false;
    }

    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
      if (draggingHorizontalSplit_)
      {
        draggingHorizontalSplit_ = false;
        if (GetCapture() == parentWindow_)
        {
          ReleaseCapture();
        }
        return true;
      }
      return false;
    }
  }

  return false;
}

RECT ReportsTabContent::getHorizontalSplitterRect() const
{
  RECT rect { 0, 0, 0, 0 };
  const int contentWidth = (std::max)(0, static_cast<int>(lastDisplayRect_.right - lastDisplayRect_.left) - 2 * kMargin);
  const int contentHeight = (std::max)(0, static_cast<int>(lastDisplayRect_.bottom - lastDisplayRect_.top) - 2 * kMargin);
  const int paneAreaWidth = (std::max)(0, contentWidth - kPaneGap);
  const int minPaneWidth = (std::min)(kMinPaneWidth, paneAreaWidth / 2);
  const int leftPaneWidth = clampValue(
    static_cast<int>(horizontalSplitRatio_ * static_cast<float>(paneAreaWidth)),
    minPaneWidth,
    (std::max)(minPaneWidth, paneAreaWidth - minPaneWidth));

  rect.left = lastDisplayRect_.left + kMargin + leftPaneWidth;
  rect.right = rect.left + kPaneGap;
  rect.top = lastDisplayRect_.top + kMargin;
  rect.bottom = rect.top + contentHeight;
  return rect;
}

void ReportsTabContent::setVisible(bool visible)
{
  if (!reportsList_ || !removeReportButton_ || !rightPane_ || !factionLabel_ || !monthLabel_ || !foundRegionsLabel_ || !visitedRegionsLabel_)
  {
    return;
  }

  ShowWindow(reportsList_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(removeReportButton_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(rightPane_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(factionLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(monthLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(foundRegionsLabel_, visible ? SW_SHOW : SW_HIDE);
  ShowWindow(visitedRegionsLabel_, visible ? SW_SHOW : SW_HIDE);

  if (visible)
  {
    updateReportsList();

    const auto repositorySize = appData_ ? appData_->reportRepository().size() : 0;
    if (repositorySize == 0)
    {
      selectedReportRow_ = -1;
    }
    else if (selectedReportRow_ >= static_cast<int>(repositorySize))
    {
      selectedReportRow_ = static_cast<int>(repositorySize) - 1;
    }

    if (selectedReportRow_ >= 0)
    {
      ListView_SetItemState(
        reportsList_,
        selectedReportRow_,
        LVIS_SELECTED | LVIS_FOCUSED,
        LVIS_SELECTED | LVIS_FOCUSED
      );
      ListView_EnsureVisible(reportsList_, selectedReportRow_, FALSE);
    }

    updateDetailPane(selectedReportRow_);
  }
}

bool ReportsTabContent::handleNotify(const NMHDR* hdr)
{
  if (!hdr || hdr->idFrom != static_cast<UINT>(kListControlId))
  {
    return false;
  }

  if (hdr->code == NM_CLICK || hdr->code == NM_SETFOCUS)
  {
    const int selectedRow = ListView_GetNextItem(reportsList_, -1, LVNI_SELECTED);
    if (selectedRow >= 0)
    {
      selectedReportRow_ = selectedRow;
    }
    updateDetailPane(selectedReportRow_);
    return false;
  }

  if (hdr->code == LVN_ITEMCHANGED)
  {
    const auto* pnmv = reinterpret_cast<const NMLISTVIEW*>(hdr);
    if ((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED))
    {
      selectedReportRow_ = pnmv->iItem;
      updateDetailPane(pnmv->iItem);
    }
    else if ((pnmv->uChanged & LVIF_STATE) && (pnmv->uOldState & LVIS_SELECTED)
      && !(pnmv->uNewState & LVIS_SELECTED) && selectedReportRow_ == pnmv->iItem)
    {
      selectedReportRow_ = ListView_GetNextItem(reportsList_, -1, LVNI_SELECTED);
      updateDetailPane(selectedReportRow_);
    }
    return false;
  }

  if (hdr->code != NM_RCLICK)
  {
    return false;
  }

  POINT cursorPos {};
  GetCursorPos(&cursorPos);

  POINT clientPos = cursorPos;
  ScreenToClient(reportsList_, &clientPos);

  LVHITTESTINFO hitInfo {};
  hitInfo.pt = clientPos;
  const int row = ListView_SubItemHitTest(reportsList_, &hitInfo);
  contextMenuRow_ = -1;

  if (row >= 0 && (hitInfo.flags & LVHT_ONITEM))
  {
    ListView_SetItemState(
      reportsList_,
      row,
      LVIS_SELECTED | LVIS_FOCUSED,
      LVIS_SELECTED | LVIS_FOCUSED
    );
    contextMenuRow_ = row;
  }
  else
  {
    ListView_SetItemState(reportsList_, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
  }

  showContextMenu(cursorPos, contextMenuRow_);
  return true;
}

bool ReportsTabContent::handleCommand(int commandId, int /*notificationCode*/)
{
  switch (commandId)
  {
    case kRemoveButtonId:
      clearReports();
      return true;

    case kContextLoadCmd:
      loadReport();
      return true;

    case kContextRemoveCmd:
      removeReportAt(contextMenuRow_);
      return true;
  }

  return false;
}

void ReportsTabContent::loadReport(bool syncFactionFromHeader)
{
  if (!appData_)
  {
    return;
  }

  auto& repository = appData_->reportRepository();
  std::wstring filePath = showFileOpenDialog();
  if (filePath.empty())
  {
    return;
  }

  if (appConfig_)
  {
    const std::filesystem::path selectedPath(filePath);
    if (selectedPath.has_parent_path())
    {
      appConfig_->setReportImportFolder(selectedPath.parent_path().wstring());
      appConfig_->save();
    }
  }

  if (repository.addFromFile(filePath,
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
                            syncFactionFromHeader))
  {
    updateReportsList();
  }
  else
  {
    std::wstring message = L"Failed to load report:\n\n" + repository.getLastError();
    MessageBoxW(parentWindow_, message.c_str(), L"Error", MB_ICONERROR | MB_OK);
  }
}

void ReportsTabContent::refresh()
{
  updateReportsList();

  const auto repositorySize = appData_ ? appData_->reportRepository().size() : 0;
  if (repositorySize == 0)
  {
    selectedReportRow_ = -1;
  }
  else if (selectedReportRow_ >= static_cast<int>(repositorySize))
  {
    selectedReportRow_ = static_cast<int>(repositorySize) - 1;
  }

  if (selectedReportRow_ >= 0)
  {
    ListView_SetItemState(
      reportsList_,
      selectedReportRow_,
      LVIS_SELECTED | LVIS_FOCUSED,
      LVIS_SELECTED | LVIS_FOCUSED
    );
  }

  updateDetailPane(selectedReportRow_);
}

void ReportsTabContent::showContextMenu(POINT screenPoint, int rowIndex)
{
  HMENU menu = CreatePopupMenu();
  if (!menu)
  {
    return;
  }

  AppendMenuW(menu, MFT_STRING, kContextLoadCmd, L"Load Report");

  const auto& repository = appData_->reportRepository();
  const bool rowIsLoadedReport = rowIndex >= 0 && static_cast<std::size_t>(rowIndex) < repository.size();
  if (rowIsLoadedReport)
  {
    AppendMenuW(menu, MFT_STRING, kContextRemoveCmd, L"Remove");
  }

  const UINT cmd = TrackPopupMenu(
    menu,
    TPM_RETURNCMD | TPM_RIGHTBUTTON,
    screenPoint.x,
    screenPoint.y,
    0,
    parentWindow_,
    nullptr
  );

  DestroyMenu(menu);

  if (cmd != 0)
  {
    handleCommand(static_cast<int>(cmd));
  }
}

void ReportsTabContent::removeReportAt(int rowIndex)
{
  if (!appData_ || rowIndex < 0)
  {
    return;
  }

  auto& repository = appData_->reportRepository();
  if (static_cast<std::size_t>(rowIndex) >= repository.size())
  {
    return;
  }

  if (!repository.removeAt(static_cast<std::size_t>(rowIndex)))
  {
    MessageBoxW(parentWindow_, repository.getLastError().c_str(), L"Error", MB_ICONERROR | MB_OK);
    return;
  }

  updateReportsList();

  if (repository.size() > 0)
  {
    const std::size_t nextSelection =
      static_cast<std::size_t>(rowIndex) < repository.size()
        ? static_cast<std::size_t>(rowIndex)
        : repository.size() - 1;

    selectedReportRow_ = static_cast<int>(nextSelection);

    ListView_SetItemState(
      reportsList_,
      static_cast<int>(nextSelection),
      LVIS_SELECTED | LVIS_FOCUSED,
      LVIS_SELECTED | LVIS_FOCUSED
    );
  }
  else
  {
    selectedReportRow_ = -1;
    updateDetailPane(-1);
  }
}

void ReportsTabContent::clearReports()
{
  if (!reportsList_ || !appData_)
  {
    return;
  }

  appData_->reportRepository().clear();
  selectedReportRow_ = -1;
  contextMenuRow_ = -1;
  updateReportsList();
  updateDetailPane(-1);
}

void ReportsTabContent::updateReportsList()
{
  if (!reportsList_ || !appData_)
  {
    return;
  }

  ListView_DeleteAllItems(reportsList_);

  const auto& repository = appData_->reportRepository();
  for (std::size_t i = 0; i < repository.size(); ++i)
  {
    std::wstring rowNumber = std::to_wstring(i + 1);
    const auto& fullPath = repository.at(i).getFilePath();
    std::wstring displayName = std::filesystem::path(fullPath).filename().wstring();
    if (displayName.empty())
    {
      displayName = fullPath;
    }

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = static_cast<int>(i);
    item.iSubItem = 0;
    item.pszText = const_cast<wchar_t*>(rowNumber.c_str());
    ListView_InsertItem(reportsList_, &item);

    ListView_SetItemText(
      reportsList_,
      static_cast<int>(i),
      1,
      const_cast<wchar_t*>(displayName.c_str())
    );
  }
}

void ReportsTabContent::updateDetailPane(int selectedRow)
{
  if (!factionLabel_ || !monthLabel_ || !foundRegionsLabel_ || !visitedRegionsLabel_ || !appData_)
  {
    return;
  }

  const auto& repository = appData_->reportRepository();
  if (selectedRow < 0 || static_cast<std::size_t>(selectedRow) >= repository.size())
  {
    SetWindowTextW(factionLabel_, L"");
    SetWindowTextW(monthLabel_, L"");
    SetWindowTextW(foundRegionsLabel_, L"");
    SetWindowTextW(visitedRegionsLabel_, L"");
    return;
  }

  const auto& report = repository.at(static_cast<std::size_t>(selectedRow));

  std::wstring factionText = L"Faction: " + report.getFactionName()
    + L" (" + std::to_wstring(report.getFactionNumber()) + L")";
  SetWindowTextW(factionLabel_, factionText.c_str());

  const int month = report.getMonth();
  const int year  = report.getYear();
  const std::wstring monthName = MonthUtils::monthNumberToName(month);
  std::wstring monthText = L"Month: " + monthName + L", " + std::to_wstring(year);
  SetWindowTextW(monthLabel_, monthText.c_str());

  const int foundRegions = report.getFoundRegions();
  std::wstring foundRegionsText = L"Found Regions: " + std::to_wstring(foundRegions);
  SetWindowTextW(foundRegionsLabel_, foundRegionsText.c_str());

  const int visitedRegions = report.getVisitedRegions();
  std::wstring visitedRegionsText = L"Visited Regions: " + std::to_wstring(visitedRegions);
  SetWindowTextW(visitedRegionsLabel_, visitedRegionsText.c_str());
}

std::wstring ReportsTabContent::showFileOpenDialog() const
{
  wchar_t szFile[MAX_PATH] = L"";
  wchar_t szDir[MAX_PATH] = L"";

  if (appConfig_ && !appConfig_->getReportImportFolder().empty())
  {
    wcsncpy_s(szDir,
              sizeof(szDir) / sizeof(szDir[0]),
              appConfig_->getReportImportFolder().c_str(),
              _TRUNCATE);
  }

  OPENFILENAMEW ofn {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = parentWindow_;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
  ofn.lpstrInitialDir = szDir[0] != L'\0' ? szDir : nullptr;
  ofn.lpstrFilter = L"Report Files (*.rep)\0*.rep\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
  ofn.nFilterIndex = 3;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameW(&ofn))
  {
    return std::wstring(szFile);
  }

  return std::wstring();
}
