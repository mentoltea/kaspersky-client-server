file(GLOB SHMEM_SOURCES "*.cpp")
file(GLOB SHMEM_HEADERS "*.h")

add_library(shmem ${SHMEM_SOURCES} ${SHMEM_HEADERS})