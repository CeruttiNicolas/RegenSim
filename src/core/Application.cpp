#include "cli/CliParser.hpp"
#include "core/AppConfig.hpp"
#include "core/Application.hpp"
#include "graphics/VulkanRenderer.hpp"
#include "io/exportMeshVTK.hpp"
#include "mesher/Mesh.hpp"
#include "mesher/Mesher.hpp"
#include <filesystem>
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

	//exportMeshVTK("C:\\Users\\Nicolas\\Desktop\\output.vtk", mesh, input);
    vulkanRenderer->run("RegenSim");
}
