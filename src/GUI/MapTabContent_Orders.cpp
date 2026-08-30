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
 * File: MapTabContent_Orders.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GUI/MapTabContent.hpp"
#include "GUI/MapTabContent_private.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/Faction.hpp"
#include "GUI/WinGuiUtils.hpp"
#include "Data/Item.hpp"
#include "Data/Region.hpp"
#include "Data/RegionRepository.hpp"
#include "Data/Skill.hpp"
#include "Data/Structure.hpp"
#include "GUI/ControlIds.hpp"
#include "GUI/OrdersEditorUtils.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/CommandSimulationService.hpp"
#include "Function/CoordinateUtils.hpp"
#include "Function/MonthUtils.hpp"
#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/OrderWarningService.hpp"
#include "Function/SkillFormattingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <commctrl.h>
#include <cwctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <windowsx.h>
#include <fstream>
#include <shellapi.h>

LRESULT CALLBACK MapTabContent::ordersEditorSubclassProc(HWND hwnd,
                                                          UINT msg,
                                                          WPARAM wp,
                                                          LPARAM lp,
                                                          UINT_PTR /*subclassId*/,
                                                          DWORD_PTR refData)
{
  auto* self = reinterpret_cast<MapTabContent*>(refData);
  if (!self)
  {
    return DefSubclassProc(hwnd, msg, wp, lp);
  }

  if (msg == WM_CONTEXTMENU)
  {
    const UINT selected = OrdersEditorUtils::showOrdersEditorMenu(hwnd, lp);
    switch (selected)
    {
      case OrdersEditorUtils::kOrdersUndoCmd:
        SendMessageW(hwnd, EM_UNDO, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersCutCmd:
        SendMessageW(hwnd, WM_CUT, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersCopyCmd:
        SendMessageW(hwnd, WM_COPY, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersPasteCmd:
        SendMessageW(hwnd, WM_PASTE, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersDeleteCmd:
        SendMessageW(hwnd, WM_CLEAR, 0, 0);
        break;
      case OrdersEditorUtils::kOrdersSelectAllCmd:
      {
        const int len = GetWindowTextLengthW(hwnd);
        SendMessageW(hwnd, EM_SETSEL, 0, len);
        break;
      }
      case OrdersEditorUtils::kOrdersFormNewUnitCmd:
      {
        int x = self->selectedRegionX_;
        int y = self->selectedRegionY_;
        int z = self->selectedZ_;
        if (x == 0 && y == 0)
        {
          const Unit* unit = self->appData_ ? self->appData_->unitRepository().findByNumber(self->selectedUnitNumber_) : nullptr;
          if (unit)
          {
            x = unit->getXCoordinate();
            y = unit->getYCoordinate();
            z = unit->getZCoordinate();
          }
        }
        const int newNumber = OrderBusinessLogic::computeNextNewUnitNumber(self->appData_, x, y, z);
        OrdersEditorUtils::insertFormBlockAtEnd(hwnd, newNumber);
      }
      break;
      case OrdersEditorUtils::kOrdersGiveCmd:
      {
        // Open a small modal dialog to insert a computed GIVE order
        self->showGiveToUnitDialog(hwnd);
        break;
      }
      default:
        break;
    }

    return 0; // suppress default menu
  }

  if (msg == WM_LBUTTONDOWN && self->selectedUnitIsNew_ && hwnd == self->ordersEditor_)
  {
    if (self->focusOriginUnitForSelectedUnitNew())
    {
      return 0;
    }
  }

  if (msg == WM_SETFOCUS && self->selectedUnitIsNew_ && hwnd == self->ordersEditor_)
  {
    const LRESULT nextLineIndex = SendMessageW(hwnd, EM_LINEINDEX, 1, 0);
    if (nextLineIndex >= 0)
    {
      SendMessageW(hwnd, EM_SETSEL, static_cast<WPARAM>(nextLineIndex), static_cast<LPARAM>(nextLineIndex));
    }
  }

  if (msg == WM_KILLFOCUS && hwnd == self->ordersEditor_)
  {
    self->saveOrdersToSelectedUnit();
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
}

// Small modal dialog for inserting GIVE orders from the selected unit
void MapTabContent::showGiveToUnitDialog(HWND parent)
{
  if (!appData_ || selectedUnitNumber_ == 0)
  {
    MessageBoxW(parent, L"No unit selected.", L"Give", MB_OK | MB_ICONERROR);
    return;
  }

  const Unit* originUnit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
  if (!originUnit)
  {
    MessageBoxW(parent, L"Origin unit not found.", L"Give", MB_OK | MB_ICONERROR);
    return;
  }

  // Register dialog window class
  const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE));
  static bool registered = false;
  const wchar_t* kClassName = L"WindowsAppGiveDialog";
  if (!registered)
  {
    WNDCLASSW wc{};
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT
    {
      if (msg == WM_CLOSE)
      {
        DestroyWindow(hwnd);
        return 0;
      }
      if (msg == WM_COMMAND)
      {
        const int id = LOWORD(wp);
        if (id == 1001 || id == 1002)
        {
          // Close button handlers: call back into MapTabContent when Give pressed
          MapTabContent* parentObj = reinterpret_cast<MapTabContent*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
          if (id == 1001 && parentObj)
          {
            parentObj->handleGiveDialogAccept(hwnd);
          }

          DestroyWindow(hwnd);
          return 0;
        }
      }
      return DefWindowProcW(hwnd, msg, wp, lp);
    };
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
    registered = true;
  }

  // Desired client area for dialog; compute real window size including non-client
  const int desiredClientWidth = 420;
  const int desiredClientHeight = 160;
  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  RECT wr{0,0, desiredClientWidth, desiredClientHeight};
  AdjustWindowRectEx(&wr, style, FALSE, 0);
  const int windowWidth = wr.right - wr.left;
  const int windowHeight = wr.bottom - wr.top;
  RECT parentRect; GetWindowRect(parent, &parentRect);
  const int x = parentRect.left + ((parentRect.right - parentRect.left) - windowWidth) / 2;
  const int y = parentRect.top + ((parentRect.bottom - parentRect.top) - windowHeight) / 2;

  HWND dlg = CreateWindowExW(0, kClassName, L"Give to unit", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                             x, y, windowWidth, windowHeight, parent, nullptr, instance, nullptr);
  if (!dlg)
  {
    return;
  }

  // store parent HWND so the dialog proc can notify the parent
  // store 'this' pointer so dialog proc can call back
  SetWindowLongPtr(dlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  // Controls: static label, edit for unit number, combo box, Give and Cancel buttons
  HWND label = CreateWindowExW(0, L"STATIC", L"Give to unit:", WS_CHILD | WS_VISIBLE,
                                12, 12, 100, 20, dlg, nullptr, instance, nullptr);
  HWND edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
                              120, 10, 80, 24, dlg, nullptr, instance, nullptr);

  HWND combo = CreateWindowExW(0, WC_COMBOBOXW, nullptr, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                               210, 10, 190, 200, dlg, nullptr, instance, nullptr);

  // Populate combo with item tokens from origin unit, preferring SILV
  std::vector<std::wstring> tokens;
  for (const auto& [token, amount] : originUnit->getItems())
  {
    if (amount <= 0) continue;
    tokens.push_back(token);
  }
  // Ensure SILV on top if present
  auto it = std::find_if(tokens.begin(), tokens.end(), [](const std::wstring& t){ return _wcsicmp(t.c_str(), L"SILV") == 0; });
  if (it != tokens.end())
  {
    std::wstring silv = *it;
    tokens.erase(it);
    tokens.insert(tokens.begin(), silv);
  }
  for (const auto& tok : tokens)
  {
    SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(tok.c_str()));
  }
  if (!tokens.empty()) SendMessageW(combo, CB_SETCURSEL, 0, 0);

  // Place buttons relative to desired client area
  HWND giveBtn = CreateWindowExW(0, L"BUTTON", L"Give", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                  desiredClientWidth - 200, desiredClientHeight - 44, 80, 28, dlg, reinterpret_cast<HMENU>(1001), instance, nullptr);
  HWND cancelBtn = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    desiredClientWidth - 100, desiredClientHeight - 44, 80, 28, dlg, reinterpret_cast<HMENU>(1002), instance, nullptr);

  // silence unused variable warnings (controls are referenced by OS via HWND)
  (void)label; (void)edit; (void)giveBtn; (void)cancelBtn;

  ShowWindow(dlg, SW_SHOW);
  UpdateWindow(dlg);

  // Modal message loop
  MSG msg;
  while (IsWindow(dlg) && GetMessageW(&msg, nullptr, 0, 0))
  {
    if (!IsDialogMessage(dlg, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    // dialog now calls back into handleGiveDialogAccept; no posted messages expected
  }
}

void MapTabContent::handleGiveDialogAccept(HWND dlgHwnd)
{
  if (!dlgHwnd)
  {
    return;
  }

  // Read fields from dialog controls and perform same logic previously in modal loop
  wchar_t buf[256] = {};
  HWND edit = FindWindowExW(dlgHwnd, nullptr, L"EDIT", nullptr);
  if (edit)
  {
    GetWindowTextW(edit, buf, (int)std::size(buf));
  }

  const std::wstring editText = StringUtils::trimWhitespace(buf);
  if (editText.empty())
  {
    MessageBoxW(dlgHwnd, L"Invalid receiving unit reference.", L"Give", MB_OK | MB_ICONERROR);
    return;
  }

  bool isNewRef = false;
  int targetUnit = 0;
  const std::wstring upperText = StringUtils::toUpper(editText);
  if (upperText.rfind(L"NEW ", 0) == 0)
  {
    const std::wstring rest = StringUtils::trimWhitespace(editText.substr(4));
    try
    {
      targetUnit = std::stoi(rest);
      if (targetUnit <= 0) throw 0;
      isNewRef = true;
    }
    catch(...) {
      MessageBoxW(dlgHwnd, L"Invalid NEW unit index.", L"Give", MB_OK | MB_ICONERROR);
      return;
    }
  }
  else
  {
    try
    {
      targetUnit = std::stoi(editText);
      if (targetUnit <= 0) throw 0;
    }
    catch(...) {
      MessageBoxW(dlgHwnd, L"Invalid receiving unit number.", L"Give", MB_OK | MB_ICONERROR);
      return;
    }
  }

  HWND combo = FindWindowExW(dlgHwnd, nullptr, WC_COMBOBOXW, nullptr);
  int sel = -1;
  if (combo)
  {
    sel = (int)SendMessageW(combo, CB_GETCURSEL, 0, 0);
  }

  wchar_t itemBuf[256] = {};
  if (combo && sel >= 0)
  {
    SendMessageW(combo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(itemBuf));
  }

  std::wstring itemToken = itemBuf;
  if (itemToken.empty())
  {
    MessageBoxW(dlgHwnd, L"No item selected.", L"Give", MB_OK | MB_ICONERROR);
    return;
  }
  if (targetUnit <= 0)
  {
    MessageBoxW(dlgHwnd, L"Invalid receiving unit number.", L"Give", MB_OK | MB_ICONERROR);
    return;
  }

  const std::wstring giveLine = OrderBusinessLogic::buildGiveCommand(
    *appData_, selectedUnitNumber_, targetUnit, isNewRef, itemToken);
  appendOrderLineToOrdersEditor(giveLine);
  return;
}


void MapTabContent::saveOrdersToSelectedUnit()
{
  if (!appData_ || selectedUnitNumber_ == 0 || !ordersEditor_)
  {
    return;
  }

  Unit* unit = appData_->unitRepository().findByNumber(selectedUnitNumber_);
  if (!unit || !canEditOrdersForUnit(unit))
  {
    return;
  }

  const std::wstring text = WinGuiUtils::getWindowText(ordersEditor_);

  const std::vector<std::wstring> orders = StringUtils::splitLines(text);
  unit->setOrders(orders);

  // Keep stale OrderRepository cleanup based on current UnitNew state, then recalc
  // after UnitNew entries are rebuilt from the just-saved FORM/END blocks.
  OrderBusinessLogic::syncOrderRepositoryForSavedUnit(*appData_, selectedUnitNumber_, false);

  // Handle FORM/END blocks: extract new unit numbers and remove previous UnitNew entries
  // originating from this unit, then create new UnitNew snapshot entries.
  appData_->unitNewRepository().removeByOriginUnit(selectedUnitNumber_);

  const std::vector<int> formUnitNumbers =
    OrderParsingUtils::extractFormNewUnitNumbers(orders);

  for (int formUnitNumber : formUnitNumbers)
  {
    // Create a UnitNew snapshot for each newly formed unit.
    // The snapshot will be orderless and marked with the origin unit number.
    const int x = unit->getXCoordinate();
    const int y = unit->getYCoordinate();
    const int z = unit->getZCoordinate();
    const std::wstring formUnitName = L"New Unit";

    appData_->unitNewRepository().add(
      formUnitNumber,
      formUnitName,
      unit->getStructureId(),  // structureId - inherit from origin unit
      x, y, z,
      unit->getFlags(),  // flags
      std::map<std::wstring, int>(),  // itemCounts
      0,  // weight
      0,  // capacityWalk
      0,  // capacityRide
      0,  // capacityFly
      0,  // capacitySwim
      std::map<std::wstring, int>(),  // skills
      unit->getMonth(),  // month
      unit->getYear(),  // year
      selectedUnitNumber_  // originUnit
    );
  }

  CommandSimulationService::recalculateAfterOrdersValues(*appData_);
  OrderWarningService::runForMainFaction(*appData_);

  updateSelectedUnitDetailsByNumber(selectedUnitNumber_);
  populateUnitsForSelectedRegion();
  updateWarningsSummaryLabel();
}


void MapTabContent::appendOrderLineToOrdersEditor(const std::wstring& orderLine)
{
  if (!ordersEditor_)
  {
    return;
  }

  const std::wstring trimmedOrderLine = StringUtils::trimWhitespace(orderLine);
  if (trimmedOrderLine.empty())
  {
    return;
  }

  std::wstring ordersText = WinGuiUtils::getWindowText(ordersEditor_);

  if (!ordersText.empty())
  {
    const wchar_t lastChar = ordersText.back();
    if (lastChar != L'\n' && lastChar != L'\r')
    {
      ordersText += L"\r\n";
    }
    else if (lastChar == L'\r')
    {
      ordersText += L"\n";
    }
  }

  ordersText += trimmedOrderLine;
  if (ordersText.empty() || ordersText.back() != L'\n')
  {
    ordersText += L"\r\n";
  }
  SetWindowTextW(ordersEditor_, ordersText.c_str());
  SendMessageW(ordersEditor_, EM_SETSEL, static_cast<WPARAM>(ordersText.size()), static_cast<LPARAM>(ordersText.size()));
  SetFocus(ordersEditor_);
}


bool MapTabContent::canEditOrdersForUnit(const Unit* unit) const
{
  if (!appData_ || !unit)
  {
    return false;
  }

  const auto& factionRepository = appData_->factionRepository();
  int mainFactionCount = 0;
  int mainFactionNumber = 0;
  for (std::size_t index = 0; index < factionRepository.size(); ++index)
  {
    const Faction& faction = factionRepository.at(index);
    if (faction.isMainFaction())
    {
      ++mainFactionCount;
      mainFactionNumber = faction.getFactionNumber();
    }
  }

  return mainFactionCount == 1 && unit->getFactionNumber() == mainFactionNumber;
}


void MapTabContent::setOrdersEditingEnabled(bool enabled)
{
  if (ordersEditor_)
  {
    // Keep the editor enabled so it can receive focus for unitNew orders,
    // but make it read-only when orders cannot be edited.
    EnableWindow(ordersEditor_, TRUE);
    SendMessageW(ordersEditor_, EM_SETREADONLY, enabled ? FALSE : TRUE, 0);
  }
  if (saveOrdersButton_)
  {
    EnableWindow(saveOrdersButton_, enabled ? TRUE : FALSE);
  }
}

