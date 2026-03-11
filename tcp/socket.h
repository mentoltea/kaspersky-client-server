#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>
#include <memory>

class I_TCPSocket_impl;

// для принятых клиентских соединений
class TCPSocket {
public:
    TCPSocket(const TCPSocket&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;
    
    TCPSocket(TCPSocket&& other) noexcept;
    TCPSocket& operator=(TCPSocket&& other) noexcept;
    
    ~TCPSocket();

    void send(const std::string& data);
    std::string receive(size_t bufferSize = 1024);
    
    void close();
    bool isOpen() const;

private:
    friend class TCPServer;
    explicit TCPSocket(std::unique_ptr<I_TCPSocket_impl> impl);
    
    std::unique_ptr<I_TCPSocket_impl> pimpl;
};

#endif // TCP_SOCKET_H