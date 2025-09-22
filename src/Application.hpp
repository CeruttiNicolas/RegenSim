#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "graphics/VulkanRenderer.hpp"

class Application {
public:
    Application(int argc, char** argv);
    ~Application();

    void run();

private:
    bool visual = false;
    std::string inputFilePath;

    struct SimulationInput {
        std::vector<glm::vec3> contour;
        glm::vec3 chamber, throat, exit;
        float ac, at, ae, bc, bt, be, wi, wo;
    };
    
    SimulationInput simInput;
    SimulationInput readInput(const std::string& path);
};
