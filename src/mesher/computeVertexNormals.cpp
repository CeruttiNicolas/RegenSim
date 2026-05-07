#include "mesher/Mesher.hpp"

std::vector<glm::dvec3> Mesher::computeVertexNormals(const std::vector<glm::dvec3>& contour) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Computing vertex normals..." << std::endl;
    std::vector<glm::dvec3> normals;
    const glm::dvec3 planeNormal(0.0, 0.0, 1.0);

    glm::dvec3 firstVector = contour[1] - contour[0];
    glm::dvec3 firstNormal = glm::normalize(glm::cross(planeNormal, firstVector));    
    normals.push_back(firstNormal);

    for(int i = 1; i < contour.size() - 1; i++) {
        glm::dvec3 previousVector = glm::normalize(contour[i] - contour[i-1]);
        glm::dvec3 nextVector = glm::normalize(contour[i+1] - contour[i]);
        glm::dvec3 vector = previousVector + nextVector;
        glm::dvec3 normal = glm::normalize(glm::cross(planeNormal, vector));
        normals.push_back(normal);
    }
    
    glm::dvec3 lastVector = contour.back() - contour.rbegin()[1];
    glm::dvec3 lastNormal = glm::normalize(glm::cross(planeNormal, lastVector));
    normals.push_back(lastNormal);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Vertex normal computation took " << elapsed.count() * 1000 << " milliseconds." << std::endl;

    return normals;
}