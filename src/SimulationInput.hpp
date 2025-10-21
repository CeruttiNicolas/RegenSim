#pragma once
#include <glm/glm.hpp>
#include <vector>

struct SimulationInput {
    std::vector<glm::vec3> contour;
    glm::vec3 chamber, throat, exit;
    float ac, at, ae, bc, bt, be, wi, wo;
    int channelNumber;
    int ni, nb, no;
    int nw, na;
};