#pragma once

#include <vector>
#include <limits>
#include <iostream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#include "utils.hpp"
#include "SimulationInput.hpp"

class Mesher {
public:
    //Mesher(const SimulationInput& input) : simInput(input) {};
    std::vector<glm::vec3> run(const std::vector<glm::vec3>& contour, float stepSize, SimulationInput& input);

    std::vector<glm::vec3> resampleContour(const std::vector<glm::vec3>& contour, float stepSize);
    std::vector<glm::vec3> computeVertexNormals(const std::vector<glm::vec3>& contour);
    std::vector<glm::vec3> generateSection(SimulationInput& input, glm::vec3 point, float a, float b);
    std::vector<glm::vec3> placeSection(std::vector<glm::vec3> section, glm::vec3&, glm::vec3& vertexNormal);
private:
    //SimulationInput simInput;
};