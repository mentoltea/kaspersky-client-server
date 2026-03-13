cmake_minimum_required(VERSION 3.10)
project(Statistic)

add_executable(statistic "main.cpp"  "../server/statistic.h")

set_target_properties(statistic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(statistic PRIVATE fifo)