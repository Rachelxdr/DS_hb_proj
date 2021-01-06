#include "../header_files/Logger.h"

Logger::Logger(string file_name) {
    log_file_name = file_name;
}


void Logger::log(string message) {
    FILE* fd = fopen(this->log_file_name.c_str(), "a+");
    fprintf(fd,"%s\n", message.c_str());
    fclose(fd);
}