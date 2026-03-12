#include "transfer_token.h"
#include <vector>
#include <algorithm>
#include <cstring>
using namespace TCP;
struct SocketTransferToken::Impl {
    std::vector<char> data;
    
    Impl() = default;
    
    explicit Impl(const void* src, size_t size)
        : data(static_cast<const char*>(src), 
               static_cast<const char*>(src) + size) {}
    
    explicit Impl(std::vector<char>&& vec) : data(std::move(vec)) {}
    
    const void* getData() const { return data.data(); }
    size_t getSize() const { return data.size(); }
};



SocketTransferToken::SocketTransferToken() 
    : pimpl(nullptr) {}

SocketTransferToken::SocketTransferToken(std::unique_ptr<Impl>&& impl)
    : pimpl(std::move(impl)) {}

SocketTransferToken::~SocketTransferToken() = default;

SocketTransferToken::SocketTransferToken(SocketTransferToken&& other) noexcept
    : pimpl(std::move(other.pimpl)) {}

SocketTransferToken& SocketTransferToken::operator=(SocketTransferToken&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

const void* SocketTransferToken::data() const {
    return pimpl ? pimpl->getData() : nullptr;
}

size_t SocketTransferToken::size() const {
    return pimpl ? pimpl->getSize() : 0;
}

bool SocketTransferToken::isValid() const {
    return pimpl != nullptr && pimpl->getSize() > 0;
}

SocketTransferToken SocketTransferToken::fromData(const void* data, size_t size) {
    if (!data || size == 0) {
        return SocketTransferToken();
    }
    auto impl = std::make_unique<Impl>(data, size);
    return SocketTransferToken(std::move(impl));
}