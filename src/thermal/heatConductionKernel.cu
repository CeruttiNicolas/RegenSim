#include "thermal/ThermalKernels.cuh"

__global__ void heatConductionKernel(
    const double* T_anchor, const double* T_read, double* T_write,
    const double* volumes,
    const double* areasX, const double* areasY, const double* areasZ,
    const double* distX, const double* distY, const double* distZ,
    const double* alphaTable, const double* kTable,
    const double* Tgas_star, const double* hgas,
    double dt, double Tref, double Lref, double alpha_stage,
    int Nx, int Ny, int Nz,
    int ni, int nb, int nw, int na)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;

    // Skip out-of-bounds and non-solid cells
    if (x >= Nx || y >= Ny || z >= Nz) return;
    if (!IS_SOLID(y, z)) return;

    // Get volume of the current cell
    int idxP = y + (z * Ny) + (x * Ny * Nz);
    double VP = volumes[idxP];

	// Read the frozen state Q(0) and the current stage state Q(i-1)
    double TP_anchor = T_anchor[idxP];
	double TP_read = T_read[idxP];

    // Retrieve material properties from LUTs
    double T_real = TP_anchor * Tref;
    double T_clamp = fmax(0.0, fmin(T_real, 3999.0));
    int idxTLUT = (int)(T_clamp); // Assuming 1 K resolution in LUT
    double frac = T_clamp - idxTLUT;
    double alphaP = alphaTable[idxTLUT] + frac * (alphaTable[idxTLUT + 1] - alphaTable[idxTLUT]);
    double kP = kTable[idxTLUT] + frac * (kTable[idxTLUT + 1] - kTable[idxTLUT]);

    // Compute Fourier number for the current cell
    double Fo = alphaP * dt / (Lref * Lref);

    double sumDT = 0.0;
    // X axis
    if (x > 0) {
        // Thermal conduction with Front neighbor
        int idxCellF = y + (z * Ny) + ((x - 1) * Ny * Nz);
        int idxFaceF = y + (z * Ny) + (x * Ny * Nz);
        sumDT += Fo * (areasX[idxFaceF] / (VP * distX[idxFaceF])) * (T_read[idxCellF] - TP_read);
    }
    if (x < Nx - 1) {
        // Thermal conduction with Back neighbor
        int idxCellB = y + (z * Ny) + ((x + 1) * Ny * Nz);
        int idxFaceB = y + (z * Ny) + ((x + 1) * Ny * Nz);
        sumDT += Fo * (areasX[idxFaceB] / (VP * distX[idxFaceB])) * (T_read[idxCellB] - TP_read);
    }

    // Y axis
    if (y > 0 && IS_SOLID(y - 1, z)) {
        // Thermal conduction with South neighbor
        int idxCellS = (y - 1) + (z * Ny) + (x * Ny * Nz);
        int idxFaceS = y + (z * (Ny + 1)) + (x * (Ny + 1) * Nz);
        sumDT += Fo * (areasY[idxFaceS] / (VP * distY[idxFaceS])) * (T_read[idxCellS] - TP_read);
    }
    else if (y == 0) {
        // Robin boundary condition for the cells wetted by the engine exhaust
        int idxFace = z * (Ny + 1) + (x * (Ny + 1) * Nz);
        double OneoverBi = kP / (fmax(hgas[x], 1e-6) * Lref);
        sumDT += Fo * (areasY[idxFace] / VP) * (Tgas_star[x] - TP_read) / (OneoverBi + distY[idxFace]);
    }

    if (y < Ny - 1 && IS_SOLID(y + 1, z)) {
        // Thermal conduction with North neighbor
        int idxCellN = (y + 1) + (z * Ny) + (x * Ny * Nz);
        int idxFaceN = (y + 1) + (z * (Ny + 1)) + (x * (Ny + 1) * Nz);
        sumDT += Fo * (areasY[idxFaceN] / (VP * distY[idxFaceN])) * (T_read[idxCellN] - TP_read);
    }
    else if (y == Ny - 1) {
        // Robin boundary condition for the cells wetted by the outer air
        int idxFace = Ny + (z * (Ny + 1)) + (x * (Ny + 1) * Nz);
        double OneoverBi = kP / (15.0 * Lref); // Assuming h_air = 15 W/m^2K
        double Tamb_star = 298.15 / Tref; // Assuming ambient temperature of 298.15 K
        sumDT += Fo * (areasY[idxFace] / VP) * (Tamb_star - TP_read) / (OneoverBi + distY[idxFace]);
    }

    // Z axis
    if (z > 0 && IS_SOLID(y, z - 1)) {
        // Thermal conduction with West neighbor
        int idxCellW = y + ((z - 1) * Ny) + (x * Ny * Nz);
        int idxFaceW = y + (z * Ny) + (x * Ny * (Nz + 1));
        sumDT += Fo * (areasZ[idxFaceW] / (VP * distZ[idxFaceW])) * (T_read[idxCellW] - TP_read);
    }
    if (z < Nz - 1 && IS_SOLID(y, z + 1)) {
        // Thermal conduction with East neighbor
        int idxCellE = y + ((z + 1) * Ny) + (x * Ny * Nz);
        int idxFaceE = y + ((z + 1) * Ny) + (x * Ny * (Nz + 1));
        sumDT += Fo * (areasZ[idxFaceE] / (VP * distZ[idxFaceE])) * (T_read[idxCellE] - TP_read);
    }

	double T_new_stage = TP_anchor + alpha_stage * sumDT;

    // Placeholder Dirichlet BC for the cells touching the coolant
    bool touchesChannel = false;
    if (y < Ny - 1 && !IS_SOLID(y + 1, z)) touchesChannel = true;
    if (y > 0 && !IS_SOLID(y - 1, z))      touchesChannel = true;
    if (z < Nz - 1 && !IS_SOLID(y, z + 1)) touchesChannel = true;
    if (z > 0 && !IS_SOLID(y, z - 1))      touchesChannel = true;

    if (touchesChannel) {
        T_write[idxP] = 400.0 / Tref;
    }
    else {
        T_write[idxP] = T_new_stage;
    }
}
