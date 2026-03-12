#include "manager.h"

#include <windows.h>
#include <processthreadsapi.h>
#include <sstream>

uint64_t ProcessManager::create(const std::string &path, const std::vector<std::string> &args) {
    std::string real_path = path;
    
    if (path.find(".exe") == std::string::npos) {
        real_path += ".exe";    
    }

    std::stringstream buffer;
    for (auto &arg: args) {
        buffer << arg << " ";
    }

    STARTUPINFOA siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    
    if (CreateProcessA(
        real_path.c_str(), 
        buffer.str().data(), 
        NULL, 
        NULL, 
        TRUE, 
        0, 
        NULL, 
        NULL, 
        &siStartInfo, 
        &piProcInfo
    ) == false) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Process creation failed. Error code: " << error;
        throw std::runtime_error(ss.str());
    }

    HANDLE hProcess = piProcInfo.hProcess;

    return (uint64_t) hProcess;
}

void ProcessManager::wait(uint64_t process) {
    HANDLE hProcess = (HANDLE) process;
    DWORD result;
    result = WaitForSingleObject(hProcess, INFINITE);
    switch (result) {
    case WAIT_OBJECT_0:
        return ;
    case WAIT_TIMEOUT:
    case WAIT_ABANDONED:
    case WAIT_FAILED:
    default:
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Process wait failed. Error code: " << error;
        throw std::runtime_error(ss.str());
    }
}

ProcessManager::WaitResult ProcessManager::wait(uint64_t process, int durationMs) {
    HANDLE hProcess = (HANDLE) process;
    DWORD result;
    result = WaitForSingleObject(hProcess, durationMs);
    switch (result) {
    case WAIT_OBJECT_0:
        return WaitResult::FINISHED;
    case WAIT_TIMEOUT:
        return WaitResult::TIMEOUT;

    case WAIT_ABANDONED:
    case WAIT_FAILED:
    default:
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Process wait failed. Error code: " << error;
        throw std::runtime_error(ss.str());
    }
}

int ProcessManager::returnCode(uint64_t process) {
    HANDLE hProcess = (HANDLE) process;
    DWORD result;
    if (GetExitCodeProcess(hProcess, &result) == false) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Process return code retrival failed. Error code: " << error;
        throw std::runtime_error(ss.str());
    }
    return result;
}

void ProcessManager::kill(uint64_t process) {
    HANDLE hProcess = (HANDLE) process;
    if (TerminateProcess(hProcess, 1) == false) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Process termination failed. Error code: " << error;
        throw std::runtime_error(ss.str());
    }
}

pid_t ProcessManager::pid(uint64_t process) {
    HANDLE hProcess = (HANDLE) process;
    DWORD pid = GetProcessId(hProcess);
    if (pid == 0) {
        DWORD err = GetLastError();
        throw std::runtime_error("GetProcessId failed: " + std::to_string(err));
    }
    return pid;
}