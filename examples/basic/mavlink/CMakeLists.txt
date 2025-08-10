cmake_minimum_required(VERSION 3.15...4.0)

project(mavlink_examples)

message("Compiling for ${PROJECT_NAME}")

# Manually add the directory file to the MAVLink
set(MAVLink_DIR "../../../external/modules/mavlink/install/lib/cmake/MAVLink")

find_package(MAVLink REQUIRED)
find_package(spdlog REQUIRED)

add_executable(mavlink_server mavlink_server.cpp)

target_link_libraries(mavlink_server PRIVATE spdlog::spdlog MAVLink::mavlink)