#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QTimer>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void generateNewData();
    void updateNoiseFactor(int value);
    void updateFunctionType();
    void animateData();

private:
    void setupUI();
    void setupLinePlot();
    void setupScatterPlot();
    void setupMatrixHeatmap();
    void setupFunctionOverlay();
    
    // UI Components
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QVBoxLayout *mainLayout;
    
    // Chart components
    QChartView *lineChartView;
    QChartView *scatterChartView;
    QChartView *heatmapChartView;
    QChartView *overlayChartView;
    
    QChart *lineChart;
    QChart *scatterChart;
    QChart *heatmapChart;
    QChart *overlayChart;
    
    // Data series
    QLineSeries *lineSeries;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    QLineSeries *matrixData;
    QLineSeries *fittedFunction;
    
    // Controls
    QPushButton *generateButton;
    QSlider *noiseSlider;
    QPushButton *functionButton;
    QLabel *noiseLabel;
    
    // Animation
    QTimer *animationTimer;
    int animationStep;
    
    // Data
    double noiseFactor;
    int functionType;
    
    // Helper functions
    std::vector<QPointF> generateSineWave(int points, double amplitude = 1.0, double frequency = 1.0, double noise = 0.0);
    std::vector<QPointF> generateMatrixData(int size);
    std::vector<QPointF> generateFittedFunction(const std::vector<QPointF>& data);
    QPointF addNoise(const QPointF& point, double noiseFactor);
};

#endif // MAINWINDOW_H