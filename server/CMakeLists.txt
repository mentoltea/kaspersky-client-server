cmake_minimum_required(VERSION 3.10)
project(Server)


set(SERVER_HEADERS "common.h" "statistic.h" "server.h" "shared_data.hpp")
set(SERVER_SOURCES "main.cpp" "common.cpp" "confupdater.cpp" "statlistener.cpp")

set(HANDLER_HEADERS "common.h" "shared_data.hpp")
set(HANDLER_SOURCES "client_handler.cpp" "common.cpp")

add_executable(server ${SERVER_SOURCES} ${SERVER_HEADERS})

add_executable(handler ${HANDLER_SOURCES} ${HANDLER_HEADERS})

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
set_target_properties(handler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(server PRIVATE tcp)
target_link_libraries(server PRIVATE fifo)
target_link_libraries(server PRIVATE process)
target_link_libraries(server PRIVATE mutex)
target_link_libraries(server PRIVATE shmem)

target_link_libraries(handler PRIVATE tcp)
target_link_libraries(handler PRIVATE fifo)
target_link_libraries(handler PRIVATE mutex)
target_link_libraries(handler PRIVATE shmem)

