#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.h"
#include "parser.h"
#include "agent/control_center.h"

#define log(message) Logger::getInstance().log(message)

using json = nlohmann::json;

ControlCenter* cc;

void process(std::string& input, int counter) {
    std::getline(std::cin, input);
        log("Input --> " + input);

        json jsonObject = json::parse(input);
        GameState gameState = jsonObject.get<GameState>();

        log("before checking counter");
        if (counter == 0) {
            log("Creating new CC");
            cc = new ControlCenter(gameState.info.envCfg["max_units"]);
        }        

        log("Before calling ACT");
        std::vector<std::vector<int>> results = cc->act();
        log("Got results");
        json json_results = {{"action", results}};
        log("Dumping results" + json_results.dump());
        std::cout << json_results.dump() << std::endl; // Pretty print with indentation of 4 spaces

        // std::cout << "0 0 0" << std::endl;
        std::cout.flush(); // Ensure output is immediately flushed
}

int main(int argc, char* argv[]) {

    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
    }

    if (verbose) {
        Logger::getInstance().enableLogging("application.log");
    }
    
    log("Mosfet daemon started");    

    std::string input;
    

    int counter = 0;

    while (true) {
        try{
            process(input, counter++);
        } catch (const std::exception& e) {
            log("Exception caught: " + std::string(e.what()));
            std::cerr << "CC ERR:" << e.what() << std::endl;
            
            //Print something so that python can handle this
            // std::cout << "unable" << std::endl;
            return -1;
        }
    }

    return 0;
}
