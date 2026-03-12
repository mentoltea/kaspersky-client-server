#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#define PROTOCOL_VERSION 1

#define FIFO_NAME "server_handler_token"
#define INIT_POSTFIX "_initialize"

#define SHMEM_NAME "server_handler_shmem"

#include "shared_data.hpp"
#include "mutex/mutex.h"

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

// MUST live in shared memory
struct ShmemNavigator {
    size_t count;

    size_t common_offset;
    size_t signature_offset;

    static ShmemNavigator* initialize(void *memory, const std::vector<json> &threats, const std::vector< std::string > &dataSignatures);

    void* common() const;
    void* signature() const;
};

struct ThreatCommon {
    char name[64];
    char description[256];
    size_t occured;
    SharedMutex mutex;
};

#endif // SERVER_COMMON_H