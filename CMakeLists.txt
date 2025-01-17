cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_executable(${CMAKE_PROJECT_NAME})

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

set(Eigen3_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/eigen3")
include_directories(${Eigen3_DIR})

# Find Eigen if installed via system package manager or vcpkg
find_package(Eigen3 REQUIRED)

# Include Eigen directories
include_directories(${EIGEN3_INCLUDE_DIR})

# If using modern Eigen 3.3+ and target-based CMake:
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Eigen3::Eigen)

set(FFTW3_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/fftw3")
include_directories(${FFTW3_DIR})

# Find FFTW3
find_package(FFTW3 REQUIRED)

# Link FFTW3 to your project
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE FFTW3::fftw3)

set(USE_SYSTEM_OPENCV
    OFF
    CACHE STRING "Use system OpenCV")
if(USE_SYSTEM_OPENCV)
  if(OS_LINUX)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${OpenCV_LIBRARIES}")
    target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PUBLIC "${OpenCV_INCLUDE_DIRS}")
  else()
    message(FATAL_ERROR "System OpenCV is only supported on Linux!")
  endif()
else()
  include(cmake/FetchOpenCV.cmake)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenCV)
endif()

# set(unofficial-sqlite3_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/unofficial-sqlite3")
# include_directories(${unofficial-sqlite3_DIR})

# set(dlib_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/dlib")
# include_directories(${dlib_DIR})

# # Include dlib
# find_package(dlib REQUIRED)

# # Link dlib to your target
# target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE dlib::dlib)

# set(Protobuf_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/protobuf")
# include_directories(${Protobuf_DIR})

# set(OpenCV_DIR "C:/Users/charl/Documents/vcpkg/installed/x64-windows/share/opencv4")
# include_directories(${OpenCV_DIR})

# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})
# target_link_libraries(your_project_name PRIVATE ${OpenCV_LIBS})

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-main.cpp src/heart_rate_source.cpp src/heart_rate_source_info.c)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/algorithm/HeartRateAlgorithm.cpp)


set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})