#include "opponent_tracker.h"
#include "logger.h"
// #include "metrics.h"
#include "game_env_config.h"
#include "symmetry_util.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include "constants.h"

void OpponentTracker::log(const std::string& message) {
    Logger::getInstance().log("OpponentTracker -> " + message);
}

OpponentTracker::OpponentTracker(GameMap &gameMap, RespawnRegistry& respawnRegistry): gameMap(gameMap), respawnRegistry(respawnRegistry) {
    opponentPreviousPositionProbabilities = nullptr;
    opponentPreviousMaxPossibleEnergies = nullptr;
    opponentPositionProbabilities = nullptr;
    opponentMaxPossibleEnergies = nullptr;
    initArrays();
}

void OpponentTracker::initArrays() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    if (opponentPreviousPositionProbabilities != nullptr) {
        delete opponentPreviousPositionProbabilities;
    }

    if (opponentPreviousMaxPossibleEnergies != nullptr) {
        delete opponentPreviousMaxPossibleEnergies;
    }

    opponentPreviousPositionProbabilities = opponentPositionProbabilities;
    opponentPreviousMaxPossibleEnergies = opponentMaxPossibleEnergies;

    opponentPositionProbabilities = new std::vector<std::vector<std::vector<double>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<double>>(gameMap.width, std::vector<double>(gameMap.height, 0.0))
    );

    opponentMaxPossibleEnergies = new std::vector<std::vector<std::vector<int>>>(
        gameEnvConfig.maxUnits, std::vector<std::vector<int>>(gameMap.width, std::vector<int>(gameMap.height, 0))
    );

    delete atleastOneShuttleProbabilities;

    atleastOneShuttleProbabilities = new std::vector<std::vector<double>>(
        gameMap.width, std::vector<double>(gameMap.height, 0.0)
    );
}

void OpponentTracker::clear() {
    // This is supposed to be called every match start
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    for (int i = 0; i < gameEnvConfig.maxUnits; i++) {
        for (int x = 0; x < gameMap.width; x++) {
            for (int y = 0; y < gameMap.height; y++) {
                (*opponentPreviousPositionProbabilities)[i][x][y] = 0.0;
                (*opponentPreviousMaxPossibleEnergies)[i][x][y] = 0;
                (*opponentPositionProbabilities)[i][x][y] = 0.0;
                (*opponentMaxPossibleEnergies)[i][x][y] = 0;
            }
        }
    }
    
    
    for (int x = 0; x < gameMap.width; x++) {
        for (int y = 0; y < gameMap.height; y++) {
            (*atleastOneShuttleProbabilities)[x][y] = 0.0;
        }
    }
}

void OpponentTracker::step() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    log("Updating opponent tracker for step " + std::to_string(state.currentStep));
    auto start = std::chrono::high_resolution_clock::now();

    auto& opponentPositionProbabilitiesCopy = *opponentPositionProbabilities;
    auto& opponentMaxPossibleEnergiesCopy = *opponentMaxPossibleEnergies;

    initArrays();

    auto& opponentPositionProbabilitiesRef = *opponentPositionProbabilities;
    auto& opponentMaxPossibleEnergiesRef = *opponentMaxPossibleEnergies;


    std::unordered_set<int> visibleOpponents;
    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        auto shuttle = gameMap.opponentShuttles[s];
        if (shuttle->visible && !shuttle->ghost) {
            // Opponent shuttle is visible
            opponentPositionProbabilitiesRef[s][shuttle->getX()][shuttle->getY()] = 1.0;
            opponentMaxPossibleEnergiesRef[s][shuttle->getX()][shuttle->getY()] = shuttle->energy;
            visibleOpponents.insert(s);
            log("shuttle " + std::to_string(s) + " was found visible with energy" + std::to_string(shuttle->energy));
        } else if (respawnRegistry.opponentUnitRespawned == s) {
            // Opponent has just spawned
            opponentMaxPossibleEnergiesRef[s][gameEnvConfig.opponentOriginX][gameEnvConfig.opponentOriginY] = 100;
            opponentPositionProbabilitiesRef[s][gameEnvConfig.opponentOriginX][gameEnvConfig.opponentOriginY] = 1.0;
            visibleOpponents.insert(s);
            log("shuttle " + std::to_string(s) + " was just spawned");
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

                if (std::abs(opponentPositionProbabilitiesCopy[s][x][y] - 0) < LOWEST_DOUBLE) {
                    // This shuttle has not reached this tile yet even in the previous step.  No point in going further
                    continue;
                }

                if (opponentMaxPossibleEnergiesCopy[s][x][y] < 0) {
                    // If this shuttle was present in this tile (x, y) it should've been dead by now.  
                    lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y];
                    continue;
                }
                
                for (int pmi = 0; pmi < POSSIBLE_MOVE_SIZE; pmi++) {
                    int xNext = POSSIBLE_MOVES[pmi][0] + x;
                    int yNext = POSSIBLE_MOVES[pmi][1] + y;
                    
                    if (!gameMap.isValidTile(xNext, yNext)) {
                        // This tile is unreachable
                        lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y] / 5.0;
                        continue;
                    }

                    GameTile& nextTile = gameMap.getTile(xNext, yNext);
                    if (gameMap.getEstimatedType(nextTile, state.currentStep) == TileType::ASTEROID && (xNext != x || yNext != y)) {
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
                        newEnergy -= gameEnvConfig.unitMoveCost;
                        
                        if (newEnergy < 0) {
                            // This shuttle cannot move anymore
                            lostProbabilityForShuttle += opponentPositionProbabilitiesCopy[s][x][y] / 5.0;
                            continue;
                        }
                    }                    

                    if (gameMap.getEstimatedType(nextTile, state.currentStep) == TileType::NEBULA && state.nebulaTileEnergyReductionSet) {
                        newEnergy -= state.nebulaTileEnergyReduction;
                    }

                    if (newEnergy > MAX_ENERGY) {
                        newEnergy = MAX_ENERGY;
                    }

                    if (newEnergy < 0) {                        
                        newEnergy = 0;
                    }
                                                
                    opponentMaxPossibleEnergiesRef[s][xNext][yNext] = std::max(opponentMaxPossibleEnergiesRef[s][xNext][yNext], newEnergy);
                    opponentPositionProbabilitiesRef[s][xNext][yNext] += opponentPositionProbabilitiesCopy[s][x][y] / 5.0; // There are 5 possible moves from this tile

                    if (opponentPositionProbabilitiesRef[s][xNext][yNext] > LOWEST_DOUBLE && !visitedArray[xNext][yNext]) {
                        visitedArray[xNext][yNext] = true;
                        lostProbabilityDistributionCount++;
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
        double totalProbability = 0.0;
        for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
            for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
                if (opponentPositionProbabilitiesRef[s][x][y] > LOWEST_DOUBLE) {
                    opponentPositionProbabilitiesRef[s][x][y] += offsetValue;
                }
                totalProbability += opponentPositionProbabilitiesRef[s][x][y];                
            }
        }

        log("shuttle " + std::to_string(s) + " has " + std::to_string(lostProbabilityDistributionCount) + " possibilities, total=" + std::to_string(totalProbability));

        //TODO:  Ideally zero lostProbabilityDistributionCount should have been dead.  After improving the death detector, revisit this to raise some noise if the probability doesn't match
        if ((totalProbability < 1.0 - LOWEST_DOUBLE || totalProbability > 1.0 + LOWEST_DOUBLE) && lostProbabilityDistributionCount > 0) {
            log("Problem, the probabilty doesn't sum to 1.0");
            std::cerr<<"Problem, the probabilty doesn't sum to 1.0"<<std::endl;
        }
    }

    computeAtleastOneShuttleProbabilities();    

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Metrics::getInstance().add("opponent_tracker_step", duration.count());
    log("Time taken for opponent_tracker_step " + std::to_string(duration.count()));
}

void OpponentTracker::computeAtleastOneShuttleProbabilities() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto& positionProbabilities = *opponentPositionProbabilities;
    auto& atleastOneShuttleProbabilitiesRef = * atleastOneShuttleProbabilities;

    int opponentOpportunities = 0;
    int opponentExploiting = 0;
    int opponentConfirmedPoints = 0;

    std::unordered_set<int> opponentOpportunitiesTiles;

    for (int x = 0; x < gameEnvConfig.mapHeight; ++x) {
        for (int y = 0; y < gameEnvConfig.mapWidth; ++y) {
            double pNotShuttle = 1.0;
            for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
                pNotShuttle *= (1.0 - positionProbabilities[s][x][y]);
            }
            atleastOneShuttleProbabilitiesRef[x][y] = 1.0 - pNotShuttle;

            // Count vantage points and halo tiles
            GameTile& tile = gameMap.getTile(x, y);
            if (tile.isHaloTile() || tile.isVantagePoint()) {
                if (!tile.isVisible()) {
                    opponentOpportunities++;
                    opponentOpportunitiesTiles.insert(tile.getId(gameMap.width));
                } else if (tile.isOpponentOccupied()) {
                    opponentExploiting++;

                    if (tile.isVantagePoint()) {
                        opponentConfirmedPoints++;
                    }
                }
            }
        }
    }

    int opponentTeamPointsDelta = gameMap.derivedGameState.opponentTeamPointsDelta;

    if (opponentTeamPointsDelta > 0 && opponentOpportunities + opponentExploiting > 0) {

        int outstandingPoints = opponentTeamPointsDelta - opponentConfirmedPoints;

        if (outstandingPoints < 0 && gameMap.derivedGameState.currentMatchStep > 0) {
            log("Problem: opponent outstanding points negative " + std::to_string(opponentTeamPointsDelta) + ", " + std::to_string(opponentConfirmedPoints));
            std::cerr<<"Problem: opponent outstanding points negative "<<std::endl;
        }

        double probabilityDistribution = static_cast<double>(outstandingPoints) / static_cast<double>(opponentOpportunities  + opponentExploiting - opponentConfirmedPoints);

        if (probabilityDistribution > 1.0) {
            log("Problem: probability distribution is greater than 1.0 " + std::to_string(probabilityDistribution));
            std::cerr<<"Problem: probability distribution is greater than 1.0 "<<std::endl;
        }

        if (probabilityDistribution < 1.0 + LOWEST_DOUBLE && probabilityDistribution > 1.0 - LOWEST_DOUBLE) {
            log("Voila! can snipe the opponent now");
        }    

        if (!gameMap.derivedGameState.isThereAHuntForRelic()) {
            for (int tileId : opponentOpportunitiesTiles) {
                int x, y;
                symmetry_utils::toXY(tileId, x, y);
                
                if (atleastOneShuttleProbabilitiesRef[x][y] < probabilityDistribution) {
                    //TODO: Back-propagate this probability to the positionProbabilities
                    atleastOneShuttleProbabilitiesRef[x][y] = probabilityDistribution;
                }
                log("Forcing atleast one shuttle probability at " + std::to_string(x) + ", " + std::to_string(y) + " to " + std::to_string(probabilityDistribution));
            }
        }
    }
}

OpponentTracker::~OpponentTracker() {
    delete opponentPositionProbabilities;
    delete opponentMaxPossibleEnergies;
    delete atleastOneShuttleProbabilities;
}


std::vector<std::vector<std::vector<double>>>& OpponentTracker::getOpponentPositionProbabilities() {
    return *opponentPositionProbabilities;
}

std::vector<std::vector<std::vector<int>>>& OpponentTracker::getOpponentMaxPossibleEnergies() {
    return *opponentMaxPossibleEnergies;
}

std::vector<std::vector<std::vector<double>>>& OpponentTracker::getOpponentPreviousPositionProbabilities() {
    if (opponentPreviousPositionProbabilities == nullptr) {
        return getOpponentPositionProbabilities();
    }
    return *opponentPreviousPositionProbabilities;
}

std::vector<std::vector<std::vector<int>>>& OpponentTracker::getOpponentPreviousMaxPossibleEnergies() {
    if (opponentPreviousMaxPossibleEnergies == nullptr) {
        return getOpponentMaxPossibleEnergies();
    }
    return *opponentPreviousMaxPossibleEnergies;
}

std::vector<std::vector<double>>& OpponentTracker::getAtleastOneShuttleProbabilities() {
    return *atleastOneShuttleProbabilities;
}


bool OpponentTracker::isOpponentOccupied(int x, int y){
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto probabilities = getOpponentPositionProbabilities();
    for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
        if (probabilities[s][x][y] > 1.0 - LOWEST_DOUBLE && probabilities[s][x][y] < 1.0 + LOWEST_DOUBLE) {
            return true;
        }
    }

    return false;
}

double OpponentTracker::expectationOfOpponentOccupancy(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto probabilities = getOpponentPositionProbabilities();
    double expectation = 0.0;
    for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
        expectation += probabilities[s][x][y];        
    }

    return expectation;
}


int OpponentTracker::getAllPossibleEnergyAt(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto energies = getOpponentMaxPossibleEnergies();
    int expectation = 0.0;
    for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
        expectation += energies[s][x][y];        
    }

    return expectation;
}

int OpponentTracker::getCountLessThanEnergyAt(int x, int y, int energy) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto energies = getOpponentMaxPossibleEnergies();
    int expectation = 0.0;
    for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
        if (energies[s][x][y] < energy) {
            expectation++;
        }
    }

    return expectation;
}