#include <cmath>
#include <iostream>
#include "thermal/ThermalSolver.cuh"

void ThermalSolver::computeFourierTimestep() {
	// Fourier number
	double dtStarMax = this->Lcrit * this->Lcrit / 6.0;
	double dtStarSafe = dtStarMax * this->knockdownFactor;

	// Characteristic time of diffusion
	this->tRef = this->Lref * this->Lref / this->alphaMax;

	// Real timestep
	double dtMax = dtStarMax * this->tRef;
	double dtSafe = dtStarSafe * this->tRef;

	// Split timestep into mantissa and exponent
	double exponent = std::floor(std::log10(dtSafe));
	double rawMantissa = dtSafe / std::pow(10.0, exponent);

	// Floor mantissa to a nice value 
	double roundedMantissa;
	if (rawMantissa >= 5.0) roundedMantissa = 5.0;
	else if (rawMantissa >= 2.0) roundedMantissa = 2.0;
	else roundedMantissa = 1.0;

	// Clean real timestep
	this->dt = roundedMantissa * std::pow(10.0, exponent);

	// Clean Fourier number
	this->dtStar = this->dt / this->tRef;

	std::cout << "\n--- Timestep Optimization ---" << std::endl;
	std::cout << "  Raw dt max   : " << dtMax << std::endl;
	std::cout << "  Raw Fo max   : " << dtStarMax << std::endl;
	std::cout << "  Clean dt     : " << this->dt << std::endl;
	std::cout << "  Clean Fo     : " << this->dtStar << std::endl;
	std::cout << "  Safety Margin: " << dtMax / this->dt - 1.0 << std::endl;
	std::cout << "-----------------------------\n" << std::endl;
}
