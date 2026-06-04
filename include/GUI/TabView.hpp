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
 * File: TabView.hpp
 */
 
﻿// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

/**
* @brief Manages a tab control and its associated content panels.
*
* Each tab owns a child HWND that acts as a content panel. Panels can be
* populated later by the caller via getPanel().
*/
class TabView
{
public:
  TabView() = default;
  ~TabView() = default;

  TabView(const TabView&) = delete;
  TabView& operator=(const TabView&) = delete;
  TabView(TabView&&) = delete;
  TabView& operator=(TabView&&) = delete;

  /**
  * @brief Creates the tab control and its panels inside a parent window.
  * @param[in] parent  Handle to the parent window.
  * @param[in] id      Child-window identifier for the tab control.
  * @return true on success.
  */
  bool create(HWND parent, int id);

  /**
  * @brief Adds a new tab.
  * @param[in] label  Text displayed on the tab.
  * @return Handle to the content panel for this tab.
  */
  HWND addTab(const std::wstring& label);

  /**
  * @brief Returns the content panel HWND for the given tab index.
  * @param[in] index  Zero-based tab index.
  * @return Panel HWND, or nullptr if the index is out of range.
  */
  HWND getPanel(int index) const;

  /**
  * @brief Resizes the tab control to fill the given rect.
  * @param[in] rc  New bounding rectangle in parent-client coordinates.
  */
  void resize(const RECT& rc);

  /**
  * @brief Must be called when the parent receives WM_NOTIFY with TCN_SELCHANGE.
  */
  void onSelectionChange();

  /**
  * @brief Returns the underlying tab control HWND.
  */
  HWND getTabControl() const;

private:
  HWND tabControl_ { nullptr };
  HWND parent_     { nullptr };

  struct Tab
  {
    std::wstring label;
    HWND         panel { nullptr };
  };

  std::vector<Tab> tabs_;

  void showTab(int index);
};
