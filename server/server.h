#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "statistic.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <limits>

#include <charconv>

#include "tcp/tcp.h"
#include "fifo/fifo.h"
#include "process/manager.h"
#include "shmem/shmem.h"

void listenStatistic( ShmemNavigator*  navigator);

void updateConf(ShmemNavigator *navigator, std::string filepath, json & conf, int port);

#endif // SERVER_H