cmake_minimum_required(VERSION 3.10)
project(PulseOBS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Include directories
include_directories(src/algorithm src/plugin)

# Add the algorithm library
add_library(algorithm_lib src/algorithm/HeartRateAlgorithm.cpp)

# Add the combined executable
add_executable(PulseOBS src/plugin/OBSPlugin.cpp)
target_link_libraries(PulseOBS algorithm_lib)