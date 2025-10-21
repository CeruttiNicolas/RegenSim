#include "Mesher.hpp"

std::vector<glm::vec3> Mesher::generateSection(SimulationInput& input, glm::vec3 point, float a, float b) {
    const float pi = std::numbers::pi_v<float>;
    int channelNumber = input.channelNumber;

    float radiusAtChannelMidpoint = point.y + input.wi + b / 2.0f;
    float channelAngle = a / radiusAtChannelMidpoint;
    float halfWallAngle = pi / channelNumber - channelAngle / 2.0f;

    // Number of subdivisions
    int ni = input.ni;
    int nb = input.nb;
    int no = input.no;
    int nw = input.nw;
    int na = input.na;

    int totalRadius = ni + nb + no + 1;
    int totalCircumference = nw * 2 + na + 1;

    // Compute how much to scale the point at each section
    float scalingFactors[totalRadius];
    for (int i = 0; i < totalRadius; i++) {
        float a;
        if (i <= ni) {
            a = (input.wi / ni) * i;
        } else if (i <= ni + nb) {
            int j = i - ni;
            a = input.wi + (b / nb) * j;
        } else {
            int j = i - (ni + nb);
            a = input.wi + b + (input.wo / no) * j;
        }
        scalingFactors[i] = 1 + a / point.y;
    }

    // Compute the angle to rotate each series of points
    float angles[totalCircumference];
    for (int i = 0; i < totalCircumference; i++) {
        float a;
        if (i <= nw) {
            a = (halfWallAngle / nw) * i;
        } else if (i <= nw + na) {
            int j = i - nw;
            a = halfWallAngle + (channelAngle / na) * j;
        } else {
            int j = i - (nw + na);
            a = halfWallAngle + channelAngle + (halfWallAngle / nw) * j;
        }
        angles[i] = a - (halfWallAngle + channelAngle / 2.0f);
    }
    
    // Scale the points as a "stencil" to then rotate
    glm::vec3 radialPoints[totalRadius];
    for (int i = 0; i < totalRadius; i++) {
        glm::vec3 scaledPoint(point.x, point.y * scalingFactors[i], point.z);
        radialPoints[i] = scaledPoint;
    }

    // For each angle computed before, add a rotated instance of the stencil
    std::vector<glm::vec3> sectionPoints;
    sectionPoints.reserve(totalRadius * totalCircumference);
    for (int i = 0; i < totalCircumference; i++) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angles[i], glm::vec3(1.0f, 0.0f, 0.0f));
        for (int j = 0; j < totalRadius; j++) {
            glm::vec4 rotated4 = rotationMatrix * glm::vec4(radialPoints[j], 1.0f);
            sectionPoints.push_back(glm::vec3(rotated4) - point);
        }
    }

    return sectionPoints;
}