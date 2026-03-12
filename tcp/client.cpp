#include "client.h"
#include "factory.h"
#include <stdexcept>

using namespace TCP;

TCPClient::TCPClient() 
    : pimpl(SocketFactory::createClientImpl()) {
    if (!pimpl) {
        throw std::runtime_error("Failed to create client implementation");
    }
}

TCPClient::TCPClient(TCPClient&& other) noexcept
    : pimpl(std::move(other.pimpl)) {}

TCPClient& TCPClient::operator=(TCPClient&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

TCPClient::~TCPClient() = default;

void TCPClient::connect(const std::string& address, int port) {
    if (!pimpl) throw std::runtime_error("Client not initialized");
    pimpl->connect(address, port);
}

void TCPClient::send(const std::string& data) {
    if (!pimpl) throw std::runtime_error("Client not initialized");
    pimpl->send(data);
}

std::string TCPClient::receive(size_t bufferSize) {
    if (!pimpl) throw std::runtime_error("Client not initialized");
    return pimpl->receive(bufferSize);
}

void TCPClient::close() {
    if (pimpl) {
        pimpl->close();
    }
}

bool TCPClient::isConnected() const {
    return pimpl && pimpl->isOpen();
}

bool TCPClient::isOpen() const {
    return pimpl && pimpl->isOpen();
}