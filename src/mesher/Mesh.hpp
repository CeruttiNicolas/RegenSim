#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vector>

struct Mesh {
	int Nx, Ny, Nz;

	std::vector<glm::dvec3> vertices;
	std::vector<uint32_t> triangleIndices;
	std::vector<uint32_t> lineIndices;

	std::vector<double> volumes;

	std::vector<double> areasX;
	std::vector<double> areasY;
	std::vector<double> areasZ;

	std::vector<double> distX;
	std::vector<double> distY;
	std::vector<double> distZ;

	double shortestEdgeLength;
};