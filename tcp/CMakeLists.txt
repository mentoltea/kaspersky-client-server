file(GLOB TCP_SOURCES "*.cpp")
file(GLOB TCP_HEADERS "*.h")

add_library(tcp ${TCP_SOURCES} ${TCP_HEADERS})

if(WIN32)
    target_link_libraries(tcp PUBLIC ws2_32)
endif()