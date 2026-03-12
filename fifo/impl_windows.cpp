#include "impl_windows.h"
#include <vector>
#include <sstream>

WinFifoServerImpl::WinFifoServerImpl(const std::string& path) 
    : hPipe(INVALID_HANDLE_VALUE) {
    
    if (path.find("\\\\.\\pipe\\") == 0) {
        pipePath = path;
    } else {
        pipePath = "\\\\.\\pipe\\" + path;
    }
    
    hPipe = CreateNamedPipeA(
        pipePath.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE |
        PIPE_READMODE_BYTE | 
        PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        4096,
        4096,
        0,
        NULL
    );
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        throwLastError("CreateNamedPipe");
    }
}

WinFifoServerImpl::~WinFifoServerImpl() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
}

WinFifoServerImpl::WinFifoServerImpl(WinFifoServerImpl&& other) noexcept
    : pipePath(std::move(other.pipePath))
    , hPipe(other.hPipe) {
    other.hPipe = INVALID_HANDLE_VALUE;
}

WinFifoServerImpl& WinFifoServerImpl::operator=(WinFifoServerImpl&& other) noexcept {
    if (this != &other) {
        if (hPipe != INVALID_HANDLE_VALUE) {
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
        
        pipePath = std::move(other.pipePath);
        hPipe = other.hPipe;
        
        other.hPipe = INVALID_HANDLE_VALUE;
    }
    return *this;
}

void WinFifoServerImpl::waitConnection() {
    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Pipe not created");
    }
    
    DisconnectNamedPipe(hPipe);
    
    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    
    if (!connected) {
        throwLastError("ConnectNamedPipe");
    }
}

void WinFifoServerImpl::write(const std::string& data) {
    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("No client connected");
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(
        hPipe,
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_NO_DATA || error == ERROR_BROKEN_PIPE) {
            throw std::runtime_error("Client disconnected");
        }
        throwLastError("WriteFile");
    }
    
    if (bytesWritten != data.size()) {
        throw std::runtime_error("Incomplete write to pipe");
    }
    
    FlushFileBuffers(hPipe);
}

void WinFifoServerImpl::throwLastError(const std::string& operation) {
    DWORD error = GetLastError();
    std::stringstream ss;
    ss << operation << " failed. Error code: " << error;
    throw std::runtime_error(ss.str());
}






WinFifoClientImpl::WinFifoClientImpl(const std::string& path) 
    : hPipe(INVALID_HANDLE_VALUE) {
    
    if (path.find("\\\\.\\pipe\\") == 0) {
        pipePath = path;
    } else {
        pipePath = "\\\\.\\pipe\\" + path;
    }
    
    hPipe = CreateFileA(
        pipePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("Server not found. Is server running?");
        }
        throwLastError("CreateFile");
    }
    
    DWORD mode = PIPE_READMODE_BYTE;
    BOOL success = SetNamedPipeHandleState(
        hPipe,
        &mode,
        NULL,
        NULL
    );
    
    if (!success) {
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
        throwLastError("SetNamedPipeHandleState");
    }
}

WinFifoClientImpl::~WinFifoClientImpl() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(hPipe);
        CloseHandle(hPipe);
    }
}

WinFifoClientImpl::WinFifoClientImpl(WinFifoClientImpl&& other) noexcept
    : pipePath(std::move(other.pipePath))
    , hPipe(other.hPipe) {
    other.hPipe = INVALID_HANDLE_VALUE;
}

WinFifoClientImpl& WinFifoClientImpl::operator=(WinFifoClientImpl&& other) noexcept {
    if (this != &other) {
        if (hPipe != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(hPipe);
            CloseHandle(hPipe);
        }
        
        pipePath = std::move(other.pipePath);
        hPipe = other.hPipe;
        
        other.hPipe = INVALID_HANDLE_VALUE;
    }
    return *this;
}

std::string WinFifoClientImpl::read(size_t bufferSize) {
    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Not connected to server");
    }
    
    std::vector<char> buffer(bufferSize);
    DWORD bytesRead = 0;
    
    BOOL result = ReadFile(
        hPipe,
        buffer.data(),
        static_cast<DWORD>(bufferSize),
        &bytesRead,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            throw std::runtime_error("Server closed connection");
        }
        throwLastError("ReadFile");
    }
    
    return std::string(buffer.data(), bytesRead);
}

void WinFifoClientImpl::throwLastError(const std::string& operation) {
    DWORD error = GetLastError();
    std::stringstream ss;
    ss << operation << " failed. Error code: " << error;
    throw std::runtime_error(ss.str());
}