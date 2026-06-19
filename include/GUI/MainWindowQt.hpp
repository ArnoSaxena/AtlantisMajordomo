#pragma once

#include <QMainWindow>
#include <memory>

class AppData;
class QTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(AppData& appData, QWidget* parent = nullptr);
    ~MainWindow();

    static constexpr const char* kAboutAppName = "Atlantis Majordomo";
    static constexpr const char* kAboutDescription =
        "Yet another Atlantis Pbem player client.";
    static constexpr const char* kAboutVersion = "1.2.22";

private:
    void setupUI();
    void createMenu();
    void initializeTabs();
    void refreshAllTabContents();

private:
    AppData& appData_;

    QTabWidget* tabWidget_;

    // Placeholder tab widgets
    QWidget* reportsTab_;
    QWidget* mapTab_;
    QWidget* eventsTab_;
    QWidget* itemsTab_;
    QWidget* skillsTab_;
    QWidget* factionsTab_;
    QWidget* battlesTab_;
};
