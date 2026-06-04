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
 * File: TabView.cpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GUI/TabView.hpp"

#include <stdexcept>
#include <windowsx.h>

static constexpr wchar_t kPanelClassName[] = L"TabPanelClass";
static constexpr DWORD   kPanelStyle       = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

static LRESULT CALLBACK panelWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
      HWND parent = GetParent(hwnd);
      if (parent)
      {
        POINT point {
          GET_X_LPARAM(lp),
          GET_Y_LPARAM(lp)
        };
        MapWindowPoints(hwnd, parent, &point, 1);
        SendMessageW(parent, msg, wp, MAKELPARAM(point.x, point.y));
      }
      break;
    }
  }

  if (msg == WM_ERASEBKGND)
  {
    RECT rc {};
    GetClientRect(hwnd, &rc);
    FillRect(reinterpret_cast<HDC>(wp),
            &rc,
            reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
    return 1;
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

static bool registerPanelClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wc {};
  if (GetClassInfoExW(hInstance, kPanelClassName, &wc))
  {
    return true; // Already registered
  }

  wc.cbSize        = sizeof(wc);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = panelWndProc;
  wc.hInstance     = hInstance;
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
  wc.lpszClassName = kPanelClassName;
  return RegisterClassExW(&wc) != 0;
}

bool TabView::create(HWND parent, int id)
{
  parent_ = parent;

  HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
  if (!registerPanelClass(hInstance))
  {
    return false;
  }

  INITCOMMONCONTROLSEX icc {};
  icc.dwSize = sizeof(icc);
  icc.dwICC  = ICC_TAB_CLASSES;
  InitCommonControlsEx(&icc);

  tabControl_ = CreateWindowExW(
    0,
    WC_TABCONTROLW,
    nullptr,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_HOTTRACK,
    0, 0, 0, 0,
    parent,
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
    reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent, GWLP_HINSTANCE)),
    nullptr
  );

  return tabControl_ != nullptr;
}

HWND TabView::addTab(const std::wstring& label)
{
  const int index = static_cast<int>(tabs_.size());

  TCITEMW item {};
  item.mask    = TCIF_TEXT;
  item.pszText = const_cast<LPWSTR>(label.c_str());
  TabCtrl_InsertItem(tabControl_, index, &item);

  HWND panel = CreateWindowExW(
    0,
    kPanelClassName,
    nullptr,
    kPanelStyle | (index == 0 ? WS_VISIBLE : 0),
    0, 0, 0, 0,
    parent_,
    nullptr,
    reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent_, GWLP_HINSTANCE)),
    nullptr
  );

  tabs_.push_back({ label, panel });
  return panel;
}

HWND TabView::getPanel(int index) const
{
  if (index < 0 || index >= static_cast<int>(tabs_.size()))
  {
    return nullptr;
  }
  return tabs_[static_cast<std::size_t>(index)].panel;
}

void TabView::resize(const RECT& rc)
{
  SetWindowPos(tabControl_, nullptr, rc.left, rc.top,
    rc.right - rc.left, rc.bottom - rc.top,
    SWP_NOZORDER | SWP_NOACTIVATE);

  // Compute the display area inside the tab control.
  RECT displayRc = rc;
  TabCtrl_AdjustRect(tabControl_, FALSE, &displayRc);

  for (auto& tab : tabs_)
  {
    if (tab.panel)
    {
      SetWindowPos(tab.panel, nullptr,
        displayRc.left, displayRc.top,
        displayRc.right  - displayRc.left,
        displayRc.bottom - displayRc.top,
        SWP_NOZORDER | SWP_NOACTIVATE);
    }
  }
}

void TabView::onSelectionChange()
{
  const int selected = TabCtrl_GetCurSel(tabControl_);
  showTab(selected);
}

HWND TabView::getTabControl() const
{
  return tabControl_;
}

void TabView::showTab(int index)
{
  for (int i = 0; i < static_cast<int>(tabs_.size()); ++i)
  {
    if (tabs_[static_cast<std::size_t>(i)].panel)
    {
      ShowWindow(
        tabs_[static_cast<std::size_t>(i)].panel,
        i == index ? SW_SHOW : SW_HIDE
      );
    }
  }
}
