#ifndef TCP_IMPL_WINDOWS_H
#define TCP_IMPL_WINDOWS_H

#include "interface.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <memory>
namespace TCP {
class WinTCPSocketImpl : public I_TCPSocket_impl {
public:
    WinTCPSocketImpl(SOCKET sock, bool takeOwnership = true);
    explicit WinTCPSocketImpl(const std::string& address, int port);
    ~WinTCPSocketImpl() override;

    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    void close() override;
    bool isOpen() const override;
    
    intptr_t native_handle() const { return (intptr_t)(sock); }

    void diagnoseSocketForDuplicate();
private:
    SOCKET sock;
    bool isConnected;
    bool owner;
    
    void throwLastError(const std::string& operation);
};

class WinTCPServerImpl : public I_TCPServer_impl {
public:
    WinTCPServerImpl(const std::string& address, int port);
    ~WinTCPServerImpl() override;

    void bind(const std::string& address, int port) override;
    void listen(int maxQueue) override;
    std::unique_ptr<I_TCPSocket_impl> accept() override;
    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    void close() override;
    bool isOpen() const override;
    
    intptr_t native_handle() const { return (intptr_t)(serverSocket); }

private:
    SOCKET serverSocket;
    bool isListening;
    bool isBound;
    
    void throwLastError(const std::string& operation);
};

class WinTCPClientImpl : public I_TCPClient_impl {
public:
    WinTCPClientImpl();
    ~WinTCPClientImpl() override;

    void connect(const std::string& address, int port) override;
    void send(const std::string& data) override;
    std::string receive(size_t bufferSize) override;
    void close() override;
    bool isOpen() const override;
    
    intptr_t native_handle() const { return (intptr_t)(clientSocket); }

private:
    SOCKET clientSocket;
    bool isConnected;
    
    void throwLastError(const std::string& operation);
};
}
#endif // TCP_IMPL_WINDOWS_H