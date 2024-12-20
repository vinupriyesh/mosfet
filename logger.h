// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance("application.log");
        return instance;
    }

    void log(const std::string& message) {
        if (log_file.is_open()) {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            log_file << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
        }
    }

private:
    std::ofstream log_file;

    Logger(const std::string& filename) {
        log_file.open(filename, std::ios::out | std::ios::app);
    }

    ~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H
