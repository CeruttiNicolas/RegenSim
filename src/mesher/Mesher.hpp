#pragma once
#include "core/SimulationInput.hpp"
#include "mesher/Mesh.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <limits>
#include <numbers>
#include <unordered_set>
#include <vector>

class Mesher {
public:
    Mesh run(SimulationInput& input);

private:
    // main pipeline
    void generateVertices(const SimulationInput& input, Mesh& mesh);
    void generateIndices(const SimulationInput& input, Mesh& mesh);
    void computeVolumes(const SimulationInput& input, Mesh& mesh);
    void computeAreas(const SimulationInput& input, Mesh& mesh);
    void computeShortestEdgeLength(const SimulationInput& input, Mesh& mesh);

    // geometry helpers
    std::vector<glm::dvec3> resampleContour(const std::vector<glm::dvec3>& contour, double stepSize);
    std::vector<glm::dvec3> computeVertexNormals(const std::vector<glm::dvec3>& contour);
	int getThroatIndex(const std::vector<glm::dvec3>& contour);
    std::vector<glm::dvec3> generateSection(const SimulationInput& input, glm::dvec3 point, double a, double b);
    std::vector<glm::dvec3> placeSection(std::vector<glm::dvec3> section, glm::dvec3&, glm::dvec3& vertexNormal);

    // math helpers
    double lerp(double start, double end, double alpha);
    glm::dvec3 intersectRaySphere(glm::dvec3 rayOrigin, glm::dvec3 rayDirection, glm::dvec3 sphereOrigin, double sphereRadius);
	double computeQuadArea(const glm::dvec3& v0, const glm::dvec3& v1, const glm::dvec3& v2, const glm::dvec3& v3);

};