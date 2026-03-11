#ifndef FIFO_H
#define FIFO_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#error "Only windows supported yet"
#endif

class FifoServer {
public:
    FifoServer(const std::string& path);
    ~FifoServer();

    void waitConnection();

    void write(const std::string& data);
private:
    std::string path;

#ifdef _WIN32
    HANDLE hPipe;
#else
#error "Only windows supported yet"
#endif
};

class FifoClient {
public:
    FifoClient(const std::string& path);
    ~FifoClient();

    std::string read(size_t bufferSize = 1024);
private:
    std::string path;

#ifdef _WIN32
    HANDLE hPipe;
#else
#error "Only windows supported yet"
#endif
};

#endif // FIFO_H