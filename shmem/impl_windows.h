#ifndef SHMEM_IMPL_WINDOWS_H
#define SHMEM_IMPL_WINDOWS_H

#include "interface.h"
#include <windows.h>
#include <string>
#include <stdexcept>
#include <sstream>

class WinSharedMemoryImpl : public I_SharedMemory_impl {
public:
    WinSharedMemoryImpl(const std::string& name, size_t size);
    
    WinSharedMemoryImpl(const std::string& name);
    
    ~WinSharedMemoryImpl() override;
    
    WinSharedMemoryImpl(const WinSharedMemoryImpl&) = delete;
    WinSharedMemoryImpl& operator=(const WinSharedMemoryImpl&) = delete;
    
    WinSharedMemoryImpl(WinSharedMemoryImpl&& other) noexcept;
    WinSharedMemoryImpl& operator=(WinSharedMemoryImpl&& other) noexcept;
    
    void* get() override { return data_; }
    size_t size() const override { return size_; }

private:
    std::string name;
    size_t size_;
    void* data_;
    HANDLE fileMapping;
    
    void throwLastError(const std::string& operation);
};

#endif // SHMEM_IMPL_WINDOWS_H