#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include <cstdint>
#include <string>
#include <vector>

// to be used in shared memory
struct SharedData {
    size_t totalSize;
    size_t contentSize;
    char content[1];
    
    template<typename T>
    T* as() const { 
        return (T*)(content); 
    }

    SharedData* next() const {
        return (SharedData*)((void*)this + totalSize);
    }
    
    // returns adress AFTER emplaced object
    static void* emplace(void* dest, const std::string& data) {
        SharedData* sd = (SharedData*) dest;
        
        if (data.size() <= sizeof(SharedData) - 2*sizeof(size_t)) {
            sd->totalSize = sizeof(SharedData);
        } else {
            sd->totalSize = 2*sizeof(size_t) + data.size();
        }
        sd->contentSize = data.size();
        
        for (int i=0; i<sd->contentSize; i++) {
            sd->content[i] = data[i];
        }
        
        return sd->next();
    }

    // returns adress AFTER emplaced array
    static void* emplace(void* dest, const std::vector< std::string > &datas) {
        for (auto &data: datas) {
            dest = emplace(dest, data);
        }
        return dest;
    }
};


#endif // SHARED_DATA_HPP