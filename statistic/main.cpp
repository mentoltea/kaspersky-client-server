#include "server/statistic.h"
#include "fifo/fifo.h"

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <cstring>
#include <cstdlib>

int main() {
    FifoClient fifo( STAT_FIFO_NAME );

    std::string header = fifo.read(sizeof(size_t));
    size_t size;
    std::memcpy((char*)&size, header.data(), sizeof(size_t));

    std::string bodyStr = fifo.read(size);

    {
        FifoServer acc(STAT_FIFO_NAME ACC_POSTFIX);
        acc.waitConnection();
    }

    auto body = json::parse(bodyStr).get< std::vector<json> >();

    if (body.empty()) {
        std::cout << "No statistic" << std::endl;
        return 0;
    }

    size_t total_occured = 0;
    for (auto &threat: body) {
        std::string name = threat["name"].get< std::string >();
        size_t occured = threat["occured"].get< size_t >();
        total_occured += occured;
        
        std::cout << "Name : " << name << std::endl;
        std::cout << "Occured: " << occured << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Total occured threats: " << total_occured << std::endl;

    return 0;
}