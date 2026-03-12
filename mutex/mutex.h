#ifndef MUTEX_H
#define MUTEX_H

#include <thread>
#include <cstdlib>
#include <string>
#include <atomic>
#include <mutex>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#error "Only windows supported yet"
#endif

class SharedMutex {
public:
    SharedMutex();
    ~SharedMutex();

    SharedMutex(const SharedMutex&) = delete;
    SharedMutex& operator=(const SharedMutex&) = delete;
    
    SharedMutex(SharedMutex&& other) noexcept;
    SharedMutex& operator=(SharedMutex&& other) noexcept;
    
    void lock();
    void unlock();
    bool try_lock();
    
    bool isValid() const;
private:
    struct PlatformData {
#ifdef _WIN32
        HANDLE hMutex;
        uint64_t handleValue;
#else
#error "Only windows supported yet"
#endif
        bool initialized;
        bool owner;
        char padding[6];
    };

    PlatformData data;
};

#endif // MUTEX_H