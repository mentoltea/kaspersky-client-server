#ifndef FIFO_H
#define FIFO_H

#include <string>
#include <memory>

class I_FifoServer_impl;
class I_FifoClient_impl;

class FifoServer {
public:
    FifoServer(const std::string& path);
    ~FifoServer();

    FifoServer(const FifoServer&) = delete;
    FifoServer& operator=(const FifoServer&) = delete;

    FifoServer(FifoServer&& other) noexcept;
    FifoServer& operator=(FifoServer&& other) noexcept;

    void waitConnection();

    void write(const std::string& data);
private:
    std::unique_ptr< I_FifoServer_impl > pimpl;
};

class FifoClient {
public:
    FifoClient(const std::string& path);
    ~FifoClient();

    FifoClient(const FifoClient&) = delete;
    FifoClient& operator=(const FifoClient&) = delete;

    FifoClient(FifoClient&& other) noexcept;
    FifoClient& operator=(FifoClient&& other) noexcept;

    std::string read(size_t bufferSize = 1024);
private:
    std::unique_ptr< I_FifoClient_impl > pimpl;
};

#endif // FIFO_H