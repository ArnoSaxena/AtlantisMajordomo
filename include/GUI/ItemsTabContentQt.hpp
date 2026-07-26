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
 * File: ItemsTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include <string>

class AppData;
class Item;
class QCheckBox;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

/**
 * @brief Owns and manages the body content for the "Items" tab (Qt build).
 *
 * Mirrors ItemsTabContent for the Win32 build. Shows a sorted list of items
 * on the left (man-type items first — LEAD at top, then others alphabetically
 * — then non-man items below a visual separator) and a large editable detail
 * form on the right (name, weight, category checkboxes, movement/capacity
 * fields, ship fields, skill-max/resources/production maps, full text).
 * A QSplitter divides the list from the form, which lives inside a
 * QScrollArea to remain usable at small window heights.
 */
class ItemsTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit ItemsTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~ItemsTabContentQt() override = default;

    ItemsTabContentQt(const ItemsTabContentQt&) = delete;
    ItemsTabContentQt& operator=(const ItemsTabContentQt&) = delete;
    ItemsTabContentQt(ItemsTabContentQt&&) = delete;
    ItemsTabContentQt& operator=(ItemsTabContentQt&&) = delete;

    /** Repopulates the list and refreshes the detail form. */
    void refresh();

    /** Selects the item identified by itemToken in the list and loads details. */
    void focusItemByToken(const std::wstring& itemToken);

private slots:
    void onItemSelectionChanged();
    void onSaveClicked();

private:
    void updateItemsList();
    void updateSelectedItemFromList();
    void loadItemToFields(const Item* item);
    void clearFields();
    void saveSelectedItem();

    AppData*        appData_                { nullptr };

    // Left pane
    QListWidget*    itemsList_              { nullptr };

    // Right pane — form fields
    QLineEdit*      tokenEdit_              { nullptr };   // read-only
    QLineEdit*      nameEdit_               { nullptr };
    QLineEdit*      weightEdit_             { nullptr };
    QCheckBox*      meeleWeaponCheck_       { nullptr };
    QCheckBox*      rangedWeaponCheck_      { nullptr };
    QCheckBox*      armourCheck_            { nullptr };
    QCheckBox*      resourceCheck_          { nullptr };
    QCheckBox*      mountCheck_             { nullptr };
    QCheckBox*      manCheck_               { nullptr };
    QLineEdit*      movesEdit_              { nullptr };
    QLineEdit*      walkCapacityEdit_       { nullptr };
    QLineEdit*      rideCapacityEdit_       { nullptr };
    QLineEdit*      swimCapacityEdit_       { nullptr };
    QLineEdit*      flyCapacityEdit_        { nullptr };
    QLineEdit*      shipSpeedEdit_          { nullptr };
    QLineEdit*      shipSailingSkillEdit_   { nullptr };
    QLineEdit*      magesStudyEdit_         { nullptr };
    QLineEdit*      defaultSkillMaxEdit_    { nullptr };
    QPlainTextEdit* skillsMaxEdit_          { nullptr };
    QPlainTextEdit* resourcesEdit_          { nullptr };
    QPlainTextEdit* productionSkillEdit_    { nullptr };
    QPlainTextEdit* productionHelpEdit_     { nullptr };
    QPlainTextEdit* fullTextEdit_           { nullptr };   // read-only
    QPushButton*    saveButton_             { nullptr };

    std::wstring    selectedItemToken_;
};
