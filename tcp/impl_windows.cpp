#include "impl_windows.h"
#include "initializer.h"

using namespace TCP;

#include <sstream>
#include <vector>
#include <iostream>


void WinTCPSocketImpl::diagnoseSocketForDuplicate() {
    std::cout << "=== Socket Diagnostic ===" << std::endl;
    
    std::cout << "Socket handle value: " << sock << std::endl;
    if (sock == INVALID_SOCKET || sock == 0) {
        std::cout << "INVALID SOCKET!" << std::endl;
        return;
    } else {
        std::cout << "Socket handle looks valid" << std::endl;
    }
    
    int type;
    int len = sizeof(type);
    int result = getsockopt(sock, SOL_SOCKET, SO_TYPE, (char*)&type, &len);
    if (result == 0) {
        std::cout << "getsockopt SO_TYPE succeeded. Type: " << type << " (1=SOCK_STREAM)" << std::endl;
    } else {
        int err = WSAGetLastError();
        std::cout << "getsockopt SO_TYPE failed. Error: " << err << std::endl;
        if (err == WSAENOTSOCK) {
            std::cout << "This is NOT a socket! (WSAENOTSOCK)" << std::endl;
        }
    }
    
    int error_code;
    socklen_t error_len = sizeof(error_code);
    result = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_len);
    if (result == 0) {
        std::cout << "getsockopt SO_ERROR succeeded. Error code: " << error_code << std::endl;
    } else {
        std::cout << "getsockopt SO_ERROR failed" << std::endl;
    }
    
    sockaddr_in localAddr;
    int addrLen = sizeof(localAddr);
    result = getsockname(sock, (sockaddr*)&localAddr, &addrLen);
    if (result == 0) {
        std::cout << "getsockname succeeded. Port: " << ntohs(localAddr.sin_port) << std::endl;
    } else {
        int err = WSAGetLastError();
        std::cout << "getsockname failed. Error: " << err << std::endl;
        if (err == WSAEINVAL) {
            std::cout << "Socket is not bound (not connected/listening?)" << std::endl;
        }
    }
    
    std::cout << "=== End Diagnostic ===" << std::endl;
}

// ----------------------------

WinTCPSocketImpl::WinTCPSocketImpl(SOCKET sock, bool takeOwnership) 
    : sock(sock), isConnected(true), owner(takeOwnership) {
    // Initializer::initialize();
}

WinTCPSocketImpl::WinTCPSocketImpl(const std::string& /*address*/, int /*port*/) 
    : sock(INVALID_SOCKET), isConnected(false), owner(true) {
    // Initializer::initialize();
    
    sock = WSASocketA(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
        throwLastError("socket");
    }
}

WinTCPSocketImpl::~WinTCPSocketImpl() {
    close();
}

void WinTCPSocketImpl::send(const std::string& data) {
    if (sock == INVALID_SOCKET) {
        throw std::runtime_error("Socket not connected");
    }
    
    int result = ::send(sock, data.c_str(), static_cast<int>(data.size()), 0);
    if (result == SOCKET_ERROR) {
        throwLastError("send");
    }
}

std::string WinTCPSocketImpl::receive(size_t bufferSize) {
    if (sock == INVALID_SOCKET) {
        throw std::runtime_error("Socket not connected");
    }
    
    std::vector<char> buffer(bufferSize);
    int result = ::recv(sock, buffer.data(), static_cast<int>(bufferSize), 0);
    
    if (result == SOCKET_ERROR) {
        throwLastError("recv");
    } else if (result == 0) {
        isConnected = false;
        return "";
    }
    
    return std::string(buffer.data(), result);
}

void WinTCPSocketImpl::close() {
    if (sock != INVALID_SOCKET && owner) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        isConnected = false;
    }
}

bool WinTCPSocketImpl::isOpen() const {
    return sock != INVALID_SOCKET;
}

void WinTCPSocketImpl::throwLastError(const std::string& operation) {
    int error = WSAGetLastError();
    std::stringstream ss;
    ss << operation << " failed with error: " << error;
    throw std::runtime_error(ss.str());
}



WinTCPServerImpl::WinTCPServerImpl(const std::string& address, int port) 
    : serverSocket(INVALID_SOCKET), isListening(false), isBound(false) {
    
    // Initializer::initialize();
    
    serverSocket = WSASocketA(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (serverSocket == INVALID_SOCKET) {
        throwLastError("socket");
    }
    
    char opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    bind(address, port);
}

WinTCPServerImpl::~WinTCPServerImpl() {
    close();
}

void WinTCPServerImpl::bind(const std::string& address, int port) {
    if (isBound) return;
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    
    if (address.empty() || address == "0.0.0.0") {
        serverAddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);
    }
    
    int result = ::bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        throwLastError("bind");
    }
    
    isBound = true;
}

void WinTCPServerImpl::listen(int maxQueue) {
    if (!isBound) {
        throw std::runtime_error("Server socket not bound");
    }
    
    int result = ::listen(serverSocket, maxQueue);
    if (result == SOCKET_ERROR) {
        throwLastError("listen");
    }
    
    isListening = true;
}

std::unique_ptr<I_TCPSocket_impl> WinTCPServerImpl::accept() {
    if (!isListening) {
        throw std::runtime_error("Server is not listening");
    }
    
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    
    SOCKET clientSocket = ::accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        throwLastError("accept");
    }
    
    return std::make_unique<WinTCPSocketImpl>(clientSocket, true);
}

void WinTCPServerImpl::send(const std::string& /*data*/) {
    throw std::runtime_error("Cannot send on server socket - use accept() first");
}

std::string WinTCPServerImpl::receive(size_t /*bufferSize*/) {
    throw std::runtime_error("Cannot receive on server socket - use accept() first");
}

void WinTCPServerImpl::close() {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        isListening = false;
        isBound = false;
    }
}

bool WinTCPServerImpl::isOpen() const {
    return serverSocket != INVALID_SOCKET;
}

void WinTCPServerImpl::throwLastError(const std::string& operation) {
    int error = WSAGetLastError();
    std::stringstream ss;
    ss << operation << " failed with error: " << error;
    throw std::runtime_error(ss.str());
}



WinTCPClientImpl::WinTCPClientImpl() 
    : clientSocket(INVALID_SOCKET), isConnected(false) {
    
    // Initializer::initialize();
    
    clientSocket = WSASocketA(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (clientSocket == INVALID_SOCKET) {
        throwLastError("socket");
    }
}

WinTCPClientImpl::~WinTCPClientImpl() {
    close();
}

void WinTCPClientImpl::connect(const std::string& address, int port) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address: " + address);
    }
    
    int result = ::connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        throwLastError("connect");
    }
    
    isConnected = true;
}

void WinTCPClientImpl::send(const std::string& data) {
    if (!isConnected) {
        throw std::runtime_error("Client not connected");
    }
    
    int result = ::send(clientSocket, data.c_str(), static_cast<int>(data.size()), 0);
    if (result == SOCKET_ERROR) {
        throwLastError("send");
    }
}

std::string WinTCPClientImpl::receive(size_t bufferSize) {
    if (!isConnected) {
        throw std::runtime_error("Client not connected");
    }
    
    std::vector<char> buffer(bufferSize);
    int result = ::recv(clientSocket, buffer.data(), static_cast<int>(bufferSize), 0);
    
    if (result == SOCKET_ERROR) {
        throwLastError("recv");
    } else if (result == 0) {
        isConnected = false;
        return "";
    }
    
    return std::string(buffer.data(), result);
}

void WinTCPClientImpl::close() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        isConnected = false;
    }
}

bool WinTCPClientImpl::isOpen() const {
    return clientSocket != INVALID_SOCKET;
}

void WinTCPClientImpl::throwLastError(const std::string& operation) {
    int error = WSAGetLastError();
    std::stringstream ss;
    ss << operation << " failed with error: " << error;
    throw std::runtime_error(ss.str());
}