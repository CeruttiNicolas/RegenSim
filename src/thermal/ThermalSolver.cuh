#pragma once
#include "core/SimulationInput.hpp"
#include "mesher/Mesh.hpp"
#include <vector>
#include <utility>

class ThermalSolver {
public:
	ThermalSolver(const SimulationInput& input, const Mesh& mesh);
	~ThermalSolver();

	void solveStep();
	void downloadTemperature(double* h_T);
	std::pair<double, double> computeResiduals();

	double getCurrentTime() const { return currentTime; }

private:
	double interpolate1D(double xQuery, const std::vector<double>& X, const std::vector<double>& Y);
	void computeFourierTimestep();

	// Material properties
	double alphaMax;

	// Stability parameters
	double Lref;
	double Lcrit;
	double knockdownFactor;

	// Reference temperature
	double Tref;

	// Grid dimensions
	int Nx, Ny, Nz;

	// Subdivisions
	int ni, nb, nw, na;
	
	// Time
	double tRef;
	double dt;
	double dtStar;
	double currentTime;

	// --- GPU pointers ---
	// Volumes
	double* d_Volumes;

	// Areas
	double* d_AreasX;
	double* d_AreasY;
	double* d_AreasZ;

	// Distances
	double* d_DistX;
	double* d_DistY;
	double* d_DistZ;

	// Temperatures
	double* d_Told;
	double* d_Tnew;

	// Gas profiles
	double* d_Tgas_star;
	double* d_hgas;

	// LUTs
	double* d_alphaTable;
	double* d_kTable;

};