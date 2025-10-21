#include "Mesher.hpp"

std::vector<glm::vec3> Mesher::placeSection(std::vector<glm::vec3> section, glm::vec3& point, glm::vec3& vertexNormal) {
    glm::mat4 rotationMatrix;
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), point);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);

    float angle = std::atan2(glm::dot(glm::cross(yAxis, vertexNormal), zAxis), glm::dot(yAxis, vertexNormal));
    if (abs(angle) < 1e-3) {
        rotationMatrix = glm::mat4(1.0f);
    } else {
        rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, zAxis);
    }

    glm::mat4 transformationMatrix = translationMatrix * rotationMatrix;

    for (glm::vec3 &p : section) {
        p = transformationMatrix * glm::vec4(p, 1.0f);
    }
    
    return section;
}