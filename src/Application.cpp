#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include "Application.hpp"
#include "graphics/VulkanRenderer.hpp"

#define _DEBUG

Application::Application(int argc, char** argv) {

    std::string helpString = "\033[32m"
        "\033[1m" "Usage:" "\033[22m" "\n"
        "    RegenSim <path-to-geometry> [options]\n"
        "\033[1m" "Options:" "\033[22m" "\n"
        "    --help, -h     Show this message and exit the program\n"
        "    --visual, -v   Enable visual mode (default: disabled)"
        "\033[0m";

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
    std::unique_ptr<VulkanRenderer> vulkanRenderer = std::make_unique<VulkanRenderer>();
    #ifdef _DEBUG
    std::cout << "Running simulation with the following input:" << std::endl
              << "ac: " << simInput.ac << ", at: " << simInput.at << std::endl
              << "ae: " << simInput.ae << ", bc: " << simInput.bc << std::endl
              << "bt: " << simInput.bt << ", be: " << simInput.be << std::endl
              << "wi: " << simInput.wi << ", wo: " << simInput.wo << std::endl
              << "Chamber: (" << simInput.chamber.x << ", " << simInput.chamber.y << ", " << simInput.chamber.z << ")" << std::endl
              << "Throat: (" << simInput.throat.x << ", " << simInput.throat.y << ", " << simInput.throat.z << ")" << std::endl
              << "Exit: (" << simInput.exit.x << ", " << simInput.exit.y << ", " << simInput.exit.z << ")" << std::endl;
    #endif
    vulkanRenderer->run("Contour");
}
