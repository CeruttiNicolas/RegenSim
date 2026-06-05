#include "thermal/ThermalKernels.cuh"

__global__ void initTemperatureKernel(double* T_anchor, double* T_read, double T_initial, size_t numCells) {
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < numCells) {
		T_anchor[idx] = T_initial;
		T_read[idx] = T_initial;
	}
}