#include "agent_role.h"
#include <memory>

RelicMinerAgentRole::RelicMinerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RelicMinerAgentRole";
}

void RelicMinerAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::RELIC_MINER) {
            RelicMinerJob* relicMinerJob = static_cast<RelicMinerJob*>(job);
            if (relicMinerJob->targetX == shuttle.getX() && relicMinerJob->targetY == shuttle.getY()) {                
                std::vector<int> bestPlan = {Direction::CENTER, 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(1);
            }
        }
    }
}
