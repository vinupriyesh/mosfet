#include "shuttle_energy_tracker.h"
#include "metrics.h"
#include "logger.h"
#include "game_env_config.h"
#include "constants.h"
#include <iostream>

void ShuttleEnergyTracker::log(const std::string& message) {
    Logger::getInstance().log("ShuttleEnergyTracker -> " + message);
}

void ShuttleEnergyTracker::step() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
        auto& shuttle = gameMap.shuttles[s];

        // Just spawned
        if (respawnRegistry.getPlayerUnitSpawnStep(s) == state.currentStep) {
            if (shuttle->energy != UNIT_SPAWN_ENERGY) {
                log("Problem:  Unit just spawned but energy mismatch - " + std::to_string(shuttle->energy));
                std::cerr<<"Problem:  Unit just spawned but energy mismatch"<<std::endl;
            }

            Metrics::getInstance().add("energy_gain_spawn", shuttle->energy);
        } else {

            //Move Cost
            

        }
        
    }
}