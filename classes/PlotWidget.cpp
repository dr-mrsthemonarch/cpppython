#include "../classes/PlotWidget.h"
#include <QtCharts/QValueAxis>
#include <QtWidgets/QVBoxLayout>
#include <cmath>

PlotWidget::PlotWidget(QWidget* parent)
    : QWidget(parent)
    , rng(std::random_device{}()) {

    // Create chart
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Set chart background and style
    chart->setBackgroundBrush(QColor(45, 45, 45));  // Dark gray background
    chart->setBackgroundVisible(true);

    // Make the plot area slightly lighter than the background
    chart->setPlotAreaBackgroundBrush(QColor(60, 60, 60));
    chart->setPlotAreaBackgroundVisible(true);

    // Set text colors to white for better visibility
    chart->setTitleBrush(QColor(255, 255, 255));


    // Create series for data points and fit line
    dataSeries = new QScatterSeries();
    dataSeries->setMarkerSize(5);
    dataSeries->setColor(QColor(0, 0, 255));

    fitSeries = new QSplineSeries();
    fitSeries->setColor(QColor(255, 0, 0));

    chart->addSeries(dataSeries);
    chart->addSeries(fitSeries);

    // Create axes
    QValueAxis* axisX = new QValueAxis;
    QValueAxis* axisY = new QValueAxis;
    axisX->setTitleText("X");
    axisY->setTitleText("Y");

    chart->setAxisX(axisX, dataSeries);
    chart->setAxisY(axisY, dataSeries);
    chart->setAxisX(axisX, fitSeries);
    chart->setAxisY(axisY, fitSeries);

    // Setup layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);

    generateSineData();
}

void PlotWidget::generateSineData() {
    x_data.clear();
    y_data.clear();
    dataSeries->clear();
    fitSeries->clear();

    const int numPoints = 100;
    std::uniform_real_distribution<double> noise(-0.5, 0.1);

    for (int i = 0; i < numPoints; i++) {
        double x = (2.0 * M_PI * i / (numPoints - 1) )+noise(rng);
        double y = sin(x) + noise(rng);
        x_data.push_back(x);
        y_data.push_back(y);
        dataSeries->append(x, y);
    }

    // Update axes ranges
    chart->axes(Qt::Horizontal).first()->setRange(0, 2 * M_PI);
    chart->axes(Qt::Vertical).first()->setRange(-1.5, 1.5);
}

void PlotWidget::setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y) {
    fitSeries->clear();
    for (size_t i = 0; i < fit_x.size(); ++i) {
        fitSeries->append(fit_x[i], fit_y[i]);
    }
}

std::vector<double> PlotWidget::getXData() const {
    return x_data;
}

std::vector<double> PlotWidget::getYData() const {
    return y_data;
}