#include "interface.h"

#ifdef _WIN32
#include "impl_windows.h"
#else
#error "Only Windows supported yet"
#endif

std::unique_ptr<I_FifoServer_impl> FifoImplFactory::createServerImpl(const std::string& path) {
#ifdef _WIN32
    return std::make_unique<WinFifoServerImpl>(path);
#else
    #error "Only Windows supported yet"
#endif
}

std::unique_ptr<I_FifoClient_impl> FifoImplFactory::createClientImpl(const std::string& path) {
#ifdef _WIN32
    return std::make_unique<WinFifoClientImpl>(path);
#else
    #error "Only Windows supported yet"
#endif
}