#include "mesher/Mesher.hpp"
#include "core/SimulationInput.hpp"
#include <array>
#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <iostream>

double computeTetrahedronVolume(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c, const glm::dvec3& d) {
	return std::abs(glm::dot(a - d, glm::cross(b - d, c - d))) / 6.0;
}

double computeHexahedronVolume(const std::array<glm::dvec3, 8>& hex) {
	double volume = 0.0;

	// First Pyramid
	volume += computeTetrahedronVolume(hex[0], hex[2], hex[3], hex[6]);
	volume += computeTetrahedronVolume(hex[0], hex[1], hex[2], hex[6]);

	// Second Pyramid
	volume += computeTetrahedronVolume(hex[0], hex[4], hex[7], hex[6]);
	volume += computeTetrahedronVolume(hex[0], hex[3], hex[7], hex[6]);

	// Third Pyramid
	volume += computeTetrahedronVolume(hex[0], hex[4], hex[5], hex[6]);
	volume += computeTetrahedronVolume(hex[0], hex[1], hex[5], hex[6]);

	return volume;
}

std::vector<double> Mesher::computeVolumes(const SimulationInput& input, const std::vector<glm::dvec3>& mesh) {
	auto start = std::chrono::high_resolution_clock::now();
	std::cout << "Computing volumes..." << std::endl;

	int heightInNodes = input.ni + input.nb + input.no + 1;
	int widthInNodes = 2 * input.nw + input.na + 1;
	int depthInNodes = mesh.size() / (widthInNodes * heightInNodes);

	int heightInCells = heightInNodes - 1;
	int widthInCells = widthInNodes - 1;
	int depthInCells = depthInNodes - 1;

	int dx = widthInNodes * heightInNodes;
	int dz = heightInNodes;

	std::vector<double> volumes(heightInCells * widthInCells * depthInCells);

	for (int x = 0; x < depthInCells; x++) {
		for (int z = 0; z < widthInCells; z++) {
			for (int y = 0; y < heightInCells; y++) {

				// index to extract the 8 vertices
				int nodeIndex = x * dx + y + z * dz;

				// index to store the volume of the cell
				int volumeIndex = y + (z * heightInCells) + (x * heightInCells * widthInCells);

				std::array<glm::dvec3, 8> cellVertices = {
					mesh[nodeIndex],
					mesh[nodeIndex + dz],
					mesh[nodeIndex + dx + dz],
					mesh[nodeIndex + dx],
					mesh[nodeIndex + 1],
					mesh[nodeIndex + dz + 1],
					mesh[nodeIndex + dx + dz + 1],
					mesh[nodeIndex + dx + 1]
				};
				
				volumes[volumeIndex] = computeHexahedronVolume(cellVertices);
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Computing volumes took " << elapsed.count() << " seconds." << std::endl;
	return volumes;
}