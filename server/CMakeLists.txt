cmake_minimum_required(VERSION 3.10)
project(Server)

add_executable(server main.cpp)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(server PRIVATE tcp)