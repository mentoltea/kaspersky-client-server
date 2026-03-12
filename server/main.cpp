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

using TCP::Initializer, TCP::TCPServer;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Too few arguments" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\tclient [path] [port]" << std::endl;
        return 1;
    }

    std::string programm_path = std::string(argv[0]);
    std::string programm_directory;
    for (int i=programm_path.size()-1; i>=0; i--) {
        if (programm_path[i] == '/' || programm_path[i] == '\\') {
            programm_directory = programm_path.substr(0, i+1);
            break;
        }
    } 
    std::cout << "programm_path: " << programm_path << std::endl;
    std::cout << "programm_directory: " << programm_directory << std::endl;
    
    std::string filepath = argv[1];
    std::string portstring = argv[2];

    Initializer::initialize();

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
    std::cout << "Listening on port " << port << std::endl;
    
    size_t count = 1;
    while (true) {
        try {
            auto clientConn = conn.accept();

            
            std::string handler_path = programm_directory + "handler";
            
            auto process = ProcessManager::create(
                handler_path, 
                
                {
                    handler_path, 
                    FIFO_NAME
                }
            );
            std::cout << "Client " << count << std::endl;
            count++;

            auto pid = ProcessManager::pid(process);

            bool initialized = false;
            size_t fail_count = 0;
            {
                while (!initialized) {
                    try {
                        FifoClient init(FIFO_NAME INIT_POSTFIX);
                        init.read(4);
                        initialized = true;
                    } catch (std::runtime_error &e) {
                        fail_count++;
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(10)
                        );
                    }

                    if (fail_count >= 10) {
                        ProcessManager::kill(process);
                        std::cerr << "Subprocess could not initialize in " << fail_count << " attempts" << std::endl;
                        break;
                    }
                }

                // std::cout << "Initialization approved" << std::endl;
            }
            if (!initialized) continue;
            std::cout << "Subprocess initialized in " << fail_count << " attempts" << std::endl;

            auto transferToken = clientConn->prepareForTransfer(pid);

            size_t data_size = transferToken.size();
            std::string header((char*) &data_size, sizeof(size_t));
            std::string data((char*) transferToken.data(), data_size);

            FifoServer fifo(FIFO_NAME);
            fifo.waitConnection();
            fifo.write(header);
            fifo.write(data);
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    return 0;
}