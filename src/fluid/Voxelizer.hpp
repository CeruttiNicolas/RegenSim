#pragma once
#include "core/SimulationInput.hpp"
#include "mesher/Mesh.hpp"
#include "core/Application.hpp"
#include <vector>
#include <chrono>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>

enum class VoxelType : uint8_t {
	SOLID = 0,
	FLUID = 1,
	WALL_BOUNCE = 2,
	INLET = 3,
	OUTLET = 4
};

struct SubDomain {
	int id;
	glm::ivec3 gridIndex;
	int neighbors[26];
	std::vector<VoxelType> voxels;
};

class Voxelizer {
public:
	Voxelizer(const SimulationInput& simInput, const Mesh& mesh, const Application& app, double dx_LBM, int blockSize = 16);
	void run() {
		generateFluidMesh();
		allocateSubDomains();
	}

	// Data to be sent to the GPU
	std::vector<SubDomain> activeSubDomains;
	std::vector<int> lookupTable;
	glm::ivec3 macroGridSize;

private:
	const SimulationInput& input;
	const Mesh& mesh;
	const Application& application;
	double dx;
	int blockSize;

	glm::dvec3 minBounds, maxBounds;

	std::vector<glm::dvec3> fluidVertices;
	std::vector<uint32_t> fluidTriangles;

	void generateFluidMesh();
	void allocateSubDomains();
};
