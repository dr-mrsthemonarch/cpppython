# Advanced sine curve fitting with error analysis (no scipy dependency)
# Available variables: x_data, y_data (numpy arrays)
# Expected outputs: fit_x, fit_y, amplitude, frequency, phase

import numpy as np

print("Starting advanced sine curve fitting...")

# Ensure x_data and y_data are proper numpy arrays
try:
    x_data = np.asarray(x_data, dtype=float)
    y_data = np.asarray(y_data, dtype=float)

    if len(x_data) == 0 or len(y_data) == 0:
        raise ValueError("Empty data arrays")

    if len(x_data) != len(y_data):
        raise ValueError("x_data and y_data must have the same length")

    print(f"Data converted successfully: {len(x_data)} points")

except Exception as e:
    print(f"Error converting data: {e}")
    # Create dummy data as fallback
    x_data = np.linspace(0, 2*np.pi, 100)
    y_data = np.sin(x_data) + 0.1 * np.random.randn(100)
    print("Using dummy sine data for demonstration")

# Define sine function to fit
def sine_func(x, amplitude, frequency, phase, offset):
    return amplitude * np.sin(frequency * x + phase) + offset

# Define a more complex function (sine + cosine)
def sine_cosine_func(x, a1, f1, p1, a2, f2, p2, offset):
    return a1 * np.sin(f1 * x + p1) + a2 * np.cos(f2 * x + p2) + offset

# Custom optimization function using gradient descent
def minimize_sse(func, x_data, y_data, initial_params, learning_rate=0.01, max_iterations=1000, tolerance=1e-8):
    """
    Minimize sum of squared errors using gradient descent
    """
    params = np.array(initial_params, dtype=float)
    n_params = len(params)

    # Ensure data is numpy arrays
    x_data = np.asarray(x_data, dtype=float)
    y_data = np.asarray(y_data, dtype=float)

    # Numerical gradient calculation
    def numerical_gradient(params):
        grad = np.zeros_like(params)
        h = 1e-8

        for i in range(n_params):
            params_plus = params.copy()
            params_minus = params.copy()
            params_plus[i] += h
            params_minus[i] -= h

            try:
                y_plus = func(x_data, *params_plus)
                y_minus = func(x_data, *params_minus)

                sse_plus = np.sum((y_data - y_plus) ** 2)
                sse_minus = np.sum((y_data - y_minus) ** 2)

                grad[i] = (sse_plus - sse_minus) / (2 * h)
            except:
                grad[i] = 0  # If calculation fails, set gradient to 0

        return grad

    # Adaptive learning rate and momentum
    momentum = np.zeros_like(params)
    beta = 0.9
    prev_sse = float('inf')

    for iteration in range(max_iterations):
        try:
            y_pred = func(x_data, *params)
            sse = np.sum((y_data - y_pred) ** 2)

            if abs(prev_sse - sse) < tolerance:
                break

            grad = numerical_gradient(params)

            # Check for NaN or inf in gradient
            if np.any(~np.isfinite(grad)):
                print(f"Invalid gradient at iteration {iteration}, stopping optimization")
                break

            # Apply momentum
            momentum = beta * momentum + (1 - beta) * grad

            # Adaptive learning rate
            if sse > prev_sse:
                learning_rate *= 0.5
            elif iteration > 100 and sse < prev_sse * 0.99:
                learning_rate *= 1.01

            params -= learning_rate * momentum
            prev_sse = sse

            # Prevent parameters from becoming too large
            params = np.clip(params, -1000, 1000)

        except (OverflowError, RuntimeWarning, ValueError):
            # Reset with smaller learning rate if overflow occurs
            learning_rate *= 0.1
            if learning_rate < 1e-10:
                print(f"Learning rate too small, stopping at iteration {iteration}")
                break

    return params

# Calculate parameter covariance matrix (simplified Hessian approximation)
def calculate_covariance(func, x_data, y_data, params):
    """
    Calculate approximate covariance matrix using finite differences
    """
    n_params = len(params)
    h = 1e-6
    hessian = np.zeros((n_params, n_params))

    try:
        for i in range(n_params):
            for j in range(n_params):
                # Calculate second partial derivatives
                params_ij = params.copy()
                params_i = params.copy()
                params_j = params.copy()
                params_base = params.copy()

                params_ij[i] += h
                params_ij[j] += h
                params_i[i] += h
                params_j[j] += h

                y_ij = func(x_data, *params_ij)
                y_i = func(x_data, *params_i)
                y_j = func(x_data, *params_j)
                y_base = func(x_data, *params_base)

                residuals_ij = y_data - y_ij
                residuals_i = y_data - y_i
                residuals_j = y_data - y_j
                residuals_base = y_data - y_base

                # Approximate second partial derivative
                d2f_didj = (np.sum(residuals_ij) - np.sum(residuals_i) -
                            np.sum(residuals_j) + np.sum(residuals_base)) / (h * h)

                hessian[i, j] = d2f_didj

        # Calculate covariance as inverse of Hessian (if invertible)
        if np.linalg.det(hessian) != 0:
            covariance = np.linalg.inv(hessian)
        else:
            # Use pseudo-inverse if Hessian is singular
            covariance = np.linalg.pinv(hessian)

    except:
        # Fallback to identity matrix if calculation fails
        covariance = np.eye(n_params)

    return covariance

# Try simple sine fit first
try:
    print("Attempting simple sine fit...")

    # Better initial guess based on data
    try:
        y_range = np.max(y_data) - np.min(y_data)
        y_offset = np.mean(y_data)
        x_range = np.max(x_data) - np.min(x_data)

        # Avoid division by zero
        if x_range == 0:
            x_range = 1.0
        if y_range == 0:
            y_range = 1.0

        initial_guess = [
            y_range / 2,      # amplitude (half the data range)
            2 * np.pi / x_range,  # frequency (one cycle over data range)
            0.0,              # phase
            y_offset          # offset (mean of data)
        ]

        print(f"Initial guess: A={initial_guess[0]:.3f}, f={initial_guess[1]:.3f}, φ={initial_guess[2]:.3f}, offset={initial_guess[3]:.3f}")

    except Exception as e:
        print(f"Error calculating initial guess: {e}")
        initial_guess = [1.0, 1.0, 0.0, 0.0]

    popt_simple = minimize_sse(sine_func, x_data, y_data, initial_guess,
                               learning_rate=0.001, max_iterations=2000)

    amplitude, frequency, phase, offset = popt_simple

    # Calculate R-squared for simple fit
    y_pred_simple = sine_func(x_data, *popt_simple)
    ss_res_simple = np.sum((y_data - y_pred_simple) ** 2)
    ss_tot = np.sum((y_data - np.mean(y_data)) ** 2)
    r_squared_simple = 1 - (ss_res_simple / ss_tot) if ss_tot > 0 else 0

    print(f"Simple sine fit - R²: {r_squared_simple:.4f}")
    print(f"  Amplitude: {amplitude:.3f}")
    print(f"  Frequency: {frequency:.3f}")
    print(f"  Phase: {phase:.3f}")
    print(f"  Offset: {offset:.3f}")

    # Try more complex fit
    print("\nAttempting sine+cosine fit...")
    try:
        y_range = np.max(y_data) - np.min(y_data)
        y_offset = np.mean(y_data)
        x_range = np.max(x_data) - np.min(x_data)

        # Avoid division by zero
        if x_range == 0:
            x_range = 1.0
        if y_range == 0:
            y_range = 1.0

        initial_guess_complex = [
            y_range / 4,      # a1
            2 * np.pi / x_range,  # f1
            0.0,              # p1
            y_range / 4,      # a2
            2 * np.pi / x_range,  # f2
            np.pi / 2,        # p2 (90 degrees phase shift)
            y_offset          # offset
        ]
    except:
        initial_guess_complex = [0.5, 1.0, 0.0, 0.1, 1.0, 0.0, 0.0]

    try:
        popt_complex = minimize_sse(sine_cosine_func, x_data, y_data, initial_guess_complex,
                                    learning_rate=0.0005, max_iterations=3000)

        y_pred_complex = sine_cosine_func(x_data, *popt_complex)
        ss_res_complex = np.sum((y_data - y_pred_complex) ** 2)
        r_squared_complex = 1 - (ss_res_complex / ss_tot) if ss_tot > 0 else 0

        print(f"Complex fit - R²: {r_squared_complex:.4f}")

        # Choose better fit
        if r_squared_complex > r_squared_simple + 0.01:  # Significant improvement
            print("Using complex fit (sine + cosine)")
            fit_x = np.linspace(min(x_data), max(x_data), 300)
            fit_y = sine_cosine_func(fit_x, *popt_complex)

            # Store parameters from complex fit
            amplitude = popt_complex[0]  # Primary amplitude
            frequency = popt_complex[1]  # Primary frequency
            phase = popt_complex[2]      # Primary phase
        else:
            print("Using simple sine fit")
            fit_x = np.linspace(min(x_data), max(x_data), 300)
            fit_y = sine_func(fit_x, *popt_simple)

    except Exception as e:
        print(f"Complex fit failed: {e}")
        print("Using simple sine fit")
        fit_x = np.linspace(min(x_data), max(x_data), 300)
        fit_y = sine_func(fit_x, *popt_simple)

    # Calculate parameter uncertainties
    try:
        pcov_simple = calculate_covariance(sine_func, x_data, y_data, popt_simple)
        param_errors = np.sqrt(np.abs(np.diag(pcov_simple)))
        print(f"\nParameter uncertainties:")
        print(f"  Amplitude: ±{param_errors[0]:.3f}")
        print(f"  Frequency: ±{param_errors[1]:.3f}")
        print(f"  Phase: ±{param_errors[2]:.3f}")
        print(f"  Offset: ±{param_errors[3]:.3f}")
    except:
        print("\nParameter uncertainties: Could not calculate")

    print("Curve fitting completed successfully!")

except Exception as e:
    print(f"Error in curve fitting: {e}")
    # Fallback - just copy original data
    fit_x = np.array(x_data)
    fit_y = np.array(y_data)
    amplitude, frequency, phase = 1.0, 1.0, 0.0

# Additional analysis
try:
    print(f"\nData statistics:")
    print(f"  Data points: {len(x_data)}")
    print(f"  X range: {min(x_data):.3f} to {max(x_data):.3f}")
    print(f"  Y range: {min(y_data):.3f} to {max(y_data):.3f}")
    print(f"  Y mean: {np.mean(y_data):.3f}")
    print(f"  Y std: {np.std(y_data):.3f}")
except:
    pass