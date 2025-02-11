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
    FOUND_DRIFT,
    UNCONCLUSIVE_DRIFT
};

class DriftDetector {
    private:        
        void log(std::string message);
        int compareDrift(GameTile &sourceTile, int x, int y);
        std::map<int, std::vector<int>> stepToDriftSpeedMap;
        std::map<int, NebulaDriftStatus> driftSpeedToStatusMap;
        GameMap& gameMap;
    public:
        bool driftFinalized;
        int finalSpeed = 0;
        void reportNebulaDrift(GameTile& gameTile);

        DriftDetector(GameMap& gameMap);
};

#endif //DRIFT_DETECTOR_H
