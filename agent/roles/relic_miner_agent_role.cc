#include "agent_role.h"

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