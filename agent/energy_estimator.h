#ifndef ENERGY_ESTIMATOR_H
#define ENERGY_ESTIMATOR_H

#include "agent/game_map.h"
#include <unordered_set>

enum EnergyDriftStatus {
    UNKNOWN_ENERGY_DRIFT,
    NO_ENERGY_DRIFT,
    FOUND_ENERGY_DRIFT
};

class EnergyEstimator {
    private:        
        GameMap& gameMap;

        std::map<int, EnergyDriftStatus> driftSpeedToStatusMap;

        double energyNodeFns[6][4] = {
            {0, 1.2, 1, 4},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            // {1, 4, 0, 2},
            {0, 1.2, 1, 4},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
            // {1, 4, 0, 0}
        };

        static void log(std::string message);
    
    public:
        int finalEnergyDriftSpeed = -1;
        EnergyEstimator(GameMap& gameMap);

        void getPossibleDrifts(int step, std::unordered_set<int> &possibleDrifts);

        void estimate(std::vector<int> energyNodeTileIds);

        void reportEnergyDrift(GameTile& tile);
};

#endif  // ENERGY_ESTIMATOR_H