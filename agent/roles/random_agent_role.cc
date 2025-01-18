#include "agent_role.h"

bool RandomAgentRole::isRolePossible() {
    return true;
}

void RandomAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen); // Generate a random number in the range
    bestPlan = {random_number, 0, 0};
}