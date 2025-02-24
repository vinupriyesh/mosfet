#ifndef OPPONENT_TRACKER_H
#define OPPONENT_TRACKER_H

#include "game_map.h"
#include "datastructures/respawn_registry.h"

class OpponentTracker {
    private:
        GameMap& gameMap;
        RespawnRegistry& respawnRegistry;

        std::vector<std::vector<std::vector<double>>> opponentPositionProbabilities; // (16, 24, 24)

        void log(std::string message);        
    public:
        OpponentTracker(GameMap& gameMap, RespawnRegistry respawnRegistry);
        void step();
};

#endif // OPPONENT_TRACKER_H