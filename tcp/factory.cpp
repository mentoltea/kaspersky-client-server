#include "factory.h"
using namespace TCP;

#ifdef _WIN32
    #include "impl_windows.h"
#else
    #error "Only windows supported yet"
#endif

std::unique_ptr<I_TCPServer_impl> SocketFactory::createServerImpl(const std::string& address, int port) {

#ifdef _WIN32
    return std::make_unique<WinTCPServerImpl>(address, port);
#else
    #error "Only windows supported yet"
#endif

}

std::unique_ptr<I_TCPClient_impl> SocketFactory::createClientImpl() {

#ifdef _WIN32
    return std::make_unique<WinTCPClientImpl>();
#else
    #error "Only windows supported yet"
#endif

}