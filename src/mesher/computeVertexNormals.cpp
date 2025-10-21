#include "Mesher.hpp"

std::vector<glm::vec3> Mesher::computeVertexNormals(const std::vector<glm::vec3>& contour) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Computing vertex normals..." << std::endl;
    std::vector<glm::vec3> normals;
    const glm::vec3 planeNormal(0.0f, 0.0f, 1.0f);

    glm::vec3 firstVector = contour[1] - contour[0];
    glm::vec3 firstNormal = glm::normalize(glm::cross(planeNormal, firstVector));    
    normals.push_back(firstNormal);

    for(int i = 1; i < contour.size() - 1; i++) {
        glm::vec3 previousVector = glm::normalize(contour[i] - contour[i-1]);
        glm::vec3 nextVector = glm::normalize(contour[i+1] - contour[i]);
        glm::vec3 vector = previousVector + nextVector;
        glm::vec3 normal = glm::normalize(glm::cross(planeNormal, vector));
        normals.push_back(normal);
    }
    
    glm::vec3 lastVector = contour.back() - contour.rbegin()[1];
    glm::vec3 lastNormal = glm::normalize(glm::cross(planeNormal, lastVector));
    normals.push_back(lastNormal);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Vertex normal computation took " << elapsed.count() << " seconds." << std::endl;

    return normals;
}