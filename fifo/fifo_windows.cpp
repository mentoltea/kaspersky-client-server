#include "fifo.h"

#include "fifo.h"
#include <stdexcept>
#include <iostream>


FifoServer::FifoServer(const std::string& path) 
    : hPipe(INVALID_HANDLE_VALUE) {
    // должен начинаться с \\.\pipe
    std::string fullPath = path;
    if (fullPath.find("\\\\.\\pipe\\") != 0) {
        fullPath = "\\\\.\\pipe\\" + fullPath;
    }
    this->path = fullPath;
}

FifoServer::~FifoServer() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
}

void FifoServer::waitConnection() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
    }

    hPipe = CreateNamedPipeA(
        path.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create named pipe. Error: " + std::to_string(GetLastError()));
    }

    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    
    if (!connected) {
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
        throw std::runtime_error("Failed to connect pipe. Error: " + std::to_string(GetLastError()));
    }
}

void FifoServer::write(const std::string& data) {
    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("No client connected. Call waitConnection() first");
    }

    DWORD bytesWritten;
    BOOL result = WriteFile(
        hPipe,
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        NULL
    );

    if (!result || bytesWritten != data.size()) {
        throw std::runtime_error("Failed to write to pipe. Error: " + std::to_string(GetLastError()));
    }
}





FifoClient::FifoClient(const std::string& path) 
    : hPipe(INVALID_HANDLE_VALUE) {
    // должен начинаться с \\.\pipe
    std::string fullPath = path;
    if (fullPath.find("\\\\.\\pipe\\") != 0) {
        fullPath = "\\\\.\\pipe\\" + fullPath;
    }
    this->path = fullPath;

    hPipe = CreateFileA(
        this->path.c_str(),
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
        throw std::runtime_error("Failed to connect to pipe. Error: " + std::to_string(error));
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
        throw std::runtime_error("Failed to set pipe mode. Error: " + std::to_string(GetLastError()));
    }
}

FifoClient::~FifoClient() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }
}

std::string FifoClient::read(size_t bufferSize) {
    if (hPipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Not connected to server");
    }

    std::string buffer(bufferSize, '\0');
    DWORD bytesRead;

    BOOL result = ReadFile(
        hPipe,
        &buffer[0],
        static_cast<DWORD>(bufferSize),
        &bytesRead,
        NULL
    );

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return "";
        }
        throw std::runtime_error("Failed to read from pipe. Error: " + std::to_string(error));
    }

    return std::string(buffer.data(), bytesRead);
}