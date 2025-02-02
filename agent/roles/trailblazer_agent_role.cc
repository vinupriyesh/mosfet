#include "agent_role.h"

TrailblazerAgentRole::TrailblazerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : NavigatorAgentRole(shuttle, gameMap){
    roleClassName = "TrailblazerAgentRole";
}

void TrailblazerAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle.energy <= gameEnvConfig.unitMoveCost) {
        // This shuttle cant move!
        return;
    }

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::TRAILBLAZER_NAVIGATOR) {
            surveyJob(jobBoard, job);
        }
    }
}