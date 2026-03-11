#ifndef FIFO_IMPL_WINDOWS_H
#define FIFO_IMPL_WINDOWS_H

#include "interface.h"
#include <windows.h>
#include <string>
#include <stdexcept>


class WinFifoServerImpl : public I_FifoServer_impl {
public:
    WinFifoServerImpl(const std::string& path);
    ~WinFifoServerImpl() override;

    void waitConnection() override;
    void write(const std::string& data) override;

private:
    std::string pipePath;
    HANDLE hPipe;
    
    void throwIfError(bool condition, const std::string& message);
    void throwLastError(const std::string& operation);
};


class WinFifoClientImpl : public I_FifoClient_impl {
public:
    WinFifoClientImpl(const std::string& path);
    ~WinFifoClientImpl() override;

    std::string read(size_t bufferSize) override;

private:
    std::string pipePath;
    HANDLE hPipe;
    
    void throwIfError(bool condition, const std::string& message);
    void throwLastError(const std::string& operation);
};

#endif // FIFO_IMPL_WINDOWS_H