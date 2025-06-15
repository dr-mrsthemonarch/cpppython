
#include "../classes/PythonEngine.h"
#include <stdexcept>
#include <iostream>

PythonEngine::PythonEngine() = default;

PythonEngine::~PythonEngine() {
    // No need to restore cout/cerr buffers since we're not redirecting them
}

void PythonEngine::setOutputWidget(QTextEdit* outputWidget) {
    this->outputWidget = outputWidget;
}

void PythonEngine::initialize() {
    if (initialized) return;

    try {
        guard = std::make_unique<pybind11::scoped_interpreter>();
        main_module = pybind11::module_::import("__main__");

        // Create a Python class that captures output and sends it to Qt
        pybind11::exec(R"(
import sys
import io

class QtOutputCapture:
    def __init__(self):
        self.output_buffer = []

    def write(self, text):
        if text and text.strip():  # Only capture non-empty text
            self.output_buffer.append(text)
        return len(text) if text else 0

    def flush(self):
        if self.output_buffer:
            # Join all buffered output and send it
            full_text = ''.join(self.output_buffer)
            self.output_buffer.clear()
            # This will be called from C++ to get the output
            return full_text
        return ""

    def get_and_clear_output(self):
        if self.output_buffer:
            full_text = ''.join(self.output_buffer)
            self.output_buffer.clear()
            return full_text
        return ""

# Create the capture instance
_qt_output_capture = QtOutputCapture()

# Store original streams
_original_stdout = sys.stdout
_original_stderr = sys.stderr

# Redirect Python output to our capture
sys.stdout = _qt_output_capture
sys.stderr = _qt_output_capture

print("Python output capture initialized successfully")
)");

        // Cross-platform Python path discovery and module initialization
        pybind11::exec(R"(
import sys
import os
import site
import platform
from pathlib import Path

def discover_python_paths():
    """Discover Python paths in a cross-platform way"""
    paths_to_add = []

    # Get system information
    system = platform.system().lower()
    python_version = f"{sys.version_info.major}.{sys.version_info.minor}"

    print(f"System: {system}")
    print(f"Python version: {python_version}")
    print(f"Python executable: {sys.executable}")

    # Use site module to get user site packages (cross-platform)
    try:
        user_site = site.getusersitepackages()
        if user_site and os.path.exists(user_site):
            paths_to_add.append(user_site)
            print(f"✓ Found user site packages: {user_site}")
    except Exception as e:
        print(f"⚠ Could not get user site packages: {e}")

    # Get site packages from site module (system-wide)
    try:
        site_packages = site.getsitepackages()
        for path in site_packages:
            if os.path.exists(path):
                paths_to_add.append(path)
                print(f"✓ Found site packages: {path}")
    except Exception as e:
        print(f"⚠ Could not get site packages: {e}")

    # Platform-specific additional paths
    if system == "windows":
        # Windows-specific paths
        possible_paths = [
            f"C:\\Python{python_version.replace('.', '')}\\Lib\\site-packages",
            f"C:\\Users\\{os.environ.get('USERNAME', 'user')}\\AppData\\Local\\Programs\\Python\\Python{python_version.replace('.', '')}\\Lib\\site-packages",
            f"C:\\Users\\{os.environ.get('USERNAME', 'user')}\\AppData\\Roaming\\Python\\Python{python_version.replace('.', '')}\\site-packages",
        ]
    elif system == "darwin":  # macOS
        home_dir = os.path.expanduser("~")
        possible_paths = [
            f"/opt/local/Library/Frameworks/Python.framework/Versions/{python_version}/lib/python{python_version}/site-packages",
            f"/usr/local/lib/python{python_version}/site-packages",
            f"{home_dir}/Library/Python/{python_version}/lib/python/site-packages",
            f"/System/Library/Frameworks/Python.framework/Versions/{python_version}/lib/python{python_version}/site-packages",
            # Homebrew paths
            f"/opt/homebrew/lib/python{python_version}/site-packages",
            f"/usr/local/Cellar/python@{python_version}/*/lib/python{python_version}/site-packages",
        ]
    else:  # Linux and other Unix-like systems
        home_dir = os.path.expanduser("~")
        possible_paths = [
            f"/usr/lib/python{python_version}/site-packages",
            f"/usr/local/lib/python{python_version}/site-packages",
            f"/usr/lib/python{python_version}/dist-packages",  # Ubuntu/Debian
            f"{home_dir}/.local/lib/python{python_version}/site-packages",
            f"/opt/python{python_version}/lib/python{python_version}/site-packages",
        ]

    # Check additional paths
    for path in possible_paths:
        # Handle glob patterns for paths like Homebrew
        if '*' in path:
            from glob import glob
            matching_paths = glob(path)
            for matching_path in matching_paths:
                if os.path.exists(matching_path):
                    paths_to_add.append(matching_path)
                    print(f"✓ Found additional path: {matching_path}")
        else:
            if os.path.exists(path):
                paths_to_add.append(path)
                print(f"✓ Found additional path: {path}")

    return paths_to_add

def add_python_paths():
    """Add discovered Python paths to sys.path"""
    discovered_paths = discover_python_paths()
    paths_added = 0

    for path in discovered_paths:
        if path not in sys.path:
            sys.path.insert(0, path)
            paths_added += 1

    if paths_added == 0:
        print("ℹ No additional Python paths needed to be added")
    else:
        print(f"✓ Added {paths_added} paths to Python path")

    return paths_added

def check_essential_modules():
    """Check for essential modules in a cross-platform way"""
    modules_status = []

    # NumPy
    try:
        import numpy as np
        modules_status.append("✓ NumPy imported successfully")
        modules_status.append(f"  NumPy version: {np.__version__}")
    except ImportError as e:
        modules_status.append(f"✗ NumPy import failed: {e}")
        modules_status.append("  Please install NumPy: pip install numpy")

    # SciPy
    try:
        import scipy
        from scipy.optimize import curve_fit
        modules_status.append("✓ SciPy imported successfully")
        modules_status.append(f"  SciPy version: {scipy.__version__}")
    except ImportError as e:
        modules_status.append(f"⚠ SciPy not available: {e}")
        modules_status.append("  SciPy is optional but recommended for advanced fitting")

    # Matplotlib
    try:
        import matplotlib
        modules_status.append("✓ Matplotlib available")
        modules_status.append(f"  Matplotlib version: {matplotlib.__version__}")
    except ImportError:
        modules_status.append("ℹ Matplotlib not available")

    # Math (built-in)
    import math
    modules_status.append("✓ Math module imported")

    return modules_status

# Execute the path discovery and module checking
print("=" * 60)
print("Python Engine Cross-Platform Initialization")
print("=" * 60)

add_python_paths()

print("\nChecking essential modules:")
print("-" * 30)
module_statuses = check_essential_modules()
for status in module_statuses:
    print(status)

print("\nPython path information:")
print("-" * 25)
print(f"Total paths in sys.path: {len(sys.path)}")
print("First 5 paths:")
for i, path in enumerate(sys.path[:5]):
    print(f"  {i+1}. {path}")

print("=" * 60)
print("Python engine initialization complete")
print("=" * 60)
)");

        // Capture and display the initialization output
        captureAndDisplayPythonOutput();

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

    if (x_data.size() != y_data.size()) {
        throw std::runtime_error("X and Y data vectors must have the same size");
    }

    if (x_data.empty()) {
        throw std::runtime_error("Data vectors cannot be empty");
    }

    try {
        main_module.attr("x_data") = pybind11::cast(x_data);
        main_module.attr("y_data") = pybind11::cast(y_data);
        main_module.attr("data_size") = pybind11::cast(x_data.size());

        // Use Python print so it goes to our capture system
        pybind11::exec("print(f'Data set successfully: {len(x_data)} points')", main_module.attr("__dict__"));
        captureAndDisplayPythonOutput();

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to set data: ") + e.what());
    }
}

void PythonEngine::executeScript(const std::string& script) {
    if (!initialized) initialize();

    if (script.empty()) {
        throw std::runtime_error("Python script is empty");
    }

    try {
        // Clear any previous results
        clearPreviousResults();

        // Execute the script
        pybind11::exec(script, main_module.attr("__dict__"));

        // Capture any output from the script execution
        captureAndDisplayPythonOutput();

        // Print completion message
        pybind11::exec("print('Script execution completed successfully')", main_module.attr("__dict__"));
        captureAndDisplayPythonOutput();

    } catch (const pybind11::error_already_set& e) {
        std::string error_msg = e.what();

        // Send error to Qt widget instead of console
        if (outputWidget) {
            QString qtError = QString("Python Execution Error: %1").arg(QString::fromStdString(error_msg));
            outputWidget->append(qtError);
        }

        throw std::runtime_error("Python script execution failed: " + error_msg);
    } catch (const std::exception& e) {
        // Send error to Qt widget instead of console
        if (outputWidget) {
            QString qtError = QString("Execution Error: %1").arg(QString::fromStdString(e.what()));
            outputWidget->append(qtError);
        }

        throw std::runtime_error(std::string("Script execution error: ") + e.what());
    }
}

void PythonEngine::captureAndDisplayPythonOutput() {
    if (!initialized || !outputWidget) return;

    try {
        // Get captured output from Python
        pybind11::object capture_result = pybind11::eval("_qt_output_capture.get_and_clear_output()", main_module.attr("__dict__"));
        std::string captured_text = capture_result.cast<std::string>();

        if (!captured_text.empty()) {
            // Convert to QString
            QString qtText = QString::fromStdString(captured_text);

            // Remove any trailing newline to avoid double spacing when using append()
            if (qtText.endsWith('\n')) {
                qtText.chop(1);
            }

            // Split by lines and append each one - this ensures proper formatting
            QStringList lines = qtText.split('\n');
            for (const QString& line : lines) {
                outputWidget->append(line);
            }

            outputWidget->ensureCursorVisible();
        }

    } catch (const std::exception& e) {
        // Fallback: if we can't capture Python output, at least show this error in Qt
        if (outputWidget) {
            outputWidget->append(QString("Error capturing Python output: %1").arg(e.what()));
        }
    }
}

std::vector<double> PythonEngine::getArray(const std::string& varName) {
    if (!initialized) {
        if (outputWidget) {
            outputWidget->append(QString("Warning: Python engine not initialized when trying to get array '%1'").arg(QString::fromStdString(varName)));
        }
        return std::vector<double>();
    }

    try {
        if (!main_module.attr("__dict__").contains(varName.c_str())) {
            if (outputWidget) {
                outputWidget->append(QString("Warning: Variable '%1' not found in Python namespace").arg(QString::fromStdString(varName)));
            }
            return std::vector<double>();
        }

        auto pyArray = main_module.attr(varName.c_str());
        auto result = pyArray.cast<std::vector<double>>();

        if (outputWidget) {
            outputWidget->append(QString("Retrieved array '%1' with %2 elements").arg(QString::fromStdString(varName)).arg(result.size()));
        }

        return result;

    } catch (const pybind11::cast_error& e) {
        if (outputWidget) {
            outputWidget->append(QString("Cast Error: Cannot convert '%1' to vector<double>: %2").arg(QString::fromStdString(varName)).arg(e.what()));
        }
        return std::vector<double>();
    } catch (const std::exception& e) {
        if (outputWidget) {
            outputWidget->append(QString("Error retrieving array '%1': %2").arg(QString::fromStdString(varName)).arg(e.what()));
        }
        return std::vector<double>();
    }
}

double PythonEngine::getScalar(const std::string& varName) {
    if (!initialized) {
        if (outputWidget) {
            outputWidget->append(QString("Warning: Python engine not initialized when trying to get scalar '%1'").arg(QString::fromStdString(varName)));
        }
        return 0.0;
    }

    try {
        if (!main_module.attr("__dict__").contains(varName.c_str())) {
            if (outputWidget) {
                outputWidget->append(QString("Warning: Variable '%1' not found in Python namespace").arg(QString::fromStdString(varName)));
            }
            return 0.0;
        }

        auto result = main_module.attr(varName.c_str()).cast<double>();

        if (outputWidget) {
            outputWidget->append(QString("Retrieved scalar '%1' = %2").arg(QString::fromStdString(varName)).arg(result));
        }

        return result;

    } catch (const pybind11::cast_error& e) {
        if (outputWidget) {
            outputWidget->append(QString("Cast Error: Cannot convert '%1' to double: %2").arg(QString::fromStdString(varName)).arg(e.what()));
        }
        return 0.0;
    } catch (const std::exception& e) {
        if (outputWidget) {
            outputWidget->append(QString("Error retrieving scalar '%1': %2").arg(QString::fromStdString(varName)).arg(e.what()));
        }
        return 0.0;
    }
}

void PythonEngine::clearPreviousResults() {
    if (!initialized) return;

    try {
        std::vector<std::string> vars_to_clear = {
            "fit_x", "fit_y", "amplitude", "frequency", "phase",
            "residuals", "r_squared", "fitted_params"
        };

        for (const auto& var : vars_to_clear) {
            if (main_module.attr("__dict__").contains(var.c_str())) {
                pybind11::exec(var + " = None", main_module.attr("__dict__"));
            }
        }

    } catch (const std::exception& e) {
        if (outputWidget) {
            outputWidget->append(QString("Note: Could not clear all previous results: %1").arg(e.what()));
        }
    }
}

std::vector<std::string> PythonEngine::getAvailableVariables() {
    std::vector<std::string> variables;

    if (!initialized) return variables;

    try {
        pybind11::exec(R"(
_available_vars = []
for name, obj in globals().items():
    if not name.startswith('_') and not callable(obj):
        _available_vars.append(name)
)", main_module.attr("__dict__"));

        auto py_vars = main_module.attr("_available_vars");
        variables = py_vars.cast<std::vector<std::string>>();

    } catch (const std::exception& e) {
        if (outputWidget) {
            outputWidget->append(QString("Error getting available variables: %1").arg(e.what()));
        }
    }

    return variables;
}