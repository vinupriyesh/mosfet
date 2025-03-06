#ifndef SHUTTLE_ENERGY_TRACKER_H
#define SHUTTLE_ENERGY_TRACKER_H

#include "game_map.h"
#include "opponent_tracker.h"
#include "datastructures/respawn_registry.h"
#include <unordered_set>

struct ShuttleEnergyChangeDistribution {

    int moveCost = 0;
    int tileEnergy = 0;
    int nebulaEnergyReduction = 0;
    int meleeSap = 0;
    int rangedDirectSap = 0;
    int rangedIndirectSap = 0;

    bool nebulaEnergyReductionFinalized = true;
    bool meleeEnergyVoidFactorFinalized = true;

    int computeEnergy(int previousEnergy) {

        int energyBeforeTileInclusion = previousEnergy - moveCost - nebulaEnergyReduction - meleeSap - rangedDirectSap - rangedIndirectSap;

        if (energyBeforeTileInclusion < 0 && (meleeSap >0 || rangedDirectSap >0 || rangedIndirectSap >0)) {
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

    std::string toString() {
        std::string result = "ShuttleEnergyChangeDistribution: ";
        result += "moveCost=" + std::to_string(moveCost) + ", ";
        result += "tileEnergy=" + std::to_string(tileEnergy) + ", ";
        result += "nebulaEnergyReduction=" + std::to_string(nebulaEnergyReduction) + ", ";
        result += "meleeSap=" + std::to_string(meleeSap) + ", ";
        result += "rangedDirectSap=" + std::to_string(rangedDirectSap) + ", ";
        result += "rangedIndirectSap=" + std::to_string(rangedIndirectSap) + ", ";
        result += "nebulaEnergyReductionFinalized=" + std::to_string(nebulaEnergyReductionFinalized) + ", ";
        result += "meleeEnergyVoidFactorFinalized=" + std::to_string(meleeEnergyVoidFactorFinalized);
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

        void tryResolvingNebulaTileEnergyReduction(GameTile& currentTile, ShuttleData& shuttle, int energyChange);

        bool resolveMovementEnergyLoss(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        bool resolveMeleeSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        bool resolveRangedDirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        bool resolveRangedIndirectSap(ShuttleData& shuttle, ShuttleEnergyChangeDistribution& distribution);
        
    public:        
        ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry)
            : gameMap(gameMap), opponentTracker(opponentTracker), respawnRegistry(respawnRegistry) {}
        
        void step();
        void updateShuttleActions(std::vector<std::vector<int>>& actions);
}; 

#endif // SHUTTLE_ENERGY_TRACKER_H