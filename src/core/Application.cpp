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
    
    double a = simInput.a;
    
    std::unique_ptr<Mesher> mesher = std::make_unique<Mesher>();
    std::vector<glm::dvec3> mesh = mesher->run(simInput);
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unique_ptr<VulkanRenderer> vulkanRenderer = std::make_unique<VulkanRenderer>(vertices, indices);


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
        if (reverseWinding) {
            // Matches your original Side face winding
            indices.push_back(bl); indices.push_back(tr); indices.push_back(br);
            indices.push_back(bl); indices.push_back(tl); indices.push_back(tr);
        }
        else {
            // Matches your original Face/Top/Bottom winding
            indices.push_back(bl); indices.push_back(br); indices.push_back(tr);
            indices.push_back(bl); indices.push_back(tr); indices.push_back(tl);
        }
    };

    for (int i = 0; i < width - 1; i++) {
        for (int j = 0; j < height - 1; j++) {
            // Skip the hole area for the caps
            if (i >= input.nw && i < input.nw + input.na &&
                j >= input.ni && j < input.ni + input.nb) continue;

            int frontZ = 0;
            int backZ = depth - 1;

            // Front Cap (Faces toward camera, normal winding)
            addQuad(getIndex(i, j, frontZ), getIndex(i + 1, j, frontZ),
                getIndex(i + 1, j + 1, frontZ), getIndex(i, j + 1, frontZ), false);

            // Back Cap (Faces away from camera, reversed winding)
            addQuad(getIndex(i, j, backZ), getIndex(i + 1, j, backZ),
                getIndex(i + 1, j + 1, backZ), getIndex(i, j + 1, backZ), true);
        }
    }

    for (int k = 0; k < depth - 1; k++) {
        for (int j = 0; j < height - 1; j++) {

            // Outer Left (x = 0) & Outer Right (x = width - 1)
            addQuad(getIndex(0, j, k), getIndex(0, j, k + 1),
                getIndex(0, j + 1, k + 1), getIndex(0, j + 1, k), true);
            addQuad(getIndex(width - 1, j, k), getIndex(width - 1, j, k + 1),
                getIndex(width - 1, j + 1, k + 1), getIndex(width - 1, j + 1, k), false);

            // Inner Walls (These only exist if 'j' is currently inside the hole's Y bounds)
            if (j >= input.ni && j < input.ni + input.nb) {
                int leftWall = input.nw;
                int rightWall = input.nw + input.na;

                // Left inner wall (Faces right, into the hole)
                addQuad(getIndex(leftWall, j, k), getIndex(leftWall, j, k + 1),
                    getIndex(leftWall, j + 1, k + 1), getIndex(leftWall, j + 1, k), false);
                // Right inner wall (Faces left, into the hole)
                addQuad(getIndex(rightWall, j, k), getIndex(rightWall, j, k + 1),
                    getIndex(rightWall, j + 1, k + 1), getIndex(rightWall, j + 1, k), true);
            }
        }
    }

    for (int k = 0; k < depth - 1; k++) {
        for (int i = 0; i < width - 1; i++) {

            // Outer Bottom (y = 0) & Outer Top (y = height - 1)
            addQuad(getIndex(i, 0, k), getIndex(i + 1, 0, k),
                getIndex(i + 1, 0, k + 1), getIndex(i, 0, k + 1), true);
            addQuad(getIndex(i, height - 1, k), getIndex(i + 1, height - 1, k),
                getIndex(i + 1, height - 1, k + 1), getIndex(i, height - 1, k + 1), false);

            // Inner Floor/Ceiling (These only exist if 'i' is currently inside the hole's X bounds)
            if (i >= input.nw && i < input.nw + input.na) {
                int floorY = input.ni;
                int ceilY = input.ni + input.nb;

                // Floor of the hole (Faces UP)
                addQuad(getIndex(i, floorY, k), getIndex(i + 1, floorY, k),
                    getIndex(i + 1, floorY, k + 1), getIndex(i, floorY, k + 1), false);
                // Ceiling of the hole (Faces DOWN)
                addQuad(getIndex(i, ceilY, k), getIndex(i + 1, ceilY, k),
                    getIndex(i + 1, ceilY, k + 1), getIndex(i, ceilY, k + 1), true);
            }
        }
    }

    
	//exportMeshVTK("C:\\Users\\Nicolas\\Desktop\\output.vtk", mesh, input);
    vulkanRenderer->run("Section");

    // TODO: Edit solid mesh creation with "target length" parameter describing desired size of mesh elements for contour resampling and section generation
}
