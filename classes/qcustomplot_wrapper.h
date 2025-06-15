// qcustomplot_wrapper.h - Wrapper to handle Qt keywords conflict
#pragma once

// Temporarily define Qt keywords if QT_NO_KEYWORDS is defined
#ifdef QT_NO_KEYWORDS
#define signals public
#define slots
#define emit
#define foreach Q_FOREACH
#define QCUSTOMPLOT_KEYWORDS_DEFINED
#endif

// Include the actual QCustomPlot header
#include "qcustomplot.h"

// Clean up our temporary definitions
#ifdef QCUSTOMPLOT_KEYWORDS_DEFINED
#undef signals
#undef slots
#undef emit
#undef foreach
#undef QCUSTOMPLOT_KEYWORDS_DEFINED
#endif