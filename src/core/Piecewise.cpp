#include "core/Piecewise.hpp"

void Piecewise::addSegment(double min_x, double max_x, const std::string& formula) {
    if (min_x >= max_x) throw std::invalid_argument("min_x must be strictly less than max_x");

    segments.emplace_back(min_x, max_x, formula);

    std::sort(segments.begin(), segments.end(), [](const FunctionSegment& a, const FunctionSegment& b) {
        return a.min_x < b.min_x;
    });

    for (size_t i = 0; i < segments.size() - 1; ++i) {
        if (segments[i].max_x > segments[i + 1].min_x) {
            throw std::invalid_argument("Overlapping segments.");
        }
    }
}

double Piecewise::evaluate(double x) const {
    if (segments.empty()) {
        throw std::runtime_error("No function segments defined.");
    }

    // find segment
    for (const auto& seg : segments) {
        if (x >= seg.min_x && x <= seg.max_x) {
            return seg.evaluator->evaluate(x);
        }
    }

    // interpolate between segments
    for (size_t i = 0; i < segments.size() - 1; ++i) {
        double b = segments[i].max_x;
        double c = segments[i + 1].min_x;

        if (x > b && x < c) {
            double y0 = segments[i].evaluator->evaluate(b);
            double y1 = segments[i + 1].evaluator->evaluate(c);

            return y0 + (x - b) * (y1 - y0) / (c - b);
        }
    }

    // clamp outside bounds
    if (x < segments.front().min_x) {
        return segments.front().evaluator->evaluate(segments.front().min_x);
    }
    if (x > segments.back().max_x) {
        return segments.back().evaluator->evaluate(segments.back().max_x);
    }

    return 0.0;
}

std::vector<double> Piecewise::generateLUT(double rangeStart, double rangeEnd, double step) const {
    if (step <= 0.0) {
        throw std::invalid_argument("Step size must be strictly positive.");
    }
    if (rangeEnd < rangeStart) {
        throw std::invalid_argument("rangeStart must be less than rangeEnd.");
    }

    std::vector<double> lut;

    int estimatedPoints = static_cast<int>(std::floor((rangeEnd - rangeStart) / step)) + 2;
    lut.reserve(estimatedPoints);

    double currentX = rangeStart;

    double eps = step * 1e-5;

    while (currentX <= rangeEnd + eps) {
        lut.push_back(evaluate(currentX));
        currentX += step;
    }

    return lut;
}