#include "mainwindow.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QtMath>
#include <QValueAxis>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , tabWidget(nullptr)
    , noiseFactor(0.1)
    , functionType(0)
    , animationStep(0)
{
    setupUI();
    setupLinePlot();
    setupScatterPlot();
    setupMatrixHeatmap();
    setupFunctionOverlay();
    
    // Setup animation timer
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::animateData);
    animationTimer->start(100); // Update every 100ms
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    
    // Create controls
    QGroupBox *controlGroup = new QGroupBox("Controls", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);
    
    generateButton = new QPushButton("Generate New Data", this);
    functionButton = new QPushButton("Switch Function", this);
    
    noiseSlider = new QSlider(Qt::Horizontal, this);
    noiseSlider->setRange(0, 100);
    noiseSlider->setValue(10);
    noiseLabel = new QLabel("Noise: 0.10", this);
    
    controlLayout->addWidget(generateButton);
    controlLayout->addWidget(functionButton);
    controlLayout->addWidget(new QLabel("Noise:", this));
    controlLayout->addWidget(noiseSlider);
    controlLayout->addWidget(noiseLabel);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlGroup);
    
    // Create tab widget for different plot types
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    // Connect signals
    connect(generateButton, &QPushButton::clicked, this, &MainWindow::generateNewData);
    connect(functionButton, &QPushButton::clicked, this, &MainWindow::updateFunctionType);
    connect(noiseSlider, &QSlider::valueChanged, this, &MainWindow::updateNoiseFactor);
    
    setWindowTitle("Qt Charts Example - Matrix Visualization & Function Fitting");
    resize(1200, 800);
}

void MainWindow::setupLinePlot()
{
    lineChart = new QChart();
    lineChart->setTitle("Line Plot with Spline Interpolation");
    
    lineSeries = new QLineSeries();
    lineSeries->setName("Raw Data");
    lineSeries->setPen(QPen(Qt::blue, 2));
    
    splineSeries = new QSplineSeries();
    splineSeries->setName("Spline Fit");
    splineSeries->setPen(QPen(Qt::red, 3));
    
    lineChart->addSeries(lineSeries);
    lineChart->addSeries(splineSeries);
    
    // Create axes
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("X");
    axisX->setRange(0, 10);
    
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Y");
    axisY->setRange(-2, 2);
    
    lineChart->addAxis(axisX, Qt::AlignBottom);
    lineChart->addAxis(axisY, Qt::AlignLeft);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);
    splineSeries->attachAxis(axisX);
    splineSeries->attachAxis(axisY);
    
    lineChart->legend()->setVisible(true);
    
    lineChartView = new QChartView(lineChart);
    lineChartView->setRenderHint(QPainter::Antialiasing);
    
    tabWidget->addTab(lineChartView, "Line Plot");
    
    // Generate initial data
    auto data = generateSineWave(50, 1.0, 0.5, noiseFactor);
    for (const auto& point : data) {
        lineSeries->append(point);
        splineSeries->append(point.x(), sin(point.x() * 0.5)); // Clean sine for spline
    }
}

void MainWindow::setupScatterPlot()
{
    scatterChart = new QChart();
    scatterChart->setTitle("Scatter Plot with Trend Analysis");
    
    scatterSeries = new QScatterSeries();
    scatterSeries->setName("Data Points");
    scatterSeries->setMarkerSize(8);
    scatterSeries->setBrush(QBrush(Qt::blue));
    
    scatterChart->addSeries(scatterSeries);
    
    // Create axes
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("X");
    axisX->setRange(0, 100);
    
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Y");
    axisY->setRange(0, 100);
    
    scatterChart->addAxis(axisX, Qt::AlignBottom);
    scatterChart->addAxis(axisY, Qt::AlignLeft);
    scatterSeries->attachAxis(axisX);
    scatterSeries->attachAxis(axisY);
    
    scatterChart->legend()->setVisible(true);
    
    scatterChartView = new QChartView(scatterChart);
    scatterChartView->setRenderHint(QPainter::Antialiasing);
    
    tabWidget->addTab(scatterChartView, "Scatter Plot");
    
    // Generate initial scatter data
    for (int i = 0; i < 100; ++i) {
        double x = QRandomGenerator::global()->bounded(100.0);
        double y = x * 0.5 + QRandomGenerator::global()->bounded(20.0) + 10;
        scatterSeries->append(x, y);
    }
}

void MainWindow::setupMatrixHeatmap()
{
    heatmapChart = new QChart();
    heatmapChart->setTitle("Matrix Data Visualization (Simulated Heatmap)");
    
    matrixData = new QLineSeries();
    matrixData->setName("Matrix Profile");
    matrixData->setPen(QPen(Qt::darkBlue, 2));
    
    heatmapChart->addSeries(matrixData);
    
    // Create axes
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("Matrix Index");
    axisX->setRange(0, 50);
    
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Value");
    axisY->setRange(-1, 1);
    
    heatmapChart->addAxis(axisX, Qt::AlignBottom);
    heatmapChart->addAxis(axisY, Qt::AlignLeft);
    matrixData->attachAxis(axisX);
    matrixData->attachAxis(axisY);
    
    heatmapChart->legend()->setVisible(true);
    
    heatmapChartView = new QChartView(heatmapChart);
    heatmapChartView->setRenderHint(QPainter::Antialiasing);
    
    tabWidget->addTab(heatmapChartView, "Matrix Heatmap");
    
    // Generate initial matrix data (simulating 2D matrix as 1D profile)
    auto matrixPoints = generateMatrixData(50);
    for (const auto& point : matrixPoints) {
        matrixData->append(point);
    }
}

void MainWindow::setupFunctionOverlay()
{
    overlayChart = new QChart();
    overlayChart->setTitle("Data with Fitted Function Overlay");
    
    // Raw data series
    QScatterSeries *rawDataSeries = new QScatterSeries();
    rawDataSeries->setName("Raw Data");
    rawDataSeries->setMarkerSize(6);
    rawDataSeries->setBrush(QBrush(Qt::blue));
    
    // Fitted function series
    fittedFunction = new QLineSeries();
    fittedFunction->setName("Fitted Function");
    fittedFunction->setPen(QPen(Qt::red, 3));
    
    overlayChart->addSeries(rawDataSeries);
    overlayChart->addSeries(fittedFunction);
    
    // Create axes
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("X");
    axisX->setRange(0, 10);
    
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Y");
    axisY->setRange(-3, 3);
    
    overlayChart->addAxis(axisX, Qt::AlignBottom);
    overlayChart->addAxis(axisY, Qt::AlignLeft);
    rawDataSeries->attachAxis(axisX);
    rawDataSeries->attachAxis(axisY);
    fittedFunction->attachAxis(axisX);
    fittedFunction->attachAxis(axisY);
    
    overlayChart->legend()->setVisible(true);
    
    overlayChartView = new QChartView(overlayChart);
    overlayChartView->setRenderHint(QPainter::Antialiasing);
    
    tabWidget->addTab(overlayChartView, "Function Overlay");
    
    // Generate initial data with fit
    auto rawData = generateSineWave(30, 1.5, 0.3, 0.3);
    for (const auto& point : rawData) {
        rawDataSeries->append(point);
    }
    
    auto fittedData = generateFittedFunction(rawData);
    for (const auto& point : fittedData) {
        fittedFunction->append(point);
    }
}

std::vector<QPointF> MainWindow::generateSineWave(int points, double amplitude, double frequency, double noise)
{
    std::vector<QPointF> data;
    for (int i = 0; i < points; ++i) {
        double x = (double)i / points * 10.0;
        double y = amplitude * sin(frequency * x);
        if (noise > 0) {
            y += (QRandomGenerator::global()->bounded(2.0) - 1.0) * noise;
        }
        data.push_back(QPointF(x, y));
    }
    return data;
}

std::vector<QPointF> MainWindow::generateMatrixData(int size)
{
    std::vector<QPointF> data;
    for (int i = 0; i < size; ++i) {
        double x = i;
        // Simulate matrix row/column profile with some complexity
        double y = sin(i * 0.2) * cos(i * 0.1) + 0.3 * sin(i * 0.05);
        data.push_back(QPointF(x, y));
    }
    return data;
}

std::vector<QPointF> MainWindow::generateFittedFunction(const std::vector<QPointF>& data)
{
    std::vector<QPointF> fitted;
    
    // Simple polynomial fit simulation
    for (int i = 0; i <= 100; ++i) {
        double x = (double)i / 100.0 * 10.0;
        double y;
        
        if (functionType == 0) {
            y = 1.2 * sin(0.3 * x); // Sine fit
        } else if (functionType == 1) {
            y = 0.1 * x * x - x + 1; // Quadratic fit
        } else {
            y = 1.5 * exp(-0.2 * x) * sin(x); // Damped oscillation
        }
        
        fitted.push_back(QPointF(x, y));
    }
    
    return fitted;
}

QPointF MainWindow::addNoise(const QPointF& point, double noiseFactor)
{
    double noise = (QRandomGenerator::global()->bounded(2.0) - 1.0) * noiseFactor;
    return QPointF(point.x(), point.y() + noise);
}

void MainWindow::generateNewData()
{
    // Clear and regenerate all plots
    lineSeries->clear();
    splineSeries->clear();
    scatterSeries->clear();
    matrixData->clear();
    
    // Regenerate line plot
    auto lineData = generateSineWave(50, 1.0, 0.5, noiseFactor);
    for (const auto& point : lineData) {
        lineSeries->append(point);
        splineSeries->append(point.x(), sin(point.x() * 0.5));
    }
    
    // Regenerate scatter plot
    for (int i = 0; i < 100; ++i) {
        double x = QRandomGenerator::global()->bounded(100.0);
        double y = x * 0.5 + QRandomGenerator::global()->bounded(20.0) + 10;
        scatterSeries->append(x, y);
    }
    
    // Regenerate matrix data
    auto matrixPoints = generateMatrixData(50);
    for (const auto& point : matrixPoints) {
        matrixData->append(point);
    }
    
    // Update function overlay
    auto overlayRawSeries = dynamic_cast<QScatterSeries*>(overlayChart->series()[0]);
    if (overlayRawSeries) {
        overlayRawSeries->clear();
        auto rawData = generateSineWave(30, 1.5, 0.3, noiseFactor);
        for (const auto& point : rawData) {
            overlayRawSeries->append(point);
        }
        
        fittedFunction->clear();
        auto fittedData = generateFittedFunction(rawData);
        for (const auto& point : fittedData) {
            fittedFunction->append(point);
        }
    }
}

void MainWindow::updateNoiseFactor(int value)
{
    noiseFactor = value / 100.0;
    noiseLabel->setText(QString("Noise: %1").arg(noiseFactor, 0, 'f', 2));
}

void MainWindow::updateFunctionType()
{
    functionType = (functionType + 1) % 3;
    
    QString functionName;
    switch (functionType) {
        case 0: functionName = "Sine Fit"; break;
        case 1: functionName = "Quadratic Fit"; break;
        case 2: functionName = "Damped Oscillation"; break;
    }
    
    functionButton->setText(QString("Function: %1").arg(functionName));
    
    // Update the fitted function
    if (fittedFunction) {
        auto overlayRawSeries = dynamic_cast<QScatterSeries*>(overlayChart->series()[0]);
        if (overlayRawSeries && overlayRawSeries->count() > 0) {
            std::vector<QPointF> rawData;
            for (int i = 0; i < overlayRawSeries->count(); ++i) {
                rawData.push_back(overlayRawSeries->at(i));
            }
            
            fittedFunction->clear();
            auto fittedData = generateFittedFunction(rawData);
            for (const auto& point : fittedData) {
                fittedFunction->append(point);
            }
        }
    }
}

void MainWindow::animateData()
{
    // Animate the matrix data to show dynamic updates
    if (matrixData && matrixData->count() > 0) {
        animationStep++;
        
        // Update a few points to create animation effect
        int pointsToUpdate = 3;
        for (int i = 0; i < pointsToUpdate; ++i) {
            int index = (animationStep + i * 10) % matrixData->count();
            QPointF oldPoint = matrixData->at(index);
            double newY = sin(oldPoint.x() * 0.2 + animationStep * 0.1) * 
                         cos(oldPoint.x() * 0.1 + animationStep * 0.05) + 
                         0.3 * sin(oldPoint.x() * 0.05 + animationStep * 0.02);
            matrixData->replace(index, QPointF(oldPoint.x(), newY));
        }
    }
}