#include "mesher/Mesher.hpp"

std::vector<glm::dvec3> Mesher::generateSection(const SimulationInput& input, glm::dvec3 point, double a, double b) {
    const double pi = std::numbers::pi_v<double>;
    int channelNumber = input.channelNumber;

    double radiusAtChannelMidpoint = point.y + input.wi + b / 2.0;
    double channelAngle = a / radiusAtChannelMidpoint;
    double halfWallAngle = pi / channelNumber - channelAngle / 2.0;

    // Number of subdivisions
    int ni = input.ni;
    int nb = input.nb;
    int no = input.no;
    int nw = input.nw;
    int na = input.na;

    int totalRadius = ni + nb + no + 1;
    int totalCircumference = nw * 2 + na + 1;

    // Compute how much to scale the point at each section
    std::vector<double> scalingFactors(totalRadius);
    for (int i = 0; i < totalRadius; i++) {
        double a;
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
    std::vector<double> angles(totalCircumference);
    for (int i = 0; i < totalCircumference; i++) {
        double a;
        if (i <= nw) {
            a = (halfWallAngle / nw) * i;
        } else if (i <= nw + na) {
            int j = i - nw;
            a = halfWallAngle + (channelAngle / na) * j;
        } else {
            int j = i - (nw + na);
            a = halfWallAngle + channelAngle + (halfWallAngle / nw) * j;
        }
        angles[i] = a - (halfWallAngle + channelAngle / 2.0);
    }
    
    // Scale the points as a "stencil" to then rotate
    std::vector<glm::dvec3> radialPoints(totalRadius);
    for (int i = 0; i < totalRadius; i++) {
        glm::dvec3 scaledPoint(point.x, point.y * scalingFactors[i], point.z);
        radialPoints[i] = scaledPoint;
    }

    // For each angle computed before, add a rotated instance of the stencil
    std::vector<glm::dvec3> sectionPoints;
    sectionPoints.reserve(totalRadius * totalCircumference);
    for (int i = 0; i < totalCircumference; i++) {
        glm::dmat4 rotationMatrix = glm::rotate(glm::dmat4(1.0), angles[i], glm::dvec3(1.0, 0.0, 0.0));
        for (int j = 0; j < totalRadius; j++) {
            glm::vec4 rotated4 = rotationMatrix * glm::vec4(radialPoints[j], 1.0);
            sectionPoints.push_back(glm::dvec3(rotated4) - point);
        }
    }

    return sectionPoints;
}