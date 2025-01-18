#include "agent_role.h"

bool RelicMinerAgentRole::isRolePossible() {
    // It mines only if this is the first shuttle in the position
    return shuttle->getTileAtPosition()->isVantagePoint() && shuttle->getTileAtPosition()->getShuttles()[0]->id == shuttle->id; 
}

void RelicMinerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    bestPlan = {Direction::CENTER, 0, 0};
}