#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

class Logger {
    public:
        string log_file_name;
        ofstream log_file_stream;
        Logger(string file_name);
        void log(string message);
};


#endif
