#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <charconv>

#include "tcp/tcp.h"
#include "fifo/fifo.h"

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

#define PROTOCOL_VERSION 1

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Too few arguments" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\tclient [path] [port]" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string portstring = argv[2];

    int port;
    auto [ptr, ec] = std::from_chars(portstring.data(), portstring.data() + portstring.size(), port);

    if (ec == std::errc::invalid_argument) {
        std::cerr << "Port must be a number" << std::endl;
        return 1;
    } else if (ec == std::errc::result_out_of_range) {
        std::cerr << "Port number is too big" << std::endl;
        return 1;
    }

    TCPServer conn("0.0.0.0", port);
    conn.listen(10);

    while (true) {
        try {
            auto clientConn = conn.accept();

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
                std::string chunk = clientConn->receive();
                readSize += chunk.size();
                std::cout << readSize << "/" << filesize << std::endl;
            }

            std::string result = R"(
                {
                    "dangerous" : false,
                    "description" : "No real check, just testing"
                }
            )";
            clientConn->send(result);
        } catch (std::exception e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    return 0;
}