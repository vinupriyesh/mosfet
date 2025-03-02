#ifndef SHUTTLE_ENERGY_TRACKER_H
#define SHUTTLE_ENERGY_TRACKER_H

#include "game_map.h"
#include "opponent_tracker.h"
#include "datastructures/respawn_registry.h"

class ShuttleEnergyTracker {    

    private:
        static void log(const std::string& message);
        GameMap& gameMap;
        OpponentTracker& opponentTracker;
        RespawnRegistry& respawnRegistry;
    
    public:
        ShuttleEnergyTracker(GameMap& gameMap, OpponentTracker& opponentTracker, RespawnRegistry& respawnRegistry)
            : gameMap(gameMap), opponentTracker(opponentTracker), respawnRegistry(respawnRegistry) {}
        
        void step();
}; 

#endif // SHUTTLE_ENERGY_TRACKER_H