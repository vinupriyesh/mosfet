#include <iostream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>

#include "visualizer_client.h"
#include "logger.h"

// Custom deleter for FILE pointers
struct FileCloser {
    void operator()(FILE* file) const {
        if (file) {
            pclose(file);
        }
    }
};

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    // Use std::unique_ptr with the custom deleter
    std::unique_ptr<FILE, FileCloser> pipe(popen(cmd, "r"));
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

std::string get_data(Shuttle** shuttles, Shuttle** enemyShuttles, Relic** relic, GameEnvConfig* gameEnvConfig) {
    // Implement the function to return the required data as a JSON string
    // return "{\"grid_size\": [24, 24], \"asteroids\": [[0,0], [0,5],[20,5]], \"blue_shuttles\": [[3,0], [7,5]], \"red_shuttles\": [[4,3], [9,5]]}";

    // Create a JSON object
    json jsonObject;

    // Add grid_size
    jsonObject["grid_size"] = {gameEnvConfig->mapWidth, gameEnvConfig->mapHeight};

    // Add asteroids
    // jsonObject["asteroids"] = {{0, 0}, {0, 5}, {20, 5}};


    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        if (shuttles[i]->position[0] == -1) {
            continue;
        }
        jsonObject["blue_shuttles"].push_back(shuttles[i]->position);
    }

    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        if (enemyShuttles[i]->position[0] == -1) {
            continue;
        }
        jsonObject["red_shuttles"].push_back(enemyShuttles[i]->position);
    }

    return jsonObject.dump();
}

int send_game_data(Shuttle** shuttle, Shuttle** enemyShuttle, Relic** relic, GameEnvConfig* gameEnvConfig) {
    std::string url = "http://localhost:8088/";
    std::string data = get_data(shuttle, enemyShuttle, relic, gameEnvConfig);
    std::string command = "curl -s --request POST --url " + url + " --header 'content-type: application/json' --data '"+ data + "'";
    Logger::getInstance().log("Command: " + command);
    /*
    curl --request POST \
  --url http://localhost:8088/ \
  --header 'content-type: application/json' \
  --data '{
  "grid_size": [24, 24],
  "asteroids": [[0,0], [0,5],[20,5]],
  "blue_shuttles": [[3,0], [7,5]],
  "red_shuttles": [[4,3], [9,5]]
    }'
    */

    try {
        std::string response = exec(command.c_str());
        Logger::getInstance().log("Response: " + response);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Exception: " + std::string(e.what()));
    }

    return 0;
}
