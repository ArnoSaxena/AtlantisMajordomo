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
 * File: SkillsTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/SkillsTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Skill.hpp"
#include "Data/SkillRepository.hpp"
#include "Function/SkillFormattingUtils.hpp"
#include "Function/StringUtils.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
// UserRole data key used to store the skill token on each list item.
// Separator items have no UserRole data (empty QVariant).
constexpr int kTokenRole = Qt::UserRole;

// Minimum height in pixels for the multiline text fields.
constexpr int kMultilineMinHeight = 80;

} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SkillsTabContentQt::SkillsTabContentQt(AppData& appData, QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
{
    // ---- Left pane: skills list --------------------------------------------
    skillsList_ = new QListWidget(this);
    skillsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    skillsList_->setMinimumWidth(170);

    // ---- Right pane: scrollable form ---------------------------------------

    // Form fields
    tokenEdit_ = new QLineEdit(this);
    tokenEdit_->setReadOnly(true);

    levelCombo_ = new QComboBox(this);

    nameEdit_ = new QLineEdit(this);

    studyCostEdit_ = new QLineEdit(this);

    productionCheck_      = new QCheckBox("Production",       this);
    magicCheck_           = new QCheckBox("Magic",            this);
    magicFoundationCheck_ = new QCheckBox("Magic Foundation", this);

    QHBoxLayout* checkboxRow = new QHBoxLayout;
    checkboxRow->setContentsMargins(0, 0, 0, 0);
    checkboxRow->setSpacing(12);
    checkboxRow->addWidget(productionCheck_);
    checkboxRow->addWidget(magicCheck_);
    checkboxRow->addWidget(magicFoundationCheck_);
    checkboxRow->addStretch();

    prerequisitesEdit_ = new QPlainTextEdit(this);
    prerequisitesEdit_->setMinimumHeight(kMultilineMinHeight);

    productionItemsEdit_ = new QPlainTextEdit(this);
    productionItemsEdit_->setMinimumHeight(kMultilineMinHeight);

    descriptionEdit_ = new QPlainTextEdit(this);
    descriptionEdit_->setReadOnly(true);
    descriptionEdit_->setMinimumHeight(kMultilineMinHeight);

    saveButton_ = new QPushButton("Save", this);
    saveButton_->setFixedWidth(120);

    QHBoxLayout* saveRow = new QHBoxLayout;
    saveRow->setContentsMargins(0, 0, 0, 0);
    saveRow->addStretch();
    saveRow->addWidget(saveButton_);

    // Assemble the form layout
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(6);
    formLayout->addRow("Skill ID:",                              tokenEdit_);
    formLayout->addRow("Level:",                                 levelCombo_);
    formLayout->addRow("Name:",                                  nameEdit_);
    formLayout->addRow("Study Silver Cost (per man-month):",     studyCostEdit_);

    QWidget* checkboxWidget = new QWidget(this);
    checkboxWidget->setLayout(checkboxRow);
    formLayout->addRow("",                                       checkboxWidget);

    formLayout->addRow(new QLabel("Prerequisites (TOKEN:LEVEL per line):", this), new QWidget(this));
    formLayout->addRow(prerequisitesEdit_);

    formLayout->addRow(new QLabel("Production Items (ITEM:AMOUNT per line):", this), new QWidget(this));
    formLayout->addRow(productionItemsEdit_);

    formLayout->addRow(new QLabel("Description (display only):", this), new QWidget(this));
    formLayout->addRow(descriptionEdit_);

    QVBoxLayout* formContainer = new QVBoxLayout;
    formContainer->setContentsMargins(4, 4, 4, 4);
    formContainer->setSpacing(0);
    formContainer->addLayout(formLayout);
    formContainer->addLayout(saveRow);
    formContainer->addStretch();

    QWidget* formWidget = new QWidget(this);
    formWidget->setLayout(formContainer);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(formWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // ---- Splitter ----------------------------------------------------------
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(skillsList_);
    splitter->addWidget(scrollArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({170, 500});

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // ---- Connections -------------------------------------------------------
    connect(skillsList_, &QListWidget::currentRowChanged,
            this, &SkillsTabContentQt::onSkillSelectionChanged);
    connect(levelCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SkillsTabContentQt::onLevelComboChanged);
    connect(saveButton_, &QPushButton::clicked,
            this, &SkillsTabContentQt::onSaveClicked);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void SkillsTabContentQt::refresh()
{
    updateSkillsList();
}

void SkillsTabContentQt::focusSkillByToken(const std::wstring& skillToken)
{
    if (!appData_ || skillToken.empty())
        return;

    selectedSkillToken_ = skillToken;
    displayedLevel_     = 0;
    updateSkillsList();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void SkillsTabContentQt::onSkillSelectionChanged()
{
    // Ignore clicks on separator items (they carry no token data).
    const QListWidgetItem* item = skillsList_->currentItem();
    if (!item || item->data(kTokenRole).toString().isEmpty())
        return;

    updateSelectedSkillFromList();
}

void SkillsTabContentQt::onLevelComboChanged(int index)
{
    if (index < 0 || selectedSkillToken_.empty())
        return;

    displayedLevel_ = levelCombo_->itemData(index).toInt();
    const Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
    loadSkillLevelToFields(skill, displayedLevel_);
}

void SkillsTabContentQt::onSaveClicked()
{
    saveSelectedSkill();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void SkillsTabContentQt::updateSkillsList()
{
    QSignalBlocker listBlocker(skillsList_);

    skillsList_->clear();

    if (!appData_)
        return;

    const auto& repository = appData_->skillRepository();

    // Partition into three groups, then sort each alphabetically by token.
    std::vector<const Skill*> nonMagicSkills;
    std::vector<const Skill*> foundationSkills;
    std::vector<const Skill*> otherMagicSkills;

    for (std::size_t i = 0; i < repository.size(); ++i)
    {
        const Skill& s = repository.at(i);
        if (s.isMagicFoundation())
            foundationSkills.push_back(&s);
        else if (s.isMagic())
            otherMagicSkills.push_back(&s);
        else
            nonMagicSkills.push_back(&s);
    }

    auto sortByToken = [](const Skill* a, const Skill* b)
    {
        return a->getIdentifierToken() < b->getIdentifierToken();
    };
    std::sort(nonMagicSkills.begin(),  nonMagicSkills.end(),  sortByToken);
    std::sort(foundationSkills.begin(), foundationSkills.end(), sortByToken);
    std::sort(otherMagicSkills.begin(), otherMagicSkills.end(), sortByToken);

    const bool hasMagicSkills = !foundationSkills.empty() || !otherMagicSkills.empty();

    int selectedRow = -1;
    int rowIndex    = 0;

    auto addSkillItem = [&](const Skill* skill)
    {
        const QString token = QString::fromStdWString(skill->getIdentifierToken());
        QListWidgetItem* item = new QListWidgetItem(token);
        item->setData(kTokenRole, token);
        skillsList_->addItem(item);

        if (skill->getIdentifierToken() == selectedSkillToken_)
            selectedRow = rowIndex;

        ++rowIndex;
    };

    for (const Skill* s : nonMagicSkills)
        addSkillItem(s);

    // Visual separator between non-magic and magic groups
    if (!nonMagicSkills.empty() && hasMagicSkills)
    {
        QListWidgetItem* sep = new QListWidgetItem(QString(32, QChar(0x2500))); // '─' repeated
        sep->setFlags(Qt::NoItemFlags);
        sep->setForeground(QApplication::palette().color(QPalette::Mid));
        skillsList_->addItem(sep);
        ++rowIndex;
    }

    for (const Skill* s : foundationSkills)
        addSkillItem(s);

    for (const Skill* s : otherMagicSkills)
        addSkillItem(s);

    // Restore selection
    if (selectedRow >= 0)
    {
        skillsList_->setCurrentRow(selectedRow);
        skillsList_->scrollToItem(skillsList_->item(selectedRow));
        updateSelectedSkillFromList();
    }
    else
    {
        selectedSkillToken_.clear();
        displayedLevel_ = 0;
        {
            QSignalBlocker comboBlocker(levelCombo_);
            levelCombo_->clear();
        }
        clearFields();
    }
}

void SkillsTabContentQt::updateSelectedSkillFromList()
{
    const QListWidgetItem* item = skillsList_->currentItem();
    if (!item)
    {
        selectedSkillToken_.clear();
        displayedLevel_ = 0;
        {
            QSignalBlocker comboBlocker(levelCombo_);
            levelCombo_->clear();
        }
        clearFields();
        return;
    }

    const QString tokenStr = item->data(kTokenRole).toString();
    if (tokenStr.isEmpty())
        return;  // separator item

    selectedSkillToken_ = tokenStr.toStdWString();

    const Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
    populateLevelCombo(skill);

    if (skill)
    {
        const auto levels = skill->getLevels();
        if (!levels.empty())
        {
            // Try to keep the previously displayed level if it is still available.
            bool kept = false;
            if (displayedLevel_ > 0)
            {
                for (int i = 0; i < levelCombo_->count(); ++i)
                {
                    if (levelCombo_->itemData(i).toInt() == displayedLevel_)
                    {
                        QSignalBlocker b(levelCombo_);
                        levelCombo_->setCurrentIndex(i);
                        kept = true;
                        break;
                    }
                }
            }
            if (!kept)
            {
                QSignalBlocker b(levelCombo_);
                levelCombo_->setCurrentIndex(0);
                displayedLevel_ = levelCombo_->itemData(0).toInt();
            }
            loadSkillLevelToFields(skill, displayedLevel_);
            return;
        }
    }

    displayedLevel_ = 0;
    clearFields();
}

void SkillsTabContentQt::populateLevelCombo(const Skill* skill)
{
    QSignalBlocker b(levelCombo_);
    levelCombo_->clear();

    if (!skill)
        return;

    for (int lv = Skill::kMinLevel; lv <= Skill::kMaxLevel; ++lv)
    {
        if (!skill->hasLevel(lv))
            continue;
        levelCombo_->addItem(QString("Level %1").arg(lv), QVariant(lv));
    }
}

void SkillsTabContentQt::loadSkillLevelToFields(const Skill* skill, int level)
{
    if (!skill || level <= 0)
    {
        clearFields();
        return;
    }

    tokenEdit_->setText(QString::fromStdWString(skill->getIdentifierToken()));
    nameEdit_->setText(QString::fromStdWString(skill->getName()));
    studyCostEdit_->setText(QString::number(skill->getStudyCost()));

    productionCheck_->setChecked(skill->isProduction(level));
    magicCheck_->setChecked(skill->isMagic());
    magicFoundationCheck_->setChecked(skill->isMagicFoundation());

    const std::wstring prerequisitesText =
        SkillFormattingUtils::formatPrerequisites(skill->getPrerequisites());
    prerequisitesEdit_->setPlainText(QString::fromStdWString(prerequisitesText));

    const std::wstring productionItemsText =
        StringUtils::formatStringIntMap(skill->getProductionItems(level));
    productionItemsEdit_->setPlainText(QString::fromStdWString(productionItemsText));

    descriptionEdit_->setPlainText(QString::fromStdWString(skill->getDescription(level)));
}

void SkillsTabContentQt::clearFields()
{
    tokenEdit_->clear();
    nameEdit_->clear();
    studyCostEdit_->clear();
    prerequisitesEdit_->clear();
    productionItemsEdit_->clear();
    descriptionEdit_->clear();

    productionCheck_->setChecked(false);
    magicCheck_->setChecked(false);
    magicFoundationCheck_->setChecked(false);
}

void SkillsTabContentQt::saveSelectedSkill()
{
    if (!appData_ || selectedSkillToken_.empty() || displayedLevel_ <= 0)
        return;

    Skill* skill = appData_->skillRepository().findByIdentifier(selectedSkillToken_);
    if (!skill || !skill->hasLevel(displayedLevel_))
        return;

    // Token is immutable — warn the user if they tried to change it.
    const std::wstring editedToken =
        StringUtils::trimWhitespace(tokenEdit_->text().toStdWString());
    if (!editedToken.empty() && editedToken != selectedSkillToken_)
    {
        QMessageBox::warning(this, "Skills",
            "Skill ID is immutable in this editor. Other fields were saved.");
    }

    skill->setName(nameEdit_->text().toStdWString());
    skill->setMagicFoundation(magicFoundationCheck_->isChecked());
    skill->setMagic(magicCheck_->isChecked());

    std::map<std::wstring, int> productionItems =
        StringUtils::parseStringIntMap(productionItemsEdit_->toPlainText().toStdWString());
    if (!productionCheck_->isChecked())
        productionItems.clear();
    skill->setProductionItems(displayedLevel_, std::move(productionItems));

    const int studyCostValue = std::max(
        0, StringUtils::parseIntSafe(studyCostEdit_->text().toStdWString()));
    skill->setStudyCost(studyCostValue);

    skill->setPrerequisites(
        SkillFormattingUtils::parsePrerequisites(
            prerequisitesEdit_->toPlainText().toStdWString()));

    updateSkillsList();
}
