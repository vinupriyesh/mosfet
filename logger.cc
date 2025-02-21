#include "logger.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setPlayerName(const std::string& name) {
    player_name = name;
}

void Logger::setStepId(const std::string& id) {
    step_id = id;
}

void Logger::enableLogging(const std::string& filename) {
    if (!log_file.is_open()) {
        log_file.open(filename, std::ios::out | std::ios::app);
    }
}

bool Logger::isDebugEnabled() {
    return log_file.is_open();
}

void Logger::log(const std::string& message) {
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

Logger::~Logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}
