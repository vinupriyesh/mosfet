#include "agent_role.h"

bool HaloNodeExplorerAgentRole::isRolePossible() { 
    log("Inside checking HaloNodeExplorerAgentRole.isRolePossible");   
    int totalTile = this->cc->gameEnvConfig->mapHeight * this->cc->gameEnvConfig->mapWidth;
    float percentageExplored = static_cast<float>(this->cc->tilesExplored) / totalTile;
    return shuttle->getTileAtPosition()->isHaloTile() && percentageExplored >= 0.33;
}

void HaloNodeExplorerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen);
    bestPlan = {random_number, 0, 0};
}