# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.15...4.0)

set(project_name ProtoCore)
# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  ProtoCore
  VERSION 1.0
  LANGUAGES CXX)

set(EXTERNAL_MODULES_DIR "${CMAKE_SOURCE_DIR}/external/modules")

set(EXTERNAL_PATHS
    "${EXTERNAL_MODULES_DIR}/mavlink/install"
    "${EXTERNAL_MODULES_DIR}/spdlog/build"
)

list(APPEND CMAKE_PREFIX_PATH "${EXTERNAL_PATHS}")

find_package(MAVLink REQUIRED)
find_package(spdlog REQUIRED)

if (WIN32)
    add_library(ws2_32 SHARED IMPORTED)
    set_target_properties(ws2_32 PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/dll/ws2_32.dll"
    )
endif()

add_executable(${project_name} mavlink_listener.cpp)

target_include_directories(${project_name} PRIVATE
    include
    include/protocore/utils
    external/modules/spdlog/include/spdlog
)

target_link_libraries(${project_name} PRIVATE spdlog::spdlog MAVLink::mavlink)

if (WIN32)
    target_link_libraries(${project_name} PRIVATE ws2_32)
endif()


