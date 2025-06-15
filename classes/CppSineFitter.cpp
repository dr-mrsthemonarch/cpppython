#include "CppSineFitter.h"
#include <stdexcept>
#include <complex>

CppSineFitter::CppSineFitter(const std::vector<double>& x_data, const std::vector<double>& y_data)
    : x_data(x_data), y_data(y_data) {
    validateData();
}

void CppSineFitter::validateData() const {
    if (x_data.empty() || y_data.empty()) {
        throw std::invalid_argument("Empty data arrays");
    }
    if (x_data.size() != y_data.size()) {
        throw std::invalid_argument("x_data and y_data must have the same length");
    }
    if (x_data.size() < 4) {
        throw std::invalid_argument("Need at least 4 data points for sine fitting");
    }
}

double CppSineFitter::sineModel(double x, double amplitude, double frequency, double phase, double offset) {
    return amplitude * std::sin(frequency * x + phase) + offset;
}

std::vector<double> CppSineFitter::sineModel(const std::vector<double>& x, const std::array<double, 4>& params) {
    std::vector<double> result;
    result.reserve(x.size());
    for (double xi : x) {
        result.push_back(sineModel(xi, params[0], params[1], params[2], params[3]));
    }
    return result;
}

std::array<double, 4> CppSineFitter::estimateInitialParams() const {
    // Basic statistics
    double y_mean = std::accumulate(y_data.begin(), y_data.end(), 0.0) / y_data.size();
    
    auto [y_min, y_max] = std::minmax_element(y_data.begin(), y_data.end());
    auto [x_min, x_max] = std::minmax_element(x_data.begin(), x_data.end());
    
    double y_range = *y_max - *y_min;
    double x_range = *x_max - *x_min;
    
    // Calculate standard deviation
    double y_var = 0.0;
    for (double y : y_data) {
        y_var += (y - y_mean) * (y - y_mean);
    }
    double y_std = std::sqrt(y_var / y_data.size());
    
    // Estimate parameters
    double amplitude_est = 2.0 * y_std;
    double frequency_est = estimateFrequency();
    double phase_est = estimatePhase(frequency_est);
    
    return {amplitude_est, frequency_est, phase_est, y_mean};
}

double CppSineFitter::estimateFrequency() const {
    try {
        // Simple frequency estimation using zero crossings
        double y_mean = std::accumulate(y_data.begin(), y_data.end(), 0.0) / y_data.size();
        
        // Count zero crossings of detrended signal
        int crossings = 0;
        bool last_positive = (y_data[0] - y_mean) > 0;
        
        for (size_t i = 1; i < y_data.size(); ++i) {
            bool current_positive = (y_data[i] - y_mean) > 0;
            if (current_positive != last_positive) {
                crossings++;
            }
            last_positive = current_positive;
        }
        
        // Estimate frequency from crossings
        if (crossings > 0) {
            double x_range = x_data.back() - x_data.front();
            return M_PI * crossings / x_range; // Each full cycle has 2 zero crossings
        }
    } catch (...) {
        // Fallback
    }
    
    // Default: assume one cycle over data range
    double x_range = x_data.back() - x_data.front();
    return (x_range > 0) ? 2.0 * M_PI / x_range : 1.0;
}

double CppSineFitter::estimatePhase(double frequency) const {
    try {
        // Linear regression approach: y = a*sin(fx) + b*cos(fx) + c
        size_t n = x_data.size();
        
        // Create design matrix
        std::vector<double> sin_terms(n), cos_terms(n), ones(n, 1.0);
        for (size_t i = 0; i < n; ++i) {
            sin_terms[i] = std::sin(frequency * x_data[i]);
            cos_terms[i] = std::cos(frequency * x_data[i]);
        }
        
        // Solve normal equations for sin and cos coefficients
        double sin_sum = std::accumulate(sin_terms.begin(), sin_terms.end(), 0.0);
        double cos_sum = std::accumulate(cos_terms.begin(), cos_terms.end(), 0.0);
        double y_sum = std::accumulate(y_data.begin(), y_data.end(), 0.0);
        
        double sin_y = 0.0, cos_y = 0.0, sin_sin = 0.0, cos_cos = 0.0, sin_cos = 0.0;
        for (size_t i = 0; i < n; ++i) {
            sin_y += sin_terms[i] * y_data[i];
            cos_y += cos_terms[i] * y_data[i];
            sin_sin += sin_terms[i] * sin_terms[i];
            cos_cos += cos_terms[i] * cos_terms[i];
            sin_cos += sin_terms[i] * cos_terms[i];
        }
        
        // Simplified solution (assuming orthogonality)
        double sin_coeff = (sin_y - sin_sum * y_sum / n) / (sin_sin - sin_sum * sin_sum / n + 1e-12);
        double cos_coeff = (cos_y - cos_sum * y_sum / n) / (cos_cos - cos_sum * cos_sum / n + 1e-12);
        
        return std::atan2(cos_coeff, sin_coeff);
    } catch (...) {
        return 0.0;
    }
}

std::array<double, 4> CppSineFitter::levenbergMarquardt(const std::array<double, 4>& initial_params, 
                                                         int max_iter, double lambda_init) const {
    std::array<double, 4> params = initial_params;
    double lambda_lm = lambda_init;
    
    for (int iteration = 0; iteration < max_iter; ++iteration) {
        // Compute residuals
        std::vector<double> residuals;
        residuals.reserve(x_data.size());
        for (size_t i = 0; i < x_data.size(); ++i) {
            double predicted = sineModel(x_data[i], params[0], params[1], params[2], params[3]);
            residuals.push_back(y_data[i] - predicted);
        }
        
        // Compute Jacobian
        auto jacobian = computeJacobian(params);
        
        // Compute JtJ and Jtr
        std::array<std::array<double, 4>, 4> JtJ = {};
        std::array<double, 4> Jtr = {};
        
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (size_t k = 0; k < x_data.size(); ++k) {
                    JtJ[i][j] += jacobian[k][i] * jacobian[k][j];
                }
            }
            for (size_t k = 0; k < x_data.size(); ++k) {
                Jtr[i] += jacobian[k][i] * residuals[k];
            }
        }
        
        // Add Levenberg-Marquardt damping
        for (int i = 0; i < 4; ++i) {
            JtJ[i][i] += lambda_lm * JtJ[i][i];
        }
        
        // Solve linear system (simplified Gaussian elimination)
        std::array<double, 4> delta = {};
        try {
            // Simple 4x4 system solver (you might want to use a proper linear algebra library)
            // For now, using a simplified approach
            for (int i = 0; i < 4; ++i) {
                delta[i] = Jtr[i] / (JtJ[i][i] + 1e-12);
            }
            
            std::array<double, 4> new_params;
            for (int i = 0; i < 4; ++i) {
                new_params[i] = params[i] + delta[i];
            }
            
            // Evaluate improvement
            double current_cost = objective(params);
            double new_cost = objective(new_params);
            
            if (new_cost < current_cost) {
                params = new_params;
                lambda_lm *= 0.1;
                
                // Check convergence
                double delta_norm = 0.0;
                for (double d : delta) {
                    delta_norm += d * d;
                }
                if (std::sqrt(delta_norm) < 1e-8) {
                    break;
                }
            } else {
                lambda_lm *= 10.0;
            }
        } catch (...) {
            lambda_lm *= 10.0;
        }
    }
    
    return params;
}

std::array<double, 4> CppSineFitter::differentialEvolution(const std::vector<std::pair<double, double>>& bounds, 
                                                            int max_iter) const {
    const int pop_size = 40;
    const double F = 0.8;
    const double CR = 0.9;
    
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Initialize population
    std::vector<std::array<double, 4>> population(pop_size);
    for (int i = 0; i < pop_size; ++i) {
        for (int j = 0; j < 4; ++j) {
            double range = bounds[j].second - bounds[j].first;
            population[i][j] = bounds[j].first + dis(gen) * range;
        }
    }
    
    // Evaluate initial population
    std::vector<double> fitness(pop_size);
    for (int i = 0; i < pop_size; ++i) {
        fitness[i] = objective(population[i]);
    }
    
    for (int generation = 0; generation < max_iter; ++generation) {
        for (int i = 0; i < pop_size; ++i) {
            // Select three random individuals
            std::vector<int> candidates;
            for (int j = 0; j < pop_size; ++j) {
                if (j != i) candidates.push_back(j);
            }
            std::shuffle(candidates.begin(), candidates.end(), gen);
            
            int a = candidates[0], b = candidates[1], c = candidates[2];
            
            // Mutation and crossover
            std::array<double, 4> trial = population[i];
            for (int j = 0; j < 4; ++j) {
                if (dis(gen) < CR) {
                    double mutant = population[a][j] + F * (population[b][j] - population[c][j]);
                    trial[j] = std::max(bounds[j].first, std::min(bounds[j].second, mutant));
                }
            }
            
            // Selection
            double trial_fitness = objective(trial);
            if (trial_fitness < fitness[i]) {
                population[i] = trial;
                fitness[i] = trial_fitness;
            }
        }
    }
    
    // Return best individual
    int best_idx = std::min_element(fitness.begin(), fitness.end()) - fitness.begin();
    return population[best_idx];
}

std::vector<std::vector<double>> CppSineFitter::computeJacobian(const std::array<double, 4>& params) const {
    std::vector<std::vector<double>> jacobian(x_data.size(), std::vector<double>(4));
    
    for (size_t i = 0; i < x_data.size(); ++i) {
        double x = x_data[i];
        double amplitude = params[0], frequency = params[1], phase = params[2];
        
        // Partial derivatives
        jacobian[i][0] = std::sin(frequency * x + phase);                    // dA
        jacobian[i][1] = amplitude * x * std::cos(frequency * x + phase);    // df
        jacobian[i][2] = amplitude * std::cos(frequency * x + phase);        // dphi
        jacobian[i][3] = 1.0;                                                // dc
    }
    
    return jacobian;
}

double CppSineFitter::objective(const std::array<double, 4>& params) const {
    try {
        double sse = 0.0;
        for (size_t i = 0; i < x_data.size(); ++i) {
            double predicted = sineModel(x_data[i], params[0], params[1], params[2], params[3]);
            double residual = y_data[i] - predicted;
            sse += residual * residual;
        }
        return sse;
    } catch (...) {
        return 1e10;
    }
}

CppSineFitter::Metrics CppSineFitter::calculateMetrics(const std::array<double, 4>& params) const {
    Metrics metrics = {};
    
    // Calculate predictions
    std::vector<double> y_pred;
    y_pred.reserve(x_data.size());
    for (size_t i = 0; i < x_data.size(); ++i) {
        y_pred.push_back(sineModel(x_data[i], params[0], params[1], params[2], params[3]));
    }
    
    // Calculate R-squared
    double y_mean = std::accumulate(y_data.begin(), y_data.end(), 0.0) / y_data.size();
    double ss_res = 0.0, ss_tot = 0.0;
    for (size_t i = 0; i < y_data.size(); ++i) {
        double residual = y_data[i] - y_pred[i];
        ss_res += residual * residual;
        double total_dev = y_data[i] - y_mean;
        ss_tot += total_dev * total_dev;
    }
    metrics.r_squared = (ss_tot > 0) ? 1.0 - (ss_res / ss_tot) : 0.0;
    
    // Calculate RMSE
    metrics.rmse = std::sqrt(ss_res / y_data.size());
    
    // Calculate AIC
    metrics.aic = y_data.size() * std::log(ss_res / y_data.size()) + 2 * 4;
    
    // Parameter errors (simplified)
    try {
        auto jacobian = computeJacobian(params);
        // Simplified error calculation
        for (int i = 0; i < 4; ++i) {
            double sum_sq = 0.0;
            for (size_t j = 0; j < x_data.size(); ++j) {
                sum_sq += jacobian[j][i] * jacobian[j][i];
            }
            metrics.param_errors[i] = (sum_sq > 0) ? std::sqrt(ss_res / (y_data.size() - 4) / sum_sq) : 0.0;
        }
    } catch (...) {
        metrics.param_errors.fill(0.0);
    }
    
    return metrics;
}

CppSineFitter::FitResult CppSineFitter::fit(int num_fit_points) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    FitResult result = {};
    
    // Step 1: Initial parameter estimation
    auto initial_params = estimateInitialParams();
    
    // Step 2: Global optimization with Differential Evolution
    auto [y_min, y_max] = std::minmax_element(y_data.begin(), y_data.end());
    auto [x_min, x_max] = std::minmax_element(x_data.begin(), x_data.end());
    
    double y_range = *y_max - *y_min;
    double x_range = *x_max - *x_min;
    
    std::vector<std::pair<double, double>> bounds = {
        {-3.0 * y_range, 3.0 * y_range},           // amplitude
        {0.1 / x_range, 10.0 / x_range},           // frequency
        {-2.0 * M_PI, 2.0 * M_PI},                 // phase
        {*y_min - y_range, *y_max + y_range}       // offset
    };
    
    auto global_params = differentialEvolution(bounds, 200);
    
    // Step 3: Local refinement with Levenberg-Marquardt
    auto final_params = levenbergMarquardt(global_params, 100);
    
    // Extract parameters
    result.amplitude = final_params[0];
    result.frequency = final_params[1];
    result.phase = final_params[2];
    result.offset = final_params[3];
    
    // Generate smooth fit curve
    result.fit_x.reserve(num_fit_points);
    result.fit_y.reserve(num_fit_points);
    
    double x_start = *x_min;
    double x_step = x_range / (num_fit_points - 1);
    
    for (int i = 0; i < num_fit_points; ++i) {
        double x = x_start + i * x_step;
        result.fit_x.push_back(x);
        result.fit_y.push_back(sineModel(x, final_params[0], final_params[1], final_params[2], final_params[3]));
    }
    
    // Calculate metrics
    auto metrics = calculateMetrics(final_params);
    result.r_squared = metrics.r_squared;
    result.rmse = metrics.rmse;
    result.aic = metrics.aic;
    result.param_errors = metrics.param_errors;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.fit_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    return result;
}