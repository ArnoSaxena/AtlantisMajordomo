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
 * File: SkillsTabContentQt.hpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <QWidget>

#include <string>

class AppData;
class Skill;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

/**
 * @brief Owns and manages the body content for the "Skills" tab (Qt build).
 *
 * Mirrors SkillsTabContent for the Win32 build.  Shows a sorted list of skills
 * on the left (non-magic first, magic below a visual separator) and an
 * editable detail form on the right (skill ID read-only, level dropdown, name,
 * study cost, production/magic checkboxes, prerequisites, production items,
 * and a read-only description). A QSplitter divides the list from the form.
 * The right pane lives inside a QScrollArea so it remains usable at small
 * window heights.
 */
class SkillsTabContentQt : public QWidget
{
    Q_OBJECT

public:
    explicit SkillsTabContentQt(AppData& appData, QWidget* parent = nullptr);
    ~SkillsTabContentQt() override = default;

    SkillsTabContentQt(const SkillsTabContentQt&) = delete;
    SkillsTabContentQt& operator=(const SkillsTabContentQt&) = delete;
    SkillsTabContentQt(SkillsTabContentQt&&) = delete;
    SkillsTabContentQt& operator=(SkillsTabContentQt&&) = delete;

    /** Repopulates the list and refreshes the detail form. */
    void refresh();

    /**
     * @brief Selects the skill identified by @p skillToken in the list and
     *        loads its detail form. Does nothing when the token is empty or
     *        not found.
     */
    void focusSkillByToken(const std::wstring& skillToken);

private slots:
    void onSkillSelectionChanged();
    void onLevelComboChanged(int index);
    void onSaveClicked();

private:
    void updateSkillsList();
    void updateSelectedSkillFromList();
    void populateLevelCombo(const Skill* skill);
    void loadSkillLevelToFields(const Skill* skill, int level);
    void clearFields();
    void saveSelectedSkill();

    AppData*        appData_              { nullptr };

    // Left pane
    QListWidget*    skillsList_           { nullptr };

    // Right pane — form fields
    QLineEdit*      tokenEdit_            { nullptr };   // read-only
    QComboBox*      levelCombo_           { nullptr };
    QLineEdit*      nameEdit_             { nullptr };
    QLineEdit*      studyCostEdit_        { nullptr };
    QCheckBox*      productionCheck_      { nullptr };
    QCheckBox*      magicCheck_           { nullptr };
    QCheckBox*      magicFoundationCheck_ { nullptr };
    QPlainTextEdit* prerequisitesEdit_    { nullptr };
    QPlainTextEdit* productionItemsEdit_  { nullptr };
    QPlainTextEdit* descriptionEdit_      { nullptr };   // read-only
    QPushButton*    saveButton_           { nullptr };

    std::wstring    selectedSkillToken_;
    int             displayedLevel_       { 0 };
};
