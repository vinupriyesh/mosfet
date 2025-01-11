// metrics.h
#ifndef METRICS_H
#define METRICS_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

class Metrics {
public:
    static Metrics& getInstance() {
        static Metrics instance;
        return instance;
    }

    void setPlayerName(const std::string& name) {
        player_name = name;
    }

    void setStepId(const std::string& id) {
        step_id = id;
    }

    void enableMetrics(const std::string& filename) {
        if (!log_file.is_open()) {
            log_file.open(filename, std::ios::out | std::ios::app);
            log_file << "timestep,player_id,dimension,value" << std::endl;
        }
    }

    bool isMetricEnabled() {
        return log_file.is_open();
    }

    void add(const std::string& name, const float value){
        if (log_file.is_open()) {
            log_file << step_id << "," << player_name << "," << name << "," << value << std::endl;
        }
    }

private:
    std::string step_id = "-1";
    std::string player_name = "Unknown";
    std::ofstream log_file;

    Metrics() = default;
    ~Metrics() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    // Delete copy constructor and assignment operator
    Metrics(const Metrics&) = delete;
    Metrics& operator=(const Metrics&) = delete;
};

#endif // METRICS_H
