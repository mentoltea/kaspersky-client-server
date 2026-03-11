file(GLOB FIFO_SOURCES "*.cpp")
file(GLOB FIFO_HEADERS "*.h")

add_library(fifo ${FIFO_SOURCES} ${FIFO_HEADERS})