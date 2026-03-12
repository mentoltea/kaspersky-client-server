#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <charconv>

#include "tcp/tcp.h"
#include "fifo/fifo.h"
#include "process/manager.h"

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

#include "common.h"

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
            
            auto pid = ProcessManager::create("./client_handler", {"./client_handler"});
            auto transferToken = clientConn->prepareForTransfer(pid);

            FifoServer fifo(FIFO_NAME);

            size_t data_size = transferToken.size();
            std::string header((char*) &data_size, sizeof(size_t));
            std::string data((char*) transferToken.data(), data_size);

            fifo.waitConnection();
            fifo.write(header);
            fifo.write(data);
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    return 0;
}