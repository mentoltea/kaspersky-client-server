#include "socket.h"
#include "interface.h"
#include <stdexcept>

TCPSocket::TCPSocket(std::unique_ptr<I_TCPSocket_impl> impl)
    : pimpl(std::move(impl)) {
    if (!pimpl) {
        throw std::runtime_error("Invalid socket implementation");
    }
}

TCPSocket::TCPSocket(TCPSocket&& other) noexcept
    : pimpl(std::move(other.pimpl)) {}

TCPSocket& TCPSocket::operator=(TCPSocket&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

TCPSocket::~TCPSocket() = default;

void TCPSocket::send(const std::string& data) {
    if (!pimpl) throw std::runtime_error("Socket not initialized");
    pimpl->send(data);
}

std::string TCPSocket::receive(size_t bufferSize) {
    if (!pimpl) throw std::runtime_error("Socket not initialized");
    return pimpl->receive(bufferSize);
}

void TCPSocket::close() {
    if (pimpl) {
        pimpl->close();
    }
}

bool TCPSocket::isOpen() const {
    return pimpl && pimpl->isOpen();
}