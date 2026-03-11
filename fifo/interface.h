#ifndef FIFO_INTERFACE_H
#define FIFO_INTERFACE_H

#include <string>
#include <memory>

class I_FifoServer_impl {
public:
    virtual ~I_FifoServer_impl() = default;
    
    virtual void waitConnection() = 0;
    virtual void write(const std::string& data) = 0;
};

class I_FifoClient_impl {
public:
    virtual ~I_FifoClient_impl() = default;
    
    virtual std::string read(size_t bufferSize) = 0;
};

class FifoImplFactory {
public:
    static std::unique_ptr<I_FifoServer_impl> createServerImpl(const std::string& path);
    static std::unique_ptr<I_FifoClient_impl> createClientImpl(const std::string& path);
};

#endif // FIFO_INTERFACE_H