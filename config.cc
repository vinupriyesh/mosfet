#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool Config::enableLogging = false;
bool Config::enableMetrics = false;
bool Config::livePlayPlayer0 = false;
bool Config::livePlayPlayer1 = false;
bool Config::recordPlayer0 = false;
bool Config::recordPlayer1 = false;
int Config::portPlayer0 = 0;
int Config::portPlayer1 = 0;
int Config::seed = 0;
bool Config::phaseOutConstraints = true;
int Config::prioritizationStrategy = 0;
int Config::prioritizationTolerance = 3;

void Config::parseConfig(const std::string& filename) {
    std::ifstream configFile(filename);
    std::string line;
    std::map<std::string, std::string> configMap;

    if (configFile.is_open()) {
        while (getline(configFile, line)) {
            if (line[0] == '#' || line.empty()) { // Skip comments and empty lines
                continue;
            }
            std::istringstream lineStream(line);
            std::string key, value;
            if (getline(lineStream, key, '=') && getline(lineStream, value)) {
                configMap[key] = value;
            }
        }
        configFile.close();
    } else {
        std::cerr << "Unable to open config file: " << filename << std::endl;
        return;
    }

    enableLogging = (configMap["enable_logging"] == "true");
    enableMetrics = (configMap["enable_metrics"] == "true");
    livePlayPlayer0 = (configMap["live_play_player0"] == "true");
    livePlayPlayer1 = (configMap["live_play_player1"] == "true");
    recordPlayer0 = (configMap["record_player0"] == "true");
    recordPlayer1 = (configMap["record_player1"] == "true");
    portPlayer0 = livePlayPlayer0 ? std::stoi(configMap["port_player0"]) : 0;
    portPlayer1 = livePlayPlayer1 ? std::stoi(configMap["port_player1"]) : 0;
    phaseOutConstraints = (configMap["phase_out_constraints"] == "true");
    prioritizationStrategy = std::stoi(configMap["prioritization_strategy"]);
    prioritizationTolerance = std::stoi(configMap["prioritization_tolerance"]);
    seed = std::stoi(configMap["seed"]);
}
