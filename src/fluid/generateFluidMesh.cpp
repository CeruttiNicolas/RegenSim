#include "fluid/Voxelizer.hpp"
#include "io/exportMeshVTK.hpp"

void Voxelizer::generateFluidMesh() {
	auto startTime = std::chrono::high_resolution_clock::now();
	std::cout << "Generating virtual fluid mesh..." << std::endl;

	int height = mesh.Ny + 1;
	int width = mesh.Nz + 1;

	auto getIndex = [&](int x, int y, int z) {
		return y + (z * height) + (x * height * width);
		};

	fluidVertices = mesh.vertices;

	auto addQuad = [&](int v0, int v1, int v2, int v3) {
		fluidTriangles.push_back(v0); fluidTriangles.push_back(v1); fluidTriangles.push_back(v2);
		fluidTriangles.push_back(v0); fluidTriangles.push_back(v2); fluidTriangles.push_back(v3);
		};

	// Compute bounding box and add padding along x for the inlet
	glm::dvec3 originalMin(1e9), originalMax(-1e9);
	for (const auto& v : fluidVertices) {
		originalMin = glm::min(originalMin, v);
		originalMax = glm::max(originalMax, v);
	}

	double extrudeLength = 0.002 / input.refLength; // Add 2mm (non-dimensionalized) of straight tube for the inlet
	double targetMaxX = originalMax.x + extrudeLength;

	// Reserve space for one additional layer of vertices on the inlet side
	fluidVertices.reserve(fluidVertices.size() + height * width);

	// Extrude inlet face vertices
	for (int z = 0; z < width; z++) {
		for (int y = 0; y < height; y++) {
			glm::dvec3 v = fluidVertices[getIndex(mesh.Nx, y, z)];
			v.x = targetMaxX;
			fluidVertices.push_back(v);
		}
	}

	// Extract inner walls from the original curved channel
	for (int x = 0; x <= mesh.Nx; x++) {
		for (int z = input.nw; z < input.nw + input.na; z++) {
			// Bottom Wall
			addQuad(getIndex(x, input.ni, z), getIndex(x + 1, input.ni, z),
				getIndex(x + 1, input.ni, z + 1), getIndex(x, input.ni, z + 1));
			// Top Wall (Reverse Winding)
			addQuad(getIndex(x, input.ni + input.nb, z + 1), getIndex(x + 1, input.ni + input.nb, z + 1),
				getIndex(x + 1, input.ni + input.nb, z), getIndex(x, input.ni + input.nb, z));
		}
	}
	for (int x = 0; x <= mesh.Nx; x++) {
		for (int y = input.ni; y < input.ni + input.nb; y++) {
			// Left wall
			addQuad(getIndex(x, y, input.nw), getIndex(x, y + 1, input.nw),
				getIndex(x + 1, y + 1, input.nw), getIndex(x + 1, y, input.nw));
			// Right wall
			addQuad(getIndex(x + 1, y, input.nw + input.na), getIndex(x + 1, y + 1, input.nw + input.na),
				getIndex(x, y + 1, input.nw + input.na), getIndex(x, y, input.nw + input.na));
		}
	}

	// Add end caps for inlet and outlet
	for (int z = input.nw; z < input.nw + input.na; z++) {
		for (int y = input.ni; y < input.ni + input.nb; y++) {
			// Outlet Cap
			addQuad(getIndex(0, y, z), getIndex(0, y, z + 1),
				getIndex(0, y + 1, z + 1), getIndex(0, y + 1, z));

			// Inlet Cap
			addQuad(getIndex(mesh.Nx + 1, y, z), getIndex(mesh.Nx + 1, y + 1, z),
				getIndex(mesh.Nx + 1, y + 1, z + 1), getIndex(mesh.Nx + 1, y, z + 1));
		}
	}

	// Update bounding boxes for the Voxelizer Ray-Casting Logic
	minBounds = originalMin;
	maxBounds = originalMax;
	maxBounds.x = targetMaxX;

	// Add safety padding
	double padding = 2.0 * dx;
	minBounds.x -= padding; maxBounds.x += padding;
	minBounds.y -= padding; minBounds.z -= padding;
	maxBounds.y += padding; maxBounds.z += padding;

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = endTime - startTime;
	std::cout << "Virtual fluid mesh generated in " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}
