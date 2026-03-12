set(TCP_HEADERS
    "interface.h"
    "tcp.h"
    "client.h"
    "server.h"
    "factory.h"
    "socket.h"
    "transfer_token.h"
    "transfer_factory.h"
)

set(TCP_SOURCES
    "client.cpp"
    "server.cpp"
    "factory.cpp"
    "socket.cpp"
    "transfer_token.cpp"
)

if (WIN32) 
    list(APPEND TCP_HEADERS "impl_windows.h")
    list(APPEND TCP_SOURCES 
        "impl_windows.cpp"
        "impl_transfer_windows.cpp"
    )
else()
    message(FATAL_ERROR "TCP implementation is not available for this platform. Currently only Windows is supported.")
endif()

add_library(tcp ${TCP_SOURCES} ${TCP_HEADERS})

if(WIN32)
    target_link_libraries(tcp PUBLIC ws2_32)
endif()