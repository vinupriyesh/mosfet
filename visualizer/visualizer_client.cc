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

std::string get_data(Shuttle** shuttles, Shuttle** enemyShuttles, Relic** relic, GameEnvConfig* gameEnvConfig, GameMap* gameMap) {
    // Implement the function to return the required data as a JSON string
    // return "{\"grid_size\": [24, 24], \"asteroids\": [[0,0], [0,5],[20,5]], \"blue_shuttles\": [[3,0], [7,5]], \"red_shuttles\": [[4,3], [9,5]]}";

    // Create a JSON object
    json jsonObject;

    // Add grid_size
    jsonObject["grid_size"] = {gameEnvConfig->mapWidth, gameEnvConfig->mapHeight};

    // Add relics
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        if (!relic[i]->revealed) {
            continue;
        }
        jsonObject["relics"].push_back(relic[i]->position);
    }

    // Add asteroids
    for (int i = 0; i < gameEnvConfig->mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig->mapWidth; ++j) {
            if (gameMap->getTile(i, j).getLastKnownType() == TileType::ASTEROID) {
                jsonObject["asteroids"].push_back({i, j});
            }
            if (gameMap->getTile(i, j).getLastKnownType() == TileType::NEBULA) {
                jsonObject["nebula"].push_back({i, j});
            }
            if (gameMap->getTile(i, j).isHaloTile()) {
                jsonObject["halo_tiles"].push_back({i, j});
            }
            if (gameMap->getTile(i, j).isVantagePoint()) {
                jsonObject["vantage_points"].push_back({i, j});
            }
        }
    }

    // Blue is the current player
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        if (shuttles[i]->position[0] == -1) {
            continue;
        }
        jsonObject["blue_shuttles"].push_back(shuttles[i]->position);
    }

    // Red is the enemy player
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        if (enemyShuttles[i]->position[0] == -1) {
            continue;
        }
        jsonObject["red_shuttles"].push_back(enemyShuttles[i]->position);
    }

    return jsonObject.dump();
}

int send_game_data(Shuttle** shuttle, Shuttle** enemyShuttle, Relic** relic, GameEnvConfig* gameEnvConfig, GameMap* gameMap, int port) {
    std::string url = "http://localhost:" + std::to_string(port) + "/";
    std::string data = get_data(shuttle, enemyShuttle, relic, gameEnvConfig, gameMap);
    std::string command = "curl -s --request POST --url " + url + " --header 'content-type: application/json' --data '"+ data + "'";
    Logger::getInstance().log("Command: " + command);            

    try {
        std::string response = exec(command.c_str());
        Logger::getInstance().log("Response: " + response);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Exception while sending live play data: " + std::string(e.what()));
    }

    return 0;
}
