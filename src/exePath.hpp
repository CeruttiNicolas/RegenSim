#pragma once

#include <string>
#include <stdexcept>

#if defined(_WIN32)
    #pragma message("Compiling for Windows platform")
    #include <windows.h>


    inline std::string getExecutableDir() {
        char path[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
        if (length == 0) {
            throw std::runtime_error("Failed to get executable path");
        }
        std::string exePath(path, length);
        size_t pos = exePath.find_last_of("\\/");
        return (pos == std::string::npos) ? "" : exePath.substr(0, pos);
    }

#elif defined(__linux__)
    #pragma message("Compiling for Linux platform")
    #include <unistd.h>
    #include <limits.h>

    inline std::string getExecutableDir() {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count == -1) {
            throw std::runtime_error("Failed to get executable path");
        }
        std::string exePath(result, count);
        size_t pos = exePath.find_last_of("/");
        return (pos == std::string::npos) ? "" : exePath.substr(0, pos);
    }

#else
    #error "Unsupported platform"
#endif
