#include "cli/CliParser.hpp"
#include "core/AppConfig.hpp"
#include "core/Application.hpp"
#include "graphics/VulkanRenderer.hpp"
#include "io/exportMeshVTK.hpp"
#include "mesher/Mesh.hpp"
#include "mesher/Mesher.hpp"
#include "fluid/Voxelizer.hpp"
#include "thermal/ThermalSolver.cuh"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

Application::Application(int argc, char** argv) {
    CliParser parser(argc, argv);

    AppConfig config = parser.parse();

    this->inputFilePath = config.inputFilePath;
    this->visual = config.visual;
}

Application::~Application() {
    
}

void Application::run() {
    std::cout << "Running application..." << std::endl;

    simInput = readInput(inputFilePath);
	simInput.nonDimensionalize();
    
    std::unique_ptr<Mesher> mesher = std::make_unique<Mesher>();
    Mesh mesh = mesher->run(simInput);
    std::vector<glm::dvec3>& meshVertices = mesh.vertices;

    std::vector<Vertex> rendererVertices;
    for (const auto& p : meshVertices) {
        rendererVertices.push_back({ {static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z)}, {0.0f, 1.0f, 0.0f} });
    }
    std::unique_ptr<VulkanRenderer> vulkanRenderer = std::make_unique<VulkanRenderer>(rendererVertices, mesh.triangleIndices, mesh.lineIndices);

    std::unique_ptr<ThermalSolver> thermal = std::make_unique<ThermalSolver>(simInput, mesh);

	//std::unique_ptr<Voxelizer> voxelizer = std::make_unique<Voxelizer>(simInput, mesh, *this, mesh.shortestEdgeLength, 8);
	//voxelizer->run();

    int totalIterations = 15000;
    int outputFrequency = 100;
    int printCount = 0;

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(now_tm, "%Y%m%d-%H%M%S");
    std::string timePrefix = ss.str();

	auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < totalIterations; i++) {
        thermal->solveStep();

        if ((i + 1) % outputFrequency == 0) {
            if (printCount % 20 == 0) {
                std::cout << "\n"
                    << std::setw(12) << std::right << "Iteration"
                    << std::setw(15) << std::right << "Time [s]"
                    << std::setw(22) << std::right << "Max dT/dt [K/s]"
                    << std::setw(22) << std::right << "Sum dT/dt [K/s]"
                    << "\n";
                std::cout << std::string(72, '-') << "\n";
            }

            auto residuals = thermal->computeResiduals();
			double maxResidual = residuals.first;
			double sumResidual = residuals.second;

            std::cout << std::setw(12) << std::right << (i + 1)
				<< std::setw(15) << std::right << std::fixed << std::setprecision(4) << thermal->getCurrentTime()
				<< std::setw(22) << std::right << std::scientific << std::setprecision(5) << maxResidual
                << std::setw(22) << std::right << std::scientific << std::setprecision(5) << sumResidual

                << "\n";

            std::cout << std::defaultfloat;
			printCount++;
   //         if ((i + 1) % (outputFrequency * 1) == 0 || i + 1 == 100) {
   //             std::cout << "Saving intermediate results...\n";
   //             std::vector<double> h_T((size_t)mesh.Nx * mesh.Ny * mesh.Nz);
   //             thermal->downloadTemperature(h_T.data());

   //             std::string filename = application.getOutputPath() + timePrefix + "_output_" + std::to_string(i + 1) + ".vtk";
   //             exportMeshVTK(filename, mesh.vertices, h_T, simInput);
			//}

            if (maxResidual < 5e-9) {
	            auto endTime = std::chrono::high_resolution_clock::now();
	            std::chrono::duration<double> elapsedSeconds = endTime - startTime;
                std::cout << "Convergence achieved at iteration " << (i + 1) << ". Stopping simulation.\n"
                          << "Simulation completed in " << elapsedSeconds.count() << " seconds.\n";
                std::cout << "Saving convergence results...\n";
                std::vector<double> h_T((size_t)mesh.Nx * mesh.Ny * mesh.Nz);
                thermal->downloadTemperature(h_T.data());

                std::string filename = outputDirectory + timePrefix + "_output_" + std::to_string(i + 1);
                exportMasterPVTU(filename, 1);
                exportPieceVTU(filename, 0, mesh.vertices, h_T, simInput);
                break;
            }
        }
        if ((i + 1)== totalIterations) {
            std::cout << "Maximum iterations reached without convergence.\n";
            std::vector<double> h_T((size_t)mesh.Nx * mesh.Ny * mesh.Nz);
            thermal->downloadTemperature(h_T.data());
            std::string filename = outputDirectory + timePrefix + "_final_output_" + std::to_string(i + 1);
            exportMasterPVTU(filename, 1);
            exportPieceVTU(filename, 0, mesh.vertices, h_T, simInput);
		}
    }
}
