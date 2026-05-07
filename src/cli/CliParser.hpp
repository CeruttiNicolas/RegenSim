#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "core/AppConfig.hpp"

class CliParser {
public:
	CliParser(int argc, char** argv);
	AppConfig parse();

private:
	void printHelp() const;
	std::vector<std::string> args;
	std::unordered_map<std::string, std::string> aliases;
	std::string helpString;
};