#include "agent_role.h"

RandomAgentRole::RandomAgentRole(Shuttle *shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RandomAgentRole";
    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

bool RandomAgentRole::isRolePossible()
{
    return true;
}

void RandomAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int random_number = dis(gen); // Generate a random number in the range
    bestPlan = {random_number, 0, 0};
}