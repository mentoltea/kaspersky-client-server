#include "server.h"
#include "socket.h"
#include "factory.h"
#include "impl_windows.h"


TCPServer::TCPServer(const std::string& address, int port) 
    : pimpl(SocketFactory::createServerImpl(address, port)) {}

TCPServer::TCPServer(TCPServer&& other) noexcept
    : pimpl(std::move(other.pimpl)) {}

TCPServer& TCPServer::operator=(TCPServer&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

TCPServer::~TCPServer() = default;

void TCPServer::listen(int maxQueue) {
    if (!pimpl) throw std::runtime_error("Server not initialized");
    pimpl->listen(maxQueue);
}

std::unique_ptr<TCPSocket> TCPServer::accept() {
    if (!pimpl) throw std::runtime_error("Server not initialized");
    
    auto acceptedImpl = pimpl->accept();
    
    return std::unique_ptr<TCPSocket>(
        new TCPSocket(std::move(acceptedImpl))
    );
}

void TCPServer::close() {
    if (pimpl) {
        pimpl->close();
    }
}

bool TCPServer::isOpen() const {
    return pimpl && pimpl->isOpen();
}