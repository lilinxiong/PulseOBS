cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

set(Eigen3_INCLUDE_DIR "C:/Users/Elizabeth Chan/Downloads/eigen-3.4.0")
include_directories(${Eigen3_INCLUDE_DIR})

set(OpenCV_DIR "C:/Users/Elizabeth Chan/Downloads/opencv/build")
set(OpenCV_INCLUDE_DIR "C:/Users/Elizabeth Chan/Downloads/opencv/build/include")
set(OpenCV_LIBS "C:/Users/Elizabeth Chan/Downloads/opencv/build/x64/vc16/lib") 
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})
link_directories(${OpenCV_LIBS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

# set(USE_SYSTEM_OPENCV OFF CACHE STRING "Use system OpenCV")
# if(USE_SYSTEM_OPENCV)
#   if(OS_LINUX)
#     find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect)
#     target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${OpenCV_LIBRARIES}")
#     target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PUBLIC "${OpenCV_INCLUDE_DIRS}")
#   else()
#     message(FATAL_ERROR "System OpenCV is only supported on Linux!")
#   endif()
# else()
#   # Use a local OpenCV installation for Windows
#   set(OpenCV_DIR "C:/Users/Elizabeth Chan/Downloads/opencv/build")
#   find_package(OpenCV REQUIRED)
#   include_directories(${OpenCV_INCLUDE_DIRS})
#   target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_LIBRARIES})
# endif()


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

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/algorithm/HeartRateAlgorithm.cpp src/algorithm/FaceDetection.cpp)


set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})