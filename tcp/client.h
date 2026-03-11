#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <memory>

class I_TCPClient_impl;

class TCPClient {
public:
    TCPClient();
    
    TCPClient(const TCPClient&) = delete;
    TCPClient& operator=(const TCPClient&) = delete;
    
    TCPClient(TCPClient&& other) noexcept;
    TCPClient& operator=(TCPClient&& other) noexcept;
    
    ~TCPClient();

    void connect(const std::string& address, int port);
    
    void send(const std::string& data);
    
    std::string receive(size_t bufferSize = 1024);
    
    void close();
    bool isConnected() const;
    bool isOpen() const;

private:
    std::unique_ptr<I_TCPClient_impl> pimpl;
};

#endif // TCP_CLIENT_H