#include "exportMeshVTK.hpp"

std::vector<Hex> generateHexes(const std::vector<glm::dvec3>& mesh, const SimulationInput& input) {
    std::vector<Hex> hexes;
    int height = input.ni + input.nb + input.no + 1;
    int width = 2 * input.nw + input.na + 1;
    int depth = mesh.size() / (height * width);

    auto getIndex = [&](int x, int y, int z) {
        return y + (z * height) + (x * height * width);
        };

    for (int i = 0; i < depth - 1; i++) {
        for (int j = 0; j < height - 1; j++) {
            for (int k = 0; k < width - 1; k++) {
                bool inChannelz = k >= input.nw && k < input.nw + input.na;
                bool inChannely = j >= input.ni && j < input.ni + input.nb;

                if (inChannely && inChannelz) {
                    continue;
                }
                hexes.push_back({ getIndex(i, j, k), getIndex(i, j, k + 1), getIndex(i + 1, j, k + 1), getIndex(i + 1, j, k),
                                  getIndex(i, j + 1, k), getIndex(i, j + 1, k + 1), getIndex(i + 1, j + 1, k + 1), getIndex(i + 1, j + 1, k)
                });
            }
        }
    }
    return hexes;
}

bool exportMeshVTK(const std::string& filename, const std::vector<glm::dvec3>& points, const SimulationInput& input) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return false;
    }
	std::vector<Hex> hexes = generateHexes(points, input);
    // --- 1. VTK Header ---
    file << "# vtk DataFile Version 3.0\n";
    file << "Hexahedron Mesh Export\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n"; // Changed from POLYDATA

    // --- 2. Points Data ---
    file << "POINTS " << points.size() << " float\n";
    for (const auto& p : points) {
        file << p.x << " " << p.y << " " << p.z << "\n";
    }

    // --- 3. Cell Data ---
    file << "CELLS " << hexes.size() << " " << hexes.size() * 9 << "\n";
    for (const auto& h : hexes) {
        file << "8 " << h.v0 << " " << h.v1 << " " << h.v2 << " " << h.v3 << " "
            << h.v4 << " " << h.v5 << " " << h.v6 << " " << h.v7 << "\n";
    }

    // --- 4. Cell Types ---
    file << "CELL_TYPES " << hexes.size() << "\n";
    for (size_t i = 0; i < hexes.size(); ++i) {
        file << "12\n";
    }

    file.close();
    return true;
}

