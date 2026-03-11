#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <charconv>

#include "tcp/tcp.h"

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

    std::ifstream file(filepath, std::ios::binary);
    size_t filesize = std::filesystem::file_size(filepath);
    
    TCPClient conn;
    conn.connect("127.0.0.1", port);

    std::string request = R"(
        {
            "protocol": {
                "version": )" + std::to_string(PROTOCOL_VERSION) + R"(
            },
            "request": "check",
            "filesize": )" + std::to_string(filesize) + R"(
        }  
    )";

    conn.send(request);

    std::string replyStr = conn.receive();
    // std::cout << replyStr << std::endl;
    json reply = json::parse(replyStr);

    std::string reply_status = reply["status"].get<std::string>();

    if (reply_status == "reject") {
        std::cout << "Request was rejected by server" << std::endl;
        std::string reason = reply["reason"].get<std::string>();
        std::cout << "Reason: " << reason << std::endl;
        return 0;
    } else if (reply_status != "accept") {
        std::cerr << "Protocol violation from server: expected `reject` or `accept`, got `" + reply_status + "`" << std::endl;
        return 1;
    }

    size_t sentSize = 0;
    size_t chunkSize = 1024;
    std::string buffer(chunkSize, '\0');
    while (sentSize < filesize) {
        file.read(&buffer[0], chunkSize);
        size_t bytesRead = file.gcount();
        if (bytesRead == chunkSize) {
            conn.send(buffer);
        } else {
            std::string chunk = buffer.substr(0, bytesRead);
            conn.send(chunk);
        }
        sentSize += bytesRead;
        // std::cout << sentSize << "/" << filesize << std::endl;
    }

    std::cout << "File sent, waiting for results..." << std::endl;

    std::string resultStr = conn.receive(4096);
    // std::cout << resultStr << std::endl;
    json result = json::parse(resultStr);

    bool dangerous = result["dangerous"].get<bool>();
    if (dangerous) {
        std::cout << "File is dangerous" << std::endl;
    } else {
        std::cout << "File is safe" << std::endl;
    }
    
    if (result.contains("description")) {
        std::cout << "Description: " << result["description"] << std::endl;
    }

    return 0;
}