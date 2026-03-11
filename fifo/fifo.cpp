#include "fifo.h"
#include "interface.h"
#include <stdexcept>


FifoServer::FifoServer(const std::string& path) 
    : pimpl(FifoImplFactory::createServerImpl(path)) {
    if (!pimpl) {
        throw std::runtime_error("Failed to create server implementation");
    }
}

FifoServer::~FifoServer() = default;

void FifoServer::waitConnection() {
    if (!pimpl) throw std::runtime_error("Server not initialized");
    pimpl->waitConnection();
}

void FifoServer::write(const std::string& data) {
    if (!pimpl) throw std::runtime_error("Server not initialized");
    pimpl->write(data);
}





FifoClient::FifoClient(const std::string& path) 
    : pimpl(FifoImplFactory::createClientImpl(path)) {
    if (!pimpl) {
        throw std::runtime_error("Failed to create client implementation");
    }
}

FifoClient::~FifoClient() = default;

std::string FifoClient::read(size_t bufferSize) {
    if (!pimpl) throw std::runtime_error("Client not initialized");
    return pimpl->read(bufferSize);
}