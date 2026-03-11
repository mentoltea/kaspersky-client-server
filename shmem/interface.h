#ifndef SHMEM_INTERFACE_H
#define SHMEM_INTERFACE_H

#include <memory>
#include <string>

class I_SharedMemory_impl {
public:
    virtual ~I_SharedMemory_impl() = default;
    
    virtual void* get() = 0;
    
    virtual size_t size() const = 0;
};

class SharedMemoryFactory {
public:
    enum class Mode {
        Create,
        Open
    };
    
    static std::unique_ptr<I_SharedMemory_impl> createImpl(
        const std::string& name, 
        size_t size,
        Mode mode
    );
};

#endif // SHMEM_INTERFACE_H