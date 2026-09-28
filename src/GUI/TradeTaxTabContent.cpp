/* 
 * Copyright (C) 2026 Arno Saxena
 *
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
 * File: TradeTaxTabContent.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/TradeTaxTabContent.hpp"

#include "Data/AppData.hpp"
#include "Function/CoordinateFormattingUtils.hpp"
#include "GUI/UiSizeProfile.hpp"
#include "GUI/WinSizingUtils.hpp"

#include <commctrl.h>

#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <utility>

namespace
{
constexpr int kTaxButtonId = 12101;
constexpr int kResourcesButtonId = 12102;
constexpr int kListId = 12103;
constexpr UINT_PTR kPanelSubclassId = 1;
constexpr int kToolbarHeight = 34;
constexpr int kGap = 6;

// Selected radio button background.
constexpr COLORREF kSelectedButtonBackgroundColor = RGB(0, 96, 32);
// Selected radio button text.
constexpr COLORREF kSelectedButtonTextColor = RGB(255, 255, 255);
// Tax/resource row is fully handled or exhausted.
constexpr COLORREF kSatisfiedRowColor = RGB(204, 255, 204);
// Tax row has less than one full potential taxer remaining.
constexpr COLORREF kPartialTaxRowColor = RGB(255, 245, 180);
// Tax/resource row still needs attention.
constexpr COLORREF kAttentionRowColor = RGB(255, 210, 210);

UiSizeProfile::Metrics resolveUiMetrics(HWND referenceWindow)
{
  const UiSizeProfile::DisplayInfo displayInfo = UiSizeProfile::queryDisplayInfoForWindow(
    referenceWindow != nullptr ? referenceWindow : GetDesktopWindow());
  const UiSizeProfile::Profile effectiveProfile = UiSizeProfile::resolveProfile(UiSizeProfile::Profile::Auto, displayInfo);
  return UiSizeProfile::getMetrics(effectiveProfile);
}

COLORREF colorForSeverity(TradeTaxUtils::RowSeverity severity)
{
  switch (severity)
  {
    case TradeTaxUtils::RowSeverity::Green:
      return kSatisfiedRowColor;
    case TradeTaxUtils::RowSeverity::Yellow:
      return kPartialTaxRowColor;
    case TradeTaxUtils::RowSeverity::Red:
    default:
      return kAttentionRowColor;
  }
}
}

bool TradeTaxTabContent::create(HWND parentWindow, HINSTANCE instance, AppData& appData)
{
  appData_ = &appData;
  const UiSizeProfile::Metrics metrics = resolveUiMetrics(parentWindow);

  panel_ = CreateWindowExW(
    0,
    L"STATIC",
    nullptr,
    WS_CHILD | WS_CLIPSIBLINGS,
    0, 0, 100, 100,
    parentWindow,
    nullptr,
    instance,
    nullptr);

  if (!panel_)
  {
    return false;
  }
  SetWindowSubclass(panel_, &TradeTaxTabContent::panelSubclassProc, kPanelSubclassId, reinterpret_cast<DWORD_PTR>(this));

  summaryLabel_ = CreateWindowExW(
    0,
    L"STATIC",
    L"Tax Regions: 0, Trade Regions: 0",
    WS_CHILD | SS_LEFT | SS_NOPREFIX,
    0, 0, 300, WinSizingUtils::scalePx(22, metrics),
    panel_,
    nullptr,
    instance,
    nullptr);

  taxButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Tax",
    WS_CHILD | BS_OWNERDRAW,
    0, 0, 100, WinSizingUtils::scalePx(28, metrics),
    panel_,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxButtonId)),
    instance,
    nullptr);

  resourcesButton_ = CreateWindowExW(
    0,
    L"BUTTON",
    L"Resources",
    WS_CHILD | BS_OWNERDRAW,
    0, 0, 130, WinSizingUtils::scalePx(28, metrics),
    panel_,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kResourcesButtonId)),
    instance,
    nullptr);

  list_ = CreateWindowExW(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEWW,
    nullptr,
    WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
    0, 0, 100, 100,
    panel_,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kListId)),
    instance,
    nullptr);

  if (!summaryLabel_ || !taxButton_ || !resourcesButton_ || !list_)
  {
    return false;
  }

  ListView_SetExtendedListViewStyle(list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
  WinSizingUtils::listViewApplyDensity(list_, metrics, nullptr, nullptr);

  updateButtonStates();
  updateSummaryLabel();
  updateListColumns();
  updateListRows();
  return true;
}

void TradeTaxTabContent::resize(const RECT& displayRect)
{
  if (!panel_)
  {
    return;
  }

  SetWindowPos(panel_, HWND_TOP,
               displayRect.left,
               displayRect.top,
               (std::max)(0L, displayRect.right - displayRect.left),
               (std::max)(0L, displayRect.bottom - displayRect.top),
               SWP_NOACTIVATE);

  RECT panelRect {};
  GetClientRect(panel_, &panelRect);
  const UiSizeProfile::Metrics metrics = resolveUiMetrics(panel_);
  const int margin = WinSizingUtils::scalePx(8, metrics);
  const int gap = WinSizingUtils::scalePx(kGap, metrics);
  const int buttonHeight = WinSizingUtils::scalePx(28, metrics);
  const int taxWidth = WinSizingUtils::scalePx(86, metrics);
  const int resourcesWidth = WinSizingUtils::scalePx(120, metrics);
  const int toolbarHeight = WinSizingUtils::scalePx(kToolbarHeight, metrics);
  const int summaryHeight = WinSizingUtils::scalePx(22, metrics);

  SetWindowPos(summaryLabel_, HWND_TOP,
               margin,
               margin,
               (std::max)(0, static_cast<int>(panelRect.right - panelRect.left) - 2 * margin),
               summaryHeight,
               SWP_NOACTIVATE);
  SetWindowPos(taxButton_, HWND_TOP, margin, margin + summaryHeight + gap, taxWidth, buttonHeight, SWP_NOACTIVATE);
  SetWindowPos(resourcesButton_, HWND_TOP, margin + taxWidth + gap, margin + summaryHeight + gap, resourcesWidth, buttonHeight, SWP_NOACTIVATE);
  SetWindowPos(list_, HWND_TOP,
               margin,
               margin + summaryHeight + gap + toolbarHeight,
               (std::max)(0, static_cast<int>(panelRect.right - panelRect.left) - 2 * margin),
               (std::max)(0, static_cast<int>(panelRect.bottom - panelRect.top) - summaryHeight - gap - toolbarHeight - 2 * margin),
               SWP_NOACTIVATE);
}

void TradeTaxTabContent::setVisible(bool visible)
{
  visible_ = visible;
  const int showCommand = visible ? SW_SHOW : SW_HIDE;
  if (panel_)
  {
    ShowWindow(panel_, showCommand);
  }
  if (taxButton_)
  {
    ShowWindow(taxButton_, showCommand);
  }
  if (summaryLabel_)
  {
    ShowWindow(summaryLabel_, showCommand);
  }
  if (resourcesButton_)
  {
    ShowWindow(resourcesButton_, showCommand);
  }
  if (list_)
  {
    ShowWindow(list_, showCommand);
  }
}

void TradeTaxTabContent::refresh()
{
  updateSummaryLabel();
  updateListRows();
}

bool TradeTaxTabContent::handleCommand(int commandId, int notificationCode)
{
  if (notificationCode != BN_CLICKED)
  {
    return false;
  }

  if (commandId == kTaxButtonId)
  {
    setMode(Mode::Tax);
    return true;
  }

  if (commandId == kResourcesButtonId)
  {
    setMode(Mode::Resources);
    return true;
  }

  return false;
}

bool TradeTaxTabContent::handleNotify(const NMHDR* hdr)
{
  notifyResult_ = 0;
  if (!hdr || hdr->hwndFrom != list_)
  {
    return false;
  }

  if (hdr->code == NM_DBLCLK)
  {
    const auto* itemActivate = reinterpret_cast<const NMITEMACTIVATE*>(hdr);
    if (itemActivate->iItem >= 0 && mapNavigationCallback_)
    {
      wchar_t coordinateText[64] = {};
      ListView_GetItemText(list_, itemActivate->iItem, 0, coordinateText,
                           static_cast<int>(sizeof(coordinateText) / sizeof(coordinateText[0])));
      int x = 0;
      int y = 0;
      int z = 1;
      if (CoordinateFormattingUtils::tryParseCoordinates(coordinateText, x, y, z))
      {
        mapNavigationCallback_(x, y, z);
      }
    }
    return true;
  }

  if (hdr->code != NM_CUSTOMDRAW)
  {
    return false;
  }

  auto* customDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(const_cast<NMHDR*>(hdr));
  if (customDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
  {
    notifyResult_ = CDRF_NOTIFYITEMDRAW;
    return true;
  }

  if (customDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
  {
    const std::size_t row = static_cast<std::size_t>(customDraw->nmcd.dwItemSpec);
    if (row < rowSeverities_.size())
    {
      customDraw->clrTextBk = colorForSeverity(rowSeverities_[row]);
    }
    notifyResult_ = CDRF_DODEFAULT;
    return true;
  }

  return false;
}

bool TradeTaxTabContent::handleDrawItem(const DRAWITEMSTRUCT* drawItem)
{
  if (!drawItem || (drawItem->CtlID != kTaxButtonId && drawItem->CtlID != kResourcesButtonId))
  {
    return false;
  }

  return drawModeButton(*drawItem);
}

LRESULT TradeTaxTabContent::getNotifyResult() const
{
  return notifyResult_;
}

void TradeTaxTabContent::setMapNavigationCallback(std::function<void(int, int, int)> callback)
{
  mapNavigationCallback_ = std::move(callback);
}

void TradeTaxTabContent::updateButtonStates()
{
  if (taxButton_)
  {
    InvalidateRect(taxButton_, nullptr, TRUE);
  }
  if (resourcesButton_)
  {
    InvalidateRect(resourcesButton_, nullptr, TRUE);
  }
}

void TradeTaxTabContent::updateSummaryLabel()
{
  if (!summaryLabel_ || !appData_)
  {
    return;
  }

  const std::size_t taxRegionCount = TradeTaxUtils::buildTaxRows(*appData_).size();
  const std::vector<TradeTaxUtils::ResourceRow> resourceRows = TradeTaxUtils::buildResourceRows(*appData_);
  std::set<std::wstring> tradeRegions;
  for (const TradeTaxUtils::ResourceRow& row : resourceRows)
  {
    tradeRegions.insert(row.coordinates);
  }
  const std::wstring label = L"Tax Regions: " + std::to_wstring(taxRegionCount)
    + L", Trade Regions: " + std::to_wstring(tradeRegions.size());
  SetWindowTextW(summaryLabel_, label.c_str());
}

void TradeTaxTabContent::updateListColumns()
{
  if (!list_)
  {
    return;
  }

  while (Header_GetItemCount(ListView_GetHeader(list_)) > 0)
  {
    ListView_DeleteColumn(list_, 0);
  }

  const UiSizeProfile::Metrics metrics = resolveUiMetrics(list_);
  struct Column
  {
    const wchar_t* title;
    int width;
  };

  const std::array<Column, 4> columns = mode_ == Mode::Tax
    ? std::array<Column, 4>{ {
        { L"coordinates", WinSizingUtils::scalePx(130, metrics) },
        { L"max tax", WinSizingUtils::scalePx(110, metrics) },
        { L"untaxed", WinSizingUtils::scalePx(110, metrics) },
        { L"potential taxers", WinSizingUtils::scalePx(150, metrics) }
      } }
    : std::array<Column, 4>{ {
        { L"coordinates", WinSizingUtils::scalePx(130, metrics) },
        { L"resource", WinSizingUtils::scalePx(110, metrics) },
        { L"max amount", WinSizingUtils::scalePx(130, metrics) },
        { L"after com", WinSizingUtils::scalePx(130, metrics) }
      } };

  for (int index = 0; index < static_cast<int>(columns.size()); ++index)
  {
    LVCOLUMNW column {};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(columns[static_cast<std::size_t>(index)].title);
    column.cx = columns[static_cast<std::size_t>(index)].width;
    column.iSubItem = index;
    ListView_InsertColumn(list_, index, &column);
  }
}

void TradeTaxTabContent::updateListRows()
{
  if (!list_)
  {
    return;
  }

  SendMessageW(list_, LVM_DELETEALLITEMS, 0, 0);
  rowSeverities_.clear();

  if (!appData_)
  {
    return;
  }

  if (mode_ == Mode::Tax)
  {
    populateTaxRows();
  }
  else
  {
    populateResourceRows();
  }
}

void TradeTaxTabContent::populateTaxRows()
{
  const std::vector<TradeTaxUtils::TaxRow> rows = TradeTaxUtils::buildTaxRows(*appData_);
  for (std::size_t index = 0; index < rows.size(); ++index)
  {
    const TradeTaxUtils::TaxRow& row = rows[index];
    const int itemIndex = static_cast<int>(index);

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(row.coordinates.c_str());
    ListView_InsertItem(list_, &item);

    const std::wstring maxTax = std::to_wstring(row.maxTax);
    const std::wstring untaxed = std::to_wstring(row.untaxed);
    const std::wstring potentialTaxers = std::to_wstring(row.potentialTaxers);
    ListView_SetItemText(list_, itemIndex, 1, const_cast<LPWSTR>(maxTax.c_str()));
    ListView_SetItemText(list_, itemIndex, 2, const_cast<LPWSTR>(untaxed.c_str()));
    ListView_SetItemText(list_, itemIndex, 3, const_cast<LPWSTR>(potentialTaxers.c_str()));
    rowSeverities_.push_back(row.severity);
  }
}

void TradeTaxTabContent::populateResourceRows()
{
  const std::vector<TradeTaxUtils::ResourceRow> rows = TradeTaxUtils::buildResourceRows(*appData_);
  for (std::size_t index = 0; index < rows.size(); ++index)
  {
    const TradeTaxUtils::ResourceRow& row = rows[index];
    const int itemIndex = static_cast<int>(index);

    LVITEMW item {};
    item.mask = LVIF_TEXT;
    item.iItem = itemIndex;
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(row.coordinates.c_str());
    ListView_InsertItem(list_, &item);

    const std::wstring maxAmount = std::to_wstring(row.maxAmount);
    const std::wstring afterCommandsAmount = std::to_wstring(row.afterCommandsAmount);
    ListView_SetItemText(list_, itemIndex, 1, const_cast<LPWSTR>(row.resourceToken.c_str()));
    ListView_SetItemText(list_, itemIndex, 2, const_cast<LPWSTR>(maxAmount.c_str()));
    ListView_SetItemText(list_, itemIndex, 3, const_cast<LPWSTR>(afterCommandsAmount.c_str()));
    rowSeverities_.push_back(row.severity);
  }
}

void TradeTaxTabContent::setMode(Mode mode)
{
  if (mode_ == mode)
  {
    return;
  }

  mode_ = mode;
  updateButtonStates();
  updateListColumns();
  updateListRows();
}

bool TradeTaxTabContent::drawModeButton(const DRAWITEMSTRUCT& drawItem) const
{
  const bool selected = (drawItem.CtlID == kTaxButtonId && mode_ == Mode::Tax)
    || (drawItem.CtlID == kResourcesButtonId && mode_ == Mode::Resources);

  const COLORREF backgroundColor = selected ? kSelectedButtonBackgroundColor : GetSysColor(COLOR_BTNFACE);
  const COLORREF textColor = selected ? kSelectedButtonTextColor : GetSysColor(COLOR_BTNTEXT);

  HBRUSH backgroundBrush = CreateSolidBrush(backgroundColor);
  FillRect(drawItem.hDC, &drawItem.rcItem, backgroundBrush);
  DeleteObject(backgroundBrush);

  DrawEdge(drawItem.hDC,
           const_cast<RECT*>(&drawItem.rcItem),
           (drawItem.itemState & ODS_SELECTED) ? BDR_SUNKENOUTER : BDR_RAISEDOUTER,
           BF_RECT);

  wchar_t text[64] = {};
  GetWindowTextW(drawItem.hwndItem, text, static_cast<int>(sizeof(text) / sizeof(text[0])));

  SetBkMode(drawItem.hDC, TRANSPARENT);
  const COLORREF oldTextColor = SetTextColor(drawItem.hDC, textColor);
  RECT textRect = drawItem.rcItem;
  const int buttonHeight = static_cast<int>(textRect.bottom - textRect.top);
  const int radioSize = (std::min)(buttonHeight - 8, 14);
  const int radioLeft = textRect.left + 8;
  const int radioTop = textRect.top + (buttonHeight - radioSize) / 2;
  HBRUSH radioBrush = CreateSolidBrush(selected ? textColor : GetSysColor(COLOR_WINDOW));
  HGDIOBJ oldBrush = SelectObject(drawItem.hDC, radioBrush);
  HPEN radioPen = CreatePen(PS_SOLID, 1, selected ? textColor : GetSysColor(COLOR_WINDOWTEXT));
  HGDIOBJ oldPen = SelectObject(drawItem.hDC, radioPen);
  Ellipse(drawItem.hDC, radioLeft, radioTop, radioLeft + radioSize, radioTop + radioSize);
  SelectObject(drawItem.hDC, oldPen);
  SelectObject(drawItem.hDC, oldBrush);
  DeleteObject(radioPen);
  DeleteObject(radioBrush);

  textRect.left += radioSize + 16;
  DrawTextW(drawItem.hDC, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  SetTextColor(drawItem.hDC, oldTextColor);

  if (drawItem.itemState & ODS_FOCUS)
  {
    RECT focusRect = drawItem.rcItem;
    InflateRect(&focusRect, -3, -3);
    DrawFocusRect(drawItem.hDC, &focusRect);
  }

  return true;
}

LRESULT CALLBACK TradeTaxTabContent::panelSubclassProc(HWND hwnd,
                                                       UINT msg,
                                                       WPARAM wp,
                                                       LPARAM lp,
                                                       UINT_PTR /*subclassId*/,
                                                       DWORD_PTR refData)
{
  auto* tabContent = reinterpret_cast<TradeTaxTabContent*>(refData);
  if (!tabContent)
  {
    return DefSubclassProc(hwnd, msg, wp, lp);
  }

  switch (msg)
  {
    case WM_COMMAND:
      if (tabContent->handleCommand(LOWORD(wp), static_cast<int>(HIWORD(wp))))
      {
        return 0;
      }
      break;

    case WM_DRAWITEM:
    {
      const auto* drawItem = reinterpret_cast<const DRAWITEMSTRUCT*>(lp);
      if (tabContent->handleDrawItem(drawItem))
      {
        return TRUE;
      }
      break;
    }

    case WM_NOTIFY:
    {
      const auto* hdr = reinterpret_cast<const NMHDR*>(lp);
      if (tabContent->handleNotify(hdr))
      {
        return tabContent->getNotifyResult();
      }
      break;
    }

    case WM_NCDESTROY:
      RemoveWindowSubclass(hwnd, &TradeTaxTabContent::panelSubclassProc, kPanelSubclassId);
      break;
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
}
