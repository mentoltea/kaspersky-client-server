#ifndef TCP_IMPL_WINDOWS_H
#define TCP_IMPL_WINDOWS_H

#include "interface.h"

namespace Win32 {
    #include <winsock2.h>
    #include <ws2tcpip.h>
}

#include <string>
#include <stdexcept>
#include <memory>

#pragma comment(lib, "ws2_32.lib")


class WinsockInitializer {
public:
    WinsockInitializer();
    ~WinsockInitializer();
    
    static void ensureInitialized();
    
private:
    static int refCount;
    static bool initialized;
};


class WindowsServerImpl : public I_TCPServer_impl {
public:
    WindowsServerImpl(int port, const std::string& address);
    ~WindowsServerImpl() override;

    void bind(const std::string& address, int port) override;
    void listen(int maxQueue) override;
    
    std::unique_ptr<I_TCPSocket_impl> accept() override;
    
    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    
    void close() override;
    bool isOpen() const override;

private:
    Win32::SOCKET serverSocket;
    bool isListening;
    bool isBound;
    
    void checkError(bool condition, const std::string& message);
    static void throwWSAError(const std::string& operation);
};


class WindowsClientImpl : public I_TCPClient_impl {
public:
    WindowsClientImpl();
    ~WindowsClientImpl() override;

    void connect(const std::string& address, int port) override;
    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    void close() override;
    bool isOpen() const override;

private:
    Win32::SOCKET clientSocket;
    bool isConnected;
    
    void checkError(bool condition, const std::string& message);
    static void throwWSAError(const std::string& operation);
};


class WindowsAcceptedImpl : public I_TCPSocket_impl {
public:
    explicit WindowsAcceptedImpl(Win32::SOCKET acceptedSocket);
    ~WindowsAcceptedImpl() override;

    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    void close() override;
    bool isOpen() const override;

private:
    Win32::SOCKET clientSocket;
    bool isConnected;
    
    void checkError(bool condition, const std::string& message);
    static void throwWSAError(const std::string& operation);
};

#endif // TCP_IMPL_WINDOWS_H