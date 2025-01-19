#include "agent_role.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(Shuttle *shuttle, ControlCenter *cc) : ExplorerAgentRole(shuttle, cc) {
    roleClassName = "HaloNodeExplorerAgentRole";
    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

bool HaloNodeExplorerAgentRole::isRolePossible() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int totalTile = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;
    float percentageExplored = static_cast<float>(this->cc->gameMap->derivedGameState.tilesExplored) / totalTile;
    return shuttle->getTileAtPosition()->isHaloTile() && percentageExplored >= 0.33;
}

void HaloNodeExplorerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen);
    bestPlan = {random_number, 0, 0};
}