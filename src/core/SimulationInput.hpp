#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

struct SimulationInput {
    std::vector<glm::dvec3> contour;
    glm::dvec3 chamber, throat, exit;
    double ac, at, ae, bc, bt, be, wi, wo;
    int channelNumber;
    int ni, nb, no;
    int nw, na;
    double a;

    double refLength;

    double getReferenceLength() const {
        auto safeDiv = [](double length, int subdivisions) {
            return (subdivisions > 0) ? (length / static_cast<double>(subdivisions)) : std::numeric_limits<double>::max();
        };

		double d_ac = safeDiv(ac, na);
		double d_at = safeDiv(at, na);
		double d_ae = safeDiv(ae, na);
		double d_bc = safeDiv(bc, nb);
		double d_bt = safeDiv(bt, nb);
		double d_be = safeDiv(be, nb);
		double d_wi = safeDiv(wi, nw);
        double d_wo = safeDiv(wo, nw);

		return std::min({ d_ac, d_at, d_ae, d_bc, d_bt, d_be, d_wi, d_wo });
	}

    void nonDimensionalize() {
        refLength = getReferenceLength();
        if (refLength <= 0.0) {
            throw std::runtime_error("Reference length must be positive for non-dimensionalization.");
		}

        for (auto& p : contour) {
            p /= refLength;
		}

        chamber /= refLength;
        throat /= refLength;
        exit /= refLength;

        ac /= refLength; at /= refLength; ae /= refLength;
        bc /= refLength; bt /= refLength; be /= refLength;
        wi /= refLength; wo /= refLength;
		
        a /= refLength;
	}
};