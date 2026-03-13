#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

// to be used in shared memory
struct SharedData {
    size_t totalSize;
    size_t contentSize;
    
    template<typename T>
    T* as() const { 
        char* content = (char*)(this + 1);
        return (T*)(content); 
    }

    SharedData* next() const {
        return (SharedData*)((char*)this + totalSize);
    }
    
    // returns adress AFTER emplaced object
    static void* emplace(void* dest, const std::string& data) {
        SharedData* sd = (SharedData*) dest;
        char* content = (char*)(sd+1);
        
        sd->contentSize = data.size();
        std::memcpy(content, data.data(), data.size());
        
        size_t rawSize = sizeof(SharedData) + data.size();
        size_t alignedSize = (rawSize + alignof(SharedData) - 1) & ~(alignof(SharedData) - 1);
        sd->totalSize = alignedSize;
        
        return sd->next();
    }

    // returns adress AFTER emplaced array
    static void* emplace(void* dest, const std::vector< std::string > &datas) {
        for (auto &data: datas) {
            dest = emplace(dest, data);
        }
        return dest;
    }

    static size_t computeSharedSize(const std::string& data) {
        size_t rawSize = sizeof(SharedData) + data.size();
        size_t alignedSize = (rawSize + alignof(SharedData) - 1) & ~(alignof(SharedData) - 1);
        return alignedSize;
    }
};


#endif // SHARED_DATA_HPP