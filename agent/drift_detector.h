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
        std::map<int, std::vector<int>> stepToDriftSpeedMap; // In this step what are all the drift speeds possible?
        std::map<int, NebulaDriftStatus> driftSpeedToStatusMap; // What is the current status for the drift speed?
        std::vector< std::vector<std::vector<TileType>>* > allDriftTileTypeVectors;
        int currentDriftTileTypeVectorIndex = 0;
        GameMap& gameMap;

        TileType getEstimatedDriftTile(int x, int y, std::vector<std::vector<TileType>> *previousStepValues);

        // void fillDriftAwareTileType(int stepId, std::vector<int>& values);
        std::vector<std::vector<TileType>>* estimateDrift(std::vector<std::vector<TileType>>* previousStepValues);
        int computeMoveCountBetween(int from, int to);
        void forecastTileTypeAt(int step, GameTile &tile, std::vector<std::vector<TileType>> &tileTypesArray);
        std::vector<std::vector<TileType>> *prepareCurrentTileTypes();
        void estimateTileTypesforFinalizedDrift();

        void exploreTile(int x, int y, TileType tileType, int driftTileTypeIndex);
    public:
        bool driftFinalized;
        int finalSpeed = 0;
        void reportNebulaDrift(GameTile& gameTile);
        void exploreTile(GameTile& gameTile);        
        void step();

        DriftDetector(GameMap& gameMap);
        ~DriftDetector();
};

#endif //DRIFT_DETECTOR_H
