#include "agent_role.h"
#include "game_env_config.h"

DefenderAgentRole::DefenderAgentRole(ShuttleData& shuttles, GameMap& gameMap) : AgentRole(shuttles, gameMap) {
    roleClassName = "DefenderAgentRole";
}

void DefenderAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle.energy <= gameEnvConfig.unitSapCost + gameEnvConfig.unitMoveCost) {
        //Shuttle doesn't have enough energy to attack and move
        return;
    }
    
    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::DEFENDER) {
            DefenderJob* defenderJob = static_cast<DefenderJob*>(job);
            
            GameTile& targetTile = gameMap.getTile(defenderJob->targetX, defenderJob->targetY);

            if (defenderJob->defendByCollision) {
                
                if (defenderJob->preferredShuttle != shuttle.id) {
                    // This job is not intended for this shuttle
                    return;
                }

                Direction direction = getDirectionTo(targetTile);

                std::vector<int> bestPlan = {directionToInt(direction), 0, 0};
                log("Colloiding with tile - " + std::to_string(defenderJob->targetX) + ", " + std::to_string(defenderJob->targetY));
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(defenderJob->kills * 100 + defenderJob->opponentEneryLoss);

            } else if (std::abs(defenderJob->targetX - shuttle.getX()) <= gameEnvConfig.unitSapRange && std::abs(defenderJob->targetY - shuttle.getY()) <= gameEnvConfig.unitSapRange) {                                

                std::tuple<int, int> relativePosition = getRelativePosition(targetTile);
                std::vector<int> bestPlan = {5, std::get<0>(relativePosition), std::get<1>(relativePosition)};
                log("Attacking tile - " + std::to_string(defenderJob->targetX) + ", " + std::to_string(defenderJob->targetY));
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(defenderJob->kills * 100 + defenderJob->opponentEneryLoss); 
            }            
        }
    }
}