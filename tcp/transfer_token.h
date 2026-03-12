#ifndef SOCKET_TRANSFER_TOKEN_H
#define SOCKET_TRANSFER_TOKEN_H

#include <memory>
#include <cstddef>

namespace TCP {
class SocketTransferToken {
public:
    SocketTransferToken();
    ~SocketTransferToken();
    
    SocketTransferToken(const SocketTransferToken&) = delete;
    SocketTransferToken& operator=(const SocketTransferToken&) = delete;
    
    SocketTransferToken(SocketTransferToken&& other) noexcept;
    SocketTransferToken& operator=(SocketTransferToken&& other) noexcept;
    
    const void* data() const;
    size_t size() const;
    
    static SocketTransferToken fromData(const void* data, size_t size);
    
    bool isValid() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
    
    explicit SocketTransferToken(std::unique_ptr<Impl>&& impl);
};
}
#endif // SOCKET_TRANSFER_TOKEN_H