#include "agent_role.h"
#include "config.h"

RandomAgentRole::RandomAgentRole(ShuttleData& shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RandomAgentRole";
    gen = std::mt19937(Config::seed);
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}

void RandomAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    //No op
}