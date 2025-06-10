#include "../classes/PythonEngine.h"
#include <stdexcept>
#include <iostream>

PythonEngine::PythonEngine() = default;

PythonEngine::~PythonEngine() {
    if (originalCoutBuffer) {
        std::cout.rdbuf(originalCoutBuffer);
    }
}

void PythonEngine::setOutputWidget(QTextEdit* outputWidget) {
    outputBuffer = std::make_unique<QtOutputBuffer>(outputWidget);
    originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.get());
}

void PythonEngine::initialize() {
    if (initialized) return;

    try {
        guard = std::make_unique<pybind11::scoped_interpreter>();
        main_module = pybind11::module_::import("__main__");

        // Redirect Python stdout to our custom buffer
        pybind11::exec(R"(
import sys
import io

class QtPythonOutput:
    def __init__(self):
        self._buffer = ""

    def write(self, text):
        if text:
            import builtins
            builtins.print(text, end='', flush=True, file=sys.__stdout__)
        return len(text)

    def flush(self):
        pass

sys.stdout = QtPythonOutput()
sys.stderr = QtPythonOutput()
)");

        // Add common Python paths and initialize required modules
        pybind11::exec(R"(
import sys
import os

possible_paths = [
    '/opt/local/Library/Frameworks/Python.framework/Versions/3.11/lib/python3.11/site-packages',
    '/opt/local/Library/Frameworks/Python.framework/Versions/3.12/lib/python3.12/site-packages',
    '/usr/local/lib/python3.11/site-packages',
    '/usr/local/lib/python3.12/site-packages',
    '/Users/morbo/Library/Python/3.11/lib/python/site-packages',
    '/Users/morbo/Library/Python/3.12/lib/python/site-packages',
]

for path in possible_paths:
    if os.path.exists(path) and path not in sys.path:
        sys.path.insert(0, path)
        print(f"Added to Python path: {path}")

try:
    import numpy as np
    print("✓ NumPy imported successfully")
except ImportError as e:
    print(f"✗ NumPy import failed: {e}")

try:
    import scipy.optimize as opt
    print("✓ SciPy imported successfully")
except ImportError as e:
    print(f"✗ SciPy import failed: {e}")

import math
print("✓ Math module imported")
print("Python engine initialized")
)");
        initialized = true;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to initialize Python: ") + e.what());
    }
}

bool PythonEngine::isInitialized() const { 
    return initialized; 
}

void PythonEngine::setData(const std::vector<double>& x_data, const std::vector<double>& y_data) {
    if (!initialized) initialize();
    main_module.attr("x_data") = pybind11::cast(x_data);
    main_module.attr("y_data") = pybind11::cast(y_data);
}

void PythonEngine::executeScript(const std::string& script) {
    if (!initialized) initialize();
    try {
        pybind11::exec(script, main_module.attr("__dict__"));
    } catch (const pybind11::error_already_set& e) {
        std::cout << "Python Error: " << e.what() << std::endl;
        throw std::runtime_error(e.what());
    }
}

std::vector<double> PythonEngine::getArray(const std::string& varName) {
    if (!initialized) return std::vector<double>();
    try {
        auto pyArray = main_module.attr(varName.c_str());
        return pyArray.cast<std::vector<double>>();
    } catch (const std::exception& e) {
        return std::vector<double>();
    }
}

double PythonEngine::getScalar(const std::string& varName) {
    if (!initialized) return 0.0;
    try {
        return main_module.attr(varName.c_str()).cast<double>();
    } catch (const std::exception& e) {
        return 0.0;
    }
}