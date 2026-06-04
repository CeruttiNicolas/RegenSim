#pragma once
#include "core/SimulationInput.hpp"
#include "fluid/Voxelizer.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

struct Hex {
    int v0, v1, v2, v3, v4, v5, v6, v7;
};

std::vector<Hex> generateHexes(const std::vector<glm::dvec3>& mesh, const SimulationInput& input);
bool exportMeshVTK(const std::string& filename, const std::vector<glm::dvec3>& points, const std::vector<double>& T, const SimulationInput& input);
bool exportVirtualFluidMeshVTK(const std::string& filename, const std::vector<glm::dvec3>& points, const std::vector<uint32_t>& triangles);

void exportSparseVoxelMeshVTK(const std::string& filename, const std::vector<SubDomain>& activeSubDomains, const glm::dvec3& minBounds, double dx, int blockSize);
void exportSubDomainMeshVTK(const std::string& filename,
    const std::vector<SubDomain>& activeSubDomains,
    const glm::dvec3& minBounds,
    double dx,
    int blockSize);
