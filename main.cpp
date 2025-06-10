// Include pybind11 headers first
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

// Qt headers - using QT_NO_KEYWORDS so no macro conflicts
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QScreen>
#include <QtCore/QTimer>

#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>
#include <memory>
#include <iostream>
#include <streambuf>

// Custom streambuf to capture Python output
class QtOutputBuffer : public std::streambuf {
private:
    QTextEdit* textEdit;
    std::string buffer;

public:
    QtOutputBuffer(QTextEdit* edit) : textEdit(edit) {}

protected:
    virtual int_type overflow(int_type c) override {
        if (c != EOF) {
            buffer += static_cast<char>(c);
            if (c == '\n') {
                // Emit signal to update GUI in main thread
                QMetaObject::invokeMethod(textEdit, [this]() {
                    textEdit->append(QString::fromStdString(buffer.substr(0, buffer.length()-1)));
                    textEdit->ensureCursorVisible();
                }, Qt::QueuedConnection);
                buffer.clear();
            }
        }
        return c;
    }
};

// Python Engine wrapper class
class PythonEngine {
private:
    std::unique_ptr<pybind11::scoped_interpreter> guard;
    pybind11::module_ main_module;
    std::unique_ptr<QtOutputBuffer> outputBuffer;
    std::streambuf* originalCoutBuffer;
    bool initialized = false;

public:
    PythonEngine() = default;

    ~PythonEngine() {
        if (originalCoutBuffer) {
            std::cout.rdbuf(originalCoutBuffer);
        }
    }

    void setOutputWidget(QTextEdit* outputWidget) {
        outputBuffer = std::make_unique<QtOutputBuffer>(outputWidget);
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(outputBuffer.get());
    }

    void initialize() {
        if (initialized) return;

        try {
            guard = std::make_unique<pybind11::scoped_interpreter>();
            main_module = pybind11::module_::import("__main__");

            // Redirect Python stdout to our custom buffer

            // In the PythonEngine::initialize() method, replace the Python stdout redirection code with:
            pybind11::exec(R"(
import sys
import io

class QtPythonOutput:
    def __init__(self):
        self._buffer = ""

    def write(self, text):
        if text:  # Only process non-empty text
            import builtins
            # Use the built-in print function directly to avoid recursion
            builtins.print(text, end='', flush=True, file=sys.__stdout__)
        return len(text)

    def flush(self):
        pass

# Redirect Python output
sys.stdout = QtPythonOutput()
sys.stderr = QtPythonOutput()
)");

            // Add common Python paths for macOS/MacPorts
            pybind11::exec(R"(
import sys
import os

# Add common MacPorts Python paths
possible_paths = [
    '/opt/local/Library/Frameworks/Python.framework/Versions/3.11/lib/python3.11/site-packages',
    '/opt/local/Library/Frameworks/Python.framework/Versions/3.12/lib/python3.12/site-packages',
    '/opt/local/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.10/site-packages',
    '/usr/local/lib/python3.11/site-packages',
    '/usr/local/lib/python3.12/site-packages',
    '/usr/local/lib/python3.10/site-packages',
    '/opt/local/bin/python3.11',
    # Pip user install paths (this is where your packages are!)
    '/Users/morbo/Library/Python/3.11/lib/python/site-packages',
    '/Users/morbo/Library/Python/3.12/lib/python/site-packages',
    '/Users/morbo/Library/Python/3.10/lib/python/site-packages',
]

for path in possible_paths:
    if os.path.exists(path) and path not in sys.path:
        sys.path.insert(0, path)
        print(f"Added to Python path: {path}")

print("Python path configuration:")
for i, path in enumerate(sys.path[:5]):  # Show first 5 paths
    print(f"  {i}: {path}")
)");

            // Try to import essential libraries with better error handling
            pybind11::exec(R"(
try:
    import numpy as np
    print("✓ NumPy imported successfully")
except ImportError as e:
    print(f"✗ NumPy import failed: {e}")
    print("Available packages in current environment:")
    try:
        import pkg_resources
        installed = [d.project_name for d in pkg_resources.working_set]
        print(f"  Found {len(installed)} packages")
        if 'numpy' in [p.lower() for p in installed]:
            print("  - numpy is installed but not importable")
        else:
            print("  - numpy is not installed")
    except:
        print("  - Cannot check installed packages")

try:
    import scipy.optimize as opt
    print("✓ SciPy imported successfully")
except ImportError as e:
    print(f"✗ SciPy import failed: {e}")

import math
print("✓ Math module imported")
print("Python engine initialized")
)");
            initialized = true;
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to initialize Python: ") + e.what());
        }
    }

    bool isInitialized() const { return initialized; }

    void setData(const std::vector<double>& x_data, const std::vector<double>& y_data) {
        if (!initialized) initialize();
        main_module.attr("x_data") = pybind11::cast(x_data);
        main_module.attr("y_data") = pybind11::cast(y_data);
    }

    void executeScript(const std::string& script) {
        if (!initialized) initialize();
        try {
            pybind11::exec(script, main_module.attr("__dict__"));
        } catch (const pybind11::error_already_set& e) {
            std::cout << "Python Error: " << e.what() << std::endl;
            throw std::runtime_error(e.what());
        }
    }

    std::vector<double> getArray(const std::string& varName) {
        if (!initialized) return std::vector<double>();
        try {
            auto pyArray = main_module.attr(varName.c_str());
            return pyArray.cast<std::vector<double>>();
        } catch (const std::exception& e) {
            return std::vector<double>();
        }
    }

    double getScalar(const std::string& varName) {
        if (!initialized) return 0.0;
        try {
            return main_module.attr(varName.c_str()).cast<double>();
        } catch (const std::exception& e) {
            return 0.0;
        }
    }
};

// Custom widget for plotting
class PlotWidget : public QWidget {
    Q_OBJECT

private:
    std::vector<double> x_data, y_data;
    std::vector<double> fit_x, fit_y;
    bool hasFit = false;
    std::mt19937 rng;

public:
    explicit PlotWidget(QWidget* parent = nullptr) : QWidget(parent), rng(std::random_device{}()) {
        setMinimumSize(400, 300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setStyleSheet("background-color: black;");

        // Generate initial sine curve data
        generateSineData();
    }

    void generateSineData() {
        x_data.clear();
        y_data.clear();

        // Generate sine curve from 0 to 2*pi with 100 points
        const int numPoints = 100;
        std::uniform_real_distribution<double> noise(-0.1, 0.1);

        for (int i = 0; i < numPoints; i++) {
            double x = 2.0 * M_PI * i / (numPoints - 1);
            double y = sin(x) + noise(rng); // Add small noise
            x_data.push_back(x);
            y_data.push_back(y);
        }

        hasFit = false;
        update();
    }

    void setFitData(const std::vector<double>& fit_x_new, const std::vector<double>& fit_y_new) {
        fit_x = fit_x_new;
        fit_y = fit_y_new;
        hasFit = true;
        update();
    }

    std::vector<double> getXData() const { return x_data; }
    std::vector<double> getYData() const { return y_data; }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect rect = this->rect();
        if (rect.width() < 50 || rect.height() < 50) return;

        // Set up coordinate system
        int margin = 50;
        int plotWidth = rect.width() - 2 * margin;
        int plotHeight = rect.height() - 2 * margin;

        if (x_data.empty()) return;

        // Find data ranges
        auto xMinMax = std::minmax_element(x_data.begin(), x_data.end());
        auto yMinMax = std::minmax_element(y_data.begin(), y_data.end());

        double xMin = *xMinMax.first;
        double xMax = *xMinMax.second;
        double yMin = *yMinMax.first;
        double yMax = *yMinMax.second;

        // Add some padding
        double yRange = yMax - yMin;
        yMin -= yRange * 0.1;
        yMax += yRange * 0.1;

        // Draw axes
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(margin, margin, margin, margin + plotHeight);
        painter.drawLine(margin, margin + plotHeight, margin + plotWidth, margin + plotHeight);

        // Draw original data points
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QBrush(Qt::blue));

        for (size_t i = 0; i < x_data.size(); i++) {
            int x = margin + (x_data[i] - xMin) / (xMax - xMin) * plotWidth;
            int y = margin + plotHeight - (y_data[i] - yMin) / (yMax - yMin) * plotHeight;
            painter.drawEllipse(x - 2, y - 2, 4, 4);
        }

        // Draw fit line if available
        if (hasFit && !fit_x.empty()) {
            painter.setPen(QPen(Qt::red, 3));

            QPolygonF fitLine;
            for (size_t i = 0; i < fit_x.size(); i++) {
                int x = margin + (fit_x[i] - xMin) / (xMax - xMin) * plotWidth;
                int y = margin + plotHeight - (fit_y[i] - yMin) / (yMax - yMin) * plotHeight;
                fitLine << QPointF(x, y);
            }
            painter.drawPolyline(fitLine);
        }

        // Draw labels
        painter.setPen(QPen(Qt::black));
        painter.drawText(margin, 25, "Original Data (Blue) + Fitted Curve (Red)");
        painter.drawText(margin - 10, margin + plotHeight + 20, "0");
        painter.drawText(margin + plotWidth - 10, margin + plotHeight + 20, "2π");

        // Draw axis labels
        painter.drawText(10, margin + plotHeight/2, "Y");
        painter.drawText(margin + plotWidth/2, rect.height() - 10, "X");
    }
};

// Main application window
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
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Data Analysis Tool - Sine Curve Fitting with External Python Script");
        setMinimumSize(1000, 700);

        setupUI();


        // Set up Python output redirection
        pythonEngine.setOutputWidget(outputTextEdit);

        // Don't initialize Python here - wait until needed
        statusLabel->setText("Ready. Python will be initialized when needed.");
        outputTextEdit->append("=== Data Analysis Tool Output ===");
        outputTextEdit->append("Ready to load and execute Python scripts.");
        outputTextEdit->append("");

        // Center the window
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            int x = (screenGeometry.width() - width()) / 2;
            int y = (screenGeometry.height() - height()) / 2;
            move(x, y);
        }
    }

private Q_SLOTS:
    void onLoadScript() {
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

    void onRunAnalysis() {
        try {
            outputTextEdit->append("=== Starting Analysis ===");

            // Ensure Python is initialized
            if (!pythonEngine.isInitialized()) {
                statusLabel->setText("Initializing Python...");
                outputTextEdit->append("Initializing Python engine...");
                QApplication::processEvents();
                pythonEngine.initialize();
            }

            statusLabel->setText("Running Python analysis...");
            outputTextEdit->append("Running Python analysis script...");
            QApplication::processEvents(); // Update UI

            // Pass data to Python
            auto x_data = plotWidget->getXData();
            auto y_data = plotWidget->getYData();
            pythonEngine.setData(x_data, y_data);

            // Execute the Python script
            pythonEngine.executeScript(pythonScript);

            // Get results
            auto fit_x = pythonEngine.getArray("fit_x");
            auto fit_y = pythonEngine.getArray("fit_y");

            if (!fit_x.empty() && !fit_y.empty()) {
                plotWidget->setFitData(fit_x, fit_y);

                // Try to get fitted parameters for display
                double amplitude = pythonEngine.getScalar("amplitude");
                double frequency = pythonEngine.getScalar("frequency");
                double phase = pythonEngine.getScalar("phase");

                // Fixed QString formatting
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

    void onRegenerateData() {
        plotWidget->generateSineData();
        statusLabel->setText("New sine curve data generated");
        outputTextEdit->append("--- New Data Generated ---");
        outputTextEdit->append("Generated new noisy sine curve data (100 points)");
        outputTextEdit->append("");
    }

    void onClearOutput() {
        outputTextEdit->clear();
        outputTextEdit->append("=== Output Cleared ===");
        outputTextEdit->append("");
    }

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

        // Create main splitter (horizontal)
        mainSplitter = new QSplitter(Qt::Horizontal, this);

        // Create plot widget
        plotWidget = new PlotWidget(this);
        plotWidget->setMinimumSize(400, 300);

        // Create right panel with vertical splitter
        rightSplitter = new QSplitter(Qt::Vertical, this);

        // Create output text widget
        outputTextEdit = new QTextEdit(this);
        outputTextEdit->setReadOnly(true);
        outputTextEdit->setMinimumSize(300, 200);
        outputTextEdit->setMaximumHeight(300);
        outputTextEdit->setStyleSheet(
            "QTextEdit { "
            "font-family: 'Courier New', monospace; "
            "font-size: 10pt; "
            // "background-color: #f0f0f0; "
            "border: 1px solid #ccc; "
            "}"
        );

        // Create control panel
        QWidget* controlPanel = new QWidget(this);
        QVBoxLayout* controlMainLayout = new QVBoxLayout(controlPanel);
        controlMainLayout->setSpacing(10);  // Add some spacing between rows
        controlMainLayout->setContentsMargins(10, 10, 10, 10);  // Add margins


        // First row of buttons
        // First row of buttons
        QWidget* buttonRow1 = new QWidget();
        QHBoxLayout* buttonLayout1 = new QHBoxLayout(buttonRow1);
        buttonLayout1->setSpacing(10);  // Add spacing between buttons
        buttonLayout1->setContentsMargins(0, 0, 0, 0);  // Remove margins for the row


        loadScriptButton = new QPushButton("Load Python Script", this);
        runAnalysisButton = new QPushButton("Run Analysis", this);
        regenerateButton = new QPushButton("Regenerate Data", this);

        buttonLayout1->addWidget(loadScriptButton);
        buttonLayout1->addWidget(runAnalysisButton);
        buttonLayout1->addWidget(regenerateButton);
        buttonLayout1->addStretch();

        // Second row of buttons
        // Second row of buttons
        QWidget* buttonRow2 = new QWidget();
        QHBoxLayout* buttonLayout2 = new QHBoxLayout(buttonRow2);
        buttonLayout2->setSpacing(10);  // Add spacing between buttons
        buttonLayout2->setContentsMargins(0, 0, 0, 0);  // Remove margins for the row


        clearOutputButton = new QPushButton("Clear Output", this);
        buttonLayout2->addWidget(clearOutputButton);
        buttonLayout2->addStretch();

        controlMainLayout->addWidget(buttonRow1);
        controlMainLayout->addWidget(buttonRow2);

        // Add widgets to right splitter
        rightSplitter->addWidget(outputTextEdit);
        rightSplitter->addWidget(controlPanel);
        rightSplitter->setStretchFactor(0, 1); // Output gets most space
        rightSplitter->setStretchFactor(1, 0); // Control panel fixed size

        // Add to main splitter
        mainSplitter->addWidget(plotWidget);
        mainSplitter->addWidget(rightSplitter);
        mainSplitter->setStretchFactor(0, 1); // Plot widget gets more space
        mainSplitter->setStretchFactor(1, 1); // Right panel gets some space
        mainSplitter->setSizes({500, 400}); // Set initial sizes

        // Status label
        statusLabel = new QLabel("Ready. Load a Python script and run analysis.", this);
        statusLabel->setStyleSheet("padding: 5px; border: 1px solid gray;");

        // Main layout
        mainLayout->addWidget(mainSplitter);

        // Add status label to main window
        statusBar()->addWidget(statusLabel, 1);

        // Connect signals
        QObject::connect(loadScriptButton, &QPushButton::clicked, this, &MainWindow::onLoadScript);
        QObject::connect(runAnalysisButton, &QPushButton::clicked, this, &MainWindow::onRunAnalysis);
        QObject::connect(regenerateButton, &QPushButton::clicked, this, &MainWindow::onRegenerateData);
        QObject::connect(clearOutputButton, &QPushButton::clicked, this, &MainWindow::onClearOutput);
    }

};

// Application class
class DataAnalysisApp : public QApplication {
public:
    DataAnalysisApp(int argc, char* argv[]) : QApplication(argc, argv) {
        setApplicationName("Data Analysis Tool");
        setApplicationVersion("1.0");
        setOrganizationName("Data Analysis Inc.");
    }
};

// Include the MOC file for Qt's meta-object system
#include "main.moc"

// Application entry point
int main(int argc, char* argv[]) {
    try {
        DataAnalysisApp app(argc, argv);

        MainWindow window;
        window.show();

        return app.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Initialization Error",
                             QString("Failed to initialize: %1").arg(e.what()));
        return -1;
    }
}