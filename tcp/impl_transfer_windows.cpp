#include "transfer_factory.h"
#include "transfer_token.h"
#include "impl_windows.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>

using namespace TCP;


struct WinTokenData {
    WSAPROTOCOL_INFO protocolInfo;
    uint64_t targetPid;
    
    uint32_t magic = 0xDEADBEEF;
    uint32_t version = 1;
};

SOCKET getNativeSocket(const I_TCPSocket_impl& socket) {
    const WinTCPSocketImpl* winSocket = dynamic_cast<const WinTCPSocketImpl*>(&socket);
    if (!winSocket) {
        throw std::runtime_error("Socket is not a Windows socket");
    }
    
    return static_cast<SOCKET>(winSocket->native_handle());
}


SocketTransferToken SocketTransferFactory::createToken(
    const I_TCPSocket_impl& socket, uint64_t targetPid) {
    
    SOCKET sock = getNativeSocket(socket);

    
    WinTokenData tokenData;
    tokenData.targetPid = targetPid;
    
    // const_cast<WinTCPSocketImpl*>(dynamic_cast<const WinTCPSocketImpl*>(&socket)) ->diagnoseSocketForDuplicate();

    if (WSADuplicateSocket(sock, targetPid, &tokenData.protocolInfo) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        std::stringstream ss;
        ss << "WSADuplicateSocket failed: " << error;
        throw std::runtime_error(ss.str());
    }
    
    return SocketTransferToken::fromData(&tokenData, sizeof(tokenData));
}

std::unique_ptr<I_TCPSocket_impl> SocketTransferFactory::createImplFromToken(
    const SocketTransferToken& token) {
    
    if (token.size() != sizeof(WinTokenData)) {
        throw std::runtime_error("Invalid token size for Windows");
    }
    
    const WinTokenData* tokenData = static_cast<const WinTokenData*>(token.data());
    
    if (tokenData->magic != 0xDEADBEEF || tokenData->version != 1) {
        throw std::runtime_error("Invalid token format");
    }
    
    SOCKET s = WSASocket(
        AF_INET,
        SOCK_STREAM,
        0,
        const_cast<LPWSAPROTOCOL_INFO>(&tokenData->protocolInfo),
        0,
        WSA_FLAG_OVERLAPPED
    );
    
    if (s == INVALID_SOCKET) {
        int error = WSAGetLastError();
        std::stringstream ss;
        ss << "WSASocket from duplicate failed: " << error;
        throw std::runtime_error(ss.str());
    }
    
    return std::make_unique<WinTCPSocketImpl>(s, true);
}