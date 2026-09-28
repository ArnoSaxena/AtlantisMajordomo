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
 * File: TradeTaxTabContent.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include "Function/TradeTaxUtils.hpp"

#include <functional>
#include <vector>

class AppData;

class TradeTaxTabContent
{
public:
  TradeTaxTabContent() = default;
  ~TradeTaxTabContent() = default;

  TradeTaxTabContent(const TradeTaxTabContent&) = delete;
  TradeTaxTabContent& operator=(const TradeTaxTabContent&) = delete;
  TradeTaxTabContent(TradeTaxTabContent&&) = delete;
  TradeTaxTabContent& operator=(TradeTaxTabContent&&) = delete;

  bool create(HWND parentWindow, HINSTANCE instance, AppData& appData);
  void resize(const RECT& displayRect);
  void setVisible(bool visible);
  void refresh();
  bool handleCommand(int commandId, int notificationCode = 0);
  bool handleNotify(const NMHDR* hdr);
  bool handleDrawItem(const DRAWITEMSTRUCT* drawItem);
  LRESULT getNotifyResult() const;
  void setMapNavigationCallback(std::function<void(int, int, int)> callback);

private:
  enum class Mode
  {
    Tax,
    Resources,
  };

  AppData* appData_ { nullptr };
  HWND panel_ { nullptr };
  HWND summaryLabel_ { nullptr };
  HWND taxButton_ { nullptr };
  HWND resourcesButton_ { nullptr };
  HWND list_ { nullptr };
  Mode mode_ { Mode::Tax };
  bool visible_ { false };
  LRESULT notifyResult_ { 0 };
  std::function<void(int, int, int)> mapNavigationCallback_;
  std::vector<TradeTaxUtils::RowSeverity> rowSeverities_;

  void updateButtonStates();
  void updateSummaryLabel();
  void updateListColumns();
  void updateListRows();
  void populateTaxRows();
  void populateResourceRows();
  void setMode(Mode mode);
  bool drawModeButton(const DRAWITEMSTRUCT& drawItem) const;
  static LRESULT CALLBACK panelSubclassProc(HWND hwnd,
                                            UINT msg,
                                            WPARAM wp,
                                            LPARAM lp,
                                            UINT_PTR subclassId,
                                            DWORD_PTR refData);
};
