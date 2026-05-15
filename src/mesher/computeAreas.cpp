#include "mesher/Mesher.hpp"

void Mesher::computeAreas(const SimulationInput& input, Mesh& mesh) {
	auto start = std::chrono::high_resolution_clock::now();
	std::cout << "Computing areas..." << std::endl;
	
	int Nx = mesh.Nx;
	int Ny = mesh.Ny;
	int Nz = mesh.Nz;

	int depthInNodes = Nx + 1;
	int heightInNodes = Ny + 1;
	int widthInNodes = Nz + 1;
	
	mesh.areasX.resize((Nx + 1) * Ny * Nz);
	mesh.areasY.resize(Nx * (Ny + 1) * Nz);
	mesh.areasZ.resize(Nx * Ny * (Nz + 1));
	
	auto getIndex = [&](int x, int y, int z) {
		return y + (z * heightInNodes) + (x * heightInNodes * widthInNodes);
	};

	// areas normal to x-axis
	for (int x = 0; x <= Nx; x++) {
		for (int z = 0; z < Nz; z++) {
			for (int y = 0; y < Ny; y++) {
				int areaIndex = y + (z * Ny) + (x * Ny * Nz);
				mesh.areasX[areaIndex] = computeQuadArea(
					mesh.vertices[getIndex(x, y, z)],
					mesh.vertices[getIndex(x, y + 1, z)],
					mesh.vertices[getIndex(x, y + 1, z + 1)],
					mesh.vertices[getIndex(x, y, z + 1)]
				);
			}
		}
	}

	// areas normal to y-axis
	for (int x = 0; x < Nx; x++) {
		for (int z = 0; z < Nz; z++) {
			for (int y = 0; y <= Ny; y++) {
				int areaIndex = y + (z * (Ny + 1)) + (x * (Ny + 1) * Nz);
				mesh.areasY[areaIndex] = computeQuadArea(
					mesh.vertices[getIndex(x, y, z)],
					mesh.vertices[getIndex(x + 1, y, z)],
					mesh.vertices[getIndex(x + 1, y, z + 1)],
					mesh.vertices[getIndex(x, y, z + 1)]
				);
			}
		}
	}

	// areas normal to z-axis
	for (int x = 0; x < Nx; x++) {
		for (int z = 0; z <= Nz; z++) {
			for (int y = 0; y < Ny; y++) {
				int areaIndex = y + (z * Ny) + (x * Ny * (Nz + 1));
				mesh.areasZ[areaIndex] = computeQuadArea(
					mesh.vertices[getIndex(x, y, z)],
					mesh.vertices[getIndex(x + 1, y, z)],
					mesh.vertices[getIndex(x + 1, y + 1, z)],
					mesh.vertices[getIndex(x, y + 1, z)]
				);
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Computing areas took " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}