#include "core/Application.hpp"
#include "core/SimulationInput.hpp"
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

SimulationInput Application::readInput(const std::string& path) {
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
    
	input.alphaMax = data["alpha_max"];
	input.knockdownFactor = data["knockdown_factor"];

    input.channelNumber = data["channel_number"];
    const auto& chamber = data["sections"]["chamber"];
    const auto& throat = data["sections"]["throat"];
    const auto& exit = data["sections"]["exit"];
    const auto& walls = data["walls"];
    const auto& subdivisions = data["subdivisions"];
	const auto& step = data["spacing_along_channel"];
    const auto& points = data["contour"];
    const auto& gasProfile = data["gas_profile"];
    const auto& alpha_T = data["alpha_T"];
    const auto& k_T = data["k_T"];

    for (auto& p : points) {
        input.contour.emplace_back(p[0], p[1], 0.0);
    }

    for (auto& x : gasProfile["x"]) {
        input.gas_xs.emplace_back(x);
    }

    for (auto& T : gasProfile["T"]) {
        input.gas_Ts.emplace_back(T);
    }
    
    for (auto& h : gasProfile["h"]) {
        input.gas_hs.emplace_back(h);
    }

    input.chamber = input.contour.front();
    input.exit = input.contour.back();
    input.throat = [](const std::vector<glm::dvec3>& contour) {
        return *std::min_element(contour.begin(), contour.end(),
            [](const glm::dvec3& a, const glm::dvec3& b) {
                return a.y < b.y;
            });
    }(input.contour);

    for (auto& piece : alpha_T) {
        input.alphaT.addSegment(piece["range"][0], piece["range"][1], piece["function"]);
    }

    for (auto& piece : k_T) {
        input.kT.addSegment(piece["range"][0], piece["range"][1], piece["function"]);
    }

    input.ac = chamber[0];
    input.bc = chamber[1];
    input.at = throat[0];
    input.bt = throat[1];
    input.ae = exit[0];
    input.be = exit[1];

    input.wi = walls["inner"];
    input.wo = walls["outer"];

    input.ni = subdivisions["inner_wall"];
    input.nb = subdivisions["channel_along_radius"];
    input.no = subdivisions["outer_wall"];

    input.nw = subdivisions["side_wall"];
    input.na = subdivisions["channel_along_circumference"];

    input.step = step;


    return input;
}
