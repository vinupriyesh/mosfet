#include "planner.h"

void Planner::log(std::string message) {
    Logger::getInstance().log("Planner -> " + message);
}

void Planner::plan() {
    log("Planning now");

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    
    //Step 1: Create jobs

    // RelicMiners
    int jobIdCounter = 0;
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& currentTile = gameMap.getTile(i, j);
            if (currentTile.isVantagePoint()) {
                RelicMinerJob* job = new RelicMinerJob(jobIdCounter++, i, j);
                log("Created RelicMiner job at " + std::to_string(i) + ", " + std::to_string(j));
                NavigatorJob* navigatorJob = new NavigatorJob(jobIdCounter++, i, j);
                log("Created (Relic) Navigator job at " + std::to_string(i) + ", " + std::to_string(j));
            }

            if (currentTile.isHaloTile()) {
                HaloNodeExplorerJob* job = new HaloNodeExplorerJob(jobIdCounter++, i, j);
                log("Created HaloNodeExplorer job at " + std::to_string(i) + ", " + std::to_string(j));
                NavigatorJob* navigatorJob = new NavigatorJob(jobIdCounter++, i, j);
                log("Created (Halo) Navigator job at " + std::to_string(i) + ", " + std::to_string(j));
            }

            if (currentTile.isOpponentOccupied()) {
                DefenderJob* job = new DefenderJob(jobIdCounter++, i, j);
                log("Created Defender job at " + std::to_string(i) + ", " + std::to_string(j));
            }
        }
    }

    TrailblazerJob* trailblazerJob = new TrailblazerJob(jobIdCounter++);
    log("Created Trailblazer job");

    Metrics::getInstance().add("jobs_created", jobIdCounter);
    //Step 2: Assign jobs to shuttles

}