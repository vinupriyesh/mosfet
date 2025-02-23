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
        int currentEnergyNode = -1;
        GameMap& gameMap;

        std::map<int, EnergyDriftStatus> driftSpeedToStatusMap;

        std::vector<std::vector<double>>* energyValuesBuffer;

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

        void getPossibleDrifts(int step, std::unordered_set<int> &possibleDrifts);

        bool estimate(int energyNodeTileId);
        void updateEstimatedEnergies();

        void clearEstimatedEnergies();

    public:
        int energyNodeDriftMagnitude = 6; //We cant find this, so using the max always
        int finalEnergyDriftSpeed = -1;
        EnergyEstimator(GameMap& gameMap);
        ~EnergyEstimator();
        void reportEnergyDrift(GameTile& tile);
        void updateEnergyNodes();

        int getEnergyNode() { return currentEnergyNode; };
};

#endif  // ENERGY_ESTIMATOR_H