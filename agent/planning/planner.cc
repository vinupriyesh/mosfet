#include "planner.h"

#include <unordered_set>
#include <memory>

void Planner::log(std::string message) {
    Logger::getInstance().log("Planner -> " + message);
}

void Planner::populateJobs(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    int jobIdCounter = 0;
    for (int x = 0; x < gameEnvConfig.mapWidth; ++x) {
        for (int y = 0; y < gameEnvConfig.mapHeight; ++y) {
            GameTile& currentTile = gameMap.getTile(x, y);
            if (currentTile.isVantagePoint()) {
                RelicMinerJob* job = new RelicMinerJob(jobIdCounter++, x, y);
                log("Created RelicMiner job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(job);

                RelicMiningNavigatorJob* navigatorJob = new RelicMiningNavigatorJob(jobIdCounter++, x, y);
                log("Created (Relic) Navigator job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(navigatorJob);
            }

            if (currentTile.isHaloTile()) {
                HaloNodeExplorerJob* job = new HaloNodeExplorerJob(jobIdCounter++, x, y);
                log("Created HaloNodeExplorer job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(job);

                HaloNodeNavigatorJob* navigatorJob = new HaloNodeNavigatorJob(jobIdCounter++, x, y);
                log("Created (Halo) Navigator job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(navigatorJob);
            }

            if (currentTile.isOpponentOccupied()) {
                DefenderJob* job = new DefenderJob(jobIdCounter++, x, y);
                log("Created Defender job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(job);
            }

            if (currentTile.isUnExploredFrontier()) {
                TrailblazerNavigatorJob* trailblazerJob = new TrailblazerNavigatorJob(jobIdCounter++, x, y);
                log("Created Trailblazer job");
                jobBoard.addJob(trailblazerJob);
            }
        }
    }


    Metrics::getInstance().add("jobs_created", jobIdCounter);
}

void Planner::plan() {
    log("Planning now");
    JobBoard jobBoard;  

    populateJobs(jobBoard);

    for(int i = 0; i < 2; ++i) {
        Shuttle* shuttle = shuttles[i];
        log("Planning for shuttle " + std::to_string(shuttle->getShuttleData().id));

        shuttles[i]->surveyJobBoard(jobBoard);
    }
    
    jobBoard.sortJobApplications();

    std::unordered_set<int> assignedShuttleIds;
    std::unordered_set<int> assignedTilesForRelicMining;
    std::unordered_set<int> assignedTilesForRelicMiningNavigation;
    std::unordered_set<int> assignedTilesForTrailblazing;
    std::unordered_set<int> assignedTilesForDefending;

    for (auto& jobApplication : jobBoard.getJobApplications()) {

        GameTile& targetTile = gameMap.getTile(jobApplication.job->targetX, jobApplication.job->targetY);
        int targetId = targetTile.getId(GameEnvConfig::getInstance().mapWidth);

        if (assignedShuttleIds.find(jobApplication.shuttleData->id) != assignedShuttleIds.end()) {
            //Shuttle already assigned to a higher priority job
            jobApplication.setStatus(JobApplicationStatus::SHUTTLE_BUSY);
            continue;
        }

        if (jobApplication.job->type == JobType::RELIC_MINER && 
            assignedTilesForRelicMining.find(targetId) != assignedTilesForRelicMining.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);            
            continue;
        }

        if (jobApplication.job->type == JobType::RELIC_MINING_NAVIGATOR && 
            assignedTilesForRelicMiningNavigation.find(targetId) != assignedTilesForRelicMiningNavigation.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->type == JobType::TRAILBLAZER_NAVIGATOR && 
            assignedTilesForTrailblazing.find(targetId) != assignedTilesForTrailblazing.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->type == JobType::DEFENDER && 
            assignedTilesForDefending.find(targetId) != assignedTilesForDefending.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        // Job Assignment Confirmed :: Below this point is a success case!
        // Copy and assign unique pointer to currentJob        
        shuttles[jobApplication.shuttleData->id]->currentJob = jobApplication.job; // Do not delete this pointer.  It is owned by the shuttle now.
        jobBoard.addJobDeletionExclusion(jobApplication.job->id);

        jobApplication.setStatus(JobApplicationStatus::ACCEPTED);
        assignedShuttleIds.insert(jobApplication.shuttleData->id);

        if (jobApplication.job->type == JobType::RELIC_MINER) {
            assignedTilesForRelicMining.insert(targetId);
        }

        if (jobApplication.job->type == JobType::RELIC_MINING_NAVIGATOR) {
            assignedTilesForRelicMiningNavigation.insert(targetId);
        }

        if (jobApplication.job->type == JobType::TRAILBLAZER_NAVIGATOR) {
            assignedTilesForTrailblazing.insert(targetId);
        }

        if (jobApplication.job->type == JobType::DEFENDER) {
            assignedTilesForDefending.insert(targetId);
        }
        
    }

}