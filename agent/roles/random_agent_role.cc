#include "agent_role.h"

RandomAgentRole::RandomAgentRole(ShuttleData& shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RandomAgentRole";
    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

void RandomAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    //No op
}