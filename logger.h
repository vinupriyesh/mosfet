// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <chrono>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setPlayerName(const std::string& name) {
        player_name = name;
    }

    void setStepId(const std::string& id) {
        step_id = id;
    }

    void enableLogging(const std::string& filename) {
        if (!log_file.is_open()) {
            log_file.open(filename, std::ios::out | std::ios::app);
        }
    }

    bool isDebugEnabled() {
        return log_file.is_open();
    }

    void log(const std::string& message) {
        if (log_file.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            std::tm tm = *std::localtime(&in_time_t);
            log_file << std::put_time(&tm, "%M:%S")
                     << '.' << std::setfill('0') << std::setw(3) << ms.count()
                     << " : " << step_id
                     << " : " << player_name << " - " << message << std::endl;
        }
    }

private:
    std::string step_id = "init";
    std::string player_name = "Unknown";
    std::ofstream log_file;

    Logger() = default;
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
