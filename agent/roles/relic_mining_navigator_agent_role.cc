#include "agent_role.h"

bool RelicMiningNavigatorAgentRole::isRolePossible() {

    int unexploitedVantagePoints = this->cc->vantagePointsFound - this->cc->vantagePointsOccupied;
    return unexploitedVantagePoints > 0 && !leastEnergyPathing->vantagePointDestinations.empty();
}

void RelicMiningNavigatorAgentRole::iteratePlan(int planIteration, Communicator &communicator) {

    int idx = -1;
    for (const auto [distance, destinationTile] : leastEnergyPathing->vantagePointDestinations) {
        idx++;
        log(std::to_string(idx) + "/" + std::to_string(leastEnergyPathing->vantagePointDestinations.size()) + " - Vantage point - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));
        if (destinationTile->isOccupied()) {
            log("Tile is occupied");
            continue;
        }

        // Move towards the destination tile
        std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[destinationTile].second;
        if (pathToDestination.size() < 2) {
            log("We are already in the closest vantage point");
            bestPlan = {Direction::CENTER, 0, 0};
            break;
        }
        Direction direction = getDirectionTo(*pathToDestination[1]);

        bestPlan = {directionToInt(direction), 0, 0};
        break;
    }
}