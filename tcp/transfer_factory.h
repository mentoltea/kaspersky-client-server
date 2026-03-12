#ifndef TCP_TRANSFER_FACTORY_H
#define TCP_TRANSFER_FACTORY_H

#include "interface.h"
#include "transfer_token.h"
#include <memory>


class SocketTransferFactory {
public:
    static SocketTransferToken createToken(const I_TCPSocket_impl& socket, uint64_t targetPid);
    
    static std::unique_ptr<I_TCPSocket_impl> createImplFromToken(const SocketTransferToken& token);
};

#endif // TCP_TRANSFER_FACTORY_H