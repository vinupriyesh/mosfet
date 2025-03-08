#include "agent_role.h"

RechargeAgentRole::RechargeAgentRole(ShuttleData& shuttle, GameMap& gameMap) : AgentRole(shuttle, gameMap) {
    roleClassName = "RechargeAgentRole";
}


void RechargeAgentRole::surveyJobBoard(JobBoard& jobBoard) {
    for (Job* job : jobBoard.getJobs()) {
        if (job->jobType == JobType::RECHARGE) {
            RechargeJob* rechargeJob = static_cast<RechargeJob*>(job);
            if (rechargeJob->preferredShuttle != shuttle.id) {
                // Recharge is not intended for this shuttle
                continue;
            }

            GameTile& startTile = gameMap.getTile(shuttle.getX(), shuttle.getY());

            if (startTile.getLastKnownEnergy() > 0) {
                // Already charging
                continue;
            }

            int shortedDistance = std::numeric_limits<int>::max();
            GameTile* shortestDestinationTile = nullptr;

            for (const auto [distance, destinationTile] : leastEnergyPathing->allDestinations) { //TODO: change this to a hashMap instead of loop

                if (distance < shortedDistance && destinationTile->getLastKnownEnergy() > 0 && gameMap.getEstimatedType(*destinationTile, gameMap.derivedGameState.currentStep) != TileType::NEBULA) {

                    // Shortest distance with positive energy
                    shortedDistance = distance;
                    shortestDestinationTile = destinationTile;
                }
            }

            if (shortestDestinationTile == nullptr) {
                // Cant do anything, not sure why.  
                continue;
            }

            std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[shortestDestinationTile].second;
            if (pathToDestination.size() < 2) {
                // We are already on the positive tile
                continue;
            }

            Direction direction = getDirectionTo(*pathToDestination[1]);
            std::vector<int> bestPlan = {directionToInt(direction), 0, 0};
            JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
            jobApplication->setPriority(-1 * (pathToDestination.size() - 1)); //Bigger number wins, hence the * -1            
            log("Shuttle " + std::to_string(shuttle.id) + " applied for recharge job with priority " + std::to_string(jobApplication->priority));
        }
    }
}