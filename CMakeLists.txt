cmake_minimum_required(VERSION 3.18)
project(RegenSim LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_ARCHITECTURES 86)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# --- Source files ---
set(SRC_FILES src/main.cpp)

# CLI source files
file(GLOB CLI_SRC_FILES src/cli/*.cpp)
list(APPEND SRC_FILES ${CLI_SRC_FILES})

# Core source files
file(GLOB CORE_SRC_FILES src/core/*.cpp)
list(APPEND SRC_FILES ${CORE_SRC_FILES})

# Graphics source files
file(GLOB GRAPHICS_SRC_FILES src/graphics/*.cpp)
list(APPEND SRC_FILES ${GRAPHICS_SRC_FILES})

# IO source files
file(GLOB IO_SRC_FILES src/io/*.cpp)
list(APPEND SRC_FILES ${IO_SRC_FILES})

# Mesher source files
file(GLOB MESHER_SRC_FILES src/mesher/*.cpp)
list(APPEND SRC_FILES ${MESHER_SRC_FILES})

# Thermal source files
file(GLOB THERMAL_SRC_FILES src/thermal/*.cpp src/thermal/*.cu)
list(APPEND SRC_FILES ${THERMAL_SRC_FILES})

# Utils source files
file(GLOB UTILS_SRC_FILES src/utils/*.cpp)
list(APPEND SRC_FILES ${UTILS_SRC_FILES})

# --- Header files ---
# CLI header files
file(GLOB CLI_HEADER_FILES src/cli/*.hpp)
list(APPEND HEADER_FILES ${CLI_HEADER_FILES})

# Core header files
file(GLOB CORE_HEADER_FILES src/core/*.hpp)
list(APPEND HEADER_FILES ${CORE_HEADER_FILES})

# Graphics header files
file(GLOB GRAPHICS_HEADER_FILES src/graphics/*.hpp)
list(APPEND HEADER_FILES ${GRAPHICS_HEADER_FILES})

# IO header files
file(GLOB IO_HEADER_FILES src/io/*.hpp)
list(APPEND HEADER_FILES ${IO_HEADER_FILES})

# Mesher header files
file(GLOB MESHER_HEADER_FILES src/mesher/*.hpp)
list(APPEND HEADER_FILES ${MESHER_HEADER_FILES})

# Thermal header files
file(GLOB THERMAL_HEADER_FILES src/mesher/*.hpp)
list(APPEND HEADER_FILES ${THERMAL_HEADER_FILES})

# Utils header files
file(GLOB UTILS_HEADER_FILES src/utils/*.hpp)
list(APPEND HEADER_FILES ${UTILS_HEADER_FILES})


add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()


# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/json/single_include
)

# Vulkan and GLFW dependencies
add_subdirectory(vendor/glfw)
find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Make sure the Vulkan SDK is installed and VULKAN_SDK is set.")
endif()

find_package(CUDAToolkit REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    Vulkan::Vulkan
    CUDA::cudart
)

# === Shader Compilation Setup ===

find_program(GLSLC_EXECUTABLE glslc HINTS "$ENV{VULKAN_SDK}/Bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Make sure Vulkan SDK is installed and VULKAN_SDK is set.")
endif()

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_DEST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
file(MAKE_DIRECTORY ${SHADER_DEST_DIR})

set(SHADERS
    shader.vert
    shader.frag
)

set(SPIRV_BINARIES "")

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILE_EXT ${SHADER} EXT)
    string(SUBSTRING ${FILE_EXT} 1 -1 FILE_NAME_EXT)

    set(SPIRV_OUTPUT ${SHADER_DEST_DIR}/${FILE_NAME_EXT}.spv)
        
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_DIR}/${SHADER} -o ${SPIRV_OUTPUT}
        DEPENDS ${SHADER_DIR}/${SHADER}
        COMMENT "Compiling shader in: ${SPIRV_OUTPUT}"
        VERBATIM
    )

    list(APPEND SPIRV_BINARIES ${SPIRV_OUTPUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_BINARIES})
add_dependencies(${PROJECT_NAME} compile_shaders)
