#include <iostream>
#include <cstdlib>

#include "cli/CliParser.hpp"

constexpr auto RED = "\033[31m";
constexpr auto GREEN = "\033[32m";
constexpr auto YELLOW = "\033[33m";
constexpr auto BOLD = "\033[1m";
constexpr auto NORMAL = "\033[22m";
constexpr auto RESET = "\033[0m";

CliParser::CliParser(int argc, char** argv)
	: args(argv + 1, argv + argc)
{
	helpString =
        std::string(GREEN) +
        BOLD + "Usage:\n" + NORMAL +
        "    RegenSim <path-to-geometry> [options]\n" +
        BOLD + "Options:\n" + NORMAL +
        "    --help, -h     Show this message and exit the program\n" +
        "    --visual, -v   Enable visual mode (default: disabled)" +
        RESET;

    aliases = {
        {"--visual", "visual"}, {"-v", "visual"},
        {"--help", "help"}, {"-h", "help"}
    };
}

void CliParser::printHelp() const {
    std::cout << helpString << std::endl;
	exit(EXIT_SUCCESS);
}

AppConfig CliParser::parse() {
    AppConfig config;
    std::unordered_set<std::string> seenArgs;

    if (args.empty()) {
        std::cerr << std::string(RED) + "Error: No arguments provided.Expected path to geometry file." + RESET;
        exit(EXIT_FAILURE);
    }

    if (args[0] == "--help" || args[0] == "-h") {
        printHelp();
    }

    if (args[0][0] == '-') {
        std::cerr << std::string(RED) + "Error: First argument is a flag. Expected path to geometry file." + RESET;
        exit(EXIT_FAILURE);
    }

    config.inputFilePath = args[0];
    args.erase(args.begin());

    for (const auto& arg : args) {
        auto aliasIt = aliases.find(arg);
        if (aliasIt == aliases.end()) {
            std::cout << std::string(YELLOW) + "Warning: Unrecognized argument " << arg << " ignored. See --help (-h) for more info." + std::string(RESET);
            continue;
        }

        const std::string& flag = aliasIt->second;
        if (seenArgs.find(flag) != seenArgs.end()) {
            std::cout << std::string(YELLOW) + "Warning: Duplicate argument " << arg << " ignored. See --help (-h) for more info." + std::string(RESET);
            continue;
        }
        seenArgs.emplace(flag);

        if (flag == "help") {
            printHelp();
        }

        if (flag == "visual") {
            config.visual = true;
            continue;
        }
    }

    return config;
}
