
#pragma once
#include <QtWidgets/QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <random>

QT_CHARTS_USE_NAMESPACE

class PlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidget(QWidget* parent = nullptr);
    void generateSineData();
    void setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y);
    std::vector<double> getXData() const;
    std::vector<double> getYData() const;

private:
    QChartView* chartView;
    QChart* chart;
    QScatterSeries* dataSeries;
    QSplineSeries* fitSeries;
    std::vector<double> x_data;
    std::vector<double> y_data;
    std::mt19937 rng;
};