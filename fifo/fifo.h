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

    void waitConnection();

    void write(const std::string& data);
private:
    std::unique_ptr< I_FifoServer_impl > pimpl;
};

class FifoClient {
public:
    FifoClient(const std::string& path);
    ~FifoClient();

    std::string read(size_t bufferSize = 1024);
private:
    std::unique_ptr< I_FifoClient_impl > pimpl;
};

#endif // FIFO_H