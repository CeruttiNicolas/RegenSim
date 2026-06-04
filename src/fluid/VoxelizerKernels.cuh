#pragma once
#include <vector>
#include <cstdint>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct TriangleAABB {
    double minX, maxX, minY, maxY, minZ, maxZ;
};

void launchVoxelizerKernel(
    const std::vector<glm::dvec3>& vertices,
    const std::vector<uint32_t>& indices,
    const std::vector<TriangleAABB>& aabbs,
    std::vector<uint8_t>& voxelGrid,
    glm::dvec3 minBounds, glm::dvec3 maxBounds, double dx,
    int Nx, int Ny, int Nz
);