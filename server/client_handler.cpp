#include <iostream>
#include <string>

#include "tcp/tcp.h"
#include "fifo/fifo.h"

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

#include "common.h"

int main() {
    FifoClient fifo(FIFO_NAME);

    std::string header = fifo.read(sizeof(size_t));
    if (header.size() != sizeof(size_t)) {
        throw std::runtime_error("FIFO: Expected header size" + std::to_string(sizeof(size_t)) + ", got " + std::to_string(header.size()));
    }
    size_t data_size;
    std::memcpy(&data_size, header.data(), sizeof(size_t));

    std::string data = fifo.read(data_size);
    if (data.size() != data_size) {
        throw std::runtime_error("FIFO: Expected data size" + std::to_string(data_size) + ", got " + std::to_string(data.size()));
    }

    auto token = SocketTransferToken::fromData(data.data(), data.size());
    auto clientConn = TCPSocket::fromTransferToken(token);

    std::string requestStr = clientConn->receive();
    std::cout << requestStr << std::endl;
    json request = json::parse(requestStr);
    
    json protocol = request["protocol"];
    int version = protocol["version"].get<int>();

    size_t filesize = request["filesize"].get<size_t>();

    if (PROTOCOL_VERSION != version) {
        std::string reply = R"(
        {
            "status" : "reject",
            "reason" : "Different versions. )" + std::to_string(PROTOCOL_VERSION) + R"( (server) vs )" + std::to_string(version) + R"( (client) "
        }
        )";
        clientConn->send(reply);
    }

    std::string reply = R"(
        {
            "status" : "accept"
        }
    )";
    clientConn->send(reply);

    size_t readSize = 0;
    size_t chunkSize = 1024;
    std::string buffer;
    while (readSize < filesize) {
        std::string chunk = clientConn->receive(chunkSize);
        readSize += chunk.size();
        std::cout << readSize << "/" << filesize << std::endl;
    }

    std::cout << buffer << std::endl;

    std::string result = R"(
        {
            "dangerous" : false,
            "description" : "No real check, just testing"
        }
    )";
    clientConn->send(result);


    return 0;
}