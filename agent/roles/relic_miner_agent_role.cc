#include "agent_role.h"
#include <memory>

RelicMinerAgentRole::RelicMinerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RelicMinerAgentRole";
}

bool RelicMinerAgentRole::isRolePossible()
{
    // It mines only if this is the first shuttle in the position
    return gameMap.getTileAtPosition(shuttle)->isVantagePoint() && gameMap.getTileAtPosition(shuttle)->getShuttles()[0]->id == shuttle.id;
}

void RelicMinerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    bestPlan = {Direction::CENTER, 0, 0};
}

void RelicMinerAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::RELIC_MINER) {
            RelicMinerJob* relicMinerJob = static_cast<RelicMinerJob*>(job);
            if (relicMinerJob->targetX == shuttle.getX() && relicMinerJob->targetY == shuttle.getY()) {                
                std::vector<int> bestPlan = {Direction::CENTER, 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(1);
            }
        }
    }
}
