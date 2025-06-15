
#include "../classes/MainWindow.h"
#include <fstream>
#include <sstream>
#include <chrono>

#include "PlotWidgetImpl.h"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Data Analysis Tool - Sine Curve Fitting with C++ and Python Comparison");
    setMinimumSize(1300, 600);

    createActions();  // Add this line
    createMenus();    // Add this line

    setupUI();

    pythonEngine.setOutputWidget(outputTextEdit);

    statusLabel->setText("Ready. Choose Python or C++ analysis, or compare both.");
    outputTextEdit->append("=== Data Analysis Tool Output ===");
    outputTextEdit->append("Ready to load and execute Python scripts or run C++ fitting.");
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

    plotWidget = new PlotWidgetImpl(this);
    plotWidget->setMinimumSize(100, 100);

    rightSplitter = new QSplitter(Qt::Vertical, this);
    QTabWidget* tabWidget = new QTabWidget(this);

    // Add script editor tab
    scriptEditor = new QTextEdit(this);
    scriptEditor->setMinimumSize(300, 200);
    scriptEditor->setStyleSheet(
        "QTextEdit { "
        "font-family: 'Courier New', monospace; "
        "font-size: 10pt; "
        "border: 1px solid #ccc; "
        "background-color: #1E1E1E; " // Dark background
        "color: #D4D4D4; "           // Light text
        "}"
    );
    tabWidget->addTab(scriptEditor, "Script");
    // Add output tab
    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setMinimumSize(300, 200);
    outputTextEdit->setStyleSheet(
        "QTextEdit { "
        "font-family: 'Courier New', monospace; "
        "font-size: 10pt; "
        "border: 1px solid #ccc; "
        "}"
    );
    tabWidget->addTab(outputTextEdit, "Output");
    // Create and attach the syntax highlighter
    pythonHighlighter = new PythonHighlighter(scriptEditor->document());

    // Replace the outputTextEdit addition with tabWidget
    rightSplitter->addWidget(tabWidget);

    // Set the output widget for the Python engine
    pythonEngine.setOutputWidget(outputTextEdit);

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
    runAnalysisButton = new QPushButton("Run Python Analysis", this);
    runCppAnalysisButton = new QPushButton("Run C++ Analysis", this);
    regenerateButton = new QPushButton("Regenerate Data", this);

    // Style the C++ button differently
    runCppAnalysisButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4CAF50; "
        "color: white; "
        "font-weight: bold; "
        "}"
    );

    buttonLayout1->addWidget(loadScriptButton);
    buttonLayout1->addWidget(runAnalysisButton);
    buttonLayout1->addWidget(runCppAnalysisButton);
    buttonLayout1->addWidget(regenerateButton);
    buttonLayout1->addStretch();

    QWidget* buttonRow2 = new QWidget();
    QHBoxLayout* buttonLayout2 = new QHBoxLayout(buttonRow2);
    buttonLayout2->setSpacing(10);
    buttonLayout2->setContentsMargins(0, 0, 0, 0);

    clearOutputButton = new QPushButton("Clear Output", this);
    saveScriptButton = new QPushButton("Save Script", this);
    compareFittingButton = new QPushButton("Compare Python vs C++", this);

    // Style the comparison button
    compareFittingButton->setStyleSheet(
        "QPushButton { "
        "background-color: #FF9800; "
        "color: white; "
        "font-weight: bold; "
        "}"
    );

    buttonLayout2->addWidget(clearOutputButton);
    buttonLayout2->addWidget(saveScriptButton);
    buttonLayout2->addWidget(compareFittingButton);
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
    QObject::connect(runCppAnalysisButton, &QPushButton::clicked, this, &MainWindow::onRunCppAnalysis);
    QObject::connect(compareFittingButton, &QPushButton::clicked, this, &MainWindow::onCompareFitting);
    QObject::connect(regenerateButton, &QPushButton::clicked, this, &MainWindow::onRegenerateData);
    QObject::connect(clearOutputButton, &QPushButton::clicked, this, &MainWindow::onClearOutput);
    QObject::connect(saveScriptButton, &QPushButton::clicked, this, &MainWindow::onSaveScript);
}

// Add new method for saving script
void MainWindow::onSaveScript() {
    pythonScript = scriptEditor->toPlainText().toStdString();
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Python Script", "", "Python files (*.py)");

    if (fileName.isEmpty()) return;

    std::ofstream file(fileName.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error", "Could not save file");
        return;
    }

    file << pythonScript;
    file.close();

    QFileInfo fileInfo(fileName);
    statusLabel->setText("Python script saved: " + fileInfo.fileName());
    outputTextEdit->append("--- Script Saved ---");
    outputTextEdit->append("File: " + fileInfo.fileName());
    outputTextEdit->append("");
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

    // Update the script editor with loaded content
    scriptEditor->setText(QString::fromStdString(pythonScript));

    QFileInfo fileInfo(fileName);
    statusLabel->setText("Python script loaded: " + fileInfo.fileName());
    outputTextEdit->append("--- Script Loaded ---");
    outputTextEdit->append("File: " + fileInfo.fileName());
    outputTextEdit->append("");
}

void MainWindow::onRunAnalysis() {
    try {
        outputTextEdit->append("=== Starting Python Analysis ===");

        // Get the current script from the editor
        QString currentScript = scriptEditor->toPlainText();
        if (currentScript.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Script editor is empty. Please load or enter a script first.");
            return;
        }

        if (!pythonEngine.isInitialized()) {
            statusLabel->setText("Initializing Python...");
            outputTextEdit->append("Initializing Python engine...");
            QApplication::processEvents();
            pythonEngine.initialize();
        }

        statusLabel->setText("Running Python analysis...");
        outputTextEdit->append("Running Python analysis script...");
        QApplication::processEvents();

        auto start_time = std::chrono::high_resolution_clock::now();

        auto x_data = plotWidget->getXData();
        auto y_data = plotWidget->getYData();
        pythonEngine.setData(x_data, y_data);
        pythonEngine.executeScript(currentScript.toStdString());

        auto end_time = std::chrono::high_resolution_clock::now();
        auto python_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        auto fit_x = pythonEngine.getArray("fit_x");
        auto fit_y = pythonEngine.getArray("fit_y");

        if (!fit_x.empty() && !fit_y.empty()) {
            // plotWidget->setFitData(fit_x, fit_y);
            plotWidget->setPythonFitData(fit_x, fit_y);

            double amplitude = pythonEngine.getScalar("amplitude");
            double frequency = pythonEngine.getScalar("frequency");
            double phase = pythonEngine.getScalar("phase");

            statusLabel->setText(QString("Python Analysis complete. Fitted: A=%1, f=%2, φ=%3 (Time: %4 µs)")
                               .arg(QString::number(amplitude, 'f', 3))
                               .arg(QString::number(frequency, 'f', 3))
                               .arg(QString::number(phase, 'f', 3))
                               .arg(python_time.count()));

            outputTextEdit->append("=== Python Results ===");
            outputTextEdit->append(QString("Amplitude: %1").arg(QString::number(amplitude, 'f', 3)));
            outputTextEdit->append(QString("Frequency: %1").arg(QString::number(frequency, 'f', 3)));
            outputTextEdit->append(QString("Phase: %1").arg(QString::number(phase, 'f', 3)));
            outputTextEdit->append(QString("Execution Time: %1 microseconds").arg(python_time.count()));
            outputTextEdit->append("Plot updated with fitted curve.");
        } else {
            statusLabel->setText("Analysis completed but no fit data returned");
            outputTextEdit->append("Warning: No fit data returned from Python script");
        }

        outputTextEdit->append("=== Python Analysis Complete ===");
        outputTextEdit->append("");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Analysis Error",
                            QString("Python Error: %1").arg(e.what()));
        statusLabel->setText("Analysis failed - check Python script");
        outputTextEdit->append("ERROR: " + QString(e.what()));
        outputTextEdit->append("");
    }
}

void MainWindow::onRunCppAnalysis() {
    try {
        outputTextEdit->append("=== Starting C++ Analysis ===");
        statusLabel->setText("Running C++ analysis...");
        QApplication::processEvents();

        runCppSineFitting();

        outputTextEdit->append("=== C++ Analysis Complete ===");
        outputTextEdit->append("");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "C++ Analysis Error",
                            QString("C++ Error: %1").arg(e.what()));
        statusLabel->setText("C++ Analysis failed");
        outputTextEdit->append("ERROR: " + QString(e.what()));
        outputTextEdit->append("");
    }
}

void MainWindow::runCppSineFitting() {
    auto x_data = plotWidget->getXData();
    auto y_data = plotWidget->getYData();

    if (x_data.empty() || y_data.empty()) {
        throw std::runtime_error("No data available for fitting");
    }

    outputTextEdit->append(QString("Processing %1 data points with C++...").arg(x_data.size()));

    CppSineFitter fitter(x_data, y_data);
    auto result = fitter.fit(300);

    // Update plot with fit results
    // plotWidget->setFitData(result.fit_x, result.fit_y);
    plotWidget->setCppFitData(result.fit_x, result.fit_y);

    // Display results
    displayCppResults(result);
}

void MainWindow::displayCppResults(const CppSineFitter::FitResult& result) {
    statusLabel->setText(QString("C++ Analysis complete. Fitted: A=%1, f=%2, φ=%3 (Time: %4 µs)")
                       .arg(QString::number(result.amplitude, 'f', 3))
                       .arg(QString::number(result.frequency, 'f', 3))
                       .arg(QString::number(result.phase, 'f', 3))
                       .arg(result.fit_time.count()));

    outputTextEdit->append("=== C++ FITTING RESULTS ===");
    outputTextEdit->append("");
    outputTextEdit->append(QString("Amplitude: %1 ± %2")
                         .arg(QString::number(result.amplitude, 'f', 4))
                         .arg(QString::number(result.param_errors[0], 'f', 4)));
    outputTextEdit->append(QString("Frequency: %1 ± %2")
                         .arg(QString::number(result.frequency, 'f', 4))
                         .arg(QString::number(result.param_errors[1], 'f', 4)));
    outputTextEdit->append(QString("Phase: %1 ± %2")
                         .arg(QString::number(result.phase, 'f', 4))
                         .arg(QString::number(result.param_errors[2], 'f', 4)));
    outputTextEdit->append(QString("Offset: %1 ± %2")
                         .arg(QString::number(result.offset, 'f', 4))
                         .arg(QString::number(result.param_errors[3], 'f', 4)));

    outputTextEdit->append("");
    outputTextEdit->append("=== FIT QUALITY ===");
    outputTextEdit->append(QString("R-squared: %1").arg(QString::number(result.r_squared, 'f', 6)));
    outputTextEdit->append(QString("RMSE: %1").arg(QString::number(result.rmse, 'f', 6)));
    outputTextEdit->append(QString("AIC: %1").arg(QString::number(result.aic, 'f', 2)));

    outputTextEdit->append("");
    outputTextEdit->append("=== PERFORMANCE ===");
    outputTextEdit->append(QString("Execution Time: %1 microseconds").arg(result.fit_time.count()));

    // Derived quantities
    double period = (result.frequency != 0) ? 2.0 * M_PI / result.frequency : std::numeric_limits<double>::infinity();
    double freq_hz = result.frequency / (2.0 * M_PI);

    outputTextEdit->append("");
    outputTextEdit->append("=== DERIVED QUANTITIES ===");
    outputTextEdit->append(QString("Period: %1").arg(QString::number(period, 'f', 4)));
    outputTextEdit->append(QString("Frequency (Hz): %1").arg(QString::number(freq_hz, 'f', 6)));
    outputTextEdit->append(QString("Phase (degrees): %1°").arg(QString::number(result.phase * 180.0 / M_PI, 'f', 2)));

    outputTextEdit->append("");
    outputTextEdit->append("C++ sine fitting completed successfully!");
}

void MainWindow::onCompareFitting() {
    plotWidget->clearFitData();

    try {
        outputTextEdit->append("=== PERFORMANCE COMPARISON: Python vs C++ ===");
        outputTextEdit->append("");

        auto x_data = plotWidget->getXData();
        auto y_data = plotWidget->getYData();

        if (x_data.empty() || y_data.empty()) {
            QMessageBox::warning(this, "Warning", "No data available. Generate data first.");
            return;
        }

        statusLabel->setText("Running performance comparison...");
        QApplication::processEvents();

        // Run C++ fitting
        outputTextEdit->append("Running C++ fitting...");
        QApplication::processEvents();

        auto cpp_start = std::chrono::high_resolution_clock::now();
        CppSineFitter fitter(x_data, y_data);
        auto cpp_result = fitter.fit(300);
        auto cpp_end = std::chrono::high_resolution_clock::now();
        auto cpp_time = std::chrono::duration_cast<std::chrono::microseconds>(cpp_end - cpp_start);

        // Run Python fitting (if available)
        std::chrono::microseconds python_time(0);
        double python_amplitude = 0, python_frequency = 0, python_phase = 0;
        bool python_success = false;
        std::vector<double> python_fit_x, python_fit_y;

        QString currentScript = scriptEditor->toPlainText();
        if (!currentScript.isEmpty()) {
            try {
                outputTextEdit->append("Running Python fitting...");
                QApplication::processEvents();

                if (!pythonEngine.isInitialized()) {
                    pythonEngine.initialize();
                }

                auto python_start = std::chrono::high_resolution_clock::now();
                pythonEngine.setData(x_data, y_data);
                pythonEngine.executeScript(currentScript.toStdString());
                auto python_end = std::chrono::high_resolution_clock::now();
                python_time = std::chrono::duration_cast<std::chrono::microseconds>(python_end - python_start);

                python_amplitude = pythonEngine.getScalar("amplitude");
                python_frequency = pythonEngine.getScalar("frequency");
                python_phase = pythonEngine.getScalar("phase");

                // Get Python fit data
                python_fit_x = pythonEngine.getArray("fit_x");
                python_fit_y = pythonEngine.getArray("fit_y");

                python_success = true;
            } catch (...) {
                outputTextEdit->append("Python fitting failed or not available");
            }
        }

        // Display comparison results
        outputTextEdit->append("");
        outputTextEdit->append("=== COMPARISON RESULTS ===");
        outputTextEdit->append("");

        outputTextEdit->append("C++ Results:");
        outputTextEdit->append(QString("  Amplitude: %1").arg(QString::number(cpp_result.amplitude, 'f', 4)));
        outputTextEdit->append(QString("  Frequency: %1").arg(QString::number(cpp_result.frequency, 'f', 4)));
        outputTextEdit->append(QString("  Phase: %1").arg(QString::number(cpp_result.phase, 'f', 4)));
        outputTextEdit->append(QString("  R²: %1").arg(QString::number(cpp_result.r_squared, 'f', 6)));
        outputTextEdit->append(QString("  Time: %1 μs").arg(cpp_time.count()));

        outputTextEdit->append("");

        if (python_success) {
            outputTextEdit->append("Python Results:");
            outputTextEdit->append(QString("  Amplitude: %1").arg(QString::number(python_amplitude, 'f', 4)));
            outputTextEdit->append(QString("  Frequency: %1").arg(QString::number(python_frequency, 'f', 4)));
            outputTextEdit->append(QString("  Phase: %1").arg(QString::number(python_phase, 'f', 4)));
            outputTextEdit->append(QString("  Time: %1 μs").arg(python_time.count()));

            // Speed comparison
            if (python_time.count() > 0) {
                double speedup = static_cast<double>(python_time.count()) / cpp_time.count();
                outputTextEdit->append("");
                outputTextEdit->append("=== PERFORMANCE ANALYSIS ===");
                outputTextEdit->append(QString("C++ is %1x faster than Python").arg(QString::number(speedup, 'f', 2)));
                outputTextEdit->append(QString("Python: %1 μs").arg(python_time.count()));
                outputTextEdit->append(QString("C++: %1 μs").arg(cpp_time.count()));

                statusLabel->setText(QString("Comparison complete: C++ is %1x faster").arg(QString::number(speedup, 'f', 1)));
            }
        } else {
            outputTextEdit->append("Python Results: Not available (load and run Python script first)");
            statusLabel->setText(QString("C++ analysis complete (%1 μs)").arg(cpp_time.count()));
        }

        // Update plot with both results
        plotWidget->setCppFitData(cpp_result.fit_x, cpp_result.fit_y);

        if (python_success && !python_fit_x.empty() && !python_fit_y.empty()) {
            plotWidget->setPythonFitData(python_fit_x, python_fit_y);
        }

        outputTextEdit->append("");
        outputTextEdit->append("=== COMPARISON COMPLETE ===");
        outputTextEdit->append("");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Comparison Error",
                            QString("Error during comparison: %1").arg(e.what()));
        statusLabel->setText("Comparison failed");
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

void MainWindow::createActions() {
    loadScriptAct = new QAction(tr("&Load Python Script..."), this);
    loadScriptAct->setShortcut(QKeySequence::Open);
    loadScriptAct->setStatusTip(tr("Load a Python script file"));
    connect(loadScriptAct, &QAction::triggered, this, &MainWindow::onLoadScript);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About Data Analysis Tool"),
            tr("Data Analysis Tool\n\n"
               "A tool for analyzing data using Python scripts and native C++ "
               "algorithms with performance comparison capabilities."));
    });
}

void MainWindow::createMenus() {
    QMenuBar* menuBar = this->menuBar();

    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(loadScriptAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}