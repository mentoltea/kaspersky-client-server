#include "mutex.h"
#include <stdexcept>
#include <sstream>

SharedMutex::SharedMutex() : data{} {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    
    data.hMutex = CreateMutex(&sa, FALSE, NULL);
    
    if (!data.hMutex) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "CreateMutex failed. Error: " << error;
        throw std::runtime_error(ss.str());
    }
    
    data.handleValue = reinterpret_cast<uint64_t>(data.hMutex);
    data.owner = true;
    data.initialized = true;
}

SharedMutex::~SharedMutex() {
    if (data.owner && data.initialized && data.hMutex && data.hMutex != INVALID_HANDLE_VALUE) {
        CloseHandle(data.hMutex);
        data.hMutex = NULL;
        data.handleValue = 0;
        data.initialized = false;
    }
}

SharedMutex::SharedMutex(SharedMutex&& other) noexcept
    : data(other.data) {
    
    other.data.hMutex = NULL;
    other.data.handleValue = 0;
    other.data.initialized = false;
    other.data.owner = false;
}

SharedMutex& SharedMutex::operator=(SharedMutex&& other) noexcept {
    if (this != &other) {
        data = other.data;
        
        other.data.hMutex = NULL;
        other.data.handleValue = 0;
        other.data.initialized = false;
        other.data.owner = false;
    }
    return *this;
}

void SharedMutex::lock() {
    if (!data.initialized) {
        throw std::runtime_error("Mutex not initialized");
    }
    
    DWORD result = WaitForSingleObject(data.hMutex, INFINITE);
    if (result != WAIT_OBJECT_0) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "WaitForSingleObject failed. Error: " << error;
        throw std::runtime_error(ss.str());
    }
}

void SharedMutex::unlock() {
    if (!data.initialized) {
        throw std::runtime_error("Mutex not initialized");
    }
    
    if (!ReleaseMutex(data.hMutex)) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "ReleaseMutex failed. Error: " << error;
        throw std::runtime_error(ss.str());
    }
}

bool SharedMutex::try_lock() {
    if (!data.initialized) {
        throw std::runtime_error("Mutex not initialized");
    }
    
    DWORD result = WaitForSingleObject(data.hMutex, 0);
    if (result == WAIT_OBJECT_0) {
        return true;
    } else if (result == WAIT_TIMEOUT) {
        return false;
    } else {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "WaitForSingleObject failed. Error: " << error;
        throw std::runtime_error(ss.str());
    }
}

bool SharedMutex::isValid() const {
    if (!data.initialized) {
        return false;
    }
    
    DWORD flags;
    return GetHandleInformation(data.hMutex, &flags) != 0;
}