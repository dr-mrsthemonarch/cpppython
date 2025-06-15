// PlotWidgetImpl.cpp - Qt/QCustomPlot implementation
#include "PlotWidgetImpl.h"
#include <cmath>
#include <QApplication>

PlotWidgetImpl::PlotWidgetImpl(QWidget* parent)
    : QWidget(parent)
    , rng(std::random_device{}()) {

    setupUI();
    setupPlot();

    // Set initial ranges
    initialXMin = 0;
    initialXMax = 2.5 * M_PI;
    initialYMin = -2;
    initialYMax = 2;

    generateSineData();
}

void PlotWidgetImpl::setupUI() {
    // Create control buttons
    zoomInButton = new QToolButton(this);
    zoomInButton->setText("+");
    QObject::connect(zoomInButton, &QToolButton::clicked, this, &PlotWidgetImpl::zoomIn);

    zoomOutButton = new QToolButton(this);
    zoomOutButton->setText("-");
    QObject::connect(zoomOutButton, &QToolButton::clicked, this, &PlotWidgetImpl::zoomOut);

    resetZoomButton = new QToolButton(this);
    resetZoomButton->setText("Reset");
    QObject::connect(resetZoomButton, &QToolButton::clicked, this, &PlotWidgetImpl::resetZoom);

    // Create layouts
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(zoomInButton);
    buttonLayout->addWidget(zoomOutButton);
    buttonLayout->addWidget(resetZoomButton);
    buttonLayout->addStretch();  // Push buttons to the left

    // Create the main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);

    // Create QCustomPlot widget
    customPlot = new QCustomPlot(this);
    mainLayout->addWidget(customPlot);

    setLayout(mainLayout);
}

void PlotWidgetImpl::setupPlot() {
    // Set dark theme
    customPlot->setBackground(QColor(45, 45, 45));
    customPlot->axisRect()->setBackground(QColor(60, 60, 60));

    // Configure axes
    customPlot->xAxis->setBasePen(QPen(QColor(255, 255, 255)));
    customPlot->yAxis->setBasePen(QPen(QColor(255, 255, 255)));
    customPlot->xAxis->setTickPen(QPen(QColor(255, 255, 255)));
    customPlot->yAxis->setTickPen(QPen(QColor(255, 255, 255)));
    customPlot->xAxis->setSubTickPen(QPen(QColor(255, 255, 255)));
    customPlot->yAxis->setSubTickPen(QPen(QColor(255, 255, 255)));
    customPlot->xAxis->setTickLabelColor(QColor(255, 255, 255));
    customPlot->yAxis->setTickLabelColor(QColor(255, 255, 255));
    customPlot->xAxis->setLabelColor(QColor(255, 255, 255));
    customPlot->yAxis->setLabelColor(QColor(255, 255, 255));

    // Set axis labels
    customPlot->xAxis->setLabel("X");
    customPlot->yAxis->setLabel("Y");

    // Enable grid
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setVisible(true);
    customPlot->xAxis->grid()->setPen(QPen(QColor(80, 80, 80)));
    customPlot->yAxis->grid()->setPen(QPen(QColor(80, 80, 80)));

    // Create graphs
    dataGraph = customPlot->addGraph();
    dataGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(0, 0, 255), QColor(0, 0, 255), 5));
    dataGraph->setLineStyle(QCPGraph::lsNone);  // Only show scatter points

    // Keep original fitGraph for backward compatibility
    fitGraph = customPlot->addGraph();
    fitGraph->setPen(QPen(QColor(255, 0, 0), 2));  // Red line, 2px width

    // Add new graphs for C++ and Python fits with custom dash patterns
    cppFitGraph = customPlot->addGraph();
    QPen cppPen(QColor(0, 255, 0), 3);  // Green line, 3px width
    // Create custom dash pattern: dash(10), space(5), dot(2), space(5)
    QVector<qreal> cppDashPattern;
    cppDashPattern << 10 << 5 << 2 << 5;
    cppPen.setDashPattern(cppDashPattern);
    cppPen.setDashOffset(0);  // No offset for C++
    cppFitGraph->setPen(cppPen);
    cppFitGraph->setName("C++ Fit");

    pythonFitGraph = customPlot->addGraph();
    QPen pythonPen(QColor(255, 0, 0), 3);  // Red line, 3px width
    // Create custom dash pattern: dash(8), space(4), dot(2), space(4)
    QVector<qreal> pythonDashPattern;
    pythonDashPattern << 8 << 4 << 2 << 4;
    pythonPen.setDashPattern(pythonDashPattern);
    pythonPen.setDashOffset(7);  // Offset by 7 pixels to phase-shift the pattern
    pythonFitGraph->setPen(pythonPen);
    pythonFitGraph->setName("Python Fit");

    // Enable interactions
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    // Connect mouse events using Qt5 syntax
    QObject::connect(customPlot, &QCustomPlot::mousePress, this, &PlotWidgetImpl::onMousePress);
    QObject::connect(customPlot, &QCustomPlot::mouseWheel, this, &PlotWidgetImpl::onMouseWheel);
}

void PlotWidgetImpl::setCppFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y) {
    QVector<double> x_vec, y_vec;

    for (size_t i = 0; i < fit_x.size(); ++i) {
        x_vec.append(fit_x[i]);
        y_vec.append(fit_y[i]);
    }

    cppFitGraph->setData(x_vec, y_vec);
    customPlot->replot();
}

void PlotWidgetImpl::setPythonFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y) {
    QVector<double> x_vec, y_vec;

    for (size_t i = 0; i < fit_x.size(); ++i) {
        x_vec.append(fit_x[i]);
        y_vec.append(fit_y[i]);
    }

    pythonFitGraph->setData(x_vec, y_vec);
    customPlot->replot();
}

void PlotWidgetImpl::clearFitData() {
    cppFitGraph->data()->clear();
    pythonFitGraph->data()->clear();
    fitGraph->data()->clear();
    customPlot->replot();
}


void PlotWidgetImpl::generateSineData() {
    x_data.clear();
    y_data.clear();

    QVector<double> x_vec, y_vec;

    const int numPoints = 100;

    // More reasonable random distributions
    std::uniform_real_distribution<double> amplitude_dist(0.8, 2.2);     // Moderate amplitude changes
    std::uniform_real_distribution<double> frequency_dist(0.8, 2.5);     // Reasonable frequency variations
    std::uniform_real_distribution<double> phase_dist(0.0, 2.0 * M_PI);  // Random phase shifts
    std::uniform_real_distribution<double> noise_dist(-0.8, 0.8);        // Stronger but reasonable noise
    std::uniform_real_distribution<double> offset_dist(-1.0, 1.0);       // Moderate DC offset

    // Generate fewer segments with smoother transitions
    const int segments = 3;  // Fewer segments for more coherent sine waves
    const int pointsPerSegment = numPoints / segments;

    // Initialize with base parameters
    double prev_amplitude = amplitude_dist(rng);
    double prev_frequency = frequency_dist(rng);
    double prev_phase = phase_dist(rng);
    double prev_offset = offset_dist(rng);

    for (int seg = 0; seg < segments; ++seg) {
        // Gradually change parameters for smoother transitions
        double amplitude = prev_amplitude + std::uniform_real_distribution<double>(-0.5, 0.5)(rng);
        double frequency = prev_frequency + std::uniform_real_distribution<double>(-0.3, 0.3)(rng);
        double phase = prev_phase + std::uniform_real_distribution<double>(-M_PI/2, M_PI/2)(rng);
        double offset = prev_offset + std::uniform_real_distribution<double>(-0.5, 0.5)(rng);

        // Keep parameters within reasonable bounds
        amplitude = std::max(0.5, std::min(3.0, amplitude));
        frequency = std::max(0.5, std::min(3.0, frequency));
        offset = std::max(-2.0, std::min(2.0, offset));

        int startIdx = seg * pointsPerSegment;
        int endIdx = (seg == segments - 1) ? numPoints : (seg + 1) * pointsPerSegment;

        for (int i = startIdx; i < endIdx; ++i) {
            // Smooth x progression with minimal randomness
            double x_base = (2.0 * M_PI * i / (numPoints - 1));
            double x_noise = noise_dist(rng) * 0.05;  // Very small x-axis noise
            double x = x_base + x_noise;

            // Main sine wave with current parameters
            double y_base = amplitude * sin(frequency * x + phase) + offset;

            // Add reasonable noise
            double y_noise = noise_dist(rng) * 0.3;  // Moderate noise level

            // Occasional small spikes (much less frequent and smaller)
            if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < 0.05) {
                y_noise += std::uniform_real_distribution<double>(-1.5, 1.5)(rng);
            }

            double y = y_base + y_noise;

            x_data.push_back(x);
            y_data.push_back(y);
            x_vec.append(x);
            y_vec.append(y);
        }

        // Update previous parameters for next segment
        prev_amplitude = amplitude;
        prev_frequency = frequency;
        prev_phase = phase;
        prev_offset = offset;
    }

    // Set data to the scatter graph
    dataGraph->setData(x_vec, y_vec);

    updateAxisRanges();
    customPlot->replot();
}

void PlotWidgetImpl::setFitData(const std::vector<double>& fit_x, const std::vector<double>& fit_y) {
    QVector<double> x_vec, y_vec;

    for (size_t i = 0; i < fit_x.size(); ++i) {
        x_vec.append(fit_x[i]);
        y_vec.append(fit_y[i]);
    }

    fitGraph->setData(x_vec, y_vec);
    customPlot->replot();
}


std::vector<double> PlotWidgetImpl::getXData() const {
    return x_data;
}

std::vector<double> PlotWidgetImpl::getYData() const {
    return y_data;
}

void PlotWidgetImpl::zoomIn() {
    // Scale both axes by 0.8 (zoom in by reducing the range by 20%)
    customPlot->xAxis->scaleRange(0.8);
    customPlot->yAxis->scaleRange(0.8);
    customPlot->replot();
}


void PlotWidgetImpl::zoomOut() {
    // Scale both axes by 1.2 (zoom out by increasing the range by 20%)
    customPlot->xAxis->scaleRange(1.2);
    customPlot->yAxis->scaleRange(1.2);
    customPlot->replot();
}


void PlotWidgetImpl::resetZoom() {
    customPlot->xAxis->setRange(initialXMin, initialXMax);
    customPlot->yAxis->setRange(initialYMin, initialYMax);
    customPlot->replot();
}

void PlotWidgetImpl::updateAxisRanges() {
    customPlot->xAxis->setRange(initialXMin, initialXMax);
    customPlot->yAxis->setRange(initialYMin, initialYMax);
}

void PlotWidgetImpl::mousePressEvent(QMouseEvent* event) {
    // Handle middle mouse button or Ctrl+Left click for panning
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        // QCustomPlot handles panning automatically with iRangeDrag interaction
        customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
    QWidget::mousePressEvent(event);
}

void PlotWidgetImpl::wheelEvent(QWheelEvent* event) {
    // Forward wheel events to QCustomPlot for zooming
    QApplication::sendEvent(customPlot, event);
}

void PlotWidgetImpl::onMousePress(QMouseEvent* event) {
    // Handle custom mouse press logic if needed
    // QCustomPlot automatically handles selection and dragging
    if (event->button() == Qt::RightButton) {
        // Right click could trigger a context menu in the future
    }
}

void PlotWidgetImpl::onMouseWheel(QWheelEvent* event) {
    // Custom wheel handling if needed
    // QCustomPlot handles zooming automatically with iRangeZoom interaction
}

#include "PlotWidgetImpl.moc"  // Include the MOC file