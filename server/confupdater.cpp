#include "server.h"

using TCP::TCPServer;

void updateConf(ShmemNavigator *navigator, std::string filepath, json & conf, int port) {
    auto threats = conf["threats"].get< std::vector<json> >();

    TCPServer localserver("127.0.0.1", port);
    localserver.listen(32);

    while (true) {
        try {
            auto conn = localserver.accept();

            std::string requestHeader = conn->receive(sizeof(size_t));
            size_t requestSize;
            std::memcpy((char*)&requestSize, requestHeader.data(), sizeof(size_t));
            
            std::string requestStr = conn->receive(requestSize);
            json request = json::parse(requestStr);
            
            std::vector<size_t> indexes = request["indexes"].get< std::vector<size_t> >();

            ThreatCommon *common = (ThreatCommon*) navigator->common();

            for (auto index: indexes) {
                if (index >= navigator->count) {
                    throw std::runtime_error("Update index is out of range");
                }
                ThreatCommon& threat = common[index];
                
                threat.mutex.lock();
                threats[index]["occured"] = threat.occured;
                threat.mutex.unlock();
            }

            conf["threats"] = threats;
            
            {
                std::ofstream confFile(filepath);
                confFile << conf.dump(4);
            }

            try {
                conn->send("Saved");
            } catch (std::runtime_error &e) {}
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}