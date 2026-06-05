#include "thermal/ThermalSolver.cuh"
#include "thermal/ThermalKernels.cuh"
#include "thermal/CudaUtils.cuh"
#include <cuda_runtime.h>

ThermalSolver::ThermalSolver(const SimulationInput& input, const Mesh& mesh) {
	std::cout << "Initializing thermal solver..." << std::endl;
	this->currentTime = 0.0;

	// Material properties
	this->alphaMax = input.alphaMax;

	// Stability parameters
	this->Lref = input.refLength;
	this->Lcrit = mesh.shortestEdgeLength;
	this->knockdownFactor = input.knockdownFactor;

	// Reference temperature
    this->Tref = input.Tref;

	// Grid dimensions
	this->Nx = mesh.Nx;
	this->Ny = mesh.Ny;
	this->Nz = mesh.Nz;

	// Subdivisions
	this->ni = input.ni;
	this->nb = input.nb;
	this->nw = input.nw;
	this->na = input.na;

	// Runge Kutta coefficients
	this->rk_alphas = input.rk_alphas;

	computeFourierTimestep();

    std::vector<double> h_Tgas_star(Nx, 0.0);
    std::vector<double> h_hgas(Nx, 0.0);

    for (int x = 0; x < Nx; x++) {
        int v1 = x * (Ny + 1) * (Nz + 1);
        int v2 = (x + 1) * (Ny + 1) * (Nz + 1);

        double cell_x = (mesh.vertices[v1].x + mesh.vertices[v2].x) / 2.0;

        h_Tgas_star[x] = interpolate1D(cell_x, input.gas_xs, input.gas_Ts);
        h_hgas[x] = interpolate1D(cell_x, input.gas_xs, input.gas_hs);
    }

    std::vector<double> h_alphaTable = input.alphaT.generateLUT(0.0, 4000.0, 1.0);
    std::vector<double> h_kTable = input.kT.generateLUT(0.0, 4000.0, 1.0);

	size_t tableSize = h_alphaTable.size();

	// Compute grid sizes
	size_t numCells = Nx * Ny * Nz;
	size_t numFacesX = (Nx + 1) * Ny * Nz;
	size_t numFacesY = Nx * (Ny + 1) * Nz;
	size_t numFacesZ = Nx * Ny * (Nz + 1);

	// Allocate GPU memory for geometry
    // Volumes
	CUDA_CHECK(cudaMalloc(&d_Volumes, numCells * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_Volumes, mesh.volumes.data(), numCells * sizeof(double), cudaMemcpyHostToDevice));

    // Areas
	CUDA_CHECK(cudaMalloc(&d_AreasX, numFacesX * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_AreasX, mesh.areasX.data(), numFacesX * sizeof(double), cudaMemcpyHostToDevice));

	CUDA_CHECK(cudaMalloc(&d_AreasY, numFacesY * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_AreasY, mesh.areasY.data(), numFacesY * sizeof(double), cudaMemcpyHostToDevice));

	CUDA_CHECK(cudaMalloc(&d_AreasZ, numFacesZ * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_AreasZ, mesh.areasZ.data(), numFacesZ * sizeof(double), cudaMemcpyHostToDevice));

    // Distances
	CUDA_CHECK(cudaMalloc(&d_DistX, numFacesX * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_DistX, mesh.distX.data(), numFacesX * sizeof(double), cudaMemcpyHostToDevice));

	CUDA_CHECK(cudaMalloc(&d_DistY, numFacesY * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_DistY, mesh.distY.data(), numFacesY * sizeof(double), cudaMemcpyHostToDevice));

	CUDA_CHECK(cudaMalloc(&d_DistZ, numFacesZ * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_DistZ, mesh.distZ.data(), numFacesZ * sizeof(double), cudaMemcpyHostToDevice));

    // Allocate T and h profiles on the GPU
    CUDA_CHECK(cudaMalloc(&d_Tgas_star, Nx * sizeof(double)));
    CUDA_CHECK(cudaMemcpy(d_Tgas_star, h_Tgas_star.data(), Nx * sizeof(double), cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc(&d_hgas, Nx * sizeof(double)));
    CUDA_CHECK(cudaMemcpy(d_hgas, h_hgas.data(), Nx * sizeof(double), cudaMemcpyHostToDevice));

	// Allocate LUTs memory on the GPU
	CUDA_CHECK(cudaMalloc(&d_alphaTable, tableSize * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_alphaTable, h_alphaTable.data(), tableSize * sizeof(double), cudaMemcpyHostToDevice));

	CUDA_CHECK(cudaMalloc(&d_kTable, tableSize * sizeof(double)));
	CUDA_CHECK(cudaMemcpy(d_kTable, h_kTable.data(), tableSize * sizeof(double), cudaMemcpyHostToDevice));

	// Allocate GPU memory for temperatures
	CUDA_CHECK(cudaMalloc(&d_T_anchor, numCells * sizeof(double)));
	CUDA_CHECK(cudaMalloc(&d_T_read, numCells * sizeof(double)));
	CUDA_CHECK(cudaMalloc(&d_T_write, numCells * sizeof(double)));

	// Populate initial temperatures on the device
	double TstarInitial = 293.15 / Tref;

	int threadsPerBlock1D = 256;
	int blocksPerGrid1D = (numCells + threadsPerBlock1D - 1) / threadsPerBlock1D;

	initTemperatureKernel KERNEL_LAUNCH(blocksPerGrid1D, threadsPerBlock1D) (d_T_anchor, d_T_read, TstarInitial, numCells);

	CUDA_CHECK(cudaMemcpy(d_T_write, d_T_anchor, numCells * sizeof(double), cudaMemcpyDeviceToDevice));

	cudaDeviceSynchronize();


	size_t free_byte;
	size_t total_byte;
	CUDA_CHECK(cudaMemGetInfo(&free_byte, &total_byte));

	double free_db = (double)free_byte;
	double total_db = (double)total_byte;
	double used_db = total_db - free_db;

	std::cout << "--- GPU VRAM Status ---------" << std::endl;
	std::cout << "  Total VRAM : " << total_db / (1024.0 * 1024.0) << " MB" << std::endl;
	std::cout << "  Used VRAM  : " << used_db / (1024.0 * 1024.0) << " MB" << std::endl;
	std::cout << "  Free VRAM  : " << free_db / (1024.0 * 1024.0) << " MB" << std::endl;
	std::cout << "-----------------------------\n" << std::endl;
	std::cout << "Initialized thermal solver.\n" << std::endl;
}

ThermalSolver::~ThermalSolver() {
    cudaFree(d_Volumes);
    cudaFree(d_AreasX);
    cudaFree(d_AreasY);
    cudaFree(d_AreasZ);
    cudaFree(d_DistX);
    cudaFree(d_DistY);
    cudaFree(d_DistZ);
    cudaFree(d_T_anchor);
	cudaFree(d_T_read);
	cudaFree(d_T_write);
    cudaFree(d_Tgas_star);
    cudaFree(d_hgas);
	cudaFree(d_alphaTable);
	cudaFree(d_kTable);
}

void ThermalSolver::solveStep() {
    dim3 threadsPerBlock(8, 8, 8);
    dim3 numBlocks(
        (Nx + threadsPerBlock.x - 1) / threadsPerBlock.x,
        (Ny + threadsPerBlock.y - 1) / threadsPerBlock.y,
        (Nz + threadsPerBlock.z - 1) / threadsPerBlock.z
    );

	// Reset the read buffer to the frozen Q(0) state
	size_t numCells = (size_t)Nx * Ny * Nz;
	CUDA_CHECK(cudaMemcpy(d_T_read, d_T_anchor, numCells * sizeof(double), cudaMemcpyDeviceToDevice));
	
	// Multi-stage loop handled by the host
	int numStages = rk_alphas.size();
	for (int stage = 0; stage < numStages; stage++) {
		heatConductionKernel KERNEL_LAUNCH(numBlocks, threadsPerBlock) (
			d_T_anchor, d_T_read, d_T_write,
			d_Volumes,
			d_AreasX, d_AreasY, d_AreasZ,
			d_DistX, d_DistY, d_DistZ,
			d_alphaTable, d_kTable,
			d_Tgas_star, d_hgas,
			dt, Tref, Lref, rk_alphas[stage],
			Nx, Ny, Nz,
			ni, nb, nw, na
			);

		// Swap read and write pointers for the next stage
		double* temp = d_T_read;
		d_T_read = d_T_write;
		d_T_write = temp;
	}

	// At the end of the loop d_T_read holds the final stage result
	// It is swapped with d_T_anchor, that now holds the next timestep
	// d_T_read holds the old timestep
	double* temp2 = d_T_anchor;
	d_T_anchor = d_T_read;
	d_T_read = temp2;

    currentTime += dt;
}

void ThermalSolver::downloadTemperature(double* h_T) {
	size_t numCells = (size_t)Nx * Ny * Nz;
	CUDA_CHECK(cudaMemcpy(h_T, d_T_anchor, numCells * sizeof(double), cudaMemcpyDeviceToHost));
}

std::pair<double, double> ThermalSolver::computeResiduals() {
	size_t numCells = (size_t)Nx * Ny * Nz;

	std::vector<double> h_T_old(numCells);
	std::vector<double> h_T_new(numCells);

    // Download both temperature fields to the host
    // Note: after the swap in solveStep, d_Told is the new step, d_Tnew is the old step
	CUDA_CHECK(cudaMemcpy(h_T_old.data(), d_T_read, numCells * sizeof(double), cudaMemcpyDeviceToHost));
	CUDA_CHECK(cudaMemcpy(h_T_new.data(), d_T_anchor, numCells * sizeof(double), cudaMemcpyDeviceToHost));

    double maxDeltaT_star = 0.0;
	double sumDeltaT_star = 0.0;

    for (size_t i = 0; i < numCells; i++) {
		int x = i / (Ny * Nz);
		int rem = i % (Ny * Nz);
		int z = rem / Ny;
		int y = rem % Ny;

		if (!IS_SOLID(y, z)) continue;


		// Nondimensional absolute temperature difference
        double diff = std::abs(h_T_old[i] - h_T_new[i]);

        if (diff > maxDeltaT_star) maxDeltaT_star = diff;
		sumDeltaT_star += diff;
    }

	// Get dimensional max rate of change in temperature (K/s)
    double maxRateOfChange = (maxDeltaT_star * Tref) / dt;
	double sumRateOfChange = (sumDeltaT_star * Tref) / dt;

    return {maxRateOfChange, sumRateOfChange};
}
