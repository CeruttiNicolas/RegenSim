#include "utils.hpp"

float lerp(float start, float end, float alpha) {
    return alpha * end + (1 - alpha) * start;
}

int getThroatIndex(const std::vector<glm::vec3>& contour) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << ">>> Finding throat index..." << std::endl;
    std::vector<glm::vec3>::const_iterator it = std::min_element(contour.begin(), contour.end(),
        [](const glm::vec3& a, const glm::vec3& b) {
            return a.y < b.y;
        });
    std::cout << ">>> Found throat at index " << std::distance(contour.cbegin(), it) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << ">>> Finding throat index took " << elapsed.count() << " seconds." << std::endl;
    return std::distance(contour.cbegin(), it);
}

glm::vec3 intersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereOrigin, float sphereRadius) {
    glm::vec3 L = rayOrigin - sphereOrigin;
    float a = glm::dot(rayDirection, rayDirection);
    float b = 2 * glm::dot(rayDirection, L);
    float c = glm::dot(L, L) - sphereRadius * sphereRadius;
    float t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    return rayOrigin + t * rayDirection;
}