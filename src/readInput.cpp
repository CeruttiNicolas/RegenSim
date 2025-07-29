#include "Application.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

Application::SimulationInput Application::readInput(const std::string& path) {
    SimulationInput input;
    using json = nlohmann::json;
    std::ifstream f(path);
    json data;
    
    if (!f.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    try {
        data = json::parse(f);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse geometry input: " + std::string(e.what()));
    }
    // TODO: Validate JSON structure according to schema, see nlohmann library
    
    const auto& chamber = data["sections"]["chamber"];
    const auto& throat = data["sections"]["throat"];
    const auto& exit = data["sections"]["exit"];
    const auto& walls = data["walls"];
    const auto& points = data["contour"];

    for (auto& p : points) {
        input.contour.emplace_back(p[0], p[1]);
    }

    input.chamber = input.contour.front();
    input.exit = input.contour.back();
    input.throat = [] (const std::vector<Point>& contour) {
        return *std::min_element(contour.begin(), contour.end(),
            [](const Point& a, const Point& b) {
                return a.y < b.y;
            });
    }(input.contour);

    input.ac = chamber[0];
    input.bc = chamber[1];
    input.at = throat[0];
    input.bt = throat[1];
    input.ae = exit[0];
    input.be = exit[1];

    input.wi = walls["inner"];
    input.wo = walls["outer"];
    
    return input;
}
