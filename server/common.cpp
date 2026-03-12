#include "common.h"

ShmemNavigator* ShmemNavigator::initialize(void *memory, const std::vector<json> &threats, const std::vector< std::string > &dataSignatures) {
    ShmemNavigator* navigator = (ShmemNavigator*) memory;

    navigator->count = threats.size();
    
    navigator->common_offset = sizeof(ShmemNavigator);
    ThreatCommon *commons = (ThreatCommon*) ((void*)navigator + navigator->common_offset);

    for (size_t i=0; i<navigator->count; i++) {
        auto &threat = threats[i];
        ThreatCommon &common = commons[i];

        std::string name = threat["name"].get<std::string>();
        size_t nameSize = name.size() > sizeof(common.name) - 1 ? sizeof(common.name) - 1 : name.size();
        memcpy(common.name, name.c_str(), nameSize);
        common.name[nameSize] = '\0';
        
        std::string desc = threat["description"].get<std::string>();
        size_t descSize = desc.size() > sizeof(common.description) - 1 ? sizeof(common.description) - 1 : desc.size();
        memcpy(common.description, desc.c_str(), descSize);
        common.description[descSize] = '\0';

        common.occured = threat["occured"].get<size_t>();

        new(&common.mutex) SharedMutex;
    }

    size_t common_size = navigator->count * sizeof(ThreatCommon);
    size_t aligned_common_size = (common_size + alignof(SharedData) - 1) & ~(alignof(SharedData) - 1);

    navigator->signature_offset = navigator->common_offset + aligned_common_size;
    SharedData *signatures = (SharedData*) ((void*)navigator + navigator->signature_offset);
    SharedData::emplace(signatures, dataSignatures);

    return navigator;
}

void* ShmemNavigator::common() const {
    return (void*)this + common_offset;
}
void* ShmemNavigator::signature() const {
    return (void*)this + signature_offset;
}