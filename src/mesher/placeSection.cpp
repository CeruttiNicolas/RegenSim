#include "mesher/Mesher.hpp"

std::vector<glm::dvec3> Mesher::placeSection(std::vector<glm::dvec3> section, glm::dvec3& point, glm::dvec3& vertexNormal) {
    glm::dmat4 rotationMatrix;
    glm::dmat4 translationMatrix = glm::translate(glm::dmat4(1.0), point);
    glm::dvec3 zAxis = glm::dvec3(0.0, 0.0, 1.0);
    glm::dvec3 yAxis = glm::dvec3(0.0, 1.0, 0.0);

    double angle = std::atan2(glm::dot(glm::cross(yAxis, vertexNormal), zAxis), glm::dot(yAxis, vertexNormal));
    if (abs(angle) < 1e-3) {
        rotationMatrix = glm::dmat4(1.0);
    } else {
        rotationMatrix = glm::rotate(glm::dmat4(1.0), angle, zAxis);
    }

    glm::mat4 transformationMatrix = translationMatrix * rotationMatrix;

    for (glm::dvec3 &p : section) {
        p = transformationMatrix * glm::dvec4(p, 1.0);
    }
    
    return section;
}