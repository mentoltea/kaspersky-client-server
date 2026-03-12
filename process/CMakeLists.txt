set(PROCESS_MANAGER_HEADERS "manager.h")

if (WIN32) 
    set(PROCESS_MANAGER_SOURCES "windows.cpp")
else()
    message(FATAL_ERROR "ProcessManager implementation is not available for this platform. Currently only Windows is supported.")
endif()

add_library(process ${PROCESS_MANAGER_SOURCES} ${PROCESS_MANAGER_HEADERS})