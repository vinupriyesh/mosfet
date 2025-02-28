#include "opponent_tracker.h"
#include "logger.h"
// #include "metrics.h"
#include "game_env_config.h"
#include <string>
#include <unordered_set>

const double LOWEST_DOUBLE = 1e-9;

void OpponentTracker::log(const std::string& message) {
    Logger::getInstance().log("OpponentTracker -> " + message);
}

OpponentTracker::OpponentTracker(GameMap &gameMap, RespawnRegistry& respawnRegistry): gameMap(gameMap), respawnRegistry(respawnRegistry) {
    initArrays();
}

void OpponentTracker::initArrays() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    opponentPositionProbabilities = new std::vector<std::vector<std::vector<double>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<double>>(gameMap.width, std::vector<double>(gameMap.height, 0.0))
    );

    opponentMaxPossibleEnergies = new std::vector<std::vector<std::vector<int>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<int>>(gameMap.width, std::vector<int>(gameMap.height, 0))
    );
}

void OpponentTracker::step() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    log("Updating opponent tracker for step " + std::to_string(state.currentStep));
    auto start = std::chrono::high_resolution_clock::now();

    auto& opponentPositionProbabilitiesCopy = *opponentPositionProbabilities;
    auto& opponentMaxPossibleEnergiesCopy = *opponentMaxPossibleEnergies;

    auto oldOpponentPositionProbabilities = opponentPositionProbabilities;
    auto oldOpponentMaxPossibleEnergies = opponentMaxPossibleEnergies;

    initArrays();

    auto& opponentPositionProbabilitiesRef = *opponentPositionProbabilities;
    auto& opponentMaxPossibleEnergiesRef = *opponentMaxPossibleEnergies;

    std::unordered_set<int> visibleOpponents;
    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        auto shuttle = gameMap.opponentShuttles[s];
        if (shuttle->visible) {
            // Opponent shuttle is visible
            opponentPositionProbabilitiesRef[s][shuttle->getX()][shuttle->getY()] = 1.0;
            opponentMaxPossibleEnergiesRef[s][shuttle->getX()][shuttle->getY()] = shuttle->energy;
            visibleOpponents.insert(s);
        }

        if (respawnRegistry.getOpponentUnitSpawnStep(s) == state.currentStep) {
            // Opponent has just spawned
            opponentMaxPossibleEnergiesRef[s][gameEnvConfig.opponentOriginX][gameEnvConfig.opponentOriginY] = 100;
            opponentPositionProbabilitiesRef[s][gameEnvConfig.opponentOriginX][gameEnvConfig.opponentOriginY] = 1.0;
            visibleOpponents.insert(s);
        }
    }

    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        if (!respawnRegistry.isOpponentShuttleAlive(s, state.currentStep)) {
            //This shuttle is not alive yet. No action
            continue;
        }

        if (visibleOpponents.find(s) != visibleOpponents.end()) {
            //This shuttle is visible, updated already
            continue;
        }

        double lostProbabilityForShuttle = 0.0;
        int lostProbabilityDistributionCount = 0;

        std::vector<std::vector<bool>> visitedArray(gameEnvConfig.mapHeight, std::vector<bool>(gameEnvConfig.mapWidth, false));

        for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
            for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
                GameTile& tile = gameMap.getTile(x, y);

                if (std::abs(opponentPositionProbabilitiesCopy[s][x][y] - 0) < LOWEST_DOUBLE && tile.getLastKnownEnergy() <= 0) {
                    // This shuttle has not reached this tile yet even in the previous step.  No point in going further
                    continue;
                }              

                if (tile.isVisible()) {
                    //This tile is visible, probability is either 0 or 1
                    lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y];
                    continue;
                }                
                
                for (int xNext = x - 1; xNext <= x + 1; ++xNext) {
                    for (int yNext = y - 1; yNext <= y + 1; ++ yNext) {                        
                        if (gameMap.isValidTile(xNext, yNext)) {
                            GameTile& nextTile = gameMap.getTile(x, y);
                            if (gameMap.getEstimatedType(nextTile, state.currentStep) == TileType::ASTEROID) {
                                // Opponent couldn't have moved to this tile, it is an asteroid
                                lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y] / 5.0;
                                continue;
                            }

                            if (nextTile.isVisible()) {
                                //Opponent couldn't have moved to this tile as it is visible
                                lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y] / 5.0;
                                continue;
                            }

                            int newEnergy = opponentMaxPossibleEnergiesCopy[s][x][y] + nextTile.getLastKnownEnergy();

                            if (x != xNext || y != yNext) {
                                newEnergy = opponentMaxPossibleEnergiesCopy[s][x][y] - gameEnvConfig.unitMoveCost;
                            }
                                                        
                            opponentMaxPossibleEnergiesRef[s][xNext][yNext] = std::max(opponentMaxPossibleEnergiesRef[s][xNext][yNext], newEnergy);
                            opponentPositionProbabilitiesRef[s][xNext][yNext] += opponentPositionProbabilitiesCopy[s][x][y] / 5.0; // There are 5 possible moves from this tile

                            if (opponentPositionProbabilitiesRef[s][xNext][yNext] > LOWEST_DOUBLE && !visitedArray[xNext][yNext]) {
                                visitedArray[xNext][yNext] = true;
                                lostProbabilityDistributionCount++;
                            }
                        }
                    }
                }

                if (opponentPositionProbabilitiesRef[s][x][y] > LOWEST_DOUBLE && !visitedArray[x][y]) {
                    visitedArray[x][y] = true;
                    lostProbabilityDistributionCount++;
                }
            }
        }

        // Now fill the offset to make sum(probability) = 1        
        double offsetValue = lostProbabilityForShuttle / lostProbabilityDistributionCount;
        for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
            for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
                if (opponentPositionProbabilitiesRef[s][x][y] > LOWEST_DOUBLE) {
                    opponentPositionProbabilitiesRef[s][x][y] += offsetValue;
                }
            }
        }

        log("shuttle " + std::to_string(s) + " has " + std::to_string(lostProbabilityDistributionCount) + " possibilities");
    }

    delete oldOpponentPositionProbabilities;
    delete oldOpponentMaxPossibleEnergies;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Metrics::getInstance().add("opponent_tracker_step", duration.count());
    log("Time taken for opponent_tracker_step " + std::to_string(duration.count()));
}



std::vector<std::vector<std::vector<double>>>& OpponentTracker::getOpponentPositionProbabilities() {
    return *opponentPositionProbabilities;
}
