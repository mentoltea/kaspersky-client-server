set(FIFO_HEADERS
    "interface.h"
    "fifo.h"
)

set(FIFO_SOURCES
    "fifo.cpp"
    "factory.cpp"
)

if (WIN32) 
    list(APPEND FIFO_HEADERS "impl_windows.h")
    list(APPEND FIFO_SOURCES "impl_windows.cpp")
else()
    message(FATAL_ERROR "FIFO implementation is not available for this platform. Currently only Windows is supported.")
endif()

add_library(fifo ${FIFO_SOURCES} ${FIFO_HEADERS})