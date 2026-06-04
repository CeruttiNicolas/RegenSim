#include "mesher/Mesher.hpp"

double Mesher::lerp(double start, double end, double alpha) {
    return alpha * end + (1 - alpha) * start;
}

int Mesher::getThroatIndex(const std::vector<glm::dvec3>& contour) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Finding throat index..." << std::endl;
    std::vector<glm::dvec3>::const_iterator it = std::min_element(contour.begin(), contour.end(),
        [](const glm::dvec3& a, const glm::dvec3& b) {
            return a.y < b.y;
        });
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Finding throat index took " << elapsed.count() * 1000 << " milliseconds." << std::endl;
    return std::distance(contour.cbegin(), it);
}

glm::dvec3 Mesher::intersectRaySphere(glm::dvec3 rayOrigin, glm::dvec3 rayDirection, glm::dvec3 sphereOrigin, double sphereRadius) {
    glm::dvec3 L = rayOrigin - sphereOrigin;
    double a = glm::dot(rayDirection, rayDirection);
    double b = 2 * glm::dot(rayDirection, L);
    double c = glm::dot(L, L) - sphereRadius * sphereRadius;
    double t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    return rayOrigin + t * rayDirection;
}

double Mesher::computeQuadArea(const glm::dvec3& v0, const glm::dvec3& v1, const glm::dvec3& v2, const glm::dvec3& v3) {
	glm::dvec3 diag1 = v2 - v0;
	glm::dvec3 diag2 = v3 - v1;
	return 0.5 * glm::length(glm::cross(diag1, diag2));
}