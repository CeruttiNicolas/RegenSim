#include <iostream>
#include <memory>
#include <cstdlib>
#include "Application.hpp"  

int main(int argc, char** argv) {
    try {
        auto app = std::make_unique<Application>(argc, argv);
        app->run();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}