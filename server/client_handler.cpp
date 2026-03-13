#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <set>

#include "tcp/tcp.h"
#include "fifo/fifo.h"
#include "shmem/shmem.h"

#include "common.h"

using TCP::SocketTransferToken, TCP::TCPSocket, TCP::TCPClient, TCP::Initializer;

void Init() {
    Initializer::initialize();
    
    FifoServer init(FIFO_NAME INIT_POSTFIX);
    init.waitConnection();
    init.write("ack");
}

bool signatureIsInBuffer (
    const char* signature,
    size_t signatureSize,
    
    const char* bufferStart,
    const char* bufferEnd
) {
    for (const char* ptr = bufferStart; ptr + signatureSize <= bufferEnd; ptr++) {
        if (std::memcmp(signature, ptr, signatureSize) == 0) return true;
    }
    return false;
}

std::set<size_t> analyzeBuffer(
    const std::vector< SharedData* > &signatures,
    size_t maxSignSize,
    const std::string &buffer,
    bool first
) {
    std::set<size_t> result; 

    for (size_t i=0; i<signatures.size(); i++) {
        auto ptr = signatures[i];
        if (ptr->contentSize > buffer.size()) {
            continue;
        }
        const char  *bufferStart, *bufferEnd; 
        if (first) bufferStart = buffer.data();
        else bufferStart = buffer.data() + maxSignSize - ptr->contentSize;

        bufferEnd = buffer.data() + buffer.size();

        const char* sign = ptr->as<const char>();

        if (signatureIsInBuffer(sign, ptr->contentSize, bufferStart, bufferEnd)) result.insert(i);
    }

    return result;
}

// indexes of threats
std::set<size_t> analyzeStream(
    std::unique_ptr<TCPSocket> &clientConn,
    size_t filesize,
    const std::vector< SharedData* > &signatures,
    size_t maxSignSize
) {
    std::set<size_t> threatIndexes;

    bool first = true;
    size_t parts = 0;
    std::string analyzedBuffer(2*maxSignSize, (char)0);

    size_t readSize = 0;
    std::string buffer;
    while (readSize < filesize) {
        std::string chunk = clientConn->receive(maxSignSize - buffer.size());
        
        readSize += chunk.size();
        buffer += chunk;
        if (buffer.size() == maxSignSize) {
            if (first) {
                std::memcpy(analyzedBuffer.data() + maxSignSize, buffer.data(), maxSignSize);
                first = false;
            } else {
                std::memcpy(analyzedBuffer.data(), analyzedBuffer.data() + maxSignSize, maxSignSize);
                std::memcpy(analyzedBuffer.data() + maxSignSize, buffer.data(), maxSignSize);

                auto threats = analyzeBuffer(signatures, maxSignSize, analyzedBuffer, parts==1);
                threatIndexes.insert(threats.begin(), threats.end());
            }
            parts++;
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        if (parts == 0) {
            // single chunk less than MSS
            std::string lastBuffer = buffer;
    
            auto threats = analyzeBuffer(signatures, maxSignSize, lastBuffer, true);
            threatIndexes.insert(threats.begin(), threats.end());
        } else {
            // tail
            std::string lastBuffer(analyzedBuffer.data() + maxSignSize, maxSignSize);
            lastBuffer += buffer;
    
            auto threats = analyzeBuffer(signatures, maxSignSize, lastBuffer, false);
            threatIndexes.insert(threats.begin(), threats.end());
        }
    }

    return threatIndexes;
}

void sendResults(
    std::unique_ptr<TCPSocket> &clientConn,
    std::set< size_t > threatIndexes,
    ThreatCommon* commons
) {
    std::stringstream result;
    if (threatIndexes.empty()) {
        result << R"({
            "dangerous" : false
        })";
    } else {
        result << R"({
            "dangerous" : true,
            "threats" : [ )";

        for (auto it = threatIndexes.begin(); it!=threatIndexes.end(); it++) {
            ThreatCommon &threat = commons[*it];
            result << "{";
            result << "\"name\" : \"";

            for (
                const char* ptr = threat.name; 
                ((size_t)(ptr - threat.name) < sizeof(threat.name)) && *ptr != '\0';
                ptr++
            ) {
                if (*ptr == '"') result << "\\\"";
                else result << *ptr;
            }
            result << "\" ," << std::endl;
            
            result << "\"description\" : \""; 
            for (
                const char* ptr = threat.description; 
                ((size_t)(ptr - threat.description) < sizeof(threat.description)) && *ptr != '\0';
                ptr++
            ) {
                if (*ptr == '"') result << "\\\"";
                else result << *ptr;
            }
            result << "\"" << std::endl;
            result << "}";
            if (std::next(it) != threatIndexes.end()) result << ",";
            result << std::endl;
        }
        result << "]}";
    }
    
    std::string resultStr = result.str();
    
    uint64_t resultSize = resultStr.size();
    std::string resultHeader((char*)&resultSize, sizeof(uint64_t));
    
    clientConn->send(resultHeader);
    clientConn->send(resultStr);
}

int main(int argc, char** argv) {
    (void)(argc);
    Init();
    // std::cout << "Inited" << std::endl;

    std::unique_ptr< FifoClient > fifo = connectFifo(FIFO_NAME, 10);
    if (!fifo) {
        throw std::runtime_error("Could not connect pipe");
    }

    std::string header = fifo->read(sizeof(size_t));
    if (header.size() != sizeof(size_t)) {
        throw std::runtime_error("FIFO: Expected header size " + std::to_string(sizeof(size_t)) + ", got " + std::to_string(header.size()));
    }
    size_t data_size;
    std::memcpy(&data_size, header.data(), sizeof(size_t));

    std::string data = fifo->read(data_size);
    if (data.size() != data_size) {
        throw std::runtime_error("FIFO: Expected data size " + std::to_string(data_size) + ", got " + std::to_string(data.size()));
    }
    // std::cout << "Fifoed" << std::endl;
    
    auto token = SocketTransferToken::fromData(data.data(), data.size());
    auto clientConn = TCPSocket::fromTransferToken(token);
    {
        FifoServer trasfer(FIFO_NAME TRANSFER_POSTFIX);
        trasfer.waitConnection();
        trasfer.write("ack");
    }
    // std::cout << "Socketed" << std::endl;

    std::string requestStr = clientConn->receive();
    // std::cout << requestStr << std::endl;
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
        return 0;
    }

    std::string reply = R"(
        {
            "status" : "accept"
        }
    )";
    clientConn->send(reply);


    std::string portstring = argv[1];
    int updaterPort;
    auto [ptr, ec] = std::from_chars(portstring.data(), portstring.data() + portstring.size(), updaterPort);
    
    if (ec == std::errc::invalid_argument) {
        std::cerr << "Port must be a number" << std::endl;
        return 1;
    } else if (ec == std::errc::result_out_of_range) {
        std::cerr << "Port number is too big" << std::endl;
        return 1;
    }

    SharedMemory shmem(SHMEM_NAME);

    ShmemNavigator *navigator = (ShmemNavigator *) shmem.get();

    std::vector< SharedData* > indexToPtr(navigator->count, nullptr);
    
    ThreatCommon* commons = (ThreatCommon*) navigator->common();
    SharedData* signatures = (SharedData*) navigator->signature();
    SharedData* current = signatures;
    size_t maxSignSize = 0;
    for (size_t i=0; i<navigator->count; i++) {
        indexToPtr[i] = current;
        if (current->contentSize > maxSignSize) {
            maxSignSize = current->contentSize;
        }
        current = current->next();
    }

    auto threatIndexes = analyzeStream(clientConn, filesize, indexToPtr, maxSignSize);
    
    sendResults(clientConn, threatIndexes, commons);    

    // update statistics

    std::stringstream updateRequest;
    updateRequest << "{ \"indexes\" : [";
    for (auto it=threatIndexes.begin(); it != threatIndexes.end(); it++) {
        size_t index = *it;
        
        ThreatCommon &threat = commons[index];    
        threat.mutex.lock();
        threat.occured++;
        threat.mutex.unlock();

        updateRequest << index;
        if (std::next(it) != threatIndexes.end()) updateRequest << ", ";
    }
    updateRequest << "] }";
    std::string updateStr = updateRequest.str();
    size_t updateSize = updateStr.size();
    std::string updateHeader((char*)&updateSize, sizeof(size_t));

    TCPClient conn;
    conn.connect("127.0.0.1", updaterPort);
    conn.send(updateHeader);
    conn.send(updateStr);

    return 0;
}