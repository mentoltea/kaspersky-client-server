#include <iostream>
#include <fstream>
#include <string>

#include <charconv>

#include "tcp/tcp.h"

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

    TCP::TCPServer("0.0.0.0", 443);

    return 0;
}