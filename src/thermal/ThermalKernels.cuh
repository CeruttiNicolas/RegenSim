#pragma once
#include <cuda_runtime.h>

#define IS_SOLID(cy, cz) (!((cy >= ni && cy < ni + nb) && (cz >= nw && cz < nw + na)))

__global__ void initTemperatureKernel(double* T_old, double* T_new, double T_initial, size_t numCells);

__global__ void heatConductionKernel(
    const double* T_old, double* T_new,
    const double* volumes,
    const double* areasX, const double* areasY, const double* areasZ,
    const double* distX, const double* distY, const double* distZ,
    const double* alphaTable, const double* kTable,
    const double* Tgas_star, const double* hgas,
    double dt, double Tref, double Lref,
    int Nx, int Ny, int Nz,
    int ni, int nb, int nw, int na);