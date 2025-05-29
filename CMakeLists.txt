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

include_directories(include)

if (WIN32)
    add_library(ws2_32 SHARED IMPORTED)
    set_target_properties(ws2_32 PROPERTIES
        # IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lib/ws2_32.dll"
        IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/dll/ws2_32.dll"
    )
endif()

add_executable(${project_name} main.cpp)

if (WIN32)
    target_link_libraries(${project_name} ws2_32)
endif()


