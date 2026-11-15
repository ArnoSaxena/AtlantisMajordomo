/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#pragma once

#include <QString>
#include <QWidget>

class AppConfig;
class AppData;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProcess;

class LocalEngineTabContentQt : public QWidget
{
    Q_OBJECT

public:
    LocalEngineTabContentQt(AppData& appData, AppConfig& appConfig, QWidget* parent = nullptr);
    ~LocalEngineTabContentQt() override;

    bool isLocalEnabled() const;
    void setMainFactionNumber(int factionNumber);
    void setOrdersPath(const QString& path);
    bool copyOrders(const QString& sourcePath, int factionNumber, QString& copiedPath);

signals:
    void beforeReportImport();
    void reportImported();

private:
    void enableLocal(bool enabled);
    void browseEngine();
    void browseOrders();
    void newGame();
    void addFaction();
    void runGame();
    void startProcess(const QString& action);
    void finishRun();
    void loadSettings();
    void saveSettings();
    void updateTurn(int turn);
    void setProcessRunning(bool running);

    AppData& appData_;
    AppConfig& appConfig_;
    QCheckBox* localCheck_;
    QWidget* controls_;
    QLineEdit* enginePath_;
    QLineEdit* factionName_;
    QLineEdit* playerNumber_;
    QLineEdit* ordersPath_;
    QLineEdit* mainFaction_;
    QLabel* turnLabel_;
    QPlainTextEdit* terminal_;
    QLineEdit* terminalInput_;
    QProcess* process_;
    QString defaultFolder_;
    int currentTurn_ { 0 };
    bool runPending_ { false };
    bool loadingSettings_ { false };
};
