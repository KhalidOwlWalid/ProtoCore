cmake_minimum_required(VERSION 3.15...4.0)

project(mavlink_examples)

message("Compiling for ${PROJECT_NAME}")

# Manually add the directory file to the MAVLink
set(MAVLink_DIR "../../../external/modules/mavlink/install/lib/cmake/MAVLink")

find_package(MAVLink REQUIRED)
find_package(spdlog REQUIRED)

macro(build_example exec_name exec_source_file lib)
    message("Building example for ${exec_name}")
    add_executable(${exec_name} ${exec_source_file})
    # lib expects a variable name, eg pass variables created by set() cmake function
    target_link_libraries(${exec_name} PRIVATE ${${lib}})
endmacro()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples)

set(COMMON_LIBS spdlog::spdlog MAVLink::mavlink)

build_example(mavlink_server mavlink_server.cpp COMMON_LIBS)
build_example(mavlink_heartbeat mavlink_heartbeat.cpp COMMON_LIBS)
build_example(mavlink_set_interval mavlink_set_msg_interval.cpp COMMON_LIBS)
