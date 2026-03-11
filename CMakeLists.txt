cmake_minimum_required(VERSION 3.10)
project(ClientServerStatistic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tcp)

add_subdirectory(client)
# add_subdirectory(server)
# add_subdirectory(statistic)

add_custom_target(all_targets ALL
    DEPENDS client server statistic
    COMMENT "Building all targets"
)