#include "cli/CliParser.hpp"
#include "core/AppConfig.hpp"
#include "core/Application.hpp"
#include "graphics/VulkanRenderer.hpp"
#include "io/exportMeshVTK.hpp"
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

    std::cout << "Non-Dimensionalized with reference length: " << simInput.refLength << std::endl;
    
    double a = simInput.a;
    
    std::unique_ptr<Mesher> mesher = std::make_unique<Mesher>();
    std::vector<glm::dvec3> mesh = mesher->run(simInput);
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> triangleIndices;
    std::vector<uint32_t> lineIndices;
    std::unique_ptr<VulkanRenderer> vulkanRenderer = std::make_unique<VulkanRenderer>(vertices, triangleIndices, lineIndices);

	double shortestEdgeLength = mesher->findShortestEdgeLength(mesh, simInput);
	std::cout << "Shortest edge length in mesh: " << shortestEdgeLength << std::endl;
    // Struct to filter duplicate quad edges
    struct Edge {
        uint32_t v1, v2;
        bool operator==(const Edge& o) const {
            return (v1 == o.v1 && v2 == o.v2) || (v1 == o.v2 && v2 == o.v1);
        }
    };
    struct EdgeHash {
        std::size_t operator()(const Edge& e) const {
            return std::hash<uint32_t>()(std::min(e.v1, e.v2)) ^ std::hash<uint32_t>()(std::max(e.v1, e.v2));
        }
    };
    std::unordered_set<Edge, EdgeHash> uniqueEdges;



    for (const auto& p : mesh) {
            vertices.push_back({{static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z)}, {1.0f, 1.0f, 1.0f}});
    }

    auto input = simInput;
    int height = input.ni + input.nb + input.no + 1;
    int width = 2 * input.nw + input.na + 1;
    int depth = mesh.size() / (width * height);

    auto getIndex = [&](int x, int y, int z) {
        return y + (x * height) + (z * height * width);
    };

    auto addQuad = [&](int bl, int br, int tr, int tl, bool reverseWinding = false) {
        // tris
        if (reverseWinding) {
            triangleIndices.push_back(bl); triangleIndices.push_back(tr); triangleIndices.push_back(br);
            triangleIndices.push_back(bl); triangleIndices.push_back(tl); triangleIndices.push_back(tr);
        }
        else {
            triangleIndices.push_back(bl); triangleIndices.push_back(br); triangleIndices.push_back(tr);
            triangleIndices.push_back(bl); triangleIndices.push_back(tr); triangleIndices.push_back(tl);
        }

        // edges
        uniqueEdges.insert({ (uint32_t)bl, (uint32_t)br });
        uniqueEdges.insert({ (uint32_t)br, (uint32_t)tr });
        uniqueEdges.insert({ (uint32_t)tr, (uint32_t)tl });
        uniqueEdges.insert({ (uint32_t)tl, (uint32_t)bl });
    };

    for (int i = 0; i < width - 1; i++) {
        for (int j = 0; j < height - 1; j++) {
            if (i >= input.nw && i < input.nw + input.na &&
                j >= input.ni && j < input.ni + input.nb) continue;

            int frontZ = 0;
            int backZ = depth - 1;

            addQuad(getIndex(i, j, frontZ), getIndex(i + 1, j, frontZ),
                getIndex(i + 1, j + 1, frontZ), getIndex(i, j + 1, frontZ), false);

            addQuad(getIndex(i, j, backZ), getIndex(i + 1, j, backZ),
                getIndex(i + 1, j + 1, backZ), getIndex(i, j + 1, backZ), true);
        }
    }

    for (int k = 0; k < depth - 1; k++) {
        for (int j = 0; j < height - 1; j++) {

            addQuad(getIndex(0, j, k), getIndex(0, j, k + 1),
                getIndex(0, j + 1, k + 1), getIndex(0, j + 1, k), true);
            addQuad(getIndex(width - 1, j, k), getIndex(width - 1, j, k + 1),
                getIndex(width - 1, j + 1, k + 1), getIndex(width - 1, j + 1, k), false);

            if (j >= input.ni && j < input.ni + input.nb) {
                int leftWall = input.nw;
                int rightWall = input.nw + input.na;

                addQuad(getIndex(leftWall, j, k), getIndex(leftWall, j, k + 1),
                    getIndex(leftWall, j + 1, k + 1), getIndex(leftWall, j + 1, k), false);
                
                addQuad(getIndex(rightWall, j, k), getIndex(rightWall, j, k + 1),
                    getIndex(rightWall, j + 1, k + 1), getIndex(rightWall, j + 1, k), true);
            }
        }
    }

    for (int k = 0; k < depth - 1; k++) {
        for (int i = 0; i < width - 1; i++) {

            addQuad(getIndex(i, 0, k), getIndex(i + 1, 0, k),
                getIndex(i + 1, 0, k + 1), getIndex(i, 0, k + 1), true);
            addQuad(getIndex(i, height - 1, k), getIndex(i + 1, height - 1, k),
                getIndex(i + 1, height - 1, k + 1), getIndex(i, height - 1, k + 1), false);

            if (i >= input.nw && i < input.nw + input.na) {
                int floorY = input.ni;
                int ceilY = input.ni + input.nb;

                addQuad(getIndex(i, floorY, k), getIndex(i + 1, floorY, k),
                    getIndex(i + 1, floorY, k + 1), getIndex(i, floorY, k + 1), false);
                
                addQuad(getIndex(i, ceilY, k), getIndex(i + 1, ceilY, k),
                    getIndex(i + 1, ceilY, k + 1), getIndex(i, ceilY, k + 1), true);
            }
        }
    }

    for (const auto& edge : uniqueEdges) {
        lineIndices.push_back(edge.v1);
        lineIndices.push_back(edge.v2);
    }

	//exportMeshVTK("C:\\Users\\Nicolas\\Desktop\\output.vtk", mesh, input);
    vulkanRenderer->run("Viewport");

    // TODO: Edit solid mesh creation with "target length" parameter describing desired size of mesh elements for contour resampling and section generation
}
