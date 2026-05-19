#include "mesher/Mesher.hpp"

void Mesher::computeDistances(const SimulationInput& input, Mesh& mesh) {
	auto start = std::chrono::high_resolution_clock::now();
	std::cout << "Computing centroid distances..." << std::endl;

    int Nx = mesh.Nx;
    int Ny = mesh.Ny;
    int Nz = mesh.Nz;

    int heightInNodes = Ny + 1;
    int widthInNodes = Nz + 1;

    mesh.distX.resize((Nx + 1) * Ny * Nz);
    mesh.distY.resize(Nx * (Ny + 1) * Nz);
    mesh.distZ.resize(Nx * Ny * (Nz + 1));

    auto getIndex = [&](int x, int y, int z) {
        return y + (z * heightInNodes) + (x * heightInNodes * widthInNodes);
	};

    auto getCellCentroid = [&](int x, int y, int z) {
		glm::dvec3 c(0.0);
        c += mesh.vertices[getIndex(x, y, z)];
        c += mesh.vertices[getIndex(x, y + 1, z)];
        c += mesh.vertices[getIndex(x, y, z + 1)];
        c += mesh.vertices[getIndex(x, y + 1, z + 1)];
        c += mesh.vertices[getIndex(x + 1, y, z)];
        c += mesh.vertices[getIndex(x + 1, y + 1, z)];
        c += mesh.vertices[getIndex(x + 1, y, z + 1)];
        c += mesh.vertices[getIndex(x + 1, y + 1, z + 1)];
        return c / 8.0;
    };

	// distances along x-axis
    for (int x = 0; x <= Nx; x++) {
        for (int z = 0; z < Nz; z++) {
            for (int y = 0; y < Ny; y++) {
                int nodeIndex = y + (z * Ny) + (x * Ny * Nz);
                if (x == 0) { // First faces
                    glm::dvec3 cellC = getCellCentroid(0, y, z);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(0, y, z)] + mesh.vertices[getIndex(0, y + 1, z)] + mesh.vertices[getIndex(0, y + 1, z + 1)] + mesh.vertices[getIndex(0, y, z + 1)]) / 4.0;
                    mesh.distX[nodeIndex] = glm::distance(cellC, faceC);
                }
                else if (x == Nx) { // Last faces
                    glm::dvec3 cellC = getCellCentroid(Nx-1, y, z);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(Nx, y, z)] + mesh.vertices[getIndex(Nx, y + 1, z)] + mesh.vertices[getIndex(Nx, y + 1, z + 1)] + mesh.vertices[getIndex(Nx, y, z + 1)]) / 4.0;
                    mesh.distX[nodeIndex] = glm::distance(cellC, faceC);
                }
                else { // Cell to cell
                    glm::dvec3 cellF = getCellCentroid(x - 1, y, z);
                    glm::dvec3 cellB = getCellCentroid(x, y, z);
                    mesh.distX[nodeIndex] = glm::distance(cellF, cellB);
                }
            }
        }
    }

    // distances along y-axis
    for (int x = 0; x < Nx; x++) {
        for (int z = 0; z < Nz; z++) {
            for (int y = 0; y <= Ny; y++) {
                int nodeIndex = y + (z * (Ny + 1)) + (x * (Ny + 1) * Nz);
                if (y == 0) { // Bottom faces
                    glm::dvec3 cellC = getCellCentroid(x, 0, z);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(x, 0, z)] + mesh.vertices[getIndex(x + 1, 0, z)] + mesh.vertices[getIndex(x + 1, 0, z + 1)] + mesh.vertices[getIndex(x, 0, z + 1)]) / 4.0;
                    mesh.distY[nodeIndex] = glm::distance(cellC, faceC);
                }
                else if (y == Ny) { // Top faces
                    glm::dvec3 cellC = getCellCentroid(x, Ny - 1, z);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(x, Ny, z)] + mesh.vertices[getIndex(x + 1, Ny, z)] + mesh.vertices[getIndex(x + 1, Ny, z + 1)] + mesh.vertices[getIndex(x, Ny, z + 1)]) / 4.0;
                    mesh.distY[nodeIndex] = glm::distance(cellC, faceC);
                }
                else { // Cell to cell
                    glm::dvec3 cellS = getCellCentroid(x, y - 1, z);
                    glm::dvec3 cellN = getCellCentroid(x, y, z);
                    mesh.distY[nodeIndex] = glm::distance(cellS, cellN);
                }
            }
        }
    }

    // distances along z-axis
    for (int x = 0; x < Nx; x++) {
        for (int z = 0; z <= Nz; z++) {
            for (int y = 0; y < Ny; y++) {
                int nodeIndex = y + (z * Ny) + (x * Ny * (Nz + 1));
                if (z == 0) { // Left faces
                    glm::dvec3 cellC = getCellCentroid(x, y, 0);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(x, y, 0)] + mesh.vertices[getIndex(x + 1, y, 0)] + mesh.vertices[getIndex(x + 1, y + 1, 0)] + mesh.vertices[getIndex(x, y + 1, 0)]) / 4.0;
                    mesh.distZ[nodeIndex] = glm::distance(cellC, faceC);
                }
                else if (z == Nz) { // Right faces
                    glm::dvec3 cellC = getCellCentroid(x, y, Nz - 1);
                    glm::dvec3 faceC = (mesh.vertices[getIndex(x, y, Nz)] + mesh.vertices[getIndex(x + 1, y, Nz)] + mesh.vertices[getIndex(x + 1, y + 1, Nz)] + mesh.vertices[getIndex(x, y + 1, Nz)]) / 4.0;
                    mesh.distZ[nodeIndex] = glm::distance(cellC, faceC);
                }
                else { // Cell to cell
                    glm::dvec3 cellW = getCellCentroid(x, y, z - 1);
                    glm::dvec3 cellE = getCellCentroid(x, y, z);
                    mesh.distZ[nodeIndex] = glm::distance(cellW, cellE);
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Computing distances took " << elapsed.count() * 1000 << " milliseconds." << std::endl;
}