#pragma once
#include "utils.hpp"
#include "core/SimulationInput.hpp"
#include <chrono>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <limits>
#include <numbers>
#include <vector>

class Mesher {
public:
    //Mesher(const SimulationInput& input) : simInput(input) {};
    std::vector<glm::dvec3> run(SimulationInput& input);

    std::vector<glm::dvec3> resampleContour(const std::vector<glm::dvec3>& contour, double stepSize);
    std::vector<glm::dvec3> computeVertexNormals(const std::vector<glm::dvec3>& contour);
    std::vector<glm::dvec3> generateSection(const SimulationInput& input, glm::dvec3 point, double a, double b);
    std::vector<glm::dvec3> placeSection(std::vector<glm::dvec3> section, glm::dvec3&, glm::dvec3& vertexNormal);
    std::vector<double> computeVolumes(const SimulationInput& input, const std::vector<glm::dvec3>& mesh);
private:
    //SimulationInput simInput;
};