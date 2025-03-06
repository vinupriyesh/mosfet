#include "agent_role.h"
#include "config.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : ExplorerAgentRole(shuttle, gameMap) {
    roleClassName = "HaloNodeExplorerAgentRole";
    // Instead of using std::random_device which is non-deterministic,
    // use Config::seed or access the global seed value
    gen = std::mt19937(Config::seed);
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

void HaloNodeExplorerAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::HALO_NODE_EXPLORER) {
            HaloNodeExplorerJob* haloNodeExplorerJob = static_cast<HaloNodeExplorerJob*>(job);
            if (haloNodeExplorerJob->targetX == shuttle.getX() && haloNodeExplorerJob->targetY == shuttle.getY()) {                
                std::vector<int> bestPlan = {dis(gen), 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(1); 
            }
        }
    }

}