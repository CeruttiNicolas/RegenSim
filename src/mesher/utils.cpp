#include "mesher/utils.hpp"

double lerp(double start, double end, double alpha) {
    return alpha * end + (1 - alpha) * start;
}

int getThroatIndex(const std::vector<glm::dvec3>& contour) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Finding throat index..." << std::endl;
    std::vector<glm::dvec3>::const_iterator it = std::min_element(contour.begin(), contour.end(),
        [](const glm::dvec3& a, const glm::dvec3& b) {
            return a.y < b.y;
        });
    std::cout << "Found throat at index " << std::distance(contour.cbegin(), it) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Finding throat index took " << elapsed.count() * 1000 << " milliseconds." << std::endl;
    return std::distance(contour.cbegin(), it);
}

glm::dvec3 intersectRaySphere(glm::dvec3 rayOrigin, glm::dvec3 rayDirection, glm::dvec3 sphereOrigin, double sphereRadius) {
    glm::dvec3 L = rayOrigin - sphereOrigin;
    double a = glm::dot(rayDirection, rayDirection);
    double b = 2 * glm::dot(rayDirection, L);
    double c = glm::dot(L, L) - sphereRadius * sphereRadius;
    double t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    return rayOrigin + t * rayDirection;
}