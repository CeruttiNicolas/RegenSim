#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "SimulationInput.hpp"
#include "graphics/VulkanRenderer.hpp"

class Application {
public:
    Application(int argc, char** argv);
    ~Application();

    void run();

private:
    bool visual = false;
    std::string inputFilePath;

    SimulationInput simInput;
    SimulationInput readInput(const std::string& path);
};
