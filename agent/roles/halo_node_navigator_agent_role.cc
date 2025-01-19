#include "agent_role.h"

HaloNodeNavigatorAgentRole::HaloNodeNavigatorAgentRole(Shuttle *shuttle, ControlCenter *cc) : NavigatorAgentRole(shuttle, cc){
    roleClassName = "HaloNodeNavigatorAgentRole";
}

bool HaloNodeNavigatorAgentRole::isRolePossible()
{
    if (unableToAct) {
        return false;
    }
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int totalTile = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;
    float percentageExplored = static_cast<float>(this->cc->gameMap->derivedGameState.tilesExplored) / totalTile; 
    return (cc->gameMap->derivedGameState.allRelicsFound || cc->gameMap->derivedGameState.allTilesExplored || percentageExplored >= 0.7) && !leastEnergyPathingStopAtHaloTiles->haloDestinations.empty();
}

void HaloNodeNavigatorAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    unableToAct = false;
    bestPlan.clear();
    int idx = -1;
    for (const auto [distance, destinationTile] : leastEnergyPathingStopAtHaloTiles->haloDestinations) {
        idx++;
        log(std::to_string(idx) + "/" + std::to_string(leastEnergyPathingStopAtHaloTiles->haloDestinations.size()) + 
            " - Halo tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + 
            ") with distance " + std::to_string(distance));
        if (destinationTile->isOccupied()) {
            log("Tile is occupied");
            continue;
        }

        // Move towards the destination tile
        std::vector<GameTile*> pathToDestination = leastEnergyPathingStopAtHaloTiles->distances[destinationTile].second;
        if (pathToDestination.size() < 2) {
            log("We are already in the closest halo tile");
            bestPlan = {Direction::CENTER, 0, 0};
            break;
        }
        Direction direction = getDirectionTo(*pathToDestination[1]);

        bestPlan = {directionToInt(direction), 0, 0};
        break;
    }

    if (bestPlan.size() == 0){
        unableToAct = true;
    }
}