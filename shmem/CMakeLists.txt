set(SHMEM_HEADERS
    "interface.h"
    "shmem.h"
)

set(SHMEM_SOURCES
    "shmem.cpp"
    "factory.cpp"
)

if (WIN32) 
    list(APPEND SHMEM_HEADERS "impl_windows.h")
    list(APPEND SHMEM_SOURCES "impl_windows.cpp")
else()
    message(FATAL_ERROR "TCP implementation is not available for this platform. Currently only Windows is supported.")
endif()

add_library(shmem ${SHMEM_SOURCES} ${SHMEM_HEADERS})