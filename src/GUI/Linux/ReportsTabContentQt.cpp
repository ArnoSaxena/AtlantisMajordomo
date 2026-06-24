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
 * File: ReportsTabContentQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/ReportsTabContentQt.hpp"

#include "AppConfig.hpp"
#include "Data/AppData.hpp"
#include "Function/AppDataUtils.hpp"
#include "Function/MonthUtils.hpp"

#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <filesystem>
#include <string>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ReportsTabContentQt::ReportsTabContentQt(AppData& appData, AppConfig& appConfig,
                                          QWidget* parent)
    : QWidget(parent)
    , appData_(&appData)
    , appConfig_(&appConfig)
{
    // ---- Left pane: list + clear button ------------------------------------
    reportsList_ = new QListWidget(this);
    reportsList_->setContextMenuPolicy(Qt::CustomContextMenu);
    reportsList_->setSelectionMode(QAbstractItemView::SingleSelection);

    clearButton_ = new QPushButton("Clear", this);
    clearButton_->setFixedHeight(30);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);
    leftLayout->addWidget(reportsList_);
    leftLayout->addWidget(clearButton_, 0, Qt::AlignRight);

    QWidget* leftWidget = new QWidget(this);
    leftWidget->setLayout(leftLayout);

    // ---- Right pane: detail labels -----------------------------------------
    factionLabel_        = new QLabel(this);
    monthLabel_          = new QLabel(this);
    foundRegionsLabel_   = new QLabel(this);
    visitedRegionsLabel_ = new QLabel(this);

    for (QLabel* lbl : { factionLabel_, monthLabel_, foundRegionsLabel_, visitedRegionsLabel_ })
    {
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        lbl->setWordWrap(false);
    }

    QFrame* detailFrame = new QFrame(this);
    detailFrame->setFrameShape(QFrame::StyledPanel);
    detailFrame->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* rightLayout = new QVBoxLayout(detailFrame);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(8);
    rightLayout->addWidget(factionLabel_);
    rightLayout->addWidget(monthLabel_);
    rightLayout->addWidget(foundRegionsLabel_);
    rightLayout->addWidget(visitedRegionsLabel_);
    rightLayout->addStretch();

    // ---- Splitter ----------------------------------------------------------
    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->addWidget(leftWidget);
    splitter_->addWidget(detailFrame);
    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 1);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(splitter_);
    setLayout(mainLayout);

    // ---- Connections -------------------------------------------------------
    connect(reportsList_, &QListWidget::itemSelectionChanged,
            this, &ReportsTabContentQt::onSelectionChanged);
    connect(reportsList_, &QListWidget::customContextMenuRequested,
            this, &ReportsTabContentQt::onContextMenuRequested);
    connect(clearButton_, &QPushButton::clicked,
            this, &ReportsTabContentQt::onClearClicked);
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void ReportsTabContentQt::loadReport(bool syncFactionFromHeader,
                                      bool rememberReportImportFolder,
                                      bool rememberDataFilePath)
{
    if (!appData_)
        return;

    const QString initialDir = rememberDataFilePath
        ? QString::fromStdWString(
              [this]() -> std::wstring {
                  const std::wstring p = appConfig_->getDataFilePath();
                  std::filesystem::path fp(p);
                  return fp.has_parent_path() ? fp.parent_path().wstring() : p;
              }())
        : QString::fromStdWString(appConfig_->getReportImportFolder());

    const QString filter = rememberDataFilePath
        ? "Data Files (*.txt *.dat);;All Files (*)"
        : "Report Files (*.rep *.txt *.html *.htm);;All Files (*)";

    const QString filePath = QFileDialog::getOpenFileName(
        this, "Load Report", initialDir, filter);

    if (filePath.isEmpty())
        return;

    const std::wstring path = filePath.toStdWString();

    if (AppDataUtils::importReportFromFile(*appData_, appConfig_, path,
                                            syncFactionFromHeader,
                                            rememberReportImportFolder,
                                            rememberDataFilePath))
    {
        updateReportsList();
    }
    else
    {
        const std::wstring msg = L"Failed to load report:\n\n"
                                 + appData_->reportRepository().getLastError();
        QMessageBox::critical(this, "Error", QString::fromStdWString(msg));
    }
}

void ReportsTabContentQt::refresh()
{
    updateReportsList();

    const auto repoSize = appData_ ? appData_->reportRepository().size() : std::size_t{0};
    if (repoSize == 0)
    {
        selectedReportRow_ = -1;
    }
    else if (selectedReportRow_ >= static_cast<int>(repoSize))
    {
        selectedReportRow_ = static_cast<int>(repoSize) - 1;
    }

    if (selectedReportRow_ >= 0 && selectedReportRow_ < reportsList_->count())
    {
        reportsList_->setCurrentRow(selectedReportRow_);
    }

    updateDetailPane(selectedReportRow_);
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void ReportsTabContentQt::onSelectionChanged()
{
    selectedReportRow_ = reportsList_->currentRow();
    updateDetailPane(selectedReportRow_);
}

void ReportsTabContentQt::onClearClicked()
{
    if (!appData_)
        return;

    appData_->reportRepository().clear();
    selectedReportRow_ = -1;
    updateReportsList();
    updateDetailPane(-1);
}

void ReportsTabContentQt::onContextMenuRequested(const QPoint& pos)
{
    const int row = reportsList_->row(reportsList_->itemAt(pos));

    QMenu menu(this);
    menu.addAction("Load Report", this, [this]() {
        loadReport();
    });

    if (row >= 0)
    {
        menu.addAction("Remove", this, [this, row]() {
            if (!appData_)
                return;

            auto& repository = appData_->reportRepository();
            if (static_cast<std::size_t>(row) >= repository.size())
                return;

            if (!repository.removeAt(static_cast<std::size_t>(row)))
            {
                QMessageBox::critical(this, "Error",
                    QString::fromStdWString(repository.getLastError()));
                return;
            }

            updateReportsList();

            const std::size_t repoSize = repository.size();
            if (repoSize > 0)
            {
                const std::size_t nextSel =
                    static_cast<std::size_t>(row) < repoSize
                        ? static_cast<std::size_t>(row)
                        : repoSize - 1;
                selectedReportRow_ = static_cast<int>(nextSel);
                reportsList_->setCurrentRow(selectedReportRow_);
            }
            else
            {
                selectedReportRow_ = -1;
                updateDetailPane(-1);
            }
        });
    }

    menu.exec(reportsList_->viewport()->mapToGlobal(pos));
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void ReportsTabContentQt::updateReportsList()
{
    if (!appData_)
        return;

    const int savedRow = selectedReportRow_;

    // Block selection-change signals while repopulating.
    reportsList_->blockSignals(true);
    reportsList_->clear();

    const auto& repository = appData_->reportRepository();
    for (std::size_t i = 0; i < repository.size(); ++i)
    {
        const std::wstring& fullPath = repository.at(i).getFilePath();
        std::wstring displayName = std::filesystem::path(fullPath).filename().wstring();
        if (displayName.empty())
            displayName = fullPath;

        const std::wstring rowText = std::to_wstring(i + 1) + L"  " + displayName;
        reportsList_->addItem(QString::fromStdWString(rowText));
    }

    reportsList_->blockSignals(false);

    // Restore selection if still valid.
    const int newCount = reportsList_->count();
    if (savedRow >= 0 && savedRow < newCount)
    {
        reportsList_->setCurrentRow(savedRow);
        selectedReportRow_ = savedRow;
    }
    else if (newCount > 0)
    {
        // Keep whatever row is closest.
    }
    else
    {
        selectedReportRow_ = -1;
    }
}

void ReportsTabContentQt::updateDetailPane(int selectedRow)
{
    if (!appData_)
        return;

    const auto& repository = appData_->reportRepository();
    if (selectedRow < 0 || static_cast<std::size_t>(selectedRow) >= repository.size())
    {
        factionLabel_->clear();
        monthLabel_->clear();
        foundRegionsLabel_->clear();
        visitedRegionsLabel_->clear();
        return;
    }

    const auto& report = repository.at(static_cast<std::size_t>(selectedRow));

    const std::wstring factionText = L"Faction: " + report.getFactionName()
        + L" (" + std::to_wstring(report.getFactionNumber()) + L")";
    factionLabel_->setText(QString::fromStdWString(factionText));

    const int month = report.getMonth();
    const int year  = report.getYear();
    const std::wstring monthName = MonthUtils::monthNumberToName(month);
    const std::wstring monthText = L"Month: " + monthName + L", " + std::to_wstring(year);
    monthLabel_->setText(QString::fromStdWString(monthText));

    const std::wstring foundText = L"Found Regions: "
        + std::to_wstring(report.getFoundRegions());
    foundRegionsLabel_->setText(QString::fromStdWString(foundText));

    const std::wstring visitedText = L"Visited Regions: "
        + std::to_wstring(report.getVisitedRegions());
    visitedRegionsLabel_->setText(QString::fromStdWString(visitedText));
}
