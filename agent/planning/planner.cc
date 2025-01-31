#include "planner.h"

void Planner::log(std::string message) {
    Logger::getInstance().log("Planner -> " + message);
}

void Planner::populateJobs() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    int jobIdCounter = 0;
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& currentTile = gameMap.getTile(i, j);
            if (currentTile.isVantagePoint()) {
                RelicMinerJob* job = new RelicMinerJob(jobIdCounter++, i, j);
                log("Created RelicMiner job at " + std::to_string(i) + ", " + std::to_string(j));
                jobBoard.addJob(*job);

                NavigatorJob* navigatorJob = new NavigatorJob(jobIdCounter++, i, j);
                log("Created (Relic) Navigator job at " + std::to_string(i) + ", " + std::to_string(j));
                jobBoard.addJob(*navigatorJob);
            }

            if (currentTile.isHaloTile()) {
                HaloNodeExplorerJob* job = new HaloNodeExplorerJob(jobIdCounter++, i, j);
                log("Created HaloNodeExplorer job at " + std::to_string(i) + ", " + std::to_string(j));
                jobBoard.addJob(*job);

                NavigatorJob* navigatorJob = new NavigatorJob(jobIdCounter++, i, j);
                log("Created (Halo) Navigator job at " + std::to_string(i) + ", " + std::to_string(j));
                jobBoard.addJob(*navigatorJob);
            }

            if (currentTile.isOpponentOccupied()) {
                DefenderJob* job = new DefenderJob(jobIdCounter++, i, j);
                log("Created Defender job at " + std::to_string(i) + ", " + std::to_string(j));
                jobBoard.addJob(*job);
            }
        }
    }

    TrailblazerJob* trailblazerJob = new TrailblazerJob(jobIdCounter++);
    log("Created Trailblazer job");
    jobBoard.addJob(*trailblazerJob);

    Metrics::getInstance().add("jobs_created", jobIdCounter);
}

void Planner::plan() {
    log("Planning now");

    populateJobs();

    for(int i = 0; i < 2; ++i) {
        Shuttle* shuttle = shuttles[i];
        log("Planning for shuttle " + std::to_string(shuttle->getShuttleData().id));

        shuttles[i]->surveyJobBoard(jobBoard);
    }
    // RelicMiners    
    //Step 2: Assign jobs to shuttles

}