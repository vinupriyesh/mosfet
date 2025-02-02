#include "agent_role.h"

RelicMiningNavigatorAgentRole::RelicMiningNavigatorAgentRole(ShuttleData& shuttle, GameMap& gameMap) : NavigatorAgentRole(shuttle, gameMap) {
    roleClassName = "RelicMiningNavigatorAgentRole";
}

bool RelicMiningNavigatorAgentRole::isRolePossible()
{
    if (unableToAct) {
        return false;
    }
    int unexploitedVantagePoints = gameMap.derivedGameState.vantagePointsFound - gameMap.derivedGameState.vantagePointsOccupied;
    return unexploitedVantagePoints > 0 && !leastEnergyPathingStopAtVantagePoints->vantagePointDestinations.empty();
}

void RelicMiningNavigatorAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    unableToAct = false;
    bestPlan.clear();
    int idx = -1;
    for (const auto [distance, destinationTile] : leastEnergyPathingStopAtVantagePoints->vantagePointDestinations) {
        idx++;
        log(std::to_string(idx) + "/" + std::to_string(leastEnergyPathingStopAtVantagePoints->vantagePointDestinations.size()) + " - Vantage point - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));
        if (destinationTile->isOccupied()) {
            log("Tile is occupied");
            continue;
        }

        // Move towards the destination tile
        std::vector<GameTile*> pathToDestination = leastEnergyPathingStopAtVantagePoints->distances[destinationTile].second;
        if (pathToDestination.size() < 2) {
            log("We are already in the closest vantage point");
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

void RelicMiningNavigatorAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle.energy <= gameEnvConfig.unitMoveCost) {
        // This shuttle cant move!
        return;
    }

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::RELIC_MINING_NAVIGATOR) {
            surveyJob(jobBoard, job);
        }
    }
}