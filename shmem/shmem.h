#ifndef SHMEM_H
#define SHMEM_H

class I_SharedMemory_impl;

#include <string>
#include <memory>

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size); // new
    SharedMemory(const std::string& name); // existing
    
    ~SharedMemory();

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    
    SharedMemory(SharedMemory&& other) noexcept;
    SharedMemory& operator=(SharedMemory&& other) noexcept;
    
    void* get();
    size_t size() const;
private:
    std::unique_ptr< I_SharedMemory_impl > pimpl;
};

#endif // SHMEM_H