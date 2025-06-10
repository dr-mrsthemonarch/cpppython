#pragma once
#include <QtWidgets/QWidget>
#include <vector>
#include <random>

class PlotWidget : public QWidget {
    Q_OBJECT

private:
    std::vector<double> x_data, y_data;
    std::vector<double> fit_x, fit_y;
    bool hasFit = false;
    std::mt19937 rng;

public:
    explicit PlotWidget(QWidget* parent = nullptr);
    void generateSineData();
    void setFitData(const std::vector<double>& fit_x_new, const std::vector<double>& fit_y_new);
    std::vector<double> getXData() const;
    std::vector<double> getYData() const;

protected:
    void paintEvent(QPaintEvent* event) override;
};