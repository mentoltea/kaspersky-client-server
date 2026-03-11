cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(statistic)

add_custom_target(all_targets ALL
    DEPENDS client server statistic
    COMMENT "Building all targets"
)