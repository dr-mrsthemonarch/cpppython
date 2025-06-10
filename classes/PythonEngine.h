#pragma once
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <QtWidgets/QTextEdit>
#include <memory>
#include <vector>
#include "../classes/QtOutputBuffer.h"

class PythonEngine {
private:
    std::unique_ptr<pybind11::scoped_interpreter> guard;
    pybind11::module_ main_module;
    std::unique_ptr<QtOutputBuffer> outputBuffer;
    std::streambuf* originalCoutBuffer;
    bool initialized = false;

public:
    PythonEngine();
    ~PythonEngine();
    
    void setOutputWidget(QTextEdit* outputWidget);
    void initialize();
    bool isInitialized() const;
    void setData(const std::vector<double>& x_data, const std::vector<double>& y_data);
    void executeScript(const std::string& script);
    std::vector<double> getArray(const std::string& varName);
    double getScalar(const std::string& varName);
};