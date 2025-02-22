#include "planner.h"

#include <unordered_set>
#include <memory>

void Planner::log(std::string message) {
    Logger::getInstance().log("Planner -> " + message);
}

void Planner::populateJobs(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    int jobIdCounter = 0;
    for (int x = 0; x < gameEnvConfig.mapWidth; ++x) {
        for (int y = 0; y < gameEnvConfig.mapHeight; ++y) {
            GameTile& currentTile = gameMap.getTile(x, y);
            int currentTileId = currentTile.getId(gameEnvConfig.mapWidth);
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

            // if (currentTile.isOpponentOccupied()) {
            //     DefenderJob* job = new DefenderJob(jobIdCounter++, x, y);
            //     log("Created Defender job at " + std::to_string(x) + ", " + std::to_string(y));
            //     jobBoard.addJob(job);
            // }
            if (gameMap.getOpponentBattlePoints().find(currentTileId) != gameMap.getOpponentBattlePoints().end()) {
                auto& battlePoint = gameMap.getOpponentBattlePoints()[currentTileId];
                if (battlePoint.second > 0 || battlePoint.first > gameEnvConfig.unitSapCost/4) {
                    DefenderJob* job = new DefenderJob(jobIdCounter++, x, y);
                    log("Created Defender job at " + std::to_string(x) + ", " + std::to_string(y));
                    jobBoard.addJob(job);
                    
                    for (int x = job->targetX - 1; x <= job->targetX + 1; ++x) {
                        for (int y = job->targetY - 1; y <= job->targetY + 1; ++y) {
                            if (gameMap.isValidTile(x, y)) {
                                GameTile& tile = gameMap.getTile(x, y);
                                if (tile.isOpponentOccupied()) {
                                    job->allOpponentPositions.push_back(std::make_pair(x, y));
                                }
                            }
                        }
                    }
                }
            }

            if (currentTile.isUnExploredFrontier()) {
                TrailblazerNavigatorJob* trailblazerJob = new TrailblazerNavigatorJob(jobIdCounter++, x, y);
                log("Created Trailblazer job at " + std::to_string(x) + ", " + std::to_string(y));
                jobBoard.addJob(trailblazerJob);
            }

            if (state.isThereAHuntForRelic() &&
                (currentTile.isRelicExplorationFrontier1() ||currentTile.isRelicExplorationFrontier2() ||currentTile.isRelicExplorationFrontier3() )){
                    TrailblazerNavigatorJob* trailblazerJob = new TrailblazerNavigatorJob(jobIdCounter++, x, y);
                    log("Created Frontier exploration job at " + std::to_string(x) + ", " + std::to_string(y));
                    jobBoard.addJob(trailblazerJob);
            }
        }
    }


    Metrics::getInstance().add("jobs_created", jobIdCounter);
}

void Planner::plan() {
    auto start = std::chrono::high_resolution_clock::now();

    log("Planning now");
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    JobBoard jobBoard;  

    populateJobs(jobBoard);

    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        Shuttle* shuttle = shuttles[i];
        log("Planning for shuttle " + std::to_string(shuttle->getShuttleData().id));

        shuttles[i]->surveyJobBoard(jobBoard);
        shuttles[i]->bestPlan.clear();
    }

    Metrics::getInstance().add("job_applications", jobBoard.getJobApplications().size());
    
    jobBoard.sortJobApplications(gameMap);

    std::unordered_set<int> assignedShuttleIds;
    std::unordered_set<int> assignedTilesForRelicMining;
    std::unordered_set<int> assignedTilesForRelicMiningNavigation;
    std::unordered_set<int> assignedTilesForHaloNodeExploration;
    std::unordered_set<int> assignedTilesForHaloNodeNavigation;
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

        if (jobApplication.job->jobType == JobType::RELIC_MINER && 
            assignedTilesForRelicMining.find(targetId) != assignedTilesForRelicMining.end()) {
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);            
            continue;
        }

        if (jobApplication.job->jobType == JobType::RELIC_MINING_NAVIGATOR && 
            (assignedTilesForRelicMiningNavigation.find(targetId) != assignedTilesForRelicMiningNavigation.end() || 
            assignedTilesForRelicMining.find(targetId) != assignedTilesForRelicMining.end())) {
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->jobType == JobType::HALO_NODE_EXPLORER && 
            assignedTilesForHaloNodeExploration.find(targetId) != assignedTilesForHaloNodeExploration.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->jobType == JobType::HALO_NODE_NAVIGATOR && 
            assignedTilesForHaloNodeNavigation.find(targetId) != assignedTilesForHaloNodeNavigation.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->jobType == JobType::TRAILBLAZER_NAVIGATOR && 
            assignedTilesForTrailblazing.find(targetId) != assignedTilesForTrailblazing.end()) {            
            jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
            continue;
        }

        if (jobApplication.job->jobType == JobType::DEFENDER) {
            if (assignedTilesForDefending.find(targetId) != assignedTilesForDefending.end()) {
                jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
                continue;
            }

            DefenderJob* defenderJob = dynamic_cast<DefenderJob*>(jobApplication.job);
            
            bool targetBusy = false;
            for (auto& opponentXYPair: defenderJob->allOpponentPositions) {
                int tileId = gameMap.getTile(opponentXYPair.first, opponentXYPair.second).getId(GameEnvConfig::getInstance().mapWidth);
                if (assignedTilesForDefending.find(tileId) != assignedTilesForDefending.end()) {
                    jobApplication.setStatus(JobApplicationStatus::TARGET_BUSY);
                    targetBusy = true;
                    continue;
                }
            }

            if (targetBusy) {
                continue;
            }
        }

        // Job Assignment Confirmed :: Below this point is a success case!
        // Copy and assign unique pointer to currentJob        //TODO: Revisit this mess!
        // shuttles[jobApplication.shuttleData->id]->currentJob = jobApplication.job; // Do not delete this pointer.  It is owned by the shuttle now.
        shuttles[jobApplication.shuttleData->id]->bestPlan = jobApplication.bestPlan;
        // jobBoard.addJobDeletionExclusion(jobApplication.id);

        log("JobApplication accepted " + jobApplication.to_string());

        jobApplication.setStatus(JobApplicationStatus::ACCEPTED);
        assignedShuttleIds.insert(jobApplication.shuttleData->id);

        if (jobApplication.job->jobType == JobType::RELIC_MINER) {
            assignedTilesForRelicMining.insert(targetId);
        }

        if (jobApplication.job->jobType == JobType::RELIC_MINING_NAVIGATOR) {
            assignedTilesForRelicMiningNavigation.insert(targetId);
        }

        if (jobApplication.job->jobType == JobType::HALO_NODE_EXPLORER) {
            assignedTilesForHaloNodeExploration.insert(targetId);
        }

        if (jobApplication.job->jobType == JobType::HALO_NODE_NAVIGATOR) {
            assignedTilesForHaloNodeNavigation.insert(targetId);
        }

        if (jobApplication.job->jobType == JobType::TRAILBLAZER_NAVIGATOR) {
            assignedTilesForTrailblazing.insert(targetId);
        }

        if (jobApplication.job->jobType == JobType::DEFENDER) {
            assignedTilesForDefending.insert(targetId);

            DefenderJob* defenderJob = dynamic_cast<DefenderJob*>(jobApplication.job);

             for (auto& opponentXYPair: defenderJob->allOpponentPositions) {
                int tileId = gameMap.getTile(opponentXYPair.first, opponentXYPair.second).getId(GameEnvConfig::getInstance().mapWidth);
                assignedTilesForDefending.insert(tileId);
             }
        }
        
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("plan_duration", duration.count());

}