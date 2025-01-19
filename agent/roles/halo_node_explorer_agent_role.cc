#include "agent_role.h"

HaloNodeExplorerAgentRole::HaloNodeExplorerAgentRole(Shuttle *shuttle, ControlCenter *cc) : ExplorerAgentRole(shuttle, cc) {
    roleClassName = "HaloNodeExplorerAgentRole";
}

bool HaloNodeExplorerAgentRole::isRolePossible()
{
    int totalTile = this->cc->gameEnvConfig->mapHeight * this->cc->gameEnvConfig->mapWidth;
    float percentageExplored = static_cast<float>(this->cc->tilesExplored) / totalTile;
    return shuttle->getTileAtPosition()->isHaloTile() && percentageExplored >= 0.33;
}

void HaloNodeExplorerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen);
    bestPlan = {random_number, 0, 0};
}