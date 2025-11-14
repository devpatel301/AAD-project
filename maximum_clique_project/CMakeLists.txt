cmake_minimum_required(VERSION 3.15)
project(MaximumCliqueBenchmark LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional CUDA support
option(ENABLE_CUDA "Enable CUDA GPU algorithms" OFF)

if(ENABLE_CUDA)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 14)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files (excluding CUDA for now)
set(SOURCES
    src/graph.cpp
    src/greedy.cpp
    src/simulated_annealing.cpp
    src/randomized_heuristic.cpp
    src/bron_kerbosch.cpp
    src/tomita.cpp
    src/degeneracy_bk.cpp
    src/ostergard.cpp
    src/cpu_optimized.cpp
)

# Add CUDA sources if enabled
if(ENABLE_CUDA)
    list(APPEND SOURCES src/gpu_optimized.cu)
endif()

# Benchmark executable
add_executable(benchmark ${SOURCES} benchmark/benchmark.cpp)

# SAT benchmark executable (forces all algorithms)
add_executable(benchmark_sat ${SOURCES} benchmark/benchmark_sat.cpp)

# Compiler flags for optimization
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(benchmark PRIVATE
        -O3
        -march=native
        -Wall
        -Wextra
        -Wpedantic
    )
    target_compile_options(benchmark_sat PRIVATE
        -O3
        -march=native
        -Wall
        -Wextra
        -Wpedantic
    )
elseif(MSVC)
    target_compile_options(benchmark PRIVATE
        /O2
        /W4
    )
    target_compile_options(benchmark_sat PRIVATE
        /O2
        /W4
    )
endif()

# Link threads library
find_package(Threads REQUIRED)
target_link_libraries(benchmark PRIVATE Threads::Threads)
target_link_libraries(benchmark_sat PRIVATE Threads::Threads)

# CUDA-specific settings
if(ENABLE_CUDA)
    set_target_properties(benchmark PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
    )
    target_compile_definitions(benchmark PRIVATE ENABLE_CUDA)
endif()

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/datasets)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/results)

# Print configuration
message(STATUS "==========================================")
message(STATUS "Maximum Clique Benchmark Configuration")
message(STATUS "==========================================")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CUDA Support: ${ENABLE_CUDA}")
if(ENABLE_CUDA)
    message(STATUS "CUDA Compiler: ${CMAKE_CUDA_COMPILER}")
    message(STATUS "CUDA Standard: ${CMAKE_CUDA_STANDARD}")
endif()
message(STATUS "==========================================")
