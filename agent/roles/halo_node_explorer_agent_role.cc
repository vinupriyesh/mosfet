#include "agent_role.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : ExplorerAgentRole(shuttle, gameMap) {
    roleClassName = "HaloNodeExplorerAgentRole";
    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

void HaloNodeExplorerAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::HALO_NODE_EXPLORER) {
            HaloNodeExplorerJob* haloNodeExplorerJob = static_cast<HaloNodeExplorerJob*>(job);
            if (haloNodeExplorerJob->targetX == shuttle.getX() && haloNodeExplorerJob->targetY == shuttle.getY()) {                
                std::vector<int> bestPlan = {dis(gen), 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(1); 
            }
        }
    }

}