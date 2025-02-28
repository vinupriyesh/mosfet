#include <iostream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>

#include "visualizer_client.h"
#include "logger.h"
#include "game_env_config.h"
#include "config.h"

// Custom deleter for FILE pointers
struct FileCloser {
    void operator()(FILE* file) const {
        if (file) {
            pclose(file);
        }
    }
};

void VisualizerClient::log(std::string message) {
    Logger::getInstance().log("VisualizerClient -> " + message);
}

std::string VisualizerClient::exec(const char* cmd) {
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

std::string VisualizerClient::getData(std::vector<std::vector<int>> actions) {
    // Implement the function to return the required data as a JSON string
    // return "{\"grid_size\": [24, 24], \"asteroids\": [[0,0], [0,5],[20,5]], \"blue_shuttles\": [[3,0], [7,5]], \"red_shuttles\": [[4,3], [9,5]]}";
    log("Collecting data");
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    
    // Create a JSON object
    json jsonObject;

    jsonObject["step"].push_back(gameMap.derivedGameState.currentStep);
    jsonObject["match_step"].push_back(gameMap.derivedGameState.currentMatchStep);
    jsonObject["points"].push_back(gameMap.derivedGameState.teamPoints);
    jsonObject["points"].push_back(gameMap.derivedGameState.opponentTeamPoints);
    jsonObject["wins"].push_back(gameMap.derivedGameState.teamWins);
    jsonObject["wins"].push_back(gameMap.derivedGameState.opponentWins);

    // Add grid_size
    jsonObject["grid_size"] = {gameEnvConfig.mapWidth, gameEnvConfig.mapHeight};
    jsonObject["unit_move_cost"] = gameEnvConfig.unitMoveCost;
    jsonObject["unit_sap_cost"] = gameEnvConfig.unitSapCost;
    jsonObject["unit_sap_range"] = gameEnvConfig.unitSapRange;
    jsonObject["unit_sensor_range"] = gameEnvConfig.unitSensorRange;

    log("Looking for relics - " + std::to_string(relics.size()));
    // Add relics
    for (const auto& pair : relics) {
        jsonObject["relics"].push_back(pair.second->position);
    }

    log("Looking for asteroids");
    // Add asteroids
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& tile = gameMap.getTile(i, j);          
            TileType tileType = gameMap.getEstimatedType(tile, gameMap.derivedGameState.currentStep);
            // TileType tileType = tile.getType();
            if (tileType == TileType::ASTEROID) {
                jsonObject["asteroids"].push_back({i, j});
            }
            if (tileType == TileType::NEBULA) {
                jsonObject["nebula"].push_back({i, j});
            }
            if (tile.isHaloTile()) {
                jsonObject["halo_tiles"].push_back({i, j});
            }
            if (tile.isVantagePoint()) {
                jsonObject["vantage_points"].push_back({i, j});
            }
            if (tile.isUnExploredFrontier()) {
                jsonObject["unexplored_frontier"].push_back({i, j});
            }
            jsonObject["energy"].push_back(tile.getLastKnownEnergy());
            jsonObject["vision"].push_back(tile.isVisible()? 1 : 0);
        }
    }

    log("Looking for player shuttles");
    // Blue is always the current player
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        if (shuttles[i]->getShuttleData().getX() == -1) {
            continue;
        }
        jsonObject["blue_shuttles"].push_back(shuttles[i]->getShuttleData().position);
        jsonObject["blue_shuttles_energy"].push_back(shuttles[i]->getShuttleData().lastKnownEnergy);
        jsonObject["blue_shuttles_actions"].push_back(actions[i]);
    }

    log("Looking for opponent shuttles");
    // Red is always the enemy player
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        log("Checking opponent shuttle " + std::to_string(i));
        if (opponentShuttles[i]->getShuttleData().getX() == -1 || opponentShuttles[i]->getShuttleData().getY() == -1) {
            continue;
        }
        jsonObject["red_shuttles"].push_back(opponentShuttles[i]->getShuttleData().position);
        jsonObject["red_shuttles_energy"].push_back(opponentShuttles[i]->getShuttleData().lastKnownEnergy);        
    }

    log("Done collecing data");
    return jsonObject.dump();
}

VisualizerClient::VisualizerClient(GameMap &gameMap, Shuttle **shuttles, Shuttle **opponentShuttles, std::map<int, Relic *>& relics) 
                                    : gameMap(gameMap), shuttles(shuttles), opponentShuttles(opponentShuttles), relics(relics) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (gameEnvConfig.teamId == 0) {
        //We are playing as blue team
        livePlayEnabled = Config::livePlayPlayer0;
        recordingEnabled = Config::recordPlayer0;
        port = Config::portPlayer0;
        teamId = 0;
    } else if(gameEnvConfig.teamId == 1) {
        livePlayEnabled = Config::livePlayPlayer1;
        recordingEnabled = Config::recordPlayer1;
        port = Config::portPlayer1;
        teamId = 1;
    }
    
    if (recordingEnabled) {
        std::string filename = "custom_replay_" + std::to_string(teamId) + ".json";
        log_file.open(filename, std::ios::out | std::ios::app);
    }
    if (log_file.is_open()) {
        log_file << "{\"data\":[" << std::endl;
    }
}

std::string VisualizerClient::uploadData(std::string data) {
    if (!livePlayEnabled) {
        return "";
    }
    try {
        std::string url = "http://localhost:" + std::to_string(port) + "/";
        std::string command = "curl -s --request POST --url " + url + " --header 'content-type: application/json' --data '"+ data + "'";
        Logger::getInstance().log("Command: " + command);        
        std::string response = exec(command.c_str());
        Logger::getInstance().log("Response: " + response);
        return response;
    } catch (const std::exception& e) {
        Logger::getInstance().log("Exception while sending live play data: " + std::string(e.what()));
    }
    
    return "";
}

int VisualizerClient::sendGameData(std::vector<std::vector<int>> actions) {
    if (!livePlayEnabled && !recordingEnabled) {
        return 0;
    }

    log("sending game data");
    auto start = std::chrono::high_resolution_clock::now();         

    std::string data = getData(actions);
    
    if (log_file.is_open()) {
        log_file << data << "," << std::endl;
    }

    if (livePlayEnabled) {
        uploadData(data);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);      
    Metrics::getInstance().add("visualizer_overhead", duration.count());
    log("game data sent");
    return 0;
}

VisualizerClient::~VisualizerClient() {
    log("Destroying visualizer");
    if (log_file.is_open()) {
        log_file<<"{}]}"<<std::endl;
        log_file.close();
    }
}

