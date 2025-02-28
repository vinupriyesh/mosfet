#include "agent_role.h"
#include "game_env_config.h"

HaloNodeNavigatorAgentRole::HaloNodeNavigatorAgentRole(ShuttleData& shuttle, GameMap& gameMap) : NavigatorAgentRole(shuttle, gameMap){
    roleClassName = "HaloNodeNavigatorAgentRole";
}

void HaloNodeNavigatorAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle.energy <= gameEnvConfig.unitMoveCost) {
        // This shuttle cant move!
        return;
    }

    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::HALO_NODE_NAVIGATOR) {
            surveyJob(jobBoard, job);
        }
    }
}