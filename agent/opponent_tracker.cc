#include "opponent_tracker.h"
#include "logger.h"
#include "metrics.h"

#include <iostream>

void OpponentTracker::log(std::string message) {
    Logger::getInstance().log("OpponentTracker -> " + message);
}

OpponentTracker::OpponentTracker(GameMap &gameMap, RespawnRegistry& respawnRegistry): gameMap(gameMap), respawnRegistry(respawnRegistry) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    opponentPositionProbabilities = std::vector<std::vector<std::vector<double>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<double>>(gameMap.width, std::vector<double>(gameMap.height, 0.0))
    );

    opponentMaxPossibleEnergies = std::vector<std::vector<std::vector<int>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<int>>(gameMap.width, std::vector<int>(gameMap.height, 0))
    );
}

void OpponentTracker::step() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    log("Updating opponent tracker for step " + std::to_string(state.currentStep));
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<std::vector<double>>> opponentPositionProbabilitiesCopy = opponentPositionProbabilities;
    std::vector<std::vector<std::vector<int>>> opponentMaxPossibleEnergiesCopy = opponentMaxPossibleEnergies;


    //TODO: do not do this.  Create new arrays instead and make the member a pointer
    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
            for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
                opponentMaxPossibleEnergies[s][x][y] = 0;
                opponentPositionProbabilities[s][x][y] = 0;
            }
        }
    }

    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
            for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
                GameTile& tile = gameMap.getTile(x, y);

                int possibleMoves = 0;

                for (int xNext = x - 1; xNext <= x + 1; ++xNext) {
                    for (int yNext = y - 1; yNext <= y + 1; ++ yNext) {                        
                        if (gameMap.isValidTile(xNext, yNext)) {
                            GameTile& nextTile = gameMap.getTile(x, y);
                            if (gameMap.getEstimatedType(nextTile, state.currentStep) == TileType::ASTEROID) {
                                continue;
                            }

                            int newEnergy = opponentMaxPossibleEnergiesCopy[s][x][y] + nextTile.getLastKnownEnergy();

                            if (x != xNext || y != yNext) {
                                newEnergy = opponentMaxPossibleEnergiesCopy[s][x][y] - gameEnvConfig.unitMoveCost;
                            }
                                                        
                            opponentMaxPossibleEnergies[s][xNext][yNext] = std::max(opponentMaxPossibleEnergies[s][xNext][yNext], newEnergy);
                            possibleMoves++;                            
                        }
                    }
                }


                for (int xNext = x - 1; xNext <= x + 1; ++xNext) {
                    for (int yNext = y - 1; yNext <= y + 1; ++ yNext) {                        
                        if (gameMap.isValidTile(xNext, yNext)) {
                            GameTile& nextTile = gameMap.getTile(x, y);

                            if (gameMap.getEstimatedType(nextTile, state.currentStep) == TileType::ASTEROID) {
                                continue;
                            }

                            opponentPositionProbabilities[s][x][y] += opponentPositionProbabilitiesCopy[s][x][y] / possibleMoves;
                        }
                    }
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("opponent_tracker_step", duration.count());
}
