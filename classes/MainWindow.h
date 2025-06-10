
#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include "../classes/PlotWidget.h"
#include "PythonEngine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    PlotWidget* plotWidget;
    QTextEdit* outputTextEdit;
    QPushButton* loadScriptButton;
    QPushButton* runAnalysisButton;
    QPushButton* regenerateButton;
    QPushButton* clearOutputButton;
    QLabel* statusLabel;
    QSplitter* mainSplitter;
    QSplitter* rightSplitter;

    PythonEngine pythonEngine;
    std::string pythonScript;

public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onLoadScript();
    void onRunAnalysis();
    void onRegenerateData();
    void onClearOutput();

private:
    void setupUI();
};