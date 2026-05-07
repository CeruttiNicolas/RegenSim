#pragma once

#include "core/SimulationInput.hpp"
#include "graphics/VulkanRenderer.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

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
