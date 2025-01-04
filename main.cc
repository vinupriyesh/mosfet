#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.h"
#include "metrics.h"
#include "parser.h"
#include "config.h"
#include "agent/control_center.h"

#define log(message) Logger::getInstance().log(message)

using json = nlohmann::json;

ControlCenter* cc = new ControlCenter();

void process(std::string& input, int counter) {
    std::getline(std::cin, input);
        log("Input --> " + input);

        json jsonObject = json::parse(input);
        GameState gameState = jsonObject.get<GameState>();

        cc->update(gameState);        
        std::vector<std::vector<int>> results = cc->act();
        
        json json_results = {{"action", results}};
        log("Output --> " + json_results.dump());
        std::cout << json_results.dump() << std::endl;
        std::cerr.flush();
        std::cout.flush();        
}

void parseConfig(const std::string& filename, std::map<std::string, std::string>& configMap) {
    std::ifstream configFile(filename);
    std::string line;

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
    }
}

int main(int argc, char* argv[]) {

    std::string configFile = (argc > 1) ? argv[1] : "config-prod.properties";
    Config::parseConfig(configFile);

    std::srand(Config::seed);

    if (Config::enableLogging) {
        Logger::getInstance().enableLogging("application.log");
    }

    if (Config::enableMetrics) {
        Metrics::getInstance().enableMetrics("metrics.csv");
    }
    
    log("Mosfet daemon started with config " + configFile);    

    std::string input;
    

    int counter = 0;

    while (true) {
        try{
            auto start = std::chrono::high_resolution_clock::now();

            process(input, counter++);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);            
            Metrics::getInstance().add("step_duration", duration.count());
        } catch (const std::exception& e) {
            log("Exception caught: " + std::string(e.what()));
            std::cerr << "Fatal:" << e.what() << std::endl;
            
            //Print something so that python can handle this
            // std::cout << "unable" << std::endl;
            return -1;
        }
    }

    delete cc;
    return 0;
}

