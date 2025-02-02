#include "agent_role.h"

RelicMiningNavigatorAgentRole::RelicMiningNavigatorAgentRole(ShuttleData& shuttle, GameMap& gameMap) : NavigatorAgentRole(shuttle, gameMap) {
    roleClassName = "RelicMiningNavigatorAgentRole";
}

void RelicMiningNavigatorAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle.energy <= gameEnvConfig.unitMoveCost) {
        // This shuttle cant move!
        return;
    }

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::RELIC_MINING_NAVIGATOR) {
            surveyJob(jobBoard, job);
        }
    }
}