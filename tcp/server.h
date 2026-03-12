#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <memory>
namespace TCP {
class I_TCPServer_impl;
class TCPSocket;

class TCPServer {
public:
    TCPServer(const std::string& address, int port);
    
    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;
    
    TCPServer(TCPServer&& other) noexcept;
    TCPServer& operator=(TCPServer&& other) noexcept;
    
    ~TCPServer();

    void listen(int maxQueue);
    
    std::unique_ptr<TCPSocket> accept();
    
    void close();
    bool isOpen() const;

private:
    std::unique_ptr<I_TCPServer_impl> pimpl;
};
}
#endif // TCP_SERVER_H