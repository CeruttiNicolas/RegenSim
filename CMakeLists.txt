cmake_minimum_required(VERSION 3.10)
project(RegenSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# --- Source files ---
set(SRC_FILES
    src/main.cpp
    src/Application.cpp
    src/readInput.cpp
)

# Graphics source files
file(GLOB GRAPHICS_SRC_FILES src/graphics/*.cpp)
list(APPEND SRC_FILES ${GRAPHICS_SRC_FILES})

# Mesher source files
file(GLOB MESHER_SRC_FILES src/mesher/*.cpp)
list(APPEND SRC_FILES ${MESHER_SRC_FILES})


# --- Header files ---
set(HEADER_FILES
    src/Application.hpp
    src/exePath.hpp
    src/simulationInput.hpp
)

# Graphics header files
file(GLOB GRAPHICS_HEADER_FILES src/graphics/*.hpp)
list(APPEND HEADER_FILES ${GRAPHICS_HEADER_FILES})

# Mesher header files
file(GLOB MESHER_HEADER_FILES src/mesher/*.hpp)
list(APPEND HEADER_FILES ${MESHER_HEADER_FILES})


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

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    Vulkan::Vulkan
)

# === Shader Compilation Setup ===

find_program(GLSLC_EXECUTABLE glslc HINTS "$ENV{VULKAN_SDK}/Bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Make sure Vulkan SDK is installed and VULKAN_SDK is set.")
endif()

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BIN_DIR})

set(SHADERS
    shader.vert
    shader.frag
)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILE_EXT ${SHADER} EXT)
    string(SUBSTRING ${FILE_EXT} 1 -1 FILE_NAME_EXT)

    set(SPIRV_OUTPUT ${SHADER_BIN_DIR}/${FILE_NAME_EXT}.spv)
    message(STATUS "Shader ${SHADER} will be compiled to ${SPIRV_OUTPUT}")
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_DIR}/${SHADER} -o ${SPIRV_OUTPUT}
        DEPENDS ${SHADER_DIR}/${SHADER}
        COMMENT "Compiling shader: ${SHADER}"
        VERBATIM
    )

    list(APPEND SPIRV_BINARIES ${SPIRV_OUTPUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_BINARIES})
add_dependencies(${PROJECT_NAME} compile_shaders)


# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
# endif()
