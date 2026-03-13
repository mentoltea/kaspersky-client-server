cmake_minimum_required(VERSION 3.10)
project(Client)

add_executable(client "main.cpp")

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(client PRIVATE tcp)