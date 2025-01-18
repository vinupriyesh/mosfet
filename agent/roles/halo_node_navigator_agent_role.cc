#include "agent_role.h"

bool HaloNodeNavigatorAgentRole::isRolePossible() {   
    int totalTile = this->cc->gameEnvConfig->mapHeight * this->cc->gameEnvConfig->mapWidth;
    float percentageExplored = static_cast<float>(this->cc->tilesExplored) / totalTile; 
    return (cc->allRelicsFound || cc->allTilesExplored || percentageExplored >= 0.7) && !leastEnergyPathing->haloDestinations.empty();
}

void HaloNodeNavigatorAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    int idx = -1;
    for (const auto [distance, destinationTile] : leastEnergyPathing->haloDestinations) {
        idx++;
        log(std::to_string(idx) + "/" + std::to_string(leastEnergyPathing->haloDestinations.size()) + 
            " - Halo tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + 
            ") with distance " + std::to_string(distance));
        if (destinationTile->isOccupied()) {
            log("Tile is occupied");
            continue;
        }

        // Move towards the destination tile
        std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[destinationTile].second;
        if (pathToDestination.size() < 2) {
            log("We are already in the closest halo tile");
            bestPlan = {Direction::CENTER, 0, 0};
            break;
        }
        Direction direction = getDirectionTo(*pathToDestination[1]);

        bestPlan = {directionToInt(direction), 0, 0};
        break;
    }
}