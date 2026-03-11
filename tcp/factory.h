#ifndef TCP_FACTORY_H
#define TCP_FACTORY_H

#include "interface.h"
#include <memory>
#include <string>

class SocketFactory {
public:
    static std::unique_ptr<I_TCPServer_impl> createServerImpl(const std::string& address, int port);
    
    static std::unique_ptr<I_TCPClient_impl> createClientImpl();
};

#endif // TCP_FACTORY_H