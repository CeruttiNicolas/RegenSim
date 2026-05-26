#include "thermal/ThermalSolver.cuh"

double ThermalSolver::interpolate1D(double xQuery, const std::vector<double>& X, const std::vector<double>& Y) {
    if (xQuery <= X.front()) return Y.front();
    if (xQuery >= X.back()) return Y.back();

    for (size_t i = 0; i < X.size(); i++) {
        if (xQuery >= X[i] && xQuery <= X[i + 1]) {
            double t = (xQuery - X[i]) / (X[i + 1] - X[i]);
            return Y[i] + t * (Y[i + 1] - Y[i]);
        }
    }

    return Y.back(); // Fallback
}