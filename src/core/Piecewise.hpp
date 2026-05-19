#pragma once

#include "core/shuntingYard.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <iostream>

struct FunctionSegment {
    double min_x;
    double max_x;
    std::shared_ptr<ExpressionEvaluator> evaluator;

    FunctionSegment(double min, double max, const std::string& formula)
        : min_x(min), max_x(max), evaluator(std::make_shared<ExpressionEvaluator>(formula)) {
    }
};

class Piecewise {
private:
    std::vector<FunctionSegment> segments;

public:
    void addSegment(double min_x, double max_x, const std::string& formula);
    double evaluate(double x) const;
    std::vector<double> generateLUT(double rangeStart, double rangeEnd, double step) const;
};