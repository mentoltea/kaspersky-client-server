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
                    statistic << "\""; 
                    for (
                        const char* ptr = threat.name; 
                        ((size_t)(ptr - threat.name) < sizeof(threat.name)) && *ptr != '\0';
                        ptr++
                    ) {
                        if (*ptr == '"') statistic << "\\\"";
                        else statistic << *ptr;
                    }
                    statistic << "\"," << std::endl;
                    
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
            
            size_t max_fail_count = 10;
            {
                auto acc = connectFifo(STAT_FIFO_NAME ACC_POSTFIX, max_fail_count);
                if (!acc) continue;
            }
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}