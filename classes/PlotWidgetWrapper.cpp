// PlotWidgetWrapper.cpp - Qt-free wrapper implementation
#include "PlotWidgetWrapper.h"
#include "PlotWidgetImpl.h"

PlotWidgetWrapper::PlotWidgetWrapper()
    : impl(std::make_unique<PlotWidgetImpl>()) {
}

PlotWidgetWrapper::~PlotWidgetWrapper() {
    // std::unique_ptr automatically handles deletion
}

void PlotWidgetWrapper::generateSineData() {
    impl->generateSineData();
}

void PlotWidgetWrapper::setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y) {
    impl->setFitData(fit_x, fit_y);
}

std::vector<double> PlotWidgetWrapper::getXData() const {
    return impl->getXData();
}

std::vector<double> PlotWidgetWrapper::getYData() const {
    return impl->getYData();
}

void PlotWidgetWrapper::zoomIn() {
    impl->zoomIn();
}

void PlotWidgetWrapper::zoomOut() {
    impl->zoomOut();
}

void PlotWidgetWrapper::resetZoom() {
    impl->resetZoom();
}

void PlotWidgetWrapper::show() {
    impl->show();
}

void PlotWidgetWrapper::hide() {
    impl->hide();
}

void* PlotWidgetWrapper::getNativeHandle() const {
    return static_cast<void*>(impl.get());
}