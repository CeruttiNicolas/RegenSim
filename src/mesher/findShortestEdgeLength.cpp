#include "mesher/Mesher.hpp"
#include "core/SimulationInput.hpp"
#include <limits>

double Mesher::findShortestEdgeLength(const std::vector<glm::dvec3>& mesh, const SimulationInput& input) {

    int height = input.ni + input.nb + input.no + 1;
    int width = 2 * input.nw + input.na + 1;
    int depth = mesh.size() / (width * height);

    auto getIndex = [&](int x, int y, int z) {
        return y + (x * height) + (z * height * width);
    };

	double shortestEdge = std::numeric_limits<double>::max();
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                // up
                if (y < height - 1) {
				    shortestEdge = std::min(shortestEdge, glm::distance(mesh[getIndex(x, y, z)], mesh[getIndex(x, y + 1, z)]));
                }
				// right
                if (x < width - 1) {
                    shortestEdge = std::min(shortestEdge, glm::distance(mesh[getIndex(x, y, z)], mesh[getIndex(x + 1, y, z)]));
                }
				// forward
                if (z < depth - 1) {
                    shortestEdge = std::min(shortestEdge, glm::distance(mesh[getIndex(x, y, z)], mesh[getIndex(x, y, z + 1)]));
                }
            }
        }
	}
	return shortestEdge;
}