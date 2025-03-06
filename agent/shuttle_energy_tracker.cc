#include "shuttle_energy_tracker.h"
#include "agent/game_map.h"
#include "metrics.h"
#include "logger.h"
#include "game_env_config.h"
#include "constants.h"
#include <iostream>

void ShuttleEnergyTracker::log(const std::string& message) {
    Logger::getInstance().log("ShuttleEnergyTracker -> " + message);
}

/**
* Assuming this shuttle is not hit by any opponent shuttle, try to resolve the energy reduction due to nebula tile
* currentTile: Current tile the shuttle stands should be a Nebula tile.  
* shuttle: The shuttle
* energyAfterMovement: The energy change due to movement excluding the nebula tile energy reduction
*/
void ShuttleEnergyTracker::tryResolvingNebulaTileEnergyReduction(GameTile& currentTile, ShuttleData& shuttle, int energyAfterMovement){    
    if (shuttle.energy <= 0 || energyAfterMovement <=0) {
        // This shuttle is so dead, cant resolve the nebula tile energy reduction with it
        return;
    }

    int targetEnergy = energyAfterMovement - shuttle.energy;     

    if (targetEnergy == 0 || targetEnergy == 1 || targetEnergy == 2 || targetEnergy == 3 || targetEnergy == 5 || targetEnergy == 25) {
        //TODO: make sure to additionally check if there is no possibility of indirectly getting sapped at this tile. if so, do not set the flag to true (i.e. it is tentative, not finalized)
        gameMap.derivedGameState.nebulaTileEnergyReduction = targetEnergy;
        gameMap.derivedGameState.nebulaTileEnergyReductionSet = true;
        log("Resolved nebula tile energy reduction as " + std::to_string(gameMap.derivedGameState.nebulaTileEnergyReduction));
    }
}

bool ShuttleEnergyTracker::resolveMovementEnergyLoss(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    //Move Cost    
    GameTile& currentTile = gameMap.getTile(shuttle.getX(), shuttle.getY());
    if (shuttle.hasMoved()) {
        distribution.moveCost = gameEnvConfig.unitMoveCost;
    }

    distribution.tileEnergy = currentTile.getLastKnownEnergy();

    if (gameMap.getEstimatedType(currentTile, state.currentStep - 1) == TileType::NEBULA) {
        if (!state.nebulaTileEnergyReductionSet) {
            tryResolvingNebulaTileEnergyReduction(currentTile, shuttle, distribution.computeEnergyForNebulaResolution(shuttle.previousEnergy));
        }
        if (state.nebulaTileEnergyReductionSet) {
            distribution.nebulaEnergyReduction = state.nebulaTileEnergyReduction;
        } else {
            distribution.nebulaEnergyReduction = 0;
            distribution.nebulaEnergyReductionFinalized = false;
        }
    }

    int energyAfterMovement = distribution.computeEnergy(shuttle.previousEnergy);

    if (shuttle.energy > energyAfterMovement && shuttle.energy > 0) {
        log("Problem: Shuttle gained mysterious energy " + std::to_string(shuttle.id) + " - " + std::to_string(shuttle.energy) + " vs " + std::to_string(energyAfterMovement));
        log("Previous Energy: " + std::to_string(shuttle.previousEnergy) + ", hasMoved: " + std::to_string(shuttle.hasMoved()) + ", currentTileEnergy: " + std::to_string(currentTile.getLastKnownEnergy()));
        log("Is nebula: " + std::to_string(gameMap.getEstimatedType(currentTile, state.currentStep - 1) == TileType::NEBULA) + ", nebulaTileEnergyReductionSet: " + std::to_string(state.nebulaTileEnergyReductionSet) + ", nebulaTileEnergyReduction: " + std::to_string(state.nebulaTileEnergyReduction));
        std::cerr<<"Problem: Shuttle gained mysterious energy"<<std::endl;
    }

    return shuttle.energy == energyAfterMovement;
}


bool ShuttleEnergyTracker::resolveMeleeSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    if (!state.unitEnergyVoidFactorSet) {
        for (int i = 0; i <POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES_SIZE;i++) {
            int sappingPower = gameMap.getCumulativeOpponentMeleeSappingPowerAt(shuttle.getX(), shuttle.getY(), POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES[i]);
            distribution.meleeSap = sappingPower;
            if (distribution.computeEnergy(shuttle.previousEnergy) == shuttle.energy) {
                state.unitEnergyVoidFactor = POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES[i];
                state.unitEnergyVoidFactorSet = true;
                log("Resolved unit energy void factor as " + std::to_string(state.unitEnergyVoidFactor));
                break;
            }
        }
    }

    if (state.unitEnergyVoidFactorSet) {
        int sappingPower = gameMap.getCumulativeOpponentMeleeSappingPowerAt(shuttle.getX(), shuttle.getY(), state.unitEnergyVoidFactor);        
        distribution.meleeSap = sappingPower;
    } else {
        distribution.meleeSap = 0;
        distribution.meleeEnergyVoidFactorFinalized = false;
    }
    
    return distribution.computeEnergy(shuttle.previousEnergy) == shuttle.energy;
}

bool ShuttleEnergyTracker::resolveRangedDirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    //TODO:  Account for multiple Saps!  We need to know how many shuttles can possibly attack to get the exact count
    distribution.rangedDirectSap = gameEnvConfig.unitSapCost;    

    return distribution.computeEnergy(shuttle.previousEnergy) == shuttle.energy;
}

bool ShuttleEnergyTracker::resolveRangedIndirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    //TODO:  Account for multiple Saps!  We need to know how many shuttles can possibly attack to get the exact count

    if (!state.unitSapDropOffFactorSet) {
        for (int i = 0; i <POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES_SIZE;i++) {
            int sappingPower = gameEnvConfig.unitSapCost * POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES[i];
            distribution.rangedDirectSap = 0;
            distribution.rangedIndirectSap = sappingPower;
            if (distribution.computeEnergy(shuttle.previousEnergy) == shuttle.energy) {
                state.unitSapDropOffFactor = POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES[i];
                state.unitSapDropOffFactorSet = true;
                log("Resolved unit sap drop off factor as " + std::to_string(state.unitSapDropOffFactor));
                break;
            }
        }
    }

    return distribution.computeEnergy(shuttle.previousEnergy);
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

            // Check for movement
            bool resolved = resolveMovementEnergyLoss(*shuttle, distribution);            

            if (!resolved) {
                log("Movement not sufficent to tally " + distribution.toString());                
                resolved = resolveMeleeSap(*shuttle, distribution);                

                if (!resolved) {
                    log("Melee sap not sufficient to tally " + distribution.toString());
                    
                    resolved = resolveRangedDirectSap(*shuttle, distribution);
                    if (!resolved) {
                        log("Direct Sap not sufficient to tally " + distribution.toString());

                        resolved = resolveRangedIndirectSap(*shuttle, distribution);

                        if (!resolved) {
                            log("Problem: Indirect sap not sufficient to tally " + distribution.toString());
                            std::cerr<<"Problem: Unable to account for all the energy changes"<<std::endl;
                        }
                    }
                }
            }

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