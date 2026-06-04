#include "fluid/Voxelizer.hpp"
#include <cmath>
#include <algorithm>
#include <chrono>
#include "fluid/VoxelizerKernels.cuh"
#include "io/exportMeshVTK.hpp"

Voxelizer::Voxelizer(const SimulationInput& input, const Mesh& mesh, const Application& app, double dx_LBM, int blockSize)
    : input(input), mesh(mesh), application(app), dx(dx_LBM), blockSize(blockSize) {
}



void Voxelizer::allocateSubDomains() {
	auto startTime = std::chrono::high_resolution_clock::now();
	std::cout << "Allocating Sparse Subdomains..." << std::endl;

	int Nx = static_cast<int>(std::ceil((maxBounds.x - minBounds.x) / dx));
	macroGridSize.x = static_cast<int>(std::ceil((double)Nx / blockSize));
	Nx = macroGridSize.x * blockSize;
	double maxExtentZ = std::max(std::abs(minBounds.z), std::abs(maxBounds.z));

	int halfBlocksZ = static_cast<int>(std::ceil(maxExtentZ / (dx * blockSize)));

	macroGridSize.z = halfBlocksZ * 2;
	int Nz = macroGridSize.z * blockSize;

	minBounds.z = -(Nz / 2.0) * dx;
	maxBounds.z = (Nz / 2.0) * dx;

	double maxExtentY = std::max(std::abs(minBounds.y), std::abs(maxBounds.y));
	int halfBlocksY = static_cast<int>(std::ceil(maxExtentY / (dx * blockSize)));
	macroGridSize.y = halfBlocksY * 2;
	int Ny = macroGridSize.y * blockSize;

	minBounds.y = -(Ny / 2.0) * dx;
	maxBounds.y = (Ny / 2.0) * dx;

	int totalMacroBlocks = macroGridSize.x * macroGridSize.y * macroGridSize.z;
	lookupTable.assign(totalMacroBlocks, -1);
	int activeBlockCount = 0;

	//  Precompute AABBs for each triangle
	std::vector<TriangleAABB> triAABBs;
	triAABBs.reserve(fluidTriangles.size() / 3);
	for (size_t i = 0; i < fluidTriangles.size(); i += 3) {
		glm::dvec3 v0 = fluidVertices[fluidTriangles[i]];
		glm::dvec3 v1 = fluidVertices[fluidTriangles[i + 1]];
		glm::dvec3 v2 = fluidVertices[fluidTriangles[i + 2]];

		TriangleAABB aabb;
		aabb.minX = std::min({ v0.x, v1.x, v2.x }); aabb.maxX = std::max({ v0.x, v1.x, v2.x });
		aabb.minY = std::min({ v0.y, v1.y, v2.y }); aabb.maxY = std::max({ v0.y, v1.y, v2.y });
		aabb.minZ = std::min({ v0.z, v1.z, v2.z }); aabb.maxZ = std::max({ v0.z, v1.z, v2.z });
		triAABBs.push_back(aabb);
	}

	// Voxelize on the GPU
	std::vector<uint8_t> denseGrid((size_t)Nx * Ny * Nz, 0);
	launchVoxelizerKernel(
		fluidVertices, fluidTriangles, triAABBs,
		denseGrid, minBounds, maxBounds, dx, Nx, Ny, Nz
	);

	// Partition into subdomains and build lookup table
	int firstFluidX = Nx;
	int lastFluidX = -1;

	// Find the range of x-indices that contain fluid
	for (int x = 0; x < Nx; x++) {
		for (int y = 0; y < Ny; y++) {
			for (int z = 0; z < Nz; z++) {
				size_t flatIdx = static_cast<size_t>(x) + (static_cast<size_t>(y) * Nx) + (static_cast<size_t>(z) * Nx * Ny);
				if (denseGrid[flatIdx] == 1) {
					if (x < firstFluidX) firstFluidX = x;
					if (x > lastFluidX) lastFluidX = x;
				}
			}
		}
	}

	for (int bx = 0; bx < macroGridSize.x; bx++) {
		for (int by = 0; by < macroGridSize.y; by++) {
			for (int bz = 0; bz < macroGridSize.z; bz++) {
				SubDomain currentBlock;
				currentBlock.gridIndex = glm::ivec3(bx, by, bz);
				currentBlock.voxels.assign(blockSize * blockSize * blockSize, VoxelType::SOLID);

				bool hasFluid = false;
				for (int vx = 0; vx < blockSize; vx++) {
					for (int vy = 0; vy < blockSize; vy++) {
						for (int vz = 0; vz < blockSize; vz++) {

							int globalX = bx * blockSize + vx;
							int globalY = by * blockSize + vy;
							int globalZ = bz * blockSize + vz;

							// Skip out-of-bounds voxels (may happen in the last blocks)
							if (globalX >= Nx || globalY >= Ny || globalZ >= Nz) continue;

							size_t flatIdx = static_cast<size_t>(globalX) + (static_cast<size_t>(globalY) * Nx) + (static_cast<size_t>(globalZ) * Nx * Ny);

							if (denseGrid[flatIdx] == 1) {
								int vIdx = vx + (vy * blockSize) + (vz * blockSize * blockSize);

								// Assing boundary types based on x position
								if (globalX == firstFluidX) {
									currentBlock.voxels[vIdx] = VoxelType::OUTLET;
								}
								else if (globalX == lastFluidX) {
									currentBlock.voxels[vIdx] = VoxelType::INLET;
								}
								else {
									currentBlock.voxels[vIdx] = VoxelType::FLUID;
								}

								hasFluid = true;
							}
						}
					}
				}

				if (hasFluid) {
					currentBlock.id = activeBlockCount;
					activeSubDomains.push_back(currentBlock);
					int lookupIdx = bx + (by * macroGridSize.x) + (bz * macroGridSize.x * macroGridSize.y);
					lookupTable[lookupIdx] = activeBlockCount;
					activeBlockCount++;
				}
			}
		}
	}

	

	//exportSubDomainMeshVTK(application.getOutputPath() + "\\subdomains.vtk", activeSubDomains, minBounds, dx, blockSize);
	//exportSparseVoxelMeshVTK(application.getOutputPath() + "\\sparse_voxel_mesh.vtk", activeSubDomains, minBounds, dx, blockSize);

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = endTime - startTime;
	std::cout << "Voxelization completed in " << elapsed.count() * 1000 << " milliseconds." << std::endl;

	std::cout << "--- Voxelization Report ---" << std::endl;
	std::cout << "  Voxel Size (dx) : " << dx << std::endl;
	std::cout << "  Global Grid     : " << Nx << "x" << Ny << "x" << Nz << " (" << (long long)Nx * Ny * Nz << " theoretical voxels)" << std::endl;
	std::cout << "  Active Blocks   : " << activeBlockCount << " / " << totalMacroBlocks << std::endl;
	double memorySaved = 100.0 - ((double)activeBlockCount / totalMacroBlocks * 100.0);
	std::cout << "  VRAM Saved      : " << memorySaved << "%" << std::endl;
	std::cout << "---------------------------" << std::endl;
}
