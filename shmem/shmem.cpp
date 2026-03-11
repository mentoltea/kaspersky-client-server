#include "shmem.h"
#include "interface.h"
#include <stdexcept>

SharedMemory::SharedMemory(const std::string& name, size_t size)
    : pimpl(SharedMemoryFactory::createImpl(
          name, 
          size, 
          SharedMemoryFactory::Mode::Create)) {
    
    if (!pimpl) {
        throw std::runtime_error("Failed to create shared memory implementation");
    }
}

SharedMemory::SharedMemory(const std::string& name)
    : pimpl(SharedMemoryFactory::createImpl(
          name, 
          0, 
          SharedMemoryFactory::Mode::Open)) {
    
    if (!pimpl) {
        throw std::runtime_error("Failed to open shared memory implementation");
    }
}

SharedMemory::~SharedMemory() = default;

SharedMemory::SharedMemory(SharedMemory&& other) noexcept
    : pimpl(std::move(other.pimpl)) {}

SharedMemory& SharedMemory::operator=(SharedMemory&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

void* SharedMemory::get() {
    if (!pimpl) throw std::runtime_error("Shared memory not initialized");
    return pimpl->get();
}

size_t SharedMemory::size() const {
    if (!pimpl) throw std::runtime_error("Shared memory not initialized");
    return pimpl->size();
}