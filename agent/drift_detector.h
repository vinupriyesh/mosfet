#ifndef DRIFT_DETECTOR_H
#define DRIFT_DETECTOR_H

#include <string>
#include <map>
#include <vector>

#include "logger.h"
#include "game_map.h"

enum NebulaDriftStatus {
    UNKNOWN_DRIFT,
    NO_DRIFT,
    POSITIVE_DRIFT,
    NEGATIVE_DRIFT,
    UNCONCLUSIVE_DRIFT
};

class DriftDetector {
    private:
        void log(std::string message);
        int compareDrift(GameTile &sourceTile, int x, int y);
        NebulaDriftStatus identifyDriftType(GameTile &gameTile, int moveCount);
        std::map<int, std::vector<int>> stepToDriftSpeedMap;
        std::map<int, NebulaDriftStatus> driftSpeedToStatusMap;
        GameMap& gameMap;
    public:
        void reportNebulaDrift(GameTile& gameTile);
        NebulaDriftStatus getCurrentNebulaDriftStatus();

        DriftDetector(GameMap& gameMap);
};

#endif //DRIFT_DETECTOR_H
