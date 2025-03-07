#include "agent_role.h"
#include "config.h"
#include "game_env_config.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : ExplorerAgentRole(shuttle, gameMap) {
    roleClassName = "HaloNodeExplorerAgentRole";
    // Instead of using std::random_device which is non-deterministic,
    // use Config::seed or access the global seed value
    gen = std::mt19937(Config::seed);
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

int HaloNodeExplorerAgentRole::removeOutOfBounds(int moveId, int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    if (moveId == 1) {
        // Move up
        if (y == 0) {
            return 0;
        }
    } else if (moveId == 2) { 
        // Move right
        if (x == gameEnvConfig.mapWidth - 1) {
            return 0;
        }
    } else if (moveId == 3) {
        // Move down
        if (y == gameEnvConfig.mapHeight - 1) {
            return 0;
        }
    } else if (moveId == 4) {
        // Move left
        if (x == 0) {
            return 0;
        }
    }
    return moveId;
}

void HaloNodeExplorerAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::HALO_NODE_EXPLORER) {
            HaloNodeExplorerJob* haloNodeExplorerJob = static_cast<HaloNodeExplorerJob*>(job);
            if (haloNodeExplorerJob->targetX == shuttle.getX() && haloNodeExplorerJob->targetY == shuttle.getY()) {
                int moveId = dis(gen);
                moveId = removeOutOfBounds(moveId, shuttle.getX(), shuttle.getY());
                std::vector<int> bestPlan = {moveId, 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
                jobApplication->setPriority(1); 
            }
        }
    }

}