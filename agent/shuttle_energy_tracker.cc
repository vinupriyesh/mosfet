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

    for (int x = shuttle.getX() - gameEnvConfig.unitSapRange - 1; x <= shuttle.getX() + gameEnvConfig.unitSapRange + 1; ++x) {
        for (int y = shuttle.getY() - gameEnvConfig.unitSapRange - 1; y <= shuttle.getY() + gameEnvConfig.unitSapRange + 1; ++y) {
            if (!gameMap.isValidTile(x, y)) {
                continue;
            }

            // GameTile& tile = gameMap.getTile(x, y);
            auto probabilities = opponentTracker.getOpponentPositionProbabilities();

            for (int s = 0; s<gameEnvConfig.maxUnits;s++) {
                if (probabilities[s][x][y] > LOWEST_DOUBLE) {
                    if (x != shuttle.getX() + gameEnvConfig.unitSapRange + 1 
                        && x != shuttle.getX() - gameEnvConfig.unitSapRange - 1 
                        && y != shuttle.getY() + gameEnvConfig.unitSapRange + 1 
                        && y != shuttle.getY() - gameEnvConfig.unitSapRange - 1) {

                        opponentShuttlesDirect.insert(s);                        
                    }

                    opponentShuttlesIndirect.insert(s);
                }
            }
            
        }
    }
}

bool ShuttleEnergyTracker::attemptResolution(ShuttleData& shuttle) {

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
        nebulaEnergyReductionSet.assign(nebulaTileEnergyReduction.begin(), nebulaTileEnergyReduction.end());
    } else {
        nebulaEnergyReductionSet.push_back(0);        
    }

    distribution.accurateResults = getPossibleMeleeSappingEnergyNearby(shuttle, distribution.meleeSapEnergies);
    std::unordered_set<int> directSappingOpponents;
    std::unordered_set<int> indirectSappingOpponents;    
    
    getPossibleDirectRangedSappingUnitsNearby(shuttle,
         directSappingOpponents, indirectSappingOpponents);

    int directSappingOpponentsSize = directSappingOpponents.size();
    int indirectSappingOpponentsSize = indirectSappingOpponents.size();

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
                            resolved = true;
                            log("Solution found -> " + distribution.toString());
                            numberOfSolutions++;
                        } else {
                            if (state.currentStep == 192) {
                                log("Shuttle resolution failed - " + std::to_string(shuttle.id));
                                log("Solution not found -> " + distribution.toString());
                                log("Energy difference -> " + std::to_string(shuttle.energy) + " vs " + std::to_string(energyAfterMovement));
                            }
                        }
                    }
                }
            }
        }
    }

    if (!resolved && distribution.accurateResults) {
        log("Problem: Unable to account for all the energy changes for shuttle" + std::to_string(shuttle.id));
        std::cerr<<"Problem: Unable to account for all the energy changes"<<std::endl;
    }

    log("Number of solutions found -> " + std::to_string(numberOfSolutions));
    return resolved;
}

void ShuttleEnergyTracker::step() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

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

            // // Check for movement
            // bool resolved = resolveMovementEnergyLoss(*shuttle, distribution);            

            // if (!resolved) {
            //     log("Movement not sufficent to tally " + distribution.toString());                
            //     resolved = resolveMeleeSap(*shuttle, distribution);                

            //     if (!resolved) {
            //         log("Melee sap not sufficient to tally " + distribution.toString());
                    
            //         resolved = resolveRangedDirectSap(*shuttle, distribution);
            //         if (!resolved) {
            //             log("Direct Sap not sufficient to tally " + distribution.toString());

            //             resolved = resolveRangedIndirectSap(*shuttle, distribution);

            //             if (!resolved) {
            //                 log("Problem: Indirect sap not sufficient to tally " + distribution.toString());
            //                 std::cerr<<"Problem: Unable to account for all the energy changes"<<std::endl;
            //             }
            //         }
            //     }
            // }

        }
    }
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