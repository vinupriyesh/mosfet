#include "agent_role.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : ExplorerAgentRole(shuttle, gameMap) {
    roleClassName = "HaloNodeExplorerAgentRole";
    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

bool HaloNodeExplorerAgentRole::isRolePossible() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int totalTile = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;
    float percentageExplored = static_cast<float>(gameMap.derivedGameState.tilesExplored) / totalTile;
    return gameMap.getTileAtPosition(shuttle)->isHaloTile() && percentageExplored >= 0.33;
}

void HaloNodeExplorerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen);
    bestPlan = {random_number, 0, 0};
}


void HaloNodeExplorerAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    // GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    // int totalTile = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;
    // float percentageExplored = static_cast<float>(gameMap.derivedGameState.tilesExplored) / totalTile;

    // if (percentageExplored < 0.33) {
    //     return;
    // }

    for (Job& job : jobBoard.getJobs()) {
        if (job.type == JobType::HALO_NODE_EXPLORER) {
            HaloNodeExplorerJob& haloNodeExplorerJob = static_cast<HaloNodeExplorerJob&>(job);
            if (haloNodeExplorerJob.haloNodeX == shuttle.getX() && haloNodeExplorerJob.haloNodeY == shuttle.getY()) {
                Job& job = jobBoard.getJobs()[0];
                
                std::vector<int> bestPlan = {dis(gen), 0, 0};
                JobApplication* jobApplication = &jobBoard.applyForJob(job, shuttle, std::move(bestPlan));
            }
        }
    }

}