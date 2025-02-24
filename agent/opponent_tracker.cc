#include "opponent_tracker.h"
#include "logger.h"
#include "metrics.h"

#include <iostream>

void OpponentTracker::log(std::string message) {
    Logger::getInstance().log("OpponentTracker -> " + message);
}

OpponentTracker::OpponentTracker(GameMap &gameMap, RespawnRegistry respawnRegistry): gameMap(gameMap), respawnRegistry(respawnRegistry) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    opponentPositionProbabilities = std::vector<std::vector<std::vector<double>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<double>>(gameMap.width, std::vector<double>(gameMap.height, 0.0))
    );
}

void OpponentTracker::step()
{
    auto start = std::chrono::high_resolution_clock::now();



    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("opponent_tracker_step", duration.count());
}
