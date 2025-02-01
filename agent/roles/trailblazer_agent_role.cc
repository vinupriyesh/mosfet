#include "agent_role.h"

TrailblazerAgentRole::TrailblazerAgentRole(ShuttleData& shuttle, GameMap& gameMap) : NavigatorAgentRole(shuttle, gameMap){
    roleClassName = "TrailblazerAgentRole";
}

bool TrailblazerAgentRole::isRolePossible()
{
    return !gameMap.derivedGameState.allTilesExplored && !leastEnergyPathing->unexploredDestinations.empty();
}

void TrailblazerAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    auto [distance, destinationTile] = leastEnergyPathing->unexploredDestinations.top();
    log("Closest unexplored tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));

    // Move towards the destination tile
    std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[destinationTile].second;
    if (pathToDestination.size() < 2) {
        log("We are already in the closest unexplored tile");
        bestPlan = {Direction::CENTER, 0, 0};
        return;
    }

    log("Trying to get the direction");
    Direction direction = getDirectionTo(*pathToDestination[1]);
    log("Got the direction");
    bestPlan = {directionToInt(direction), 0, 0};
    log("Prepared the trailblazer best plan");
}
