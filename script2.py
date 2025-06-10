# Advanced sine curve fitting with simplified code and enhanced techniques
# Available variables: x_data, y_data (numpy arrays)
# Expected outputs: fit_x, fit_y, amplitude, frequency, phase

import numpy as np
from typing import Tuple, Callable

print("Starting advanced sine curve fitting with enhanced algorithms...")

class SineFitter:
    """Advanced sine curve fitter using multiple optimization strategies"""

    def __init__(self, x_data: np.ndarray, y_data: np.ndarray):
        self.x = np.asarray(x_data, dtype=float)
        self.y = np.asarray(y_data, dtype=float)
        self._validate_data()

    def _validate_data(self):
        """Validate and prepare input data"""
        if len(self.x) == 0 or len(self.y) == 0:
            raise ValueError("Empty data arrays")
        if len(self.x) != len(self.y):
            raise ValueError("x_data and y_data must have the same length")
        if len(self.x) < 4:
            raise ValueError("Need at least 4 data points for sine fitting")

    @staticmethod
    def sine_model(x: np.ndarray, amplitude: float, frequency: float,
                   phase: float, offset: float) -> np.ndarray:
        """Standard sine model: A*sin(f*x + φ) + c"""
        return amplitude * np.sin(frequency * x + phase) + offset

    def estimate_initial_params(self) -> np.ndarray:
        """Smart parameter estimation using signal analysis"""
        # Basic statistics
        y_mean = np.mean(self.y)
        y_std = np.std(self.y)
        x_span = np.max(self.x) - np.min(self.x)

        # Estimate amplitude from data spread
        amplitude_est = 2 * y_std

        # Estimate frequency using autocorrelation
        frequency_est = self._estimate_frequency()

        # Estimate phase by finding first maximum
        phase_est = self._estimate_phase(frequency_est)

        return np.array([amplitude_est, frequency_est, phase_est, y_mean])

    def _estimate_frequency(self) -> float:
        """Estimate frequency using FFT-based spectral analysis"""
        try:
            # Remove DC component and detrend
            y_detrended = self.y - np.mean(self.y)

            # Apply window to reduce spectral leakage
            window = np.hanning(len(y_detrended))
            y_windowed = y_detrended * window

            # Compute FFT
            fft = np.fft.fft(y_windowed)
            freqs = np.fft.fftfreq(len(self.x), np.mean(np.diff(self.x)))

            # Find dominant frequency (excluding DC)
            magnitude = np.abs(fft)
            positive_freqs = freqs[:len(freqs)//2]
            positive_magnitude = magnitude[:len(magnitude)//2]

            if len(positive_freqs) > 1:
                dominant_idx = np.argmax(positive_magnitude[1:]) + 1
                return 2 * np.pi * np.abs(positive_freqs[dominant_idx])
            else:
                return 2 * np.pi / (np.max(self.x) - np.min(self.x))

        except Exception:
            # Fallback: assume one cycle over data range
            return 2 * np.pi / (np.max(self.x) - np.min(self.x))

    def _estimate_phase(self, frequency: float) -> float:
        """Estimate phase by fitting sine and cosine components"""
        try:
            # Create design matrix for linear regression
            A = np.column_stack([
                np.sin(frequency * self.x),
                np.cos(frequency * self.x),
                np.ones(len(self.x))
            ])

            # Solve using least squares
            coeffs = np.linalg.lstsq(A, self.y, rcond=None)[0]
            sin_coeff, cos_coeff = coeffs[0], coeffs[1]

            # Convert to amplitude and phase
            return np.arctan2(cos_coeff, sin_coeff)

        except Exception:
            return 0.0

    def levenberg_marquardt(self, initial_params: np.ndarray,
                            max_iter: int = 100, lambda_init: float = 1e-3) -> np.ndarray:
        """Levenberg-Marquardt optimization algorithm"""
        params = initial_params.copy()
        lambda_lm = lambda_init

        for iteration in range(max_iter):
            # Compute residuals and Jacobian
            residuals = self.y - self.sine_model(self.x, *params)
            jacobian = self._compute_jacobian(params)

            # Compute Hessian approximation and gradient
            JtJ = jacobian.T @ jacobian
            Jtr = jacobian.T @ residuals

            # Levenberg-Marquardt damping
            try:
                # Try Gauss-Newton step
                delta = np.linalg.solve(JtJ + lambda_lm * np.diag(np.diag(JtJ)), Jtr)
                new_params = params + delta

                # Evaluate improvement
                new_residuals = self.y - self.sine_model(self.x, *new_params)
                current_cost = np.sum(residuals**2)
                new_cost = np.sum(new_residuals**2)

                if new_cost < current_cost:
                    # Accept step and decrease damping
                    params = new_params
                    lambda_lm *= 0.1

                    # Check convergence
                    if np.linalg.norm(delta) < 1e-8:
                        break
                else:
                    # Reject step and increase damping
                    lambda_lm *= 10

            except np.linalg.LinAlgError:
                lambda_lm *= 10

        return params

    def _compute_jacobian(self, params: np.ndarray) -> np.ndarray:
        """Compute analytical Jacobian matrix"""
        amplitude, frequency, phase, offset = params
        x = self.x

        # Partial derivatives
        dA = np.sin(frequency * x + phase)
        df = amplitude * x * np.cos(frequency * x + phase)
        dphi = amplitude * np.cos(frequency * x + phase)
        dc = np.ones_like(x)

        return np.column_stack([dA, df, dphi, dc])

    def differential_evolution(self, bounds: list, max_iter: int = 300) -> np.ndarray:
        """Differential Evolution global optimization"""
        np.random.seed(42)  # For reproducibility

        # Population parameters
        pop_size = 40
        F = 0.8  # Differential weight
        CR = 0.9  # Crossover probability

        # Initialize population
        n_params = len(bounds)
        population = np.random.rand(pop_size, n_params)
        for i, (low, high) in enumerate(bounds):
            population[:, i] = population[:, i] * (high - low) + low

        # Evaluate initial population
        fitness = np.array([self._objective(ind) for ind in population])

        for generation in range(max_iter):
            for i in range(pop_size):
                # Select three random individuals (different from current)
                candidates = list(range(pop_size))
                candidates.remove(i)
                a, b, c = np.random.choice(candidates, 3, replace=False)

                # Mutation
                mutant = population[a] + F * (population[b] - population[c])

                # Crossover
                trial = population[i].copy()
                crossover_mask = np.random.rand(n_params) < CR
                trial[crossover_mask] = mutant[crossover_mask]

                # Ensure bounds
                for j, (low, high) in enumerate(bounds):
                    trial[j] = np.clip(trial[j], low, high)

                # Selection
                trial_fitness = self._objective(trial)
                if trial_fitness < fitness[i]:
                    population[i] = trial
                    fitness[i] = trial_fitness

        # Return best individual
        best_idx = np.argmin(fitness)
        return population[best_idx]

    def _objective(self, params: np.ndarray) -> float:
        """Objective function for optimization"""
        try:
            predicted = self.sine_model(self.x, *params)
            return np.sum((self.y - predicted) ** 2)
        except (OverflowError, RuntimeWarning):
            return 1e10

    def fit(self) -> Tuple[np.ndarray, np.ndarray, float, float, float, dict]:
        """Main fitting method using hybrid optimization"""
        print(f"\nFitting {len(self.x)} data points...")

        # Step 1: Smart initial parameter estimation
        initial_params = self.estimate_initial_params()
        print(f"\nInitial estimate: A={initial_params[0]:.3f}, f={initial_params[1]:.3f}, "
              f"φ={initial_params[2]:.3f}, offset={initial_params[3]:.3f}")

        # Step 2: Global optimization with Differential Evolution
        y_range = np.max(self.y) - np.min(self.y)
        x_range = np.max(self.x) - np.min(self.x)

        bounds = [
            (-3 * y_range, 3 * y_range),  # amplitude
            (0.1 / x_range, 10 / x_range),  # frequency
            (-2 * np.pi, 2 * np.pi),       # phase
            (np.min(self.y) - y_range, np.max(self.y) + y_range)  # offset
        ]

        global_params = self.differential_evolution(bounds, max_iter=200)
        print("Global optimization completed")

        # Step 3: Local refinement with Levenberg-Marquardt
        final_params = self.levenberg_marquardt(global_params, max_iter=100)
        amplitude, frequency, phase, offset = final_params

        # Generate smooth fit curve
        fit_x = np.linspace(np.min(self.x), np.max(self.x), 300)
        fit_y = self.sine_model(fit_x, *final_params)

        # Calculate fit quality metrics
        y_pred = self.sine_model(self.x, *final_params)
        ss_res = np.sum((self.y - y_pred) ** 2)
        ss_tot = np.sum((self.y - np.mean(self.y)) ** 2)
        r_squared = 1 - (ss_res / ss_tot) if ss_tot > 0 else 0
        rmse = np.sqrt(np.mean((self.y - y_pred) ** 2))

        # Parameter uncertainties (simplified)
        try:
            jacobian = self._compute_jacobian(final_params)
            covariance = np.linalg.inv(jacobian.T @ jacobian) * (ss_res / (len(self.y) - 4))
            param_errors = np.sqrt(np.abs(np.diag(covariance)))
        except:
            param_errors = np.zeros(4)

        metrics = {
            'r_squared': r_squared,
            'rmse': rmse,
            'param_errors': param_errors,
            'aic': len(self.y) * np.log(ss_res / len(self.y)) + 2 * 4,  # AIC
            'initial_params': initial_params,
            'final_params': final_params
        }

        return fit_x, fit_y, amplitude, frequency, phase, metrics

# Main execution
try:
    # Prepare data
    x_data = np.asarray(x_data, dtype=float)
    y_data = np.asarray(y_data, dtype=float)

    if len(x_data) == 0 or len(y_data) == 0:
        raise ValueError("Empty data arrays")

    print(f"Processing {len(x_data)} data points")

except (NameError, ValueError) as e:
    print(f"Data issue: {e}")
    print("Using demonstration data")
    x_data = np.linspace(0, 4*np.pi, 50)
    y_data = 2.5 * np.sin(1.2 * x_data + 0.5) + 1.0 + 0.2 * np.random.randn(50)

# Perform fitting
try:
    fitter = SineFitter(x_data, y_data)
    fit_x, fit_y, amplitude, frequency, phase, metrics = fitter.fit()

    # Display results
    print(f"\n=== FITTING RESULTS ===\n")
    print(f"\nAmplitude: {amplitude:.4f} ± {metrics['param_errors'][0]:.4f}")
    print(f"\nFrequency: {frequency:.4f} ± {metrics['param_errors'][1]:.4f}")
    print(f"\nPhase: {phase:.4f} ± {metrics['param_errors'][2]:.4f}")
    print(f"\nOffset: {metrics['final_params'][3]:.4f} ± {metrics['param_errors'][3]:.4f}")
    print(f"\n=== FIT QUALITY ===")
    print(f"\nR-squared: {metrics['r_squared']:.6f}")
    print(f"\nRMSE: {metrics['rmse']:.6f}")
    print(f"\nAIC: {metrics['aic']:.2f}")

    # Period and frequency in more intuitive units
    period = 2 * np.pi / frequency if frequency != 0 else np.inf
    freq_hz = frequency / (2 * np.pi)
    print(f"\n=== DERIVED QUANTITIES ===")
    print(f"\nPeriod: {period:.4f}")
    print(f"\nFrequency (Hz): {freq_hz:.6f}")
    print(f"\nPhase (degrees): {np.degrees(phase):.2f}°")

    print("\nAdvanced sine fitting completed successfully!")

except Exception as e:
    print(f"Fitting failed: {e}")
    # Fallback
    fit_x = np.array(x_data)
    fit_y = np.array(y_data)
    amplitude, frequency, phase = 1.0, 1.0, 0.0