#include <iostream>
#include <filesystem>
#include <numbers>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include "Application.hpp"
#include "mesher/Mesher.hpp"
#include "graphics/VulkanRenderer.hpp"

#define GREEN   "\033[32m"
#define BOLD    "\033[1m"
#define NORMAL  "\033[22m"
#define RESET   "\033[0m"

#define _DEBUG

Application::Application(int argc, char** argv) {

    std::string helpString =
        GREEN
        BOLD "Usage:\n" NORMAL
        "    RegenSim <path-to-geometry> [options]\n"
        BOLD "Options:\n" NORMAL
        "    --help, -h     Show this message and exit the program\n"
        "    --visual, -v   Enable visual mode (default: disabled)"
        RESET;

    std::vector<std::string> args(argv + 1, argv + argc);
    std::unordered_map<std::string, std::string> aliases {
        {"--visual", "visual"}, {"-v", "visual"},
        {"--help", "help"}, {"-h", "help"}
    };
    std::unordered_set<std::string> seenArgs;

    if (args.empty()) {
        std::cerr << "\033[31mError: No arguments provided. Expected path to geometry file.\033[0m" << std::endl;
        exit(EXIT_FAILURE);
    }
    // Handle help flag
    if (args[0] == "--help" || args[0] == "-h") {
        std::cout << helpString << std::endl;
        exit(EXIT_SUCCESS);
    }
    if (args[0][0] == '-') {
        std::cerr << "\033[31mError: First argument is a flag. Expected path to geometry file.\033[0m" << std::endl;
        exit(EXIT_FAILURE);
    } 
    inputFilePath = args[0];
    args.erase(args.begin());

    for (const auto& arg : args) {
        auto aliasIt = aliases.find(arg);
        if (aliasIt == aliases.end()) {
            std::cout << "\033[33mWarning: Unrecognized argument " << arg << " ignored. See --help (-h) for more info.\033[0m" << std::endl;
            continue;
        }

        const std::string& flag = aliasIt->second;
        if (seenArgs.find(flag) != seenArgs.end()) {
            std::cout << "\033[33mWarning: Duplicate argument " << arg << " ignored. See --help (-h) for more info.\033[0m" << std::endl;
            continue;
        }
        seenArgs.emplace(flag);

        if (flag == "help") {
            std::cout << helpString << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (flag == "visual") {
            visual = true;
            continue;
        }
    }
}

Application::~Application() {
    
}

void Application::run() {
    std::cout << "Running application..." << std::endl;
    simInput = readInput(inputFilePath);
    
    float a;
    std::cin >> a;
    
    std::unique_ptr<Mesher> mesher = std::make_unique<Mesher>();
    std::vector<glm::vec3> resampledContour = mesher->resampleContour(simInput.contour, a);
    std::vector<glm::vec3> sectionPoints = mesher->generateSection(simInput, simInput.contour[0], simInput.ac, simInput.bc);
    std::vector<glm::vec3> vertexNormals = mesher->computeVertexNormals(resampledContour);
    std::vector<glm::vec3> mesh = mesher->run(resampledContour, a, simInput);
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unique_ptr<VulkanRenderer> vulkanRenderer = std::make_unique<VulkanRenderer>(vertices, indices);
    
    // --- Show Section --------------------------------------------------------------------------------------
    float k = 175.0f;
    int i = 0;
    for (const auto& p : sectionPoints) {
        // std::cout << "Point: (" << p.z * k << ", " << p.y * k << ")" << std::endl;
        //if ((i % 43 < 6 || i % 43 > 36) || (i / 43 < 10 || i / 43 > 50)) {
        if ( (i % (simInput.ni + simInput.nb + simInput.no + 1) < simInput.ni
          ||  i % (simInput.ni + simInput.nb + simInput.no + 1) > simInput.ni + simInput.nb) 
          || (i / (simInput.ni + simInput.nb + simInput.no + 1) < simInput.nw)  ||
              i / (simInput.ni + simInput.nb + simInput.no + 1) > simInput.nw + simInput.na) {
            vertices.push_back({{k * p.z*4.0f/3.0f, -k*p.y}, {1.0f, 1.0f, 1.0f}});
        } else {
            vertices.push_back({{k * p.z*4.0f/3.0f, -k*p.y}, {0.0f, 0.0f, 1.0f}});
        }
        i++;
    }

    indices.resize(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    vulkanRenderer->run("Section");

    vertices.clear();
    indices.clear();

    // --- Show Contour --------------------------------------------------------------------------------------
    for (const auto& p : resampledContour) {
        vertices.push_back({{6 * p.x + 0.5f, 6 * -p.y}, {1.0f, 1.0f, 1.0f}});
    }

    indices.resize(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    vulkanRenderer->run("Contour");

    vertices.clear();
    indices.clear();

    // --- Show Normals --------------------------------------------------------------------------------------
    for (int i = 0; i < vertexNormals.size(); i++) {
        glm::vec3 p = resampledContour[i];
        glm::vec3 np = p + vertexNormals[i]*(simInput.wi + simInput.bt + simInput.wo);
        vertices.push_back({{6 * p.x + 0.5f, 6 * -p.y + 0.2f}, {1.0f, 1.0f, 1.0f}});
        vertices.push_back({{6 * np.x + 0.5f, 6 * -np.y + 0.2f}, {1.0f, 1.0f, 1.0f}});
    }

    indices.resize(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    vulkanRenderer->run("Normals");

    vertices.clear();
    indices.clear();

    // --- Show Mesh --------------------------------------------------------------------------------------
    for (const auto &p : mesh) {
        vertices.push_back({{6 * p.x + 0.5f, 6 * -p.y + 0.2f}, {1.0f, 1.0f, 1.0f}}); 
    }

    indices.resize(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    vulkanRenderer->run("Mesh");

    // TODO: Edit solid mesh creation with "target length" parameter describing desired size of mesh elements for contour resampling and section generation
    
    //vulkanWindow = std::make_unique<VulkanRenderer>();
    //vulkanWindow->run("Wireframe");    

    // Show Contour (if in visual mode //TODO)
    // showContour(simInput.contour); 
    // Generate Wireframe
    // Application::generateWireframe(simInput); where simInput is passed by const reference so implementation will be void Application::generateWireframe(const SimulationInput& simInput) {
    // Show Wireframe (if in visual mode //TODO)
    // showWireframe();
}
