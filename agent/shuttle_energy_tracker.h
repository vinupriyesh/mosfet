#ifndef SHUTTLE_ENERGY_TRACKER_H
#define SHUTTLE_ENERGY_TRACKER_H

#include "game_map.h"
#include "opponent_tracker.h"
#include "datastructures/respawn_registry.h"
#include <unordered_set>

class ShuttleEnergyTracker {    

    private:
        static void log(const std::string& message);
        GameMap& gameMap;
        OpponentTracker& opponentTracker;
        RespawnRegistry& respawnRegistry;
    
        std::unordered_set<int> shuttlesThatSappedLastTurn;

        void tryResolvingNebulaTileEnergyReduction(GameTile& currentTile, ShuttleData& shuttle, int& energyChange);

        int resolveMovementEnergyLoss(ShuttleData& shuttle);        
        int resolveMeleeSap(ShuttleData& shuttle, int moveCost);
        int resolveRangedDirectSap(ShuttleData& shuttle, int energyLoss);
        int resolveRangedIndirectSap(ShuttleData& shuttle, int energyLoss);
        
    public:        
        ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry)
            : gameMap(gameMap), opponentTracker(opponentTracker), respawnRegistry(respawnRegistry) {}
        
        void step();
        void updateShuttleActions(std::vector<std::vector<int>>& actions);
}; 

#endif // SHUTTLE_ENERGY_TRACKER_H