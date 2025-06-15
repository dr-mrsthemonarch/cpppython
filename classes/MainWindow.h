
#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include "../classes/PlotWidgetWrapper.h"
#include "PythonEngine.h"
#include "PythonHighlighter.h"
#include "CppSineFitter.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    PlotWidgetImpl* plotWidget;
    QTextEdit* outputTextEdit;
    QPushButton* loadScriptButton;
    QPushButton* runAnalysisButton;
    QPushButton* regenerateButton;
    QPushButton* clearOutputButton;
    QPushButton* runCppAnalysisButton;
    QPushButton* compareFittingButton;
    QLabel* statusLabel;
    QSplitter* mainSplitter;
    QSplitter* rightSplitter;
    QTextEdit* scriptEditor;
    QPushButton* saveScriptButton;
    PythonEngine pythonEngine;
    std::string pythonScript;
    PythonHighlighter* pythonHighlighter;
    void createMenus();
    void createActions();

    // Menu actions
    QAction* loadScriptAct;
    QAction* exitAct;
    QAction* aboutAct;

public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onLoadScript();
    void onSaveScript();
    void onRunAnalysis();
    void onRunCppAnalysis();
    void onCompareFitting();
    void onRegenerateData();
    void onClearOutput();

private:
    void setupUI();
    void runCppSineFitting();
    void displayCppResults(const CppSineFitter::FitResult& result);
};