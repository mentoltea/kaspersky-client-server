#ifndef TCP_INTERFACE_H
#define TCP_INTERFACE_H

#include <string>
#include <memory>

class TCPSocket;

class I_TCPSocket_impl {
public:
    virtual ~I_TCPSocket_impl() = default;
    
    virtual void send(const std::string& data) = 0;
    virtual std::string receive(size_t bufferSize = 1024) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

class I_TCPServer_impl : public I_TCPSocket_impl {
public:
    virtual void bind(const std::string& address, int port) = 0;
    virtual void listen(int maxQueue = 5) = 0;
    virtual std::unique_ptr<I_TCPSocket_impl> accept() = 0;
};

class I_TCPClient_impl : public I_TCPSocket_impl {
public:
    virtual void connect(const std::string& address, int port) = 0;
};

#endif // TCP_INTERFACE_H