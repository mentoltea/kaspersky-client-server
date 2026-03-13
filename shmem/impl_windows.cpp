#include "impl_windows.h"

WinSharedMemoryImpl::WinSharedMemoryImpl(const std::string& name, size_t size) 
    : name(name)
    , size_(size)
    , data_(nullptr)
    , fileMapping(nullptr) 
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    fileMapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        &sa,
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
    HANDLE fileMapping = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS, 
        TRUE,
        name.c_str()
    );

    if (!fileMapping) {
        throwLastError("OpenFileMapping");
    }

    data_ = MapViewOfFile(
        fileMapping,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        0
    );

    if (!data_) {
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throwLastError("MapViewOfFile");
    }

    MEMORY_BASIC_INFORMATION memInfo;
    memset(&memInfo, 0, sizeof(memInfo));
    SIZE_T result = VirtualQuery(data_, &memInfo, sizeof(memInfo));

    if (result == 0) {
        UnmapViewOfFile(data_);
        data_ = nullptr;
        CloseHandle(fileMapping);
        fileMapping = nullptr;
        throwLastError("VirtualQuery");
    }

    size_ = memInfo.RegionSize;
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