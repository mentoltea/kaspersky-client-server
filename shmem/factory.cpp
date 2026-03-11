#include "interface.h"

#ifdef _WIN32
#include "impl_windows.h"
#else
#error "Only Windows supported yet"
#endif

std::unique_ptr<I_SharedMemory_impl> SharedMemoryFactory::createImpl(
    const std::string& name, 
    size_t size,
    Mode mode) {
    
#ifdef _WIN32
    if (mode == Mode::Create) {
        return std::make_unique<WinSharedMemoryImpl>(name, size);
    } else {
        return std::make_unique<WinSharedMemoryImpl>(name);
    }
#else
    #error "Only Windows supported yet"
#endif
}