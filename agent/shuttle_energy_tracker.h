#ifndef SHUTTLE_ENERGY_TRACKER_H
#define SHUTTLE_ENERGY_TRACKER_H

#include "game_env_config.h"
#include "game_map.h"
#include "opponent_tracker.h"
#include "datastructures/respawn_registry.h"
#include <unordered_set>
#include <cmath>

#include "datastructures/range.h"

struct ShuttleEnergyChangeDistribution {

    int moveCost = 0;
    int tileEnergy = 0;
    int nebulaEnergyReduction = 0;
    int meleeSapEnergy = 0;
    int rangedDirectSapCount = 0;
    int rangedIndirectSapCount = 0;
    float rangedIndirectSapDropOffFactor = 0.0;
    float meleeEnergyVoidFactor = 0.0;

    bool nebulaEnergyReductionFinalized = true;
    bool meleeEnergyVoidFactorFinalized = true;

    int computeEnergy(int previousEnergy) {

        GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

        int energyBeforeTileInclusion = previousEnergy - moveCost - nebulaEnergyReduction
                                - static_cast<int>(std::floor(meleeSapEnergy * meleeEnergyVoidFactor)) 
                                - rangedDirectSapCount * gameEnvConfig.unitSapCost 
                                - rangedIndirectSapCount * static_cast<int>(std::floor(gameEnvConfig.unitSapCost * rangedIndirectSapDropOffFactor));

        if (energyBeforeTileInclusion < 0 && (meleeSapEnergy >0 || rangedDirectSapCount >0 || rangedIndirectSapCount >0)) {
            // This is an attack, return if the shuttle is dead already
            return energyBeforeTileInclusion;
        }

        if (energyBeforeTileInclusion + tileEnergy < 0) {
            // This tile was not dead during the attack.  So it cant die now
            return 0;
        }

        return energyBeforeTileInclusion + tileEnergy;
    }

    int computeEnergyForNebulaResolution(int previousEnergy) {
        return moveCost + tileEnergy;
    }

    Range inverseComputeMeleeDropOffEnergy(int previousEnergy, int currentEnergy) {

        GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

        int rangedIndirectSapCostFloor = static_cast<int>(std::floor(gameEnvConfig.unitSapCost * rangedIndirectSapDropOffFactor));
    
        // Compute T:
        int T = previousEnergy - moveCost - nebulaEnergyReduction 
                - rangedDirectSapCount * gameEnvConfig.unitSapCost 
                - rangedIndirectSapCount * rangedIndirectSapCostFloor;
        
        // Compute K:
        int K = T - currentEnergy; // K must be an integer.
        
        // Calculate the lower and upper bounds for meleeSapEnergy using the interval
        double lowerBound = K / meleeEnergyVoidFactor;
        double upperBound = (K + 1) / meleeEnergyVoidFactor;

        return Range(lowerBound, upperBound);
    }

    std::string toString() {
        std::string result = "ShuttleEnergyChangeDistribution: ";
        result += "moveCost=" + std::to_string(moveCost) + ", ";
        result += "tileEnergy=" + std::to_string(tileEnergy) + ", ";
        result += "nebulaEnergyReduction=" + std::to_string(nebulaEnergyReduction) + ", ";
        result += "meleeSap=" + std::to_string(meleeSapEnergy) + ", ";
        result += "rangedDirectSap=" + std::to_string(rangedDirectSapCount) + ", ";
        result += "rangedIndirectSap=" + std::to_string(rangedIndirectSapCount) + ", ";
        result += "rangedIndirectSapDropOffFactor=" + std::to_string(rangedIndirectSapDropOffFactor) + ", ";
        result += "meleeEnergyVoidFactor=" + std::to_string(meleeEnergyVoidFactor) + ", ";        
        // result += "nebulaEnergyReductionFinalized=" + std::to_string(nebulaEnergyReductionFinalized) + ", ";
        // result += "meleeEnergyVoidFactorFinalized=" + std::to_string(meleeEnergyVoidFactorFinalized);
        return result;
    }
};

class ShuttleEnergyTracker {    

    private:
        static void log(const std::string& message);
        GameMap& gameMap;
        OpponentTracker& opponentTracker;
        RespawnRegistry& respawnRegistry;
    
        std::unordered_set<int> shuttlesThatSappedLastTurn;

        std::vector<int> nebulaTileEnergyReduction;
        std::vector<float> meleeSapEnergyVoidFactor;
        std::vector<float> rangedIndirectSapEnergyDropoffFactor;

        // void tryResolvingNebulaTileEnergyReduction(GameTile& currentTile, ShuttleData& shuttle, int energyChange);

        // bool resolveMovementEnergyLoss(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        // bool resolveMeleeSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        // bool resolveRangedDirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        // bool resolveRangedIndirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);

        Range getPossibleMeleeSappingEnergyNearby(ShuttleData& shuttle);
        void getPossibleDirectRangedSappingUnitsNearby(ShuttleData& shuttle,
            std::unordered_set<int>& opponentShuttlesDirect,
            std::unordered_set<int>& opponentShuttlesIndirect);

        bool attemptResolution(ShuttleData& shuttle);
        
    public:        
        ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry);
        
        void step();
        void updateShuttleActions(std::vector<std::vector<int>>& actions);
}; 

#endif // SHUTTLE_ENERGY_TRACKER_H