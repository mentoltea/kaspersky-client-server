set(MUTEX_HEADERS
    "mutex.h"
)

set(MUTEX_SOURCES)

if (WIN32) 
    list(APPEND MUTEX_SOURCES "windows.cpp")
else()
    message(FATAL_ERROR "Mutex implementation is not available for this platform. Currently only Windows is supported.")
endif()

add_library(mutex ${MUTEX_SOURCES} ${MUTEX_HEADERS})