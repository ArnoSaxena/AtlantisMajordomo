#include "MainWindowQt.hpp"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "Data/AppData.hpp"

MainWindow::MainWindow(AppData& appData, QWidget* parent)
    : QMainWindow(parent),
      appData_(appData)
{
    setWindowTitle("Atlantis Majordomo");

    resize(1024, 768);

    setupUI();
    createMenu();
    initializeTabs();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto layout = new QVBoxLayout(centralWidget);

    tabWidget_ = new QTabWidget(this);
    layout->addWidget(tabWidget_);
}

void MainWindow::initializeTabs()
{
    reportsTab_ = new QWidget();
    mapTab_     = new QWidget();
    eventsTab_  = new QWidget();
    itemsTab_   = new QWidget();
    skillsTab_  = new QWidget();
    factionsTab_= new QWidget();
    battlesTab_ = new QWidget();

    tabWidget_->addTab(reportsTab_, "Loaded Reports");
    tabWidget_->addTab(mapTab_, "Map");
    tabWidget_->addTab(eventsTab_, "Events");
    tabWidget_->addTab(itemsTab_, "Items");
    tabWidget_->addTab(skillsTab_, "Skills");
    tabWidget_->addTab(factionsTab_, "Factions");
    tabWidget_->addTab(battlesTab_, "Battles");

    connect(tabWidget_, &QTabWidget::currentChanged,
            this, [this](int){
                refreshAllTabContents();
            });
}

void MainWindow::createMenu()
{
    auto fileMenu = menuBar()->addMenu("&File");

    auto actNew = fileMenu->addAction("&New");
    auto actOpen = fileMenu->addAction("&Open...");
    auto actSave = fileMenu->addAction("&Save");
    fileMenu->addSeparator();
    auto actExit = fileMenu->addAction("E&xit");

    connect(actNew, &QAction::triggered, this, [this]() {
        appData_.clear();
        refreshAllTabContents();
        QMessageBox::information(this, "File", "New dataset created.");
    });

    connect(actOpen, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Open", "Not implemented yet.");
    });

    connect(actSave, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Save", "Not implemented yet.");
    });

    connect(actExit, &QAction::triggered, this, &MainWindow::close);

    auto helpMenu = menuBar()->addMenu("&Help");

    auto aboutAct = helpMenu->addAction("&About");
    connect(aboutAct, &QAction::triggered, this, [this]() {
        QString text = QString("%1\n\n%2\n\nVersion %3")
            .arg(kAboutAppName)
            .arg(kAboutDescription)
            .arg(kAboutVersion);

        QMessageBox::about(this, "About", text);
    });
}

void MainWindow::refreshAllTabContents()
{
    // Placeholder – later connect real tab logic
}
