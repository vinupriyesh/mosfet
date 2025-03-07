#include "shuttle_energy_tracker.h"
#include "agent/game_map.h"
#include "metrics.h"
#include "logger.h"
#include "game_env_config.h"
#include "constants.h"
#include <iostream>
#include <unordered_set>

void ShuttleEnergyTracker::log(const std::string& message) {
    Logger::getInstance().log("ShuttleEnergyTracker -> " + message);
}

template<typename T>
std::string ShuttleEnergyTracker::vectorToString(const std::vector<T>& vec, const std::string& name) {
    std::string result = name + " values: [";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += std::to_string(vec[i]);
        if (i < vec.size() - 1) {
            result += ", ";
        }
    }
    result += "]";
    return result;
}

void ShuttleEnergyTracker::preparePlayerCollisions() {
    playerCollisions.clear();
    int totalEnergyLost = 0;
    for (ShuttleData* shuttle: gameMap.shuttles) {
        if (shuttle->previouslyVisible && !shuttle->visible && shuttle->previousEnergy >= 0) {
            log("Shuttle " + std::to_string(shuttle->id) + " was visible last turn and is not visible this turn, previous energy = " + std::to_string(shuttle->previousEnergy));
            playerCollisions.insert(shuttle->id);
            totalEnergyLost += shuttle->previousEnergy;
        }
    }

    Metrics::getInstance().add("energy_lost_in_collision", totalEnergyLost);
}

void ShuttleEnergyTracker::prepareOpponentCollisionMap() {
    confirmedCollisions.clear();
    possibleCollisions.clear();
    for (ShuttleData* shuttle: gameMap.opponentShuttles) {
        if (shuttle->previouslyVisible && !shuttle->visible && shuttle->previousEnergy >= 0) {
            // This shuttle was previously visible and had energy, it should've only turned as a ghost now. This could be a collision or the shuttle went out of vision
            bool couldHaveGoneOutOfVision = false;
            std::vector<int> playerTileIds;
            for (int i = 0; i < POSSIBLE_MOVE_SIZE; ++i) {
                int x = shuttle->getPreviousX() + POSSIBLE_MOVES[i][0];
                int y = shuttle->getPreviousY() + POSSIBLE_MOVES[i][1];

                if (!gameMap.isValidTile(x, y)) {
                    continue;
                }

                GameTile& tile = gameMap.getTile(x, y);

                if (!tile.isVisible()) {
                    couldHaveGoneOutOfVision = true;
                    continue;
                }

                if (tile.isOccupied()) {
                    playerTileIds.push_back(tile.getId(gameMap.width));
                }
            }

            for (int tileId: playerTileIds) {
                if (couldHaveGoneOutOfVision || playerTileIds.size() > 1) {
                    possibleCollisions[tileId].push_back(shuttle->id);
                } else {
                    confirmedCollisions[tileId].push_back(shuttle->id);
                }
            }
        }
    }
}

bool ShuttleEnergyTracker::getPossibleMeleeSappingEnergyNearby(ShuttleData& shuttle, std::vector<int>& meleeSapEnergies) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;
    bool foundAccurateValues = true;

    for (int i = 0; i < POSSIBLE_NEIGHBORS_SIZE; ++i) {
        int x = shuttle.getX() + POSSIBLE_NEIGHBORS[i][0];
        int y = shuttle.getY() + POSSIBLE_NEIGHBORS[i][1];

        if (!gameMap.isValidTile(x, y)) {
            continue;
        }

        GameTile& tile = gameMap.getTile(x, y);
        bool opponentPossibleThisTile = opponentTracker.expectationOfOpponentOccupancy(x, y) > LOWEST_DOUBLE;
        log("Opponent probability at " + std::to_string(x) + ", " + std::to_string(y) + " is " + std::to_string(opponentPossibleThisTile));

        if (!tile.isVisible() && opponentPossibleThisTile) {
            // This tile is invisible and there is a chance of opponent being there
            foundAccurateValues = false; 
            log("Cant predict accurately as this tile is invisible - " + std::to_string(x) + ", " + std::to_string(y) + " for shuttle - " + std::to_string(shuttle.id));
        }
        
        int netEnergy = 0;
        for (ShuttleData* shuttle: tile.opponentShuttles) {
            if (shuttle->previouslyVisible) {
                netEnergy += shuttle->previousEnergy;
                if (shuttle->hasMoved()) {
                    netEnergy -= gameEnvConfig.unitMoveCost;
                }
            } else {
                log("Cant predict accurately as this opponent shuttle is invisible last turn - " + std::to_string(shuttle->id) 
                        + " at tile - " + std::to_string(x) + ", " + std::to_string(y) + " our shuttle - " + std::to_string(shuttle->id));
                foundAccurateValues = false;
            }
        }

        // Check the ghost shuttles as well
        for (ShuttleData* shuttle: tile.opponentGhostShuttles) {
            if (shuttle->previouslyVisible) {
                netEnergy += shuttle->previousEnergy;
                if (shuttle->hasMoved()) {
                    netEnergy -= gameEnvConfig.unitMoveCost;  //TODO: Not sure if the negative energies are included in the netEnergy
                }
            } else {
                log("Cant predict accurately as this opponent shuttle is invisible last turn - " + std::to_string(shuttle->id) 
                        + " at tile - " + std::to_string(x) + ", " + std::to_string(y) + " our shuttle - " + std::to_string(shuttle->id));
                foundAccurateValues = false;
            }
        }

        // Check for the confirmed dead tiles
        if (confirmedCollisions.find(tile.getId(gameMap.width)) != confirmedCollisions.end()) {
            for (int otherShuttleId : confirmedCollisions.at(tile.getId(gameMap.width))) {
                auto& otherShuttle = gameMap.opponentShuttles[otherShuttleId];
                netEnergy += otherShuttle->previousEnergy;
                if (otherShuttle->hasMoved()) {
                    netEnergy -= gameEnvConfig.unitMoveCost;
                }
            }
        }

        // Check for the possible dead tiles
        if (possibleCollisions.find(tile.getId(gameMap.width)) != possibleCollisions.end()) {
            log("Cant predict accurately as there could have been unconfirmed collition and death of opponent unit at tile - " + std::to_string(x) + ", " + std::to_string(y));
            foundAccurateValues = false;
        }

        if (netEnergy > 0) {
            meleeSapEnergies.push_back(netEnergy);
        }
    }

    return foundAccurateValues;
}

void ShuttleEnergyTracker::getPossibleDirectRangedSappingUnitsNearby(ShuttleData& shuttle,
                                     std::unordered_set<int>& opponentShuttlesDirect,
                                     std::unordered_set<int>& opponentShuttlesIndirect) {

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    auto& probabilities = opponentTracker.getOpponentPreviousPositionProbabilities();

    log("Computing sapping possibilities for shuttle - " + std::to_string(shuttle.id));

    for (int x = shuttle.getX() - gameEnvConfig.unitSapRange - 1; x <= shuttle.getX() + gameEnvConfig.unitSapRange + 1; ++x) {
        for (int y = shuttle.getY() - gameEnvConfig.unitSapRange - 1; y <= shuttle.getY() + gameEnvConfig.unitSapRange + 1; ++y) {
            if (!gameMap.isValidTile(x, y)) {
                continue;
            }

            // GameTile& tile = gameMap.getTile(x, y);

            for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
                if (probabilities[s][x][y] > LOWEST_DOUBLE) {
                    // if (x != shuttle.getX() + gameEnvConfig.unitSapRange + 1 
                    //     && x != shuttle.getX() - gameEnvConfig.unitSapRange - 1 
                    //     && y != shuttle.getY() + gameEnvConfig.unitSapRange + 1 
                    //     && y != shuttle.getY() - gameEnvConfig.unitSapRange - 1) {

                                                
                    // }
                    // log("Adding " + std::to_string(s) + " to direct sapping set");
                    opponentShuttlesDirect.insert(s);
                    opponentShuttlesIndirect.insert(s);
                    
                }
            }
            
        }
    }
    log("After resolving direct sap for shuttle - " + std::to_string(shuttle.id) + " - Direct sap size - " + std::to_string(opponentShuttlesDirect.size()) + " - Indirect sap size - " + std::to_string(opponentShuttlesIndirect.size()));
}

bool ShuttleEnergyTracker::attemptResolution(ShuttleData& shuttle) {
    log("Attempting to resolve energy for " + std::to_string(shuttle.id));

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    ShuttleEnergyChangeDistribution distribution;
    bool resolved = false;

    if (shuttle.hasMoved()) {
        distribution.moveCost = gameEnvConfig.unitMoveCost;
    } else {
        distribution.moveCost = 0;
    }

    GameTile& currentTile = gameMap.getTile(shuttle.getX(), shuttle.getY());
    distribution.tileEnergy = currentTile.getLastKnownEnergy();

    for (auto& stackedShuttle: currentTile.shuttles) {
        if (stackedShuttle->previouslyVisible && stackedShuttle->id != shuttle.id) {
            distribution.unitStackCount++;
        }
    }

    for (auto& stackedShuttle: currentTile.ghostShuttles) {
        if (stackedShuttle->previouslyVisible && stackedShuttle->id != shuttle.id) {
            distribution.unitStackCount++;
        }
    }

    std::vector<int> nebulaEnergyReductionSet;

    if (gameMap.getEstimatedType(currentTile, state.currentStep - 1) == TileType::NEBULA) {
        log("Shuttle was in nebula " + std::to_string(shuttle.id));
        nebulaEnergyReductionSet.assign(nebulaTileEnergyReduction.begin(), nebulaTileEnergyReduction.end());
    } else {
        nebulaEnergyReductionSet.push_back(0);        
    }

    if (gameMap.getEstimatedType(currentTile, state.currentStep - 1) == TileType::UNKNOWN_TILE) {
        log("Cant do accurate results, shuttle was in unknown tile " + std::to_string(shuttle.id));
        distribution.accurateResults = false;
    }

    bool canPredictAccurateMeleeEnergy = getPossibleMeleeSappingEnergyNearby(shuttle, distribution.meleeSapEnergies);
    if (!canPredictAccurateMeleeEnergy) {
        distribution.accurateResults = false;
    }
    std::unordered_set<int> directSappingOpponents;
    std::unordered_set<int> indirectSappingOpponents;    
    
    getPossibleDirectRangedSappingUnitsNearby(shuttle,
         directSappingOpponents, indirectSappingOpponents);

    int directSappingOpponentsSize = directSappingOpponents.size() + 1;
    int indirectSappingOpponentsSize = indirectSappingOpponents.size() + 1;

    std::vector<float> meleeSapEnergyVoidFactorSet = meleeSapEnergyVoidFactor;
    std::vector<float> rangedIndirectSapEnergyDropoffFactorSet = rangedIndirectSapEnergyDropoffFactor;

    if (directSappingOpponentsSize == 0) {
        directSappingOpponentsSize = 1;
    }

    if (indirectSappingOpponentsSize == 0) {
        indirectSappingOpponentsSize = 1;
        rangedIndirectSapEnergyDropoffFactorSet.clear();
        rangedIndirectSapEnergyDropoffFactorSet.push_back(1.0);
    }    

    if (distribution.meleeSapEnergies.size() == 0) {
        meleeSapEnergyVoidFactorSet.clear();
        meleeSapEnergyVoidFactorSet.push_back(1.0);
    }

    log("Iteration size -> " + std::to_string(nebulaEnergyReductionSet.size()) + ", " + std::to_string(directSappingOpponentsSize) + ", " + std::to_string(indirectSappingOpponentsSize));
    log("drop off factor size -> " + std::to_string(rangedIndirectSapEnergyDropoffFactorSet.size()) + ", " + std::to_string(meleeSapEnergyVoidFactorSet.size()));
    int numberOfSolutions = 0;

    std::unordered_set<int> possibleNebulaEnergyReduction;
    std::unordered_set<float> possibleMeleeSapEnergyVoidFactor;
    std::unordered_set<float> possibleRangedIndirectSapEnergyDropoffFactor;

    for (int nebulaEnergyReduction: nebulaEnergyReductionSet) {
        distribution.nebulaEnergyReduction = nebulaEnergyReduction;        

        for (int sapRangedIdx = 0; sapRangedIdx < directSappingOpponentsSize; sapRangedIdx ++) {
            distribution.rangedDirectSapCount = sapRangedIdx;            
            
            for (int sapIndirectRangedIdx = 0; sapIndirectRangedIdx < indirectSappingOpponentsSize; sapIndirectRangedIdx ++) {

                for (int dropOffFactorId = 0; dropOffFactorId < rangedIndirectSapEnergyDropoffFactorSet.size(); dropOffFactorId++) {                    
                    float dropOffFactor = rangedIndirectSapEnergyDropoffFactorSet[dropOffFactorId];

                    if (sapIndirectRangedIdx == 0) {
                        dropOffFactor = 1.0;
                        dropOffFactorId = rangedIndirectSapEnergyDropoffFactorSet.size(); //Ending the loop
                    }
                // for (float dropOffFactor: rangedIndirectSapEnergyDropoffFactorSet) {
    
                    distribution.rangedIndirectSapDropOffFactor = dropOffFactor;
                    distribution.rangedIndirectSapCount = sapIndirectRangedIdx;
                    
                    for (float meleeSapEnergyVoidFactor: meleeSapEnergyVoidFactorSet) {                        
                        
                        distribution.meleeEnergyVoidFactor = meleeSapEnergyVoidFactor;

                        int energyAfterMovement = distribution.computeEnergy(shuttle.previousEnergy);

                        if (energyAfterMovement == shuttle.energy) {

                            if (!resolved) {
                                //Add metrics from 1st solution.  It doesn't matter if there could be multiple possibilities!
                                energyLostInMovements += distribution.moveCost;
                                energyLostInEnergyFields += distribution.tileEnergy;
                                energyLostInRangedSap += distribution.rangedDirectSapCount + distribution.rangedIndirectSapCount;
                                energyLostInMeleeSap += distribution.computedMeleeSapEnergy;
                                energyLostInNebula += distribution.nebulaEnergyReduction;
                            }
                            resolved = true;
                            log("Solution found -> " + distribution.toString());
                            numberOfSolutions++;

                            if (gameMap.getEstimatedType(currentTile, state.currentStep - 1) == TileType::NEBULA) {
                                possibleNebulaEnergyReduction.insert(distribution.nebulaEnergyReduction);
                            }
                            if (distribution.computedMeleeSapEnergy > 0) {
                                possibleMeleeSapEnergyVoidFactor.insert(distribution.meleeEnergyVoidFactor);
                            }
                            if (distribution.rangedIndirectSapCount > 0 && distribution.rangedIndirectSapDropOffFactor < 1.0) { // 1.0 cannot be identified this way
                                possibleRangedIndirectSapEnergyDropoffFactor.insert(distribution.rangedIndirectSapDropOffFactor);
                            }
                        } else {
                            // if (state.currentStep == 41 || state.currentStep == 61) {
                            //     log("Shuttle resolution failed - " + std::to_string(shuttle.id));
                            //     log("Nebula? " + std::to_string(currentTile.getPreviousTypeImmediate()) + " previousNebula? " + std::to_string(gameMap.getEstimatedType(currentTile, state.currentStep - 1)));
                            //     log("Solution not found -> " + distribution.toString());
                            //     log("Energy difference -> " + std::to_string(shuttle.energy) + " vs " + std::to_string(energyAfterMovement) + ", starting at " + std::to_string(shuttle.previousEnergy));
                            // }
                        }
                    }
                }
            }
        }
    }

    if (resolved && distribution.accurateResults ) {       
        if (!possibleNebulaEnergyReduction.empty()) {
            std::unordered_set<int> toBePruned;
            
            for (int value : nebulaTileEnergyReduction) {
                if (possibleNebulaEnergyReduction.find(value) == possibleNebulaEnergyReduction.end()) {
                    toBePruned.insert(value);
                }
            }
            
            nebulaTileEnergyReduction.erase(std::remove_if(nebulaTileEnergyReduction.begin(), nebulaTileEnergyReduction.end(), [&toBePruned](int x) {
                return toBePruned.find(x) != toBePruned.end();
            }), nebulaTileEnergyReduction.end());
            
            if (nebulaTileEnergyReduction.empty()) {
                log("Problem: Identified wrong value for nebula energy reduction");
                std::cerr<<"Problem: Identified wrong value for nebula energy reduction"<<std::endl;
            }

            if (nebulaTileEnergyReduction.size() == 1) {                
                state.nebulaTileEnergyReduction = nebulaTileEnergyReduction[0];
                state.nebulaTileEnergyReductionSet = true;
                log("Resolved nebula energy reduction to " + std::to_string(state.nebulaTileEnergyReduction));
            } else {
                log("Current nebula energy reduction values -> " + vectorToString(nebulaTileEnergyReduction, "nebulaTileEnergyReduction"));
            }
        }
        
        
        if (!possibleMeleeSapEnergyVoidFactor.empty()) {
            std::unordered_set<float> toBePruned;
            for (float factor : meleeSapEnergyVoidFactor) {
                if (possibleMeleeSapEnergyVoidFactor.find(factor) == possibleMeleeSapEnergyVoidFactor.end()) {
                    toBePruned.insert(factor);
                }
            }

            meleeSapEnergyVoidFactor.erase(std::remove_if(meleeSapEnergyVoidFactor.begin(), meleeSapEnergyVoidFactor.end(), [&toBePruned](float x) {
                return toBePruned.find(x) != toBePruned.end();
            }), meleeSapEnergyVoidFactor.end());

            if (meleeSapEnergyVoidFactor.empty()) {
                log("Problem: Identified wrong value for melee void factor");
                std::cerr<<"Problem: Identified wrong value for melee void factor"<<std::endl;
            }

            if (meleeSapEnergyVoidFactor.size() == 1) {
                state.unitSapDropOffFactor = meleeSapEnergyVoidFactor[0];
                state.unitSapDropOffFactorSet = true;
                log("Resolved meleeSapEnergyVoidFactor to " + std::to_string(state.unitSapDropOffFactor));
            } else {
                log("Current meleeSapEnergyVoidFactor values -> " + vectorToString(meleeSapEnergyVoidFactor, "meleeSapEnergyVoidFactor"));
            }
        }
        
        if (!possibleRangedIndirectSapEnergyDropoffFactor.empty()) {
            std::unordered_set<float> toBePruned;
            for (float factor : rangedIndirectSapEnergyDropoffFactor) {
                if (possibleRangedIndirectSapEnergyDropoffFactor.find(factor) == possibleRangedIndirectSapEnergyDropoffFactor.end()) { // 1.0 cannot be identified by this method
                    toBePruned.insert(factor);
                }
            }
            rangedIndirectSapEnergyDropoffFactor.erase(std::remove_if(rangedIndirectSapEnergyDropoffFactor.begin(), rangedIndirectSapEnergyDropoffFactor.end(), [&toBePruned](float x) {
                return toBePruned.find(x) != toBePruned.end();
            }), rangedIndirectSapEnergyDropoffFactor.end());
            
            
            if (rangedIndirectSapEnergyDropoffFactor.empty()) {
                log("Problem: Identified wrong value for rangedIndirectSapEnergyDropoffFactor");
                std::cerr<<"Problem: Identified wrong value for rangedIndirectSapEnergyDropoffFactor"<<std::endl;
            }

            if (rangedIndirectSapEnergyDropoffFactor.size() == 1) {
                state.unitEnergyVoidFactor = rangedIndirectSapEnergyDropoffFactor[0];
                state.unitEnergyVoidFactorSet = true;
                log("Resolved rangedIndirectSapEnergyDropoffFactor to " + std::to_string(state.unitEnergyVoidFactor));
            } else {
                log("Current rangedIndirectSapEnergyDropoffFactor values -> " + vectorToString(rangedIndirectSapEnergyDropoffFactor, "rangedIndirectSapEnergyDropoffFactor"));
            }
        }
    }

    // if (!resolved && distribution.accurateResults) {
    //     log("Problem: Unable to account for all the energy changes for shuttle" + std::to_string(shuttle.id));
    //     std::cerr<<"Problem: Unable to account for all the energy changes"<<std::endl;
    // }

    log("Number of solutions found -> " + std::to_string(numberOfSolutions));
    return resolved;
}

void ShuttleEnergyTracker::step() {
    auto start = std::chrono::high_resolution_clock::now();

    energyLostInMovements = 0;
    energyLostInEnergyFields = 0;
    energyLostInRangedSap = 0;
    energyLostInMeleeSap = 0;
    energyLostInNebula = 0;

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    preparePlayerCollisions();
    prepareOpponentCollisionMap();

    for (int s = 0; s< gameEnvConfig.maxUnits;++s) {

        auto& shuttle = gameMap.shuttles[s];

        if (shuttle->visible && shuttle->ghost || !shuttle->visible && shuttle->previouslyVisible) {
            respawnRegistry.pushPlayerUnit(s, state.currentMatchStep);
        }        

        if (shuttle->energy == UNIT_SPAWN_ENERGY && shuttle->visible && shuttle->previouslyVisible && shuttle->getX() == gameEnvConfig.originX && shuttle->getY() == gameEnvConfig.originY
            && shuttle->getPreviousX() !=  gameEnvConfig.originX && shuttle->getPreviousY() !=  gameEnvConfig.originY && respawnRegistry.playerUnitRespawned != s) {
            // This can be a false positive but extremely rare.
            log("Identified collision that happened at spawn for shuttle " + std::to_string(s));
            int actualUnitExpectedToSpawn = respawnRegistry.playerUnitRespawned;
            if (actualUnitExpectedToSpawn != -1) {
                respawnRegistry.pushPlayerUnit(actualUnitExpectedToSpawn, state.currentMatchStep);
            }

            respawnRegistry.playerUnitRespawned = s;
        }
        
    }

    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        auto& shuttle = gameMap.shuttles[s];        

        if (!shuttle->visible) {
            //TODO: invisible tiles ideally wont affect anything.  Verify if we need to add anything here
            continue;
        }
        
        if (respawnRegistry.playerUnitRespawned == s) {
            if (shuttle->energy != UNIT_SPAWN_ENERGY) {
                log("Problem:  Unit " + std::to_string(s) +" just spawned but energy mismatch - " + std::to_string(shuttle->energy));
                std::cerr<<"Problem:  Unit just spawned but energy mismatch"<<std::endl;
            }

            if (shuttle->getPreviousX() != -1 || shuttle->getPreviousY() != -1) {
                log("shuttle " + std::to_string(s) + " just spawned, but previous position is not (-1,-1) instead (" + std::to_string(shuttle->getPreviousX()) + "," + std::to_string(shuttle->getPreviousY()) + ")");
                // std::cerr<<"Problem, shuttle just spawned, but previous position is not (-1,-1)"<<std::endl;
            }

            if (shuttle->getX() != gameEnvConfig.originX || shuttle->getY() != gameEnvConfig.originY) {
                log("shuttle " + std::to_string(s) + " just spawned, but current position is not (" + std::to_string(gameEnvConfig.originX) + "," + std::to_string(gameEnvConfig.originY) + ") instead (" + std::to_string(shuttle->getX()) + "," + std::to_string(shuttle->getY()) + ")");
                std::cerr<<"Problem, shuttle just spawned, but current position is not (org, org)"<<std::endl;
            }

            Metrics::getInstance().add("energy_gain", shuttle->energy);
        } else {

            GameTile& currentTile = gameMap.getTile(shuttle->getX(), shuttle->getY());
            ShuttleEnergyChangeDistribution distribution;

            attemptResolution(*shuttle);

        }
    }

    Metrics::getInstance().add("movement_loss", energyLostInMovements);
    Metrics::getInstance().add("energy_fields", energyLostInEnergyFields);
    Metrics::getInstance().add("sap_loss", energyLostInRangedSap);
    Metrics::getInstance().add("melee_loss", energyLostInMeleeSap);
    Metrics::getInstance().add("nebula_loss", energyLostInNebula);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("shuttle_energy_tracking", duration.count());
}

void ShuttleEnergyTracker::updateShuttleActions(std::vector<std::vector<int>>& actions) {
    shuttlesThatSappedLastTurn.clear();

    for (int s = 0; s < actions.size(); ++s) {
        if (actions[s][0] == 5) {
            shuttlesThatSappedLastTurn.insert(s);
        }
    }

}

ShuttleEnergyTracker::ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry)
            : gameMap(gameMap), opponentTracker(opponentTracker), respawnRegistry(respawnRegistry) {


    nebulaTileEnergyReduction = std::vector<int>(std::begin(POSSIBLE_NEBULA_ENERGY_REDUCTION_VALUES), std::end(POSSIBLE_NEBULA_ENERGY_REDUCTION_VALUES));
    meleeSapEnergyVoidFactor = std::vector<float>(std::begin(POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES), std::end(POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES));
    rangedIndirectSapEnergyDropoffFactor = std::vector<float>(std::begin(POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES), std::end(POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES));

}