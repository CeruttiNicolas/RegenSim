#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

float lerp(float start, float end, float alpha);
int getThroatIndex(const std::vector<glm::vec3>& contour);
glm::vec3 intersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereOrigin, float sphereRadius);