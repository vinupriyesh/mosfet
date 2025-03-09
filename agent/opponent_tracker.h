#ifndef OPPONENT_TRACKER_H
#define OPPONENT_TRACKER_H

#include "game_map.h"
#include "datastructures/respawn_registry.h"

class OpponentTracker {
    private:
        GameMap& gameMap;
        RespawnRegistry& respawnRegistry;

        std::vector<std::vector<std::vector<double>>>* opponentPreviousPositionProbabilities; // (16, 24, 24)
        std::vector<std::vector<std::vector<int>>>* opponentPreviousMaxPossibleEnergies; // (16, 24, 24)

        std::vector<std::vector<std::vector<double>>>* opponentPositionProbabilities; // (16, 24, 24)
        std::vector<std::vector<std::vector<int>>>* opponentMaxPossibleEnergies; // (16, 24, 24)

        std::vector<std::vector<double>>* atleastOneShuttleProbabilities;

        void log(const std::string& message);
        void initArrays();

        void computeAtleastOneShuttleProbabilities();
    public:
        OpponentTracker(GameMap& gameMap, RespawnRegistry& respawnRegistry);
        void clear();
        void step();

        std::vector<std::vector<std::vector<double>>>& getOpponentPositionProbabilities();
        std::vector<std::vector<std::vector<int>>>& getOpponentMaxPossibleEnergies();

        std::vector<std::vector<std::vector<double>>>& getOpponentPreviousPositionProbabilities();
        std::vector<std::vector<std::vector<int>>>& getOpponentPreviousMaxPossibleEnergies();

        std::vector<std::vector<double>>& getAtleastOneShuttleProbabilities();

        bool isOpponentOccupied(int x, int y);
        double expectationOfOpponentOccupancy(int x, int y);

        int getAllPossibleEnergyAt(int x, int y);
        int getCountLessThanEnergyAt(int x, int y, int energy);

        ~OpponentTracker();
};

#endif // OPPONENT_TRACKER_H