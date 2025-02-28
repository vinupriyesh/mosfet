#ifndef OPPONENT_TRACKER_H
#define OPPONENT_TRACKER_H

#include "game_map.h"
#include "datastructures/respawn_registry.h"

class OpponentTracker {
    private:
        GameMap& gameMap;
        RespawnRegistry& respawnRegistry;

        std::vector<std::vector<std::vector<double>>>* opponentPositionProbabilities; // (16, 24, 24)
        std::vector<std::vector<std::vector<int>>>* opponentMaxPossibleEnergies; // (16, 24, 24)

        void log(const std::string& message);
        void initArrays();
    public:
        OpponentTracker(GameMap& gameMap, RespawnRegistry& respawnRegistry);
        void step();

        std::vector<std::vector<std::vector<double>>>& getOpponentPositionProbabilities();
};

#endif // OPPONENT_TRACKER_H