#include "thermal/ThermalKernels.cuh"

__global__ void initTemperatureKernel(double* T_old, double* T_new, double T_initial, size_t numCells) {
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < numCells) {
		T_old[idx] = T_initial;
		T_new[idx] = T_initial;
	}
}