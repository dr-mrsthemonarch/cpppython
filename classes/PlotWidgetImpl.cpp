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
    initialXMax = 2 * M_PI;
    initialYMin = -1.5;
    initialYMax = 1.5;

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

    fitGraph = customPlot->addGraph();
    fitGraph->setPen(QPen(QColor(255, 0, 0), 2));  // Red line, 2px width

    // Enable interactions
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    // Connect mouse events using Qt5 syntax
    QObject::connect(customPlot, &QCustomPlot::mousePress, this, &PlotWidgetImpl::onMousePress);
    QObject::connect(customPlot, &QCustomPlot::mouseWheel, this, &PlotWidgetImpl::onMouseWheel);
}

void PlotWidgetImpl::generateSineData() {
    x_data.clear();
    y_data.clear();

    QVector<double> x_vec, y_vec;

    const int numPoints = 100;
    std::uniform_real_distribution<double> noise(-0.5, 0.1);

    for (int i = 0; i < numPoints; i++) {
        double x = (2.0 * M_PI * i / (numPoints - 1)) + noise(rng);
        double y = sin(x) + noise(rng);
        x_data.push_back(x);
        y_data.push_back(y);
        x_vec.append(x);
        y_vec.append(y);
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