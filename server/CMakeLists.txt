cmake_minimum_required(VERSION 3.10)
project(Server)

set(SERVER_HEADERS "common.h" "shared_data.hpp")
set(HANDLER_HEADERS "common.h" "shared_data.hpp")

add_executable(server "main.cpp" ${SERVER_HEADERS})

add_executable(handler "client_handler.cpp" ${HANDLER_HEADERS})

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
set_target_properties(handler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(server PRIVATE tcp)
target_link_libraries(server PRIVATE fifo)
target_link_libraries(server PRIVATE process)

target_link_libraries(handler PRIVATE tcp)
target_link_libraries(handler PRIVATE fifo)

