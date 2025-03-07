#ifndef SHUTTLE_ENERGY_TRACKER_H
#define SHUTTLE_ENERGY_TRACKER_H

#include "game_env_config.h"
#include "game_map.h"
#include "opponent_tracker.h"
#include "datastructures/respawn_registry.h"
#include <unordered_set>
#include <cmath>
#include "constants.h"

struct ShuttleEnergyChangeDistribution {

    int unitStackCount = 1;
    int moveCost = 0;
    int tileEnergy = 0;
    int nebulaEnergyReduction = 0;
    std::vector<int> meleeSapEnergies;
    int rangedDirectSapCount = 0;
    int rangedIndirectSapCount = 0;
    float rangedIndirectSapDropOffFactor = 0.0;
    float meleeEnergyVoidFactor = 0.0;

    bool accurateResults = true;

    int computedMeleeSapEnergy = -5000;

    int computeSapContributions() {
        double result = 0;
        for (int energy: meleeSapEnergies) {
            if (energy > 0) {
                // result += static_cast<int>(std::floor(energy * meleeEnergyVoidFactor));
                result += energy * meleeEnergyVoidFactor;
            }
        }
        
        int floorResult = static_cast<int>(std::floor(result));

        computedMeleeSapEnergy = floorResult;
        return floorResult;        
    }

    bool isAttack(int meleeSapEnergy) {        
        return meleeSapEnergy > 0 || rangedDirectSapCount > 0 || rangedIndirectSapCount > 0;
    }

    int computeEnergy(int previousEnergy) {

        GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

        int meleeSapEnergy = computeSapContributions();

        int computedEnergy = previousEnergy - moveCost                                 
                                - rangedDirectSapCount * gameEnvConfig.unitSapCost 
                                - rangedIndirectSapCount * static_cast<int>(std::floor(gameEnvConfig.unitSapCost * rangedIndirectSapDropOffFactor));

        // if (computedEnergy < 0 && isAttack(meleeSapEnergy)) {
        //     // Unit is not alive anymore
        //     return computedEnergy;
        // }

        computedEnergy -= meleeSapEnergy / unitStackCount;

        int energyGain = tileEnergy - nebulaEnergyReduction;

        if (computedEnergy < 0 && computedEnergy + energyGain < 0 && isAttack(meleeSapEnergy)) {
            // This is an attack, so the energy can go to negative  
            return computedEnergy;
        }

        computedEnergy += energyGain;

        if (computedEnergy < 0) {
            // This is not an attack, so the energy can only go to 0
            return 0;
        }

        if (computedEnergy > MAX_ENERGY) {
            return MAX_ENERGY;
        }

        return computedEnergy;
    }

    std::string toString() {
        std::string result = "ShuttleEnergyChangeDistribution: ";
        result += "moveCost=" + std::to_string(moveCost) + ", ";
        result += "tileEnergy=" + std::to_string(tileEnergy) + ", ";
        result += "nebulaEnergyReduction=" + std::to_string(nebulaEnergyReduction) + ", ";
        result += "meleeSap=" + std::to_string(computedMeleeSapEnergy) + ", ";
        result += "meleeSapCount=" + std::to_string(meleeSapEnergies.size()) + ", ";
        result += "rangedDirectSap=" + std::to_string(rangedDirectSapCount) + ", ";
        result += "rangedIndirectSapCount=" + std::to_string(rangedIndirectSapCount) + ", ";
        result += "rangedIndirectSapDropOffFactor=" + std::to_string(rangedIndirectSapDropOffFactor) + ", ";
        result += "meleeEnergyVoidFactor=" + std::to_string(meleeEnergyVoidFactor) + ", ";        
        result += "unitStackCount=" + std::to_string(unitStackCount) + ", ";
        result += "accurateResults=" + std::to_string(accurateResults);        
        return result;
    }
};

class ShuttleEnergyTracker {    

    private:
        static void log(const std::string& message);
        template<typename T>
        std::string vectorToString(const std::vector<T>& vec, const std::string& name);

        GameMap& gameMap;
        OpponentTracker& opponentTracker;
        RespawnRegistry& respawnRegistry;
    
        std::unordered_set<int> shuttlesThatSappedLastTurn;

        std::vector<int> nebulaTileEnergyReduction;
        std::vector<float> meleeSapEnergyVoidFactor;
        std::vector<float> rangedIndirectSapEnergyDropoffFactor;

        std::unordered_map<int, std::vector<int>> confirmedCollisions;
        std::unordered_map<int, std::vector<int>> possibleCollisions;

        std::unordered_set<int> playerCollisions;

        void prepareOpponentCollisionMap();
        void preparePlayerCollisions();

        bool getPossibleMeleeSappingEnergyNearby(ShuttleData& shuttle, std::vector<int>& meleeSapEnergies);
        void getPossibleDirectRangedSappingUnitsNearby(ShuttleData& shuttle,
            std::unordered_set<int>& opponentShuttlesDirect,
            std::unordered_set<int>& opponentShuttlesIndirect);

        bool attemptResolution(ShuttleData& shuttle);
        
        int energyLostInMovements;
        int energyLostInEnergyFields;
        int energyLostInRangedSap;
        int energyLostInMeleeSap;
        int energyLostInNebula;

    public:        
        ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry);
        
        void step();
        void updateShuttleActions(std::vector<std::vector<int>>& actions);
}; 

#endif // SHUTTLE_ENERGY_TRACKER_H