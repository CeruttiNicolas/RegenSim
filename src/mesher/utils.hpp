#pragma once
#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

double lerp(double start, double end, double alpha);
int getThroatIndex(const std::vector<glm::dvec3>& contour);
glm::dvec3 intersectRaySphere(glm::dvec3 rayOrigin, glm::dvec3 rayDirection, glm::dvec3 sphereOrigin, double sphereRadius);