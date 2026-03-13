#include "server.h"



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
    // std::cout << "programm_path: " << programm_path << std::endl;
    // std::cout << "programm_directory: " << programm_directory << std::endl;
    
    std::string filepath = argv[1];
    std::string portstring = argv[2];

    json conf;
    {
        std::ifstream confFile(filepath);
        conf = json::parse(confFile);
    }
    
    auto threats = conf["threats"].get< std::vector<json> >();

    size_t signatureTotalSize = 0; 
    std::vector< std::string > dataSignatures;
    for (auto &threat: threats) {
        threat["name"].get<std::string>();
        threat["description"].get<std::string>();
        threat["occured"].get<size_t>();
        std::vector< int > signature = threat["signature"].get< std::vector< int > >();
        
        std::string data(signature.size(), (char)0);
        for (size_t i=0; i<signature.size(); i++) {
            int sign = signature[i];
            if (sign < 0 || sign > 255) {
                throw std::runtime_error("Signature " + std::to_string(sign) + " is outside char limits");
            }
            data[i] = static_cast<char>(static_cast<unsigned char>(sign));
        }
        dataSignatures.push_back(data);

        signatureTotalSize += SharedData::computeSharedSize(data);
    }

    size_t shmem_size = sizeof(ShmemNavigator) + threats.size() * sizeof(ThreatCommon) + signatureTotalSize;

    SharedMemory shmem(SHMEM_NAME, shmem_size);

    ShmemNavigator* navigator = ShmemNavigator::initialize(shmem.get(), threats, dataSignatures);


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
    std::cout << "Protocol version " << PROTOCOL_VERSION << std::endl;
    std::cout << "Listening on port " << port << std::endl;
    

    std::jthread statListener(listenStatistic, navigator);
    int updaterPort = port+1;
    std::jthread confUpdater(updateConf, navigator, filepath, std::ref(conf), updaterPort);
    
    
    size_t count = 1;
    while (true) {
        try {
            // std::cout << "Waiting new connection..." << std::endl;
            auto clientConn = conn.accept();

            
            std::string handler_path = programm_directory + "handler";
            
            auto process = ProcessManager::create(
                handler_path, 
                
                {
                    handler_path, 
                    std::to_string(updaterPort)
                }
            );
            std::cout << "Client " << count << std::endl;
            count++;

            auto pid = ProcessManager::pid(process);

            size_t max_fail_count = 10;
            {
                auto init = connectFifo(FIFO_NAME INIT_POSTFIX, max_fail_count);
                if (!init) {
                    ProcessManager::kill(process);
                    std::cerr << "Subprocess could not initialize in " << max_fail_count << " attempts" << std::endl;
                    continue;
                }
                init->read(4);
            }

            // std::cout << "Init confirmed" << std::endl;

            auto transferToken = clientConn->prepareForTransfer(pid);

            size_t data_size = transferToken.size();
            std::string header((char*) &data_size, sizeof(size_t));
            std::string data((char*) transferToken.data(), data_size);

            FifoServer fifo(FIFO_NAME);
            fifo.waitConnection();
            fifo.write(header);
            fifo.write(data);

            {
                auto transfer = connectFifo(FIFO_NAME TRANSFER_POSTFIX, max_fail_count);
                if (!transfer) {
                    ProcessManager::kill(process);
                    std::cerr << "Subprocess could not finish client socket transfer in " << max_fail_count << " attempts" << std::endl;
                }
                transfer->read(4);
            }
            // std::cout << "Transfer confirmed" << std::endl;

        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    return 0;
}