#include "impl_windows.h"

WinSharedMemoryImpl::WinSharedMemoryImpl(const std::string& name, size_t size) 
    : name(name)
    , size_(size)
    , data_(nullptr)
    , fileMapping(nullptr) 
{
    fileMapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        static_cast<DWORD>(size >> 32),
        static_cast<DWORD>(size & 0xFFFFFFFF),
        name.c_str()
    );
    
    if (!fileMapping) {
        throwLastError("CreateFileMapping");
    }
    
    DWORD lastError = GetLastError();
    if (lastError == ERROR_ALREADY_EXISTS) {
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throw std::runtime_error("Shared memory object already exists: " + name);
    }
    
    data_ = MapViewOfFile(
        fileMapping,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        size
    );
    
    if (!data_) {
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throwLastError("MapViewOfFile");
    }
    
    memset(data_, 0, size);
}

WinSharedMemoryImpl::WinSharedMemoryImpl(const std::string& name) 
    : name(name)
    , size_(0)
    , data_(nullptr)
    , fileMapping(nullptr) 
{
    fileMapping = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS, 
        FALSE,
        name.c_str()
    );
    
    if (!fileMapping) {
        throwLastError("OpenFileMapping");
    }
    
    BY_HANDLE_FILE_INFORMATION fileInfo;
    if (!GetFileInformationByHandle(fileMapping, &fileInfo)) {
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throwLastError("GetFileInformationByHandle");
    }
    
    // размер хранится в двух DWORD
    size_ = (static_cast<size_t>(fileInfo.nFileSizeHigh) << 32) | 
             static_cast<size_t>(fileInfo.nFileSizeLow);
    
    data_ = MapViewOfFile(
        fileMapping,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        size_
    );
    
    if (!data_) {
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throwLastError("MapViewOfFile");
    }
}

WinSharedMemoryImpl::~WinSharedMemoryImpl() {
    if (data_) {
        UnmapViewOfFile(data_);
    }
    if (fileMapping) {
        CloseHandle(fileMapping);
    }
}

WinSharedMemoryImpl::WinSharedMemoryImpl(WinSharedMemoryImpl&& other) noexcept
    : name(std::move(other.name))
    , size_(other.size_)
    , data_(other.data_)
    , fileMapping(other.fileMapping) {
    
    other.data_ = nullptr;
    other.fileMapping = nullptr;
    other.size_ = 0;
}

WinSharedMemoryImpl& WinSharedMemoryImpl::operator=(WinSharedMemoryImpl&& other) noexcept {
    if (this != &other) {
        name = std::move(other.name);
        size_ = other.size_;
        data_ = other.data_;
        fileMapping = other.fileMapping;
        
        other.data_ = nullptr;
        other.fileMapping = nullptr;
        other.size_ = 0;
    }
    return *this;
}

void WinSharedMemoryImpl::throwLastError(const std::string& operation) {
    std::stringstream ss;
    ss << operation << " failed. Error code: " << GetLastError();
    throw std::runtime_error(ss.str());
}