#include "../classes/MainWindow.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Data Analysis Tool - Sine Curve Fitting with External Python Script");
    setMinimumSize(1000, 700);

    setupUI();

    pythonEngine.setOutputWidget(outputTextEdit);

    statusLabel->setText("Ready. Python will be initialized when needed.");
    outputTextEdit->append("=== Data Analysis Tool Output ===");
    outputTextEdit->append("Ready to load and execute Python scripts.");
    outputTextEdit->append("");

    // Center window
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    mainSplitter = new QSplitter(Qt::Horizontal, this);

    plotWidget = new PlotWidget(this);
    plotWidget->setMinimumSize(400, 300);

    rightSplitter = new QSplitter(Qt::Vertical, this);

    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setMinimumSize(300, 200);
    outputTextEdit->setMaximumHeight(300);
    outputTextEdit->setStyleSheet(
        "QTextEdit { "
        "font-family: 'Courier New', monospace; "
        "font-size: 10pt; "
        "border: 1px solid #ccc; "
        "}"
    );

    QWidget* controlPanel = new QWidget(this);
    QVBoxLayout* controlMainLayout = new QVBoxLayout(controlPanel);
    controlMainLayout->setSpacing(10);
    controlMainLayout->setContentsMargins(10, 10, 10, 10);

    QWidget* buttonRow1 = new QWidget();
    QHBoxLayout* buttonLayout1 = new QHBoxLayout(buttonRow1);
    buttonLayout1->setSpacing(10);
    buttonLayout1->setContentsMargins(0, 0, 0, 0);

    loadScriptButton = new QPushButton("Load Python Script", this);
    runAnalysisButton = new QPushButton("Run Analysis", this);
    regenerateButton = new QPushButton("Regenerate Data", this);

    buttonLayout1->addWidget(loadScriptButton);
    buttonLayout1->addWidget(runAnalysisButton);
    buttonLayout1->addWidget(regenerateButton);
    buttonLayout1->addStretch();

    QWidget* buttonRow2 = new QWidget();
    QHBoxLayout* buttonLayout2 = new QHBoxLayout(buttonRow2);
    buttonLayout2->setSpacing(10);
    buttonLayout2->setContentsMargins(0, 0, 0, 0);

    clearOutputButton = new QPushButton("Clear Output", this);
    buttonLayout2->addWidget(clearOutputButton);
    buttonLayout2->addStretch();

    controlMainLayout->addWidget(buttonRow1);
    controlMainLayout->addWidget(buttonRow2);

    rightSplitter->addWidget(outputTextEdit);
    rightSplitter->addWidget(controlPanel);
    rightSplitter->setStretchFactor(0, 1);
    rightSplitter->setStretchFactor(1, 0);

    mainSplitter->addWidget(plotWidget);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes({500, 400});

    statusLabel = new QLabel("Ready. Load a Python script and run analysis.", this);
    statusLabel->setStyleSheet("padding: 5px; border: 1px solid gray;");

    mainLayout->addWidget(mainSplitter);

    statusBar()->addWidget(statusLabel, 1);

    QObject::connect(loadScriptButton, &QPushButton::clicked, this, &MainWindow::onLoadScript);
    QObject::connect(runAnalysisButton, &QPushButton::clicked, this, &MainWindow::onRunAnalysis);
    QObject::connect(regenerateButton, &QPushButton::clicked, this, &MainWindow::onRegenerateData);
    QObject::connect(clearOutputButton, &QPushButton::clicked, this, &MainWindow::onClearOutput);
}

void MainWindow::onLoadScript() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Choose Python script file", "", "Python files (*.py)");

    if (fileName.isEmpty()) return;

    std::ifstream file(fileName.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error", "Could not open file");
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    pythonScript = buffer.str();

    QFileInfo fileInfo(fileName);
    statusLabel->setText("Python script loaded: " + fileInfo.fileName());
    outputTextEdit->append("--- Script Loaded ---");
    outputTextEdit->append("File: " + fileInfo.fileName());
    outputTextEdit->append("");
}

void MainWindow::onRunAnalysis() {
    try {
        outputTextEdit->append("=== Starting Analysis ===");

        if (!pythonEngine.isInitialized()) {
            statusLabel->setText("Initializing Python...");
            outputTextEdit->append("Initializing Python engine...");
            QApplication::processEvents();
            pythonEngine.initialize();
        }

        statusLabel->setText("Running Python analysis...");
        outputTextEdit->append("Running Python analysis script...");
        QApplication::processEvents();

        auto x_data = plotWidget->getXData();
        auto y_data = plotWidget->getYData();
        pythonEngine.setData(x_data, y_data);
        pythonEngine.executeScript(pythonScript);

        auto fit_x = pythonEngine.getArray("fit_x");
        auto fit_y = pythonEngine.getArray("fit_y");

        if (!fit_x.empty() && !fit_y.empty()) {
            plotWidget->setFitData(fit_x, fit_y);

            double amplitude = pythonEngine.getScalar("amplitude");
            double frequency = pythonEngine.getScalar("frequency");
            double phase = pythonEngine.getScalar("phase");

            statusLabel->setText(QString("Analysis complete. Fitted: A=%1, f=%2, φ=%3")
                               .arg(QString::number(amplitude, 'f', 3))
                               .arg(QString::number(frequency, 'f', 3))
                               .arg(QString::number(phase, 'f', 3)));

            outputTextEdit->append("=== Results ===");
            outputTextEdit->append(QString("Amplitude: %1").arg(QString::number(amplitude, 'f', 3)));
            outputTextEdit->append(QString("Frequency: %1").arg(QString::number(frequency, 'f', 3)));
            outputTextEdit->append(QString("Phase: %1").arg(QString::number(phase, 'f', 3)));
            outputTextEdit->append("Plot updated with fitted curve.");
        } else {
            statusLabel->setText("Analysis completed but no fit data returned");
            outputTextEdit->append("Warning: No fit data returned from Python script");
        }

        outputTextEdit->append("=== Analysis Complete ===");
        outputTextEdit->append("");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Analysis Error",
                            QString("Python Error: %1").arg(e.what()));
        statusLabel->setText("Analysis failed - check Python script");
        outputTextEdit->append("ERROR: " + QString(e.what()));
        outputTextEdit->append("");
    }
}

void MainWindow::onRegenerateData() {
    plotWidget->generateSineData();
    statusLabel->setText("New sine curve data generated");
    outputTextEdit->append("--- New Data Generated ---");
    outputTextEdit->append("Generated new noisy sine curve data (100 points)");
    outputTextEdit->append("");
}

void MainWindow::onClearOutput() {
    outputTextEdit->clear();
    outputTextEdit->append("=== Output Cleared ===");
    outputTextEdit->append("");
}