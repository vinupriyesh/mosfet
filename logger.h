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
    static Logger& getInstance();

    void setPlayerName(const std::string& name);
    void setStepId(const std::string& id);
    void enableLogging(const std::string& filename);
    bool isDebugEnabled();
    void log(const std::string& message);

private:
    std::string step_id = "init";
    std::string player_name = "Unknown";
    std::ofstream log_file;

    Logger() = default;
    ~Logger();

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H
