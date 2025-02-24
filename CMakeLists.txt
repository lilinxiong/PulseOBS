cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(UNIX)
  set(CMAKE_C_FLAGS "-Wall -std=c99 -fPIC -Wno-unused-parameter -Wno-unused-private-field")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g")
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -Os -DNDEBUG -fvisibility=hidden")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG -fvisibility=hidden")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -O2 -g -fvisibility=hidden")

  set(CMAKE_CXX_FLAGS "-Wall -ffunction-sections -fdata-sections -fPIC -Wno-unused-parameter -Wno-unused-private-field")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS} -Os -DNDEBUG -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -O2 -g -fvisibility=hidden")
endif()

message(
  "=======CMAKE cur_ver: ${CMAKE_VERSION}, cur_build_type:${CMAKE_BUILD_TYPE}, CMAKE_SYSTEM_NAME:${CMAKE_SYSTEM_NAME}"
)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Eigen3::Eigen)

message("MSVC_VERSION:${MSVC_VERSION}")

if(UNIX)
  message("Configuring OpenCV for MacOS and Linux")
  set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/binary/opencv-4.11.0-macos/lib/cmake/opencv4)
elseif(WIN32)
  set(OpenCV_STATIC ON)
  set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/binary/opencv-4.11.0-windows/)
endif()

message("OpenCV_DIR:${OpenCV_DIR}")

find_package(OpenCV CONFIG REQUIRED)
if(OpenCV_FOUND)
  message("OpenCV_FOUND:${OpenCV_FOUND}")
  message("OpenCV_VERSION:${OpenCV_VERSION}")
  message("OpenCV_INCLUDE_DIRS:${OpenCV_INCLUDE_DIRS}")
  message("OpenCV_LIBRARIES:${OpenCV_LIBRARIES}")
endif()

include_directories(${OpenCV_INCLUDE_DIR})
link_directories(${OpenCV_LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_LIBRARIES})

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/algorithm/FaceDetection.cpp
    src/algorithm/HeartRateAlgorithm.cpp
    src/plugin-main.cpp
    src/heart_rate_source.cpp
    src/heart_rate_source_info.c
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
