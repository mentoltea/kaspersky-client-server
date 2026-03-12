#include "initializer.h"
#include "interface.h"
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else 
#error "Only windows supported yet"
#endif

namespace TCP {

#ifdef _WIN32
static int refCount = 0;
static bool initialized = false;

void ensureWinsock() {
    if (!initialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
        }
        initialized = true;
        
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        // std::cout << "WSA initialized" << std::endl;
        // std::cout << hostname << std::endl;
    }
    refCount++;
}

#else 
#error "Only windows supported yet"
#endif

void Initializer::initialize() {
#ifdef _WIN32
    ensureWinsock();
#else 
#error "Only windows supported yet"
#endif
}

void Initializer::cleanup() {
#ifdef _WIN32
    refCount--;
    if (refCount == 0 && initialized) {
        WSACleanup();
        initialized = false;
    }
#else 
#error "Only windows supported yet"
#endif
}

}