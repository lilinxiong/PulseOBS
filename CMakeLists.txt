cmake_minimum_required(VERSION 3.10)
project(PulseOBS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Include directories
include_directories(algorithm plugin)

# Add the algorithm library
add_library(algorithm_lib algorithm/HeartRateAlgorithm.cpp)

# Add the combined executable
add_executable(PulseOBS plugin/OBSPlugin.cpp)
target_link_libraries(PulseOBS algorithm_lib)