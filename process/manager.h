#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>
#include <thread>
#include <cstdint>

class ProcessManager {
public:
     static uint64_t create(const std::string &path, const std::vector<std::string> &args);

     static void wait(uint64_t process); 

     enum class WaitResult {
          FINISHED,
          TIMEOUT,
     };
     static WaitResult wait(uint64_t process, int durationMs); 

     static int returnCode(uint64_t process);
     
     static void kill(uint64_t process);

     static pid_t pid(uint64_t process);
};

#endif // PROCESS_H