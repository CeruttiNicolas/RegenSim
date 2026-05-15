#include "mesher/Mesher.hpp"
#include "core/SimulationInput.hpp"
#include <limits>

void Mesher::computeShortestEdgeLength(const SimulationInput& input, Mesh& mesh) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Finding shortest edge length..." << std::endl;

    int depthInNodes = mesh.Nx + 1;
    int heightInNodes = mesh.Ny + 1;
    int widthInNodes = mesh.Nz + 1;

    auto getIndex = [&](int x, int y, int z) {
        return y + (z * heightInNodes) + (x * heightInNodes * widthInNodes);
    };

	mesh.shortestEdgeLength = std::numeric_limits<double>::max();
    for (int x = 0; x < depthInNodes; x++) {
        for (int z = 0; z < widthInNodes; z++) {
            for (int y = 0; y < heightInNodes; y++) {
                // up
                if (y < heightInNodes - 1) {
				    mesh.shortestEdgeLength = std::min(mesh.shortestEdgeLength, glm::distance(mesh.vertices[getIndex(x, y, z)], mesh.vertices[getIndex(x, y + 1, z)]));
                }
				// right
                if (x < widthInNodes - 1) {
                    mesh.shortestEdgeLength = std::min(mesh.shortestEdgeLength, glm::distance(mesh.vertices[getIndex(x, y, z)], mesh.vertices[getIndex(x + 1, y, z)]));
                }
				// forward
                if (z < depthInNodes - 1) {
                    mesh.shortestEdgeLength = std::min(mesh.shortestEdgeLength, glm::distance(mesh.vertices[getIndex(x, y, z)], mesh.vertices[getIndex(x, y, z + 1)]));
                }
            }
        }
	}
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
	std::cout << "Shortest edge length found: " << mesh.shortestEdgeLength << " in " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}