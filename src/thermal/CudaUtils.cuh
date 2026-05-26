#pragma once
#include <iostream>
#include <cuda_runtime.h>

#ifdef __INTELLISENSE__
    // If intellisense is parsing the code, we avoid the CUDA-specific syntax that it doesn't understand
    #define KERNEL_LAUNCH(grid, block)
#else
    // When the actual compiler is parsing, we have to expand this to the proper CUDA kernel launch syntax.
    #define KERNEL_LAUNCH(grid, block) <<< grid, block >>>
#endif

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA Error:\n" \
                      << "  File: " << __FILE__ << "\n" \
                      << "  Line: " << __LINE__ << "\n" \
                      << "  Error code: " << err << "\n" \
                      << "  Error text: " << cudaGetErrorString(err) << std::endl; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (0)