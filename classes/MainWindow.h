
#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include "../classes/PlotWidgetWrapper.h"
#include "PythonEngine.h"
#include "PythonHighlighter.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    PlotWidgetImpl* plotWidget;
    QTextEdit* outputTextEdit;
    QPushButton* loadScriptButton;
    QPushButton* runAnalysisButton;
    QPushButton* regenerateButton;
    QPushButton* clearOutputButton;
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
    void onRegenerateData();
    void onClearOutput();

private:
    void setupUI();
};