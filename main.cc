#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.h"
#include "parser.h"
#include "agent/control_center.h"


using json = nlohmann::json;

void log(const std::string& message) {
    Logger::getInstance().log(message);
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
    ControlCenter* cc;

    int counter = 0;

    while (true) {
        log("Waiting for input");
        std::getline(std::cin, input);
        log("Input --> " + input);

        json jsonObject = json::parse(input);
        GameState gameState = jsonObject.get<GameState>();

        if (counter == 0) {
            cc = new ControlCenter(gameState.info.envCfg["max_units"]);
        }
        counter++;

        std::vector<std::vector<int>> results = cc->act();
        json json_results = {{"action", results}};
        log("Dumping results" + json_results.dump());
        std::cout << json_results.dump() << std::endl; // Pretty print with indentation of 4 spaces

        // std::cout << "0 0 0" << std::endl;
        std::cout.flush(); // Ensure output is immediately flushed
    }

    return 0;
}
