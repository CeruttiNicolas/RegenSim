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


bool exportMeshVTK(
    const std::string& filename,
    const std::vector<glm::dvec3>& points,
    const std::vector<double>& T,
    const SimulationInput& input)
{
    int nodesY = input.ni + input.nb + input.no + 1;
    int nodesZ = 2 * input.nw + input.na + 1;
    int nodesX = points.size() / (nodesY * nodesZ);

    int Nx = nodesX - 1;
    int Ny = nodesY - 1;
    int Nz = nodesZ - 1;

    if (T.size() != (size_t)Nx * Ny * Nz) {
        std::cerr << "Error: T field size (" << T.size() << ") must match logical cell count (" << (Nx * Ny * Nz) << ").\n";
        return false;
    }

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
    file << "DATASET UNSTRUCTURED_GRID\n";

    // --- 2. Points Data ---
    file << "POINTS " << points.size() << " float\n";

    for (const auto& p : points) {
        file << p.x << " "
             << p.y << " "
             << p.z << "\n";
    }

    // --- 3. Cell Data ---
    file << "CELLS " << hexes.size() << " " << hexes.size() * 9 << "\n";

    for (const auto& h : hexes) {
        file << "8 "
            << h.v0 << " "
            << h.v1 << " "
            << h.v2 << " "
            << h.v3 << " "
            << h.v4 << " "
            << h.v5 << " "
            << h.v6 << " "
            << h.v7 << "\n";
    }

    // --- 4. Cell Types ---
    file << "CELL_TYPES " << hexes.size() << "\n";

    for (size_t i = 0; i < hexes.size(); ++i) {
        file << "12\n";
    }

    // --- 5. Scalar Field: Temperature ---
    file << "CELL_DATA " << hexes.size() << "\n";
    file << "SCALARS T float 1\n";
    file << "LOOKUP_TABLE default\n";

    for (int i = 0; i < Nx; i++) {
        for (int j = 0; j < Ny; j++) {
            for (int k = 0; k < Nz; k++) {

                bool inChannelz = k >= input.nw && k < input.nw + input.na;
                bool inChannely = j >= input.ni && j < input.ni + input.nb;

                if (inChannely && inChannelz) {
                    continue;
                }

                int idxCell = j + (k * Ny) + (i * Ny * Nz);
                file << T[idxCell] << "\n";
            }
        }
    }

    file.close();
    return true;
}

bool exportVirtualFluidMeshVTK(
    const std::string& filename,
    const std::vector<glm::dvec3>& points,
    const std::vector<uint32_t>& triangles)
{
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return false;
    }

    int numTriangles = triangles.size() / 3;

    // --- 1. VTK Header ---
    file << "# vtk DataFile Version 3.0\n";
    file << "Virtual Fluid Surface Mesh Export\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n";

    // --- 2. Points Data ---
    file << "POINTS " << points.size() << " float\n";
    for (const auto& p : points) {
        file << p.x << " " << p.y << " " << p.z << "\n";
    }

    // --- 3. Cell Data (Triangles) ---
    file << "CELLS " << numTriangles << " " << numTriangles * 4 << "\n";
    for (size_t i = 0; i < triangles.size(); i += 3) {
        file << "3 " << triangles[i] << " " << triangles[i + 1] << " " << triangles[i + 2] << "\n";
    }

    // --- 4. Cell Types ---
    file << "CELL_TYPES " << numTriangles << "\n";
    for (int i = 0; i < numTriangles; ++i) {
        file << "5\n";
    }

    file.close();
    std::cout << "Fluid mesh exported to " << filename << std::endl;
    return true;
}


void exportSparseVoxelMeshVTK(const std::string& filename,
    const std::vector<SubDomain>& activeSubDomains,
    const glm::dvec3& minBounds,
    double dx,
    int blockSize)
{
    std::cout << "Esportazione della mesh Voxel LBM in VTK in corso..." << std::endl;

    // 1. Contiamo quanti voxel "non solidi" abbiamo in totale
    int activeVoxelsCount = 0;
    for (const auto& block : activeSubDomains) {
        for (const auto& voxel : block.voxels) {
            if (voxel != VoxelType::SOLID) {
                activeVoxelsCount++;
            }
        }
    }

    if (activeVoxelsCount == 0) {
        std::cerr << "Attenzione: Nessun voxel fluido da esportare!" << std::endl;
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file: " << filename << std::endl;
        return;
    }

    // --- HEADER VTK ---
    file << "# vtk DataFile Version 3.0\n";
    file << "RegenSim Sparse Voxel LBM Mesh\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n";

    // --- PUNTI (VERTICI DEI VOXEL) ---
    // Ogni voxel indipendente avrà 8 punti
    file << "POINTS " << (activeVoxelsCount * 8) << " double\n";

    for (const auto& block : activeSubDomains) {
        int bx = block.gridIndex.x;
        int by = block.gridIndex.y;
        int bz = block.gridIndex.z;

        for (int vx = 0; vx < blockSize; vx++) {
            for (int vy = 0; vy < blockSize; vy++) {
                for (int vz = 0; vz < blockSize; vz++) {
                    int vIdx = vx + (vy * blockSize) + (vz * blockSize * blockSize);

                    if (block.voxels[vIdx] != VoxelType::SOLID) {
                        // Calcola l'angolo in basso-sinistra-dietro del voxel
                        double x0 = minBounds.x + (bx * blockSize + vx) * dx;
                        double y0 = minBounds.y + (by * blockSize + vy) * dx;
                        double z0 = minBounds.z + (bz * blockSize + vz) * dx;

                        // Scrive gli 8 vertici dell'esaedro nell'ordine corretto per VTK (Type 12)
                        file << x0 << " " << y0 << " " << z0 << "\n";
                        file << x0 + dx << " " << y0 << " " << z0 << "\n";
                        file << x0 + dx << " " << y0 + dx << " " << z0 << "\n";
                        file << x0 << " " << y0 + dx << " " << z0 << "\n";

                        file << x0 << " " << y0 << " " << z0 + dx << "\n";
                        file << x0 + dx << " " << y0 << " " << z0 + dx << "\n";
                        file << x0 + dx << " " << y0 + dx << " " << z0 + dx << "\n";
                        file << x0 << " " << y0 + dx << " " << z0 + dx << "\n";
                    }
                }
            }
        }
    }

    // --- CELLE (CONNETTIVITA') ---
    // activeVoxelsCount celle, ogni riga ha 9 valori: (8 punti) + (Indice p0...p7)
    file << "\nCELLS " << activeVoxelsCount << " " << (activeVoxelsCount * 9) << "\n";
    int ptOffset = 0;
    for (int i = 0; i < activeVoxelsCount; i++) {
        file << "8 " << ptOffset << " " << ptOffset + 1 << " " << ptOffset + 2 << " " << ptOffset + 3 << " "
            << ptOffset + 4 << " " << ptOffset + 5 << " " << ptOffset + 6 << " " << ptOffset + 7 << "\n";
        ptOffset += 8;
    }

    // --- TIPI DI CELLA ---
    // 12 = VTK_HEXAHEDRON
    file << "\nCELL_TYPES " << activeVoxelsCount << "\n";
    for (int i = 0; i < activeVoxelsCount; i++) {
        file << "12\n";
    }

    // --- DATI DELLA CELLA (IL VOXEL TYPE!) ---
    file << "\nCELL_DATA " << activeVoxelsCount << "\n";
    file << "SCALARS VoxelType int 1\n";
    file << "LOOKUP_TABLE default\n";

    for (const auto& block : activeSubDomains) {
        for (int vx = 0; vx < blockSize; vx++) {
            for (int vy = 0; vy < blockSize; vy++) {
                for (int vz = 0; vz < blockSize; vz++) {
                    int vIdx = vx + (vy * blockSize) + (vz * blockSize * blockSize);

                    if (block.voxels[vIdx] != VoxelType::SOLID) {
                        // Cast dell'enum a intero (es. FLUID=1, INLET=2, OUTLET=3)
                        file << static_cast<int>(block.voxels[vIdx]) << "\n";
                    }
                }
            }
        }
    }

    file.close();
    std::cout << "Mesh esportata con successo in: " << filename << std::endl;
}

void exportSubDomainMeshVTK(const std::string& filename,
    const std::vector<SubDomain>& activeSubDomains,
    const glm::dvec3& minBounds,
    double dx,
    int blockSize)
{
    std::cout << "Esportazione dei SubDomain (Macro-Blocchi) in VTK..." << std::endl;

    if (activeSubDomains.empty()) {
        std::cerr << "Attenzione: Nessun SubDomain attivo da esportare!" << std::endl;
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file: " << filename << std::endl;
        return;
    }

    int blockCount = activeSubDomains.size();

    // Dimensione fisica del lato del Macro-Blocco
    double L = blockSize * dx;

    // --- HEADER VTK ---
    file << "# vtk DataFile Version 3.0\n";
    file << "RegenSim Sparse SubDomain Blocks\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n";

    // --- PUNTI (VERTICI DEI MACRO-BLOCCHI) ---
    file << "POINTS " << (blockCount * 8) << " double\n";

    for (const auto& block : activeSubDomains) {
        // Calcolo dell'origine (angolo in basso-sinistra-dietro) del Macro-Blocco
        double x0 = minBounds.x + (block.gridIndex.x * L);
        double y0 = minBounds.y + (block.gridIndex.y * L);
        double z0 = minBounds.z + (block.gridIndex.z * L);

        // Scrittura degli 8 vertici del SubDomain
        file << x0 << " " << y0 << " " << z0 << "\n";
        file << x0 + L << " " << y0 << " " << z0 << "\n";
        file << x0 + L << " " << y0 + L << " " << z0 << "\n";
        file << x0 << " " << y0 + L << " " << z0 << "\n";

        file << x0 << " " << y0 << " " << z0 + L << "\n";
        file << x0 + L << " " << y0 << " " << z0 + L << "\n";
        file << x0 + L << " " << y0 + L << " " << z0 + L << "\n";
        file << x0 << " " << y0 + L << " " << z0 + L << "\n";
    }

    // --- CELLE (CONNETTIVITA') ---
    file << "\nCELLS " << blockCount << " " << (blockCount * 9) << "\n";
    int ptOffset = 0;
    for (int i = 0; i < blockCount; i++) {
        file << "8 " << ptOffset << " " << ptOffset + 1 << " " << ptOffset + 2 << " " << ptOffset + 3 << " "
            << ptOffset + 4 << " " << ptOffset + 5 << " " << ptOffset + 6 << " " << ptOffset + 7 << "\n";
        ptOffset += 8;
    }

    // --- TIPI DI CELLA (12 = VTK_HEXAHEDRON) ---
    file << "\nCELL_TYPES " << blockCount << "\n";
    for (int i = 0; i < blockCount; i++) {
        file << "12\n";
    }

    // --- DATI DELLA CELLA (Assegniamo l'ID del blocco per colorarli!) ---
    file << "\nCELL_DATA " << blockCount << "\n";
    file << "SCALARS SubDomainID int 1\n";
    file << "LOOKUP_TABLE default\n";

    for (const auto& block : activeSubDomains) {
        file << block.id << "\n";
    }

    file.close();
    std::cout << "SubDomain esportati con successo in: " << filename << std::endl;
}