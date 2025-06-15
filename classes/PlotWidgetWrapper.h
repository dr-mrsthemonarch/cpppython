// PlotWidgetWrapper.h - This is what gets exposed to pybind11
#pragma once
#include <vector>
#include <memory>

// Forward declaration - no Qt headers in this file!
class PlotWidgetImpl;

class PlotWidgetWrapper {
public:
    PlotWidgetWrapper();
    ~PlotWidgetWrapper();

    // Same interface as your original class
    void generateSineData();
    void setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y);
    std::vector<double> getXData() const;
    std::vector<double> getYData() const;
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void show();
    void hide();

    // Get native handle for integration with other Qt code
    void* getNativeHandle() const;

private:
    std::unique_ptr<PlotWidgetImpl> impl;
};