#include "impl_windows.h"
#include "socket.h"

#include <iostream>

using namespace Win32;

int WinsockInitializer::refCount = 0;
bool WinsockInitializer::initialized = false;

WinsockInitializer::WinsockInitializer() {
    ensureInitialized();
}

WinsockInitializer::~WinsockInitializer() {
    refCount--;
    if (refCount == 0 && initialized) {
        WSACleanup();
        initialized = false;
    }
}

void WinsockInitializer::ensureInitialized() {
    if (!initialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
        }
        initialized = true;
    }
    refCount++;
}





WindowsServerImpl::WindowsServerImpl(int port, const std::string& address) 
    : serverSocket(INVALID_SOCKET), isListening(false), isBound(false) {
    
    WinsockInitializer::ensureInitialized();
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        throwWSAError("socket");
    }
    
    char opt = 1; // переиспользование адресса
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    bind(address, port);
}

WindowsServerImpl::~WindowsServerImpl() {
    close();
}

void WindowsServerImpl::bind(const std::string& address, int port) {
    if (isBound) return;
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (address.empty() || address == "0.0.0.0") {
        serverAddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);
    }
    
    int result = ::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        throwWSAError("bind");
    }
    
    isBound = true;
}

void WindowsServerImpl::listen(int maxQueue) {
    if (!isBound) {
        throw std::runtime_error("Server socket not bound");
    }
    
    int result = ::listen(serverSocket, maxQueue);
    if (result == SOCKET_ERROR) {
        throwWSAError("listen");
    }
    
    isListening = true;
}

std::unique_ptr<I_TCPSocket_impl> WindowsServerImpl::accept() {
    if (!isListening) {
        throw std::runtime_error("Server is not listening");
    }
    
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    
    SOCKET clientSocket = ::accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        throwWSAError("accept");
    }
    
    return std::make_unique<WindowsAcceptedImpl>(clientSocket);
}

void WindowsServerImpl::send(const std::string& data) {
    throw std::runtime_error("Cannot send on server socket - use accept() first");
}

std::string WindowsServerImpl::receive(size_t bufferSize) {
    throw std::runtime_error("Cannot receive on server socket - use accept() first");
}

void WindowsServerImpl::close() {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        isListening = false;
        isBound = false;
    }
}

bool WindowsServerImpl::isOpen() const {
    return serverSocket != INVALID_SOCKET;
}

void WindowsServerImpl::throwWSAError(const std::string& operation) {
    int error = WSAGetLastError();
    throw std::runtime_error(operation + " failed with error: " + std::to_string(error));
}

void WindowsServerImpl::checkError(bool condition, const std::string& message) {
    if (condition) {
        throw std::runtime_error(message);
    }
}

WindowsClientImpl::WindowsClientImpl() 
    : clientSocket(INVALID_SOCKET), isConnected(false) {
    
    WinsockInitializer::ensureInitialized();
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        throwWSAError("socket");
    }
}

WindowsClientImpl::~WindowsClientImpl() {
    close();
}

void WindowsClientImpl::connect(const std::string& address, int port) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address: " + address);
    }
    
    int result = ::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        throwWSAError("connect");
    }
    
    isConnected = true;
}

void WindowsClientImpl::send(const std::string& data) {
    if (!isConnected) {
        throw std::runtime_error("Client not connected");
    }
    
    int result = ::send(clientSocket, data.c_str(), data.size(), 0);
    if (result == SOCKET_ERROR) {
        throwWSAError("send");
    }
}

std::string WindowsClientImpl::receive(size_t bufferSize) {
    if (!isConnected) {
        throw std::runtime_error("Client not connected");
    }
    
    auto buffer = std::make_unique<char[]>(bufferSize);
    int result = ::recv(clientSocket, buffer.get(), bufferSize, 0);
    
    if (result == SOCKET_ERROR) {
        throwWSAError("recv");
    } else if (result == 0) {
        isConnected = false;
        return ""; // соединение закрыто
    }
    
    return std::string(buffer.get(), result);
}

void WindowsClientImpl::close() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        isConnected = false;
    }
}

bool WindowsClientImpl::isOpen() const {
    return clientSocket != INVALID_SOCKET;
}

void WindowsClientImpl::throwWSAError(const std::string& operation) {
    int error = WSAGetLastError();
    throw std::runtime_error(operation + " failed with error: " + std::to_string(error));
}

void WindowsClientImpl::checkError(bool condition, const std::string& message) {
    if (condition) {
        throw std::runtime_error(message);
    }
}

WindowsAcceptedImpl::WindowsAcceptedImpl(SOCKET acceptedSocket) 
    : clientSocket(acceptedSocket), isConnected(true) {
    
    WinsockInitializer::ensureInitialized();
}

WindowsAcceptedImpl::~WindowsAcceptedImpl() {
    close();
}

void WindowsAcceptedImpl::send(const std::string& data) {
    if (!isConnected) {
        throw std::runtime_error("Socket not connected");
    }
    
    int result = ::send(clientSocket, data.c_str(), data.size(), 0);
    if (result == SOCKET_ERROR) {
        throwWSAError("send");
    }
}

std::string WindowsAcceptedImpl::receive(size_t bufferSize) {
    if (!isConnected) {
        throw std::runtime_error("Socket not connected");
    }
    
    auto buffer = std::make_unique<char[]>(bufferSize);
    int result = ::recv(clientSocket, buffer.get(), bufferSize, 0);
    
    if (result == SOCKET_ERROR) {
        throwWSAError("recv");
    } else if (result == 0) {
        isConnected = false;
        return "";
    }
    
    return std::string(buffer.get(), result);
}

void WindowsAcceptedImpl::close() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        isConnected = false;
    }
}

bool WindowsAcceptedImpl::isOpen() const {
    return clientSocket != INVALID_SOCKET;
}

void WindowsAcceptedImpl::throwWSAError(const std::string& operation) {
    int error = WSAGetLastError();
    throw std::runtime_error(operation + " failed with error: " + std::to_string(error));
}

void WindowsAcceptedImpl::checkError(bool condition, const std::string& message) {
    if (condition) {
        throw std::runtime_error(message);
    }
}