#include "Mesher.hpp"

std::vector<glm::vec3> Mesher::run(const std::vector<glm::vec3>& contour, float stepSize, SimulationInput& input){
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Beginning meshing..." << std::endl;

    std::vector<glm::vec3> meshVertices;
    std::vector<glm::vec3> resampledContour = resampleContour(contour, stepSize);
    std::vector<glm::vec3> vertexNormals = computeVertexNormals(contour);

    float ac = input.ac;
    float at = input.at;
    float ae = input.ae;
    float bc = input.bc;
    float bt = input.bt;
    float be = input.be;

    float chamberRadius = input.chamber.y;
    float throatRadius = input.throat.y;
    float exitRadius = input.exit.y;
    int throatIndex = getThroatIndex(resampledContour);
    for (int i = 0; i < resampledContour.size(); i++) {
        glm::vec3 point = resampledContour[i];
        float a, b;
        if (i < throatIndex) {
            float alpha = (point.y - throatRadius) / (chamberRadius - throatRadius);
            a = lerp(at, ac, alpha);
            b = lerp(bt, bc, alpha);
        } else {
            float alpha = (point.y - throatRadius) / (exitRadius - throatRadius);
            a = lerp(at, ae, alpha);
            b = lerp(bt, be, alpha);
        }
        std::vector<glm::vec3> section = generateSection(input, point, a, b);
        glm::vec3 normal = vertexNormals[i];
        section = placeSection(section, point, normal);
        meshVertices.insert(meshVertices.end(), section.begin(), section.end());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Meshing took " << elapsed.count() << " seconds to generate " << meshVertices.size() << " vertices." << std::endl;
    return meshVertices;
}
