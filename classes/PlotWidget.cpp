#include "../classes/PlotWidget.h"
#include <QtGui/QPainter>
#include <algorithm>
#include <cmath>

PlotWidget::PlotWidget(QWidget* parent) : QWidget(parent), rng(std::random_device{}()) {
    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: black;");

    generateSineData();
}

void PlotWidget::generateSineData() {
    x_data.clear();
    y_data.clear();

    const int numPoints = 100;
    std::uniform_real_distribution<double> noise(-0.1, 0.1);

    for (int i = 0; i < numPoints; i++) {
        double x = 2.0 * M_PI * i / (numPoints - 1);
        double y = sin(x) + noise(rng);
        x_data.push_back(x);
        y_data.push_back(y);
    }

    hasFit = false;
    update();
}

void PlotWidget::setFitData(const std::vector<double>& fit_x_new, const std::vector<double>& fit_y_new) {
    fit_x = fit_x_new;
    fit_y = fit_y_new;
    hasFit = true;
    update();
}

std::vector<double> PlotWidget::getXData() const {
    return x_data;
}

std::vector<double> PlotWidget::getYData() const {
    return y_data;
}

void PlotWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    if (rect.width() < 50 || rect.height() < 50) return;

    int margin = 50;
    int plotWidth = rect.width() - 2 * margin;
    int plotHeight = rect.height() - 2 * margin;

    if (x_data.empty()) return;

    auto xMinMax = std::minmax_element(x_data.begin(), x_data.end());
    auto yMinMax = std::minmax_element(y_data.begin(), y_data.end());

    double xMin = *xMinMax.first;
    double xMax = *xMinMax.second;
    double yMin = *yMinMax.first;
    double yMax = *yMinMax.second;

    double yRange = yMax - yMin;
    yMin -= yRange * 0.1;
    yMax += yRange * 0.1;

    // Draw axes
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(margin, margin, margin, margin + plotHeight);
    painter.drawLine(margin, margin + plotHeight, margin + plotWidth, margin + plotHeight);

    // Draw data points
    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(QBrush(Qt::blue));

    for (size_t i = 0; i < x_data.size(); i++) {
        int x = margin + (x_data[i] - xMin) / (xMax - xMin) * plotWidth;
        int y = margin + plotHeight - (y_data[i] - yMin) / (yMax - yMin) * plotHeight;
        painter.drawEllipse(x - 2, y - 2, 4, 4);
    }

    // Draw fit line
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
    painter.drawText(10, margin + plotHeight/2, "Y");
    painter.drawText(margin + plotWidth/2, rect.height() - 10, "X");
}