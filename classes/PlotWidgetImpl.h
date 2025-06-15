// PlotWidgetImpl.h - Internal implementation with Qt headers
#pragma once
#include <random>
#include <vector>
#include "qcustomplot_wrapper.h"

class PlotWidgetImpl : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidgetImpl(QWidget* parent = nullptr);
    void generateSineData();
    void setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y);
    std::vector<double> getXData() const;
    std::vector<double> getYData() const;
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setCppFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y);
    void setPythonFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y);
    void clearFitData();


protected:
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;



private Q_SLOTS:
    void onMousePress(QMouseEvent* event);
    void onMouseWheel(QWheelEvent* event);

private:
    QCustomPlot* customPlot;
    QCPGraph* dataGraph;      // For scatter data points
    QCPGraph* fitGraph;       // For fit curve
    QCPGraph* cppFitGraph;
    QCPGraph* pythonFitGraph;
    std::vector<double> x_data;
    std::vector<double> y_data;
    std::mt19937 rng;

    // Zoom and pan state
    double initialXMin;
    double initialXMax;
    double initialYMin;
    double initialYMax;

    // UI elements
    QToolButton* zoomInButton;
    QToolButton* zoomOutButton;
    QToolButton* resetZoomButton;

    void setupPlot();
    void setupUI();
    void updateAxisRanges();
};