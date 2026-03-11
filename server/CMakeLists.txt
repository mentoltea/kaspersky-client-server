cmake_minimum_required(VERSION 3.10)
project(Server)

file(GLOB SERVER_HEADERS "*.h" "*.hpp")

add_executable(server "main.cpp" ${SERVER_HEADERS})
add_executable(handler "client_handler.cpp" ${SERVER_HEADERS})

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(server PRIVATE tcp)