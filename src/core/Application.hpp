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

    std::string getOutputPath() const {
        return outputDirectory;
	}

    void setOutputPath(std::string path) {
        outputDirectory = path;
    }
    
private:
    bool visual = false;
    std::string inputFilePath;
    std::string outputDirectory;

    SimulationInput simInput;
    SimulationInput readInput(const std::string& path);


};
