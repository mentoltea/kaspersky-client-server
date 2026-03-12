#include "server.h"

void listenStatistic(ShmemNavigator *navigator) {
    FifoServer fifo(STAT_FIFO_NAME);
    while (true) {
        try {
            fifo.waitConnection();

            size_t count = navigator->count;
            ThreatCommon* common = (ThreatCommon*) navigator->common();
            
            std::stringstream statistic;
            statistic << "[" << std::endl;
            for (size_t i=0; i<count; i++) {
                ThreatCommon &threat = common[i];
                statistic << "{" << std::endl;
                
                
                    statistic << "\"name\" : ";
                    statistic << "\"" << threat.name << "\"," << std::endl;
                    
                    statistic << "\"occured\" : ";
                    
                    threat.mutex.lock();
                    statistic << threat.occured << std::endl;
                    threat.mutex.unlock();
                
                statistic << "}";
                if (i != count-1) statistic << ",";
                statistic << std::endl;
            }
            statistic << "]";

            std::string data = statistic.str();
            
            size_t size = data.size();
            std::string header((char*)&size, sizeof(size_t));

            fifo.write(header);
            fifo.write(data);
            
            bool accepted = false;
            size_t fail_count = 0;
            while (!accepted) {
                try {
                    FifoClient acc(STAT_FIFO_NAME ACC_POSTFIX);
                    accepted = true;
                } catch (std::runtime_error &e) {
                    fail_count++;
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(10)
                    );
                }
                if (fail_count >= 10) {
                    break;
                }
            }
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}