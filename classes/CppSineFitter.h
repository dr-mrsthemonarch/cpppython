#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <random>

class CppSineFitter {
public:
    struct FitResult {
        std::vector<double> fit_x;
        std::vector<double> fit_y;
        double amplitude;
        double frequency;
        double phase;
        double offset;
        double r_squared;
        double rmse;
        double aic;
        std::chrono::microseconds fit_time;
        std::array<double, 4> param_errors;
    };

    struct Metrics {
        double r_squared;
        double rmse;
        double aic;
        std::array<double, 4> param_errors;
        std::array<double, 4> initial_params;
        std::array<double, 4> final_params;
    };

private:
    std::vector<double> x_data;
    std::vector<double> y_data;
    
    // Validation
    void validateData() const;
    
    // Parameter estimation
    std::array<double, 4> estimateInitialParams() const;
    double estimateFrequency() const;
    double estimatePhase(double frequency) const;
    
    // Optimization algorithms
    std::array<double, 4> levenbergMarquardt(const std::array<double, 4>& initial_params, 
                                             int max_iter = 100, double lambda_init = 1e-3) const;
    std::array<double, 4> differentialEvolution(const std::vector<std::pair<double, double>>& bounds, 
                                                 int max_iter = 300) const;
    
    // Helper functions
    std::vector<std::vector<double>> computeJacobian(const std::array<double, 4>& params) const;
    double objective(const std::array<double, 4>& params) const;
    Metrics calculateMetrics(const std::array<double, 4>& params) const;
    
    // FFT-based frequency estimation (simplified version)
    std::vector<std::complex<double>> simpleFFT(const std::vector<double>& data) const;
    
public:
    CppSineFitter(const std::vector<double>& x_data, const std::vector<double>& y_data);
    
    // Static sine model function
    static double sineModel(double x, double amplitude, double frequency, double phase, double offset);
    static std::vector<double> sineModel(const std::vector<double>& x, const std::array<double, 4>& params);
    
    // Main fitting method
    FitResult fit(int num_fit_points = 300);
};