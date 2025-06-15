// pybind11_bindings.cpp - Clean bindings with no Qt headers
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// ONLY include the wrapper header - NO Qt headers!
#include "PlotWidgetWrapper.h"

namespace py = pybind11;

PYBIND11_MODULE(plot_module, m) {
    m.doc() = "QCustomPlot-based plotting widget for Python";

    py::class_<PlotWidgetWrapper>(m, "PlotWidget")
        .def(py::init<>())
        .def("generate_sine_data", &PlotWidgetWrapper::generateSineData,
             "Generate sample sine wave data with noise")
        .def("set_fit_data", &PlotWidgetWrapper::setFitData,
             "Set the fit curve data",
             py::arg("fit_x"), py::arg("fit_y"))
        .def("get_x_data", &PlotWidgetWrapper::getXData,
             "Get the X data points")
        .def("get_y_data", &PlotWidgetWrapper::getYData,
             "Get the Y data points")
        .def("zoom_in", &PlotWidgetWrapper::zoomIn,
             "Zoom in on the plot")
        .def("zoom_out", &PlotWidgetWrapper::zoomOut,
             "Zoom out on the plot")
        .def("reset_zoom", &PlotWidgetWrapper::resetZoom,
             "Reset zoom to initial view")
        .def("show", &PlotWidgetWrapper::show,
             "Show the widget")
        .def("hide", &PlotWidgetWrapper::hide,
             "Hide the widget")
        .def("get_native_handle", &PlotWidgetWrapper::getNativeHandle,
             "Get the native Qt widget handle (for advanced integration)",
             py::return_value_policy::reference_internal);
}