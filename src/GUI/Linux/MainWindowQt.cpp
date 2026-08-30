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
 * File: MainWindowQt.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#include "GUI/MainWindowQt.hpp"
#include "GUI/ReportsTabContentQt.hpp"
#include "GUI/EventsTabContentQt.hpp"
#include "GUI/BattlesTabContentQt.hpp"
#include "GUI/FactionsTabContentQt.hpp"
#include "GUI/ItemsTabContentQt.hpp"
#include "GUI/SkillsTabContentQt.hpp"
#include "GUI/SettingsDialogQt.hpp"
#include "GUI/MapTabContentQt.hpp"

#include "Data/AppData.hpp"
#include "Data/Commands.hpp"
#include "Data/DataSerializer.hpp"
#include "Data/Faction.hpp"
#include "Data/ReportRepository.hpp"
#include "Data/Unit.hpp"
#include "Function/StartupAutoLoadUtils.hpp"
#include "Function/TabRefreshUtils.hpp"
#include "Function/OrderWarningService.hpp"

#include "Function/OrderBusinessLogic.hpp"
#include "Function/OrderParsingUtils.hpp"
#include "Function/CommandSimulationService.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFont>
#include <QGuiApplication>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScreen>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <array>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Local helpers  (mirrors the anonymous-namespace helpers in MainWindowWin.cpp)
// ---------------------------------------------------------------------------
namespace
{

enum class QtUiProfile
{
    Auto,
    Compact,
    Standard,
    Large,
};

struct QtUiMetrics
{
    int baseFontPt { 10 };
    int controlHeightPx { 24 };
    int spacingPx { 6 };
    int marginPx { 8 };
    double dialogWidthScale { 1.0 };
    double dialogHeightScale { 1.0 };
};

constexpr int kDefaultMainWindowWidth = 900;
constexpr int kDefaultMainWindowHeight = 600;

QtUiProfile profileFromConfigMode(const std::wstring& configuredMode)
{
    std::wstring normalized = configuredMode;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), std::towlower);

    if (normalized == L"compact") return QtUiProfile::Compact;
    if (normalized == L"standard") return QtUiProfile::Standard;
    if (normalized == L"large") return QtUiProfile::Large;
    return QtUiProfile::Auto;
}

QtUiProfile detectAutoProfile(QScreen* screen)
{
    if (screen == nullptr)
    {
        return QtUiProfile::Standard;
    }

    const QRect available = screen->availableGeometry();
    const int width = available.width();
    const int height = available.height();
    const qreal dpi = screen->logicalDotsPerInch();

    if (width <= 1366 || height <= 768)
    {
        return QtUiProfile::Compact;
    }

    if (width >= 2560 || height >= 1440)
    {
        return QtUiProfile::Large;
    }

    if (dpi >= 168.0)
    {
        return QtUiProfile::Large;
    }

    if (dpi >= 144.0 && (width >= 1920 || height >= 1200))
    {
        return QtUiProfile::Large;
    }

    return QtUiProfile::Standard;
}

QtUiProfile resolveProfile(QtUiProfile requested, QScreen* screen)
{
    return requested == QtUiProfile::Auto ? detectAutoProfile(screen) : requested;
}

QtUiMetrics metricsForProfile(QtUiProfile profile)
{
    switch (profile)
    {
        case QtUiProfile::Compact:
            return QtUiMetrics { .baseFontPt = 9,
                                 .controlHeightPx = 22,
                                 .spacingPx = 4,
                                 .marginPx = 6,
                                 .dialogWidthScale = 0.82,
                                 .dialogHeightScale = 0.82 };
        case QtUiProfile::Large:
            return QtUiMetrics { .baseFontPt = 11,
                                 .controlHeightPx = 30,
                                 .spacingPx = 8,
                                 .marginPx = 10,
                                 .dialogWidthScale = 1.0,
                                 .dialogHeightScale = 1.0 };
        case QtUiProfile::Auto:
        case QtUiProfile::Standard:
        default:
            return QtUiMetrics { .baseFontPt = 10,
                                 .controlHeightPx = 26,
                                 .spacingPx = 6,
                                 .marginPx = 8,
                                 .dialogWidthScale = 0.90,
                                 .dialogHeightScale = 0.90 };
    }
}

QtUiMetrics resolveMetricsForConfig(const AppConfig& appConfig, QScreen* screen)
{
    const QtUiProfile requested = profileFromConfigMode(appConfig.getUiSizeMode());
    return metricsForProfile(resolveProfile(requested, screen));
}

struct MainFactionExportContext
{
    int factionNumber { 0 };
    std::wstring password;
    int nextMonth { 0 };
    int nextYear  { 0 };
};

bool isLaterPeriod(int year, int month, int refYear, int refMonth)
{
    return (year > refYear) || (year == refYear && month > refMonth);
}

bool tryBuildMainFactionExportContext(const AppData& appData,
                                      MainFactionExportContext& ctx,
                                      std::wstring& errorMessage)
{
    const auto& factionRepo = appData.factionRepository();
    const Faction* mainFaction = nullptr;
    int mainFactionCount = 0;

    for (std::size_t i = 0; i < factionRepo.size(); ++i)
    {
        const Faction& faction = factionRepo.at(i);
        if (faction.isMainFaction())
        {
            ++mainFactionCount;
            mainFaction = &faction;
        }
    }

    if (mainFactionCount == 0 || mainFaction == nullptr)
    {
        errorMessage = L"Cannot export orders: no main faction is selected.";
        return false;
    }

    if (mainFactionCount > 1)
    {
        errorMessage = L"Cannot export orders: more than one main faction is selected.";
        return false;
    }

    int latestYear = 0, latestMonth = 0;
    bool hasPeriod = false;

    const auto& reportRepository = appData.reportRepository();
    for (std::size_t i = 0; i < reportRepository.size(); ++i)
    {
        const Report& report = reportRepository.at(i);
        const int month = report.getMonth();
        const int year  = report.getYear();
        if (month < 1 || month > 12) continue;
        if (!hasPeriod || isLaterPeriod(year, month, latestYear, latestMonth))
        {
            latestYear  = year;
            latestMonth = month;
            hasPeriod   = true;
        }
    }

    if (!hasPeriod)
    {
        errorMessage = L"Cannot export orders: no valid month/year was found in loaded reports.";
        return false;
    }

    ++latestMonth;
    if (latestMonth > 12) { latestMonth = 1; ++latestYear; }

    ctx.factionNumber = mainFaction->getFactionNumber();
    ctx.password      = mainFaction->getPassword();
    ctx.nextMonth     = latestMonth;
    ctx.nextYear      = latestYear;
    errorMessage.clear();
    return true;
}

std::wstring formatTwoDigits(int value)
{
    return (value < 10 ? L"0" : L"") + std::to_wstring(value);
}

std::wstring buildSuggestedOrdersFilename(const MainFactionExportContext& ctx)
{
    return L"orders_"
        + std::to_wstring(ctx.factionNumber)
        + L"_"
        + std::to_wstring(ctx.nextYear)
        + L"_"
        + formatTwoDigits(ctx.nextMonth)
        + L".ord";
}

// Returns the number of units updated on success, or -1 on error.
// Parses a .ord file and updates the order fields for matching units.
int importOrdersFromContent(AppData& appData, const std::wstring& fileContent, std::wstring& errorMessage)
{
    errorMessage.clear();
    int unitsUpdated = 0;

    // Split content into lines
    std::vector<std::wstring> lines;
    {
        std::wistringstream iss(fileContent);
        std::wstring line;
        while (std::getline(iss, line))
        {
            // Remove trailing whitespace
            while (!line.empty() && (line.back() == L'\r' || line.back() == L' ' || line.back() == L'\t'))
            {
                line.pop_back();
            }
            lines.push_back(line);
        }
    }

    // Parse the file
    int currentUnitNumber = -1;
    std::vector<std::wstring> currentOrders;

    for (const auto& line : lines)
    {
        // Skip empty lines and comments
        if (line.empty() || line[0] == L'#')
        {
            continue;
        }

        // Trim leading whitespace
        std::size_t startPos = 0;
        while (startPos < line.size() && (line[startPos] == L' ' || line[startPos] == L'\t'))
        {
            ++startPos;
        }

        std::wstring trimmedLine = line.substr(startPos);

        // Check for "unit <number>" directive
        if (trimmedLine.size() > 5 && trimmedLine.substr(0, 5) == L"unit ")
        {
            // Save previous unit's orders if any
            if (currentUnitNumber != -1 && !currentOrders.empty())
            {
                Unit* unit = appData.unitRepository().findByNumber(currentUnitNumber);
                if (unit)
                {
                    unit->setOrders(currentOrders);
                    ++unitsUpdated;

                    // Synchronize the order repository from the saved Unit (defer global recompute)
                    OrderBusinessLogic::syncOrderRepositoryForSavedUnit(appData, currentUnitNumber, false);

                    // Rebuild UnitNew snapshots originating from this unit based on FORM blocks
                    appData.unitNewRepository().removeByOriginUnit(currentUnitNumber);
                    const std::vector<int> formUnitNumbers =
                        OrderParsingUtils::extractFormNewUnitNumbers(currentOrders);
                    for (int formUnitNumber : formUnitNumbers)
                    {
                        const int x = unit->getXCoordinate();
                        const int y = unit->getYCoordinate();
                        const int z = unit->getZCoordinate();
                        const std::wstring formUnitName = L"New Unit";

                        appData.unitNewRepository().add(
                            formUnitNumber,
                            formUnitName,
                            unit->getStructureId(),
                            x, y, z,
                            unit->getFlags(),
                            std::map<std::wstring, int>(),
                            0, 0, 0, 0, 0,
                            std::map<std::wstring, int>(),
                            unit->getMonth(),
                            unit->getYear(),
                            currentUnitNumber
                        );
                    }
                }
            }

            // Parse new unit number
            try
            {
                currentUnitNumber = std::stoi(trimmedLine.substr(5));
                currentOrders.clear();
            }
            catch (const std::exception&)
            {
                errorMessage = L"Invalid unit number format in file.";
                return -1;
            }
        }
        else if (currentUnitNumber != -1)
        {
            // This is an order line for the current unit
            currentOrders.push_back(trimmedLine);
        }
    }

    // Save the last unit's orders if any (same handling as above)
    if (currentUnitNumber != -1 && !currentOrders.empty())
    {
        Unit* unit = appData.unitRepository().findByNumber(currentUnitNumber);
        if (unit)
        {
            unit->setOrders(currentOrders);
            ++unitsUpdated;

            OrderBusinessLogic::syncOrderRepositoryForSavedUnit(appData, currentUnitNumber, false);

            appData.unitNewRepository().removeByOriginUnit(currentUnitNumber);
            const std::vector<int> formUnitNumbers =
                OrderParsingUtils::extractFormNewUnitNumbers(currentOrders);
            for (int formUnitNumber : formUnitNumbers)
            {
                const int x = unit->getXCoordinate();
                const int y = unit->getYCoordinate();
                const int z = unit->getZCoordinate();
                const std::wstring formUnitName = L"New Unit";

                appData.unitNewRepository().add(
                    formUnitNumber,
                    formUnitName,
                    unit->getStructureId(),
                    x, y, z,
                    unit->getFlags(),
                    std::map<std::wstring, int>(),
                    0, 0, 0, 0, 0,
                    std::map<std::wstring, int>(),
                    unit->getMonth(),
                    unit->getYear(),
                    currentUnitNumber
                );
            }
        }
    }

    // One global recompute after all imports
    CommandSimulationService::recalculateAfterOrdersValues(appData);
    OrderWarningService::runForMainFaction(appData);

    return unitsUpdated;
}

std::wstring buildOrdersExportContent(const AppData& appData,
                                       int mainFactionNumber,
                                       const std::wstring& password)
{
    std::vector<const Unit*> units;
    const auto& unitRepo = appData.unitRepository();
    for (std::size_t i = 0; i < unitRepo.size(); ++i)
    {
        const Unit& unit = unitRepo.at(i);
        if (unit.getFactionNumber() == mainFactionNumber)
            units.push_back(&unit);
    }

    std::sort(units.begin(), units.end(),
              [](const Unit* a, const Unit* b) {
                  return a->getUnitNumber() < b->getUnitNumber();
              });

    std::wostringstream out;
    out << L"#atlantis " << mainFactionNumber << L" \"" << password << L"\"\n";
    for (const Unit* unit : units)
    {
        out << L"\nunit " << unit->getUnitNumber() << L"\n";
        for (const std::wstring& line : unit->getOrders())
            out << line << L"\n";
    }
    out << L"\n#end\n";
    return out.str();
}

bool saveTextFile(const std::wstring& path, const std::wstring& content)
{
    std::wofstream file{std::filesystem::path(path)};
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

// Returns the parent directory of a path as a QString, or empty if unavailable.
QString dirFromPath(const std::wstring& path)
{
    if (path.empty()) return {};
    std::filesystem::path p(path);
    if (p.has_parent_path())
        return QString::fromStdWString(p.parent_path().wstring());
    return {};
}

} // namespace

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

MainWindow::MainWindow(AppData& appData, QWidget* parent)
    : QMainWindow(parent)
    , appData_(appData)
{
    appConfig_.load();
    applyConfigToAppData();
    applyQtUiSizing();

    setWindowTitle(kAboutAppName);
    setWindowIcon(QIcon(QStringLiteral(":/icons/AtlantisMajordomo_256.png")));

    const QtUiMetrics metrics = resolveMetricsForConfig(appConfig_, screen());
    int startupWidth = appConfig_.getMainWindowWidth();
    int startupHeight = appConfig_.getMainWindowHeight();
    const bool usingDefaultConfigSize =
        startupWidth == kDefaultMainWindowWidth && startupHeight == kDefaultMainWindowHeight;
    if (usingDefaultConfigSize)
    {
        startupWidth = static_cast<int>(static_cast<double>(startupWidth) * metrics.dialogWidthScale);
        startupHeight = static_cast<int>(static_cast<double>(startupHeight) * metrics.dialogHeightScale);
    }

    QScreen* startupScreen = screen();
    if (startupScreen == nullptr)
    {
        startupScreen = QGuiApplication::primaryScreen();
    }
    const QRect available = startupScreen != nullptr
        ? startupScreen->availableGeometry()
        : QRect(0, 0, 1920, 1080);

    const int minStartupWidth = 720;
    const int minStartupHeight = 500;
    startupWidth = std::clamp(startupWidth, minStartupWidth, (std::max)(minStartupWidth, available.width()));
    startupHeight = std::clamp(startupHeight, minStartupHeight, (std::max)(minStartupHeight, available.height()));

    resize(startupWidth, startupHeight);
    move(available.left() + (available.width() - startupWidth) / 2,
         available.top() + (available.height() - startupHeight) / 2);

    setupMenus();
    setupTabs();

    // Defer heavy initialisation so the window is visible before any I/O.
    QTimer::singleShot(0, this, &MainWindow::deferredInit);
}

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------
// Config / AppData wiring
// ---------------------------------------------------------------------------

void MainWindow::applyConfigToAppData()
{
    appData_.setFlyingShipsCsv(appConfig_.getFlyingShipsCsv());
    appData_.setMagicSkillTriggersCsv(appConfig_.getMagicSkillTriggersCsv());
    appData_.setOnlyLeaderCanTeach(appConfig_.getOnlyLeaderCanTeach());
    appData_.setLeaderMages(appConfig_.getLeaderMages());
    Commands::setFullMonthOrderKeywordsCsv(appConfig_.getFullMonthOrdersCsv());
}

void MainWindow::applyQtUiSizing()
{
    const QtUiMetrics metrics = resolveMetricsForConfig(appConfig_, screen());

    QFont appFont = font();
    appFont.setPointSize(metrics.baseFontPt);
    setFont(appFont);

    const QString style = QString(
        "QWidget { font-size: %1pt; }"
        "QPushButton, QLineEdit, QComboBox, QAbstractSpinBox { min-height: %2px; }"
        "QTabBar::tab { min-height: %3px; padding: 4px 10px; }"
        "QListView::item, QTreeView::item, QTableView::item { min-height: %4px; }"
    )
        .arg(metrics.baseFontPt)
        .arg(metrics.controlHeightPx)
        .arg((std::max)(20, metrics.controlHeightPx - 2))
        .arg((std::max)(18, metrics.controlHeightPx - 4));

    setStyleSheet(style);

    if (tabWidget_ != nullptr)
    {
        tabWidget_->setDocumentMode(true);
    }
}

// ---------------------------------------------------------------------------
// Menu setup
// ---------------------------------------------------------------------------

void MainWindow::setupMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New",              this, &MainWindow::onFileNew);
    fileMenu->addAction("&Open...",          this, &MainWindow::onFileOpen);
    fileMenu->addAction("&Save",             this, &MainWindow::onFileSave);
    fileMenu->addSeparator();
    fileMenu->addAction("&Load Report",      this, &MainWindow::onFileLoadReport);
    fileMenu->addAction("&Import Data...",   this, &MainWindow::onFileImportData);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xport Orders...", this, &MainWindow::onFileExportOrders);
    fileMenu->addAction("&Import Orders...", this, &MainWindow::onFileImportOrders);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit",             this, &QWidget::close);

    QMenu* settingsMenu = menuBar()->addMenu("&Settings");
    settingsMenu->addAction("&Options",      this, &MainWindow::onSettingsOptions);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&Description",      this, &MainWindow::onHelpDescription);
    helpMenu->addAction("&About",            this, &MainWindow::onHelpAbout);
}

// ---------------------------------------------------------------------------
// Tab setup
// ---------------------------------------------------------------------------

void MainWindow::setupTabs()
{
    tabWidget_ = new QTabWidget(this);
    setCentralWidget(tabWidget_);

    auto addTab = [this](const QString& label) -> QWidget* {
        QWidget* w = new QWidget(tabWidget_);
        tabWidget_->addTab(w, label);
        return w;
    };

    reportsTab_  = addTab("Loaded Reports");
    mapTab_      = addTab("Map");
    eventsTab_   = addTab("Events");
    itemsTab_    = addTab("Items");
    skillsTab_   = addTab("Skills");
    factionsTab_ = addTab("Factions");
    battlesTab_  = addTab("Battles");
}

// ---------------------------------------------------------------------------
// Deferred initialisation  (equivalent to WM_APP_INIT + WM_APP_AUTOLOAD)
// ---------------------------------------------------------------------------

void MainWindow::deferredInit()
{
    // Reports tab
    reportsTabContent_ = new ReportsTabContentQt(appData_, appConfig_, reportsTab_);
    QVBoxLayout* reportsLayout = new QVBoxLayout(reportsTab_);
    reportsLayout->setContentsMargins(0, 0, 0, 0);
    reportsLayout->addWidget(reportsTabContent_);

    // Events tab
    eventsTabContent_ = new EventsTabContentQt(appData_, eventsTab_);
    QVBoxLayout* eventsLayout = new QVBoxLayout(eventsTab_);
    eventsLayout->setContentsMargins(0, 0, 0, 0);
    eventsLayout->addWidget(eventsTabContent_);

    // Battles tab
    battlesTabContent_ = new BattlesTabContentQt(appData_, battlesTab_);
    QVBoxLayout* battlesLayout = new QVBoxLayout(battlesTab_);
    battlesLayout->setContentsMargins(0, 0, 0, 0);
    battlesLayout->addWidget(battlesTabContent_);
        connect(battlesTabContent_, &BattlesTabContentQt::navigateToMap,
            this, &MainWindow::onNavigateToMap);

    // Items tab
    itemsTabContent_ = new ItemsTabContentQt(appData_, itemsTab_);
    QVBoxLayout* itemsLayout = new QVBoxLayout(itemsTab_);
    itemsLayout->setContentsMargins(0, 0, 0, 0);
    itemsLayout->addWidget(itemsTabContent_);

    // Factions tab
    factionsTabContent_ = new FactionsTabContentQt(appData_, factionsTab_);
    QVBoxLayout* factionsLayout = new QVBoxLayout(factionsTab_);
    factionsLayout->setContentsMargins(0, 0, 0, 0);
    factionsLayout->addWidget(factionsTabContent_);

    // Skills tab
    skillsTabContent_ = new SkillsTabContentQt(appData_, skillsTab_);
    QVBoxLayout* skillsLayout = new QVBoxLayout(skillsTab_);
    skillsLayout->setContentsMargins(0, 0, 0, 0);
    skillsLayout->addWidget(skillsTabContent_);

    // Map tab
    mapTabContent_ = new MapTabContentQt(appData_, appConfig_, mapTab_);
    QVBoxLayout* mapLayout = new QVBoxLayout(mapTab_);
    mapLayout->setContentsMargins(0, 0, 0, 0);
    mapLayout->addWidget(mapTabContent_);
    connect(mapTabContent_, &MapTabContentQt::navigateToBattle,
            this, &MainWindow::onNavigateToBattle);
    connect(mapTabContent_, &MapTabContentQt::navigateToSkill,
            this, &MainWindow::onNavigateToSkill);
    connect(mapTabContent_, &MapTabContentQt::navigateToItem,
            this, &MainWindow::onNavigateToItem);
    connect(mapTabContent_, &MapTabContentQt::warningsChanged,
            this, [this]() {
                if (eventsTabContent_) {
                    eventsTabContent_->refresh();
                }
            });

    // TODO: Create remaining Qt tab content widgets here as they are implemented.

    autoLoad();
}

void MainWindow::autoLoad()
{
    StartupAutoLoadUtils::AutoLoadResult autoLoadResult =
        StartupAutoLoadUtils::runAutoLoad(appData_, appConfig_);

    if (!autoLoadResult.dataFileError.empty())
    {
        QMessageBox::warning(this,
                             "Startup Load Error",
                             QString::fromStdWString(autoLoadResult.dataFileError));
    }

    const std::wstring reportFolderError =
        StartupAutoLoadUtils::buildReportFolderErrorMessage(autoLoadResult.reportLoadErrors, 10);
    if (!reportFolderError.empty())
    {
        QMessageBox::warning(this,
                             "Startup Load Error",
                             QString::fromStdWString(reportFolderError));
    }

    refreshAllTabs();
}

void MainWindow::refreshAllTabs()
{
    TabRefreshUtils::runRefreshContract(TabRefreshUtils::RefreshCallbacks{
        [this]() { if (reportsTabContent_) { reportsTabContent_->refresh(); } },
        [this]() { if (mapTabContent_) { mapTabContent_->refresh(); } },
        [this]() { if (eventsTabContent_) { eventsTabContent_->refresh(); } },
        [this]() { if (itemsTabContent_) { itemsTabContent_->refresh(); } },
        [this]() { if (skillsTabContent_) { skillsTabContent_->refresh(); } },
        [this]() { if (factionsTabContent_) { factionsTabContent_->refresh(); } },
        [this]() { if (battlesTabContent_) { battlesTabContent_->refresh(); } },
        {}
    });

    // TODO: Call refresh() on remaining Qt tab content widgets as they are implemented.
}

// ---------------------------------------------------------------------------
// Qt event overrides
// ---------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* event)
{
    appConfig_.setMainWindowWidth(width());
    appConfig_.setMainWindowHeight(height());
    appConfig_.save();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    appConfig_.setMainWindowWidth(width());
    appConfig_.setMainWindowHeight(height());
    appConfig_.save();
}

// ---------------------------------------------------------------------------
// Menu slots — File
// ---------------------------------------------------------------------------

void MainWindow::onFileNew()
{
    appData_.clear();
    refreshAllTabs();
    QMessageBox::information(this, "New", "New session started.");
}

void MainWindow::onFileOpen()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Dataset",
        QString::fromStdWString(appConfig_.getSaveFilePath()),
        "Dataset Files (*.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    if (!DataSerializer::loadFromFile(appData_, path))
    {
        QMessageBox::critical(this, "Open Failed",
            QString("Failed to open dataset:\n\n%1")
                .arg(QString::fromStdWString(DataSerializer::getLastError())));
        return;
    }

    appConfig_.setSaveFilePath(path);
    appConfig_.save();
    refreshAllTabs();
    QMessageBox::information(this, "Open", "Dataset loaded successfully.");
}

void MainWindow::onFileSave()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Dataset",
        QString::fromStdWString(appConfig_.getSaveFilePath()),
        "Dataset Files (*.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    if (!DataSerializer::saveToFile(appData_, path))
    {
        QMessageBox::critical(this, "Save Failed",
            QString("Failed to save dataset:\n\n%1")
                .arg(QString::fromStdWString(DataSerializer::getLastError())));
        return;
    }

    appConfig_.setSaveFilePath(path);
    appConfig_.save();
    QMessageBox::information(this, "Save", "Dataset saved successfully.");
}

void MainWindow::onFileLoadReport()
{
    const QString initialDir = QString::fromStdWString(appConfig_.getReportImportFolder());
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Load Report",
        initialDir,
        "Report Files (*.rep *.txt *.html *.htm);;All Files (*)");

    if (files.isEmpty()) return;

    auto& reportRepo = appData_.reportRepository();
    std::vector<std::wstring> failedReports;
    bool savedFolder = false;

    for (const QString& file : files)
    {
        const std::wstring path = file.toStdWString();
        if (!reportRepo.addFromFile(path,
                                    appData_.factionRepository(),
                                    appData_.regionRepository(),
                                    appData_.unitRepository(),
                                    appData_.battleRepository(),
                                    appData_.eventRepository(),
                                    appData_.itemRepository(),
                                    appData_.skillRepository(),
                                    appData_.structureRepository(),
                                    appData_.structInfoRepository(),
                                    appData_.orderRepository(),
                                    appData_.getShipStructureIdThreshold(),
                                    appData_.getFlyingShipTypeTokens(),
                                    appData_.getMagicSkillTriggerPhrases(),
                                    true))
        {
            failedReports.push_back(
                std::filesystem::path(path).filename().wstring() + L": " + reportRepo.getLastError());
        }
        else if (!savedFolder)
        {
            appConfig_.setReportImportFolder(
                std::filesystem::path(path).parent_path().wstring());
            appConfig_.save();
            savedFolder = true;
        }
    }

    refreshAllTabs();

    if (!failedReports.empty())
    {
        std::wstring msg = L"Some reports failed to load:\n\n";
        for (const auto& err : failedReports)
            msg += err + L"\n";
        QMessageBox::warning(this, "Load Report", QString::fromStdWString(msg));
    }
}

void MainWindow::onFileImportData()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import Data",
        QString::fromStdWString(appConfig_.getDataFilePath()),
        "Data Files (*.rep *.txt *.html *.htm *.dat);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring path = filePath.toStdWString();
    auto& reportRepo = appData_.reportRepository();
    if (!reportRepo.addFromFile(path,
                                appData_.factionRepository(),
                                appData_.regionRepository(),
                                appData_.unitRepository(),
                                appData_.battleRepository(),
                                appData_.eventRepository(),
                                appData_.itemRepository(),
                                appData_.skillRepository(),
                                appData_.structureRepository(),
                                appData_.structInfoRepository(),
                                appData_.orderRepository(),
                                appData_.getShipStructureIdThreshold(),
                                appData_.getFlyingShipTypeTokens(),
                                appData_.getMagicSkillTriggerPhrases(),
                                false))
    {
        QMessageBox::critical(this, "Import Data Failed",
            QString("Failed to import data:\n\n%1")
                .arg(QString::fromStdWString(reportRepo.getLastError())));
        return;
    }

    appConfig_.setDataFilePath(path);
    appConfig_.save();
    refreshAllTabs();
}

void MainWindow::onFileExportOrders()
{
    MainFactionExportContext ctx;
    std::wstring errorMessage;
    if (!tryBuildMainFactionExportContext(appData_, ctx, errorMessage))
    {
        QMessageBox::warning(this, "Export Orders", QString::fromStdWString(errorMessage));
        return;
    }

    // Determine initial save path: prefer configured export folder, fall back to save file dir
    QString initialDir = QString::fromStdWString(appConfig_.getExportOrdersFolder());
    if (initialDir.isEmpty())
        initialDir = dirFromPath(appConfig_.getSaveFilePath());

    const QString suggestedName = QString::fromStdWString(buildSuggestedOrdersFilename(ctx));
    const QString initialPath   = initialDir.isEmpty()
        ? suggestedName
        : initialDir + QLatin1Char('/') + suggestedName;

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Orders",
        initialPath,
        "Order Files (*.ord);;All Files (*)");

    if (filePath.isEmpty()) return;

    const std::wstring content = buildOrdersExportContent(appData_, ctx.factionNumber, ctx.password);
    if (!saveTextFile(filePath.toStdWString(), content))
    {
        QMessageBox::critical(this, "Export Orders",
            QString("Failed to write orders file:\n\n%1").arg(filePath));
        return;
    }

    appConfig_.setExportOrdersFolder(
        std::filesystem::path(filePath.toStdWString()).parent_path().wstring());
    appConfig_.save();

    QMessageBox::information(this, "Export Orders",
        QString("Orders exported successfully to:\n\n%1").arg(filePath));
}

void MainWindow::onFileImportOrders()
{
    // Determine initial directory: prefer configured export folder, fall back to save file dir
    QString initialDir = QString::fromStdWString(appConfig_.getExportOrdersFolder());
    if (initialDir.isEmpty())
        initialDir = dirFromPath(appConfig_.getSaveFilePath());

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import Orders",
        initialDir,
        "Order Files (*.ord);;All Files (*)");

    if (filePath.isEmpty()) return;

    // Read file content
    std::wifstream file(std::filesystem::path(filePath.toStdWString()));
    if (!file.is_open())
    {
        QMessageBox::critical(this, "Import Orders",
            QString("Failed to open file:\n\n%1").arg(filePath));
        return;
    }

    std::wstring fileContent((std::istreambuf_iterator<wchar_t>(file)),
                             std::istreambuf_iterator<wchar_t>());

    std::wstring errorMessage;
    int unitsUpdated = importOrdersFromContent(appData_, fileContent, errorMessage);
    if (unitsUpdated < 0)
    {
        QMessageBox::critical(this, "Import Orders",
            QString("Failed to import orders:\n\n%1").arg(QString::fromStdWString(errorMessage)));
        return;
    }

    appConfig_.setExportOrdersFolder(
        std::filesystem::path(filePath.toStdWString()).parent_path().wstring());
    appConfig_.save();

    // Recalculate warnings now that new orders are in place.
    OrderWarningService::runForMainFaction(appData_);

    refreshAllTabs();

    QMessageBox::information(this, "Import Orders",
        QString("Orders imported successfully for %1 unit(s).\n").arg(unitsUpdated));
}

// ---------------------------------------------------------------------------
// Menu slots — Settings
// ---------------------------------------------------------------------------

void MainWindow::onSettingsOptions()
{
    SettingsDialogQt dlg(appData_, appConfig_, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        applyConfigToAppData();
        applyQtUiSizing();
        refreshAllTabs();
    }
}

// ---------------------------------------------------------------------------
// Navigation slots — triggered by MapTabContentQt signals
// ---------------------------------------------------------------------------

void MainWindow::onNavigateToBattle(int x, int y, int z, int month, int year)
{
    if (tabWidget_ && battlesTab_)
        tabWidget_->setCurrentWidget(battlesTab_);
    if (battlesTabContent_)
        battlesTabContent_->focusBattleByRegion(x, y, z, month, year);
}

void MainWindow::onNavigateToMap(int x, int y, int z)
{
    if (tabWidget_ && mapTab_)
        tabWidget_->setCurrentWidget(mapTab_);
    if (mapTabContent_)
        mapTabContent_->focusRegion(x, y, z);
}

void MainWindow::onNavigateToSkill(const QString& skillToken)
{
    if (tabWidget_ && skillsTab_)
        tabWidget_->setCurrentWidget(skillsTab_);
    if (skillsTabContent_)
        skillsTabContent_->focusSkillByToken(skillToken.toStdWString());
}

void MainWindow::onNavigateToItem(const QString& itemToken)
{
    if (tabWidget_ && itemsTab_)
        tabWidget_->setCurrentWidget(itemsTab_);
    if (itemsTabContent_)
        itemsTabContent_->focusItemByToken(itemToken.toStdWString());
}

// ---------------------------------------------------------------------------
// Menu slots — Help
// ---------------------------------------------------------------------------

void MainWindow::onHelpDescription()
{
    const QString text =
        "Map View Legend:\n"
        "- Settlement marker in the center of the region:\n"
        "  - City: big black dot\n"
        "  - Town: black ring with small black dot in center\n"
        "  - Village: black ring\n"
        "- Hex fill color: the region terrain type\n"
        "- Grey line from hex centre to border: road connection in the respective direction\n"
        "- Gray dot at top left: indicates a structure in the region\n"
        "- Orange dot at top left: one of the present structures is a caravanserai\n"
        "- Black outline on the top left dot: one of the present structures is a shaft\n"
        "- Light blue dot at bottom left: one or more ships are present in the region\n"
        "- Black outline on the ship dot: one or more of the present ships are airborne\n"
        "- Red x at the bottom: a battle has taken place in the region in the last turn\n"
        "\n"
        "Unit List Legend:\n"
        "- Green background of unit number and name: the unit is on guard\n"
        "- Green structure name: the unit is owner of the structure\n";

    QMessageBox box(this);
    box.setWindowTitle("Description");
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void MainWindow::onHelpAbout()
{
    const QString text =
        QString("<b>%1</b><br><br>%2<br><br>Version %3")
            .arg(kAboutAppName)
            .arg(kAboutDescription)
            .arg(kAboutVersion);

    QMessageBox::about(this,
        QString("About %1").arg(kAboutAppName),
        text);
}
