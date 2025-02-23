#ifndef ENERGY_ESTIMATOR_H
#define ENERGY_ESTIMATOR_H

#include "agent/game_map.h"

class EnergyEstimator {
    private:        
        GameMap& gameMap;

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
        EnergyEstimator(GameMap& gameMap) : gameMap(gameMap) {}

        void estimate(std::vector<int> energyNodeTileIds);

};

#endif  // ENERGY_ESTIMATOR_H