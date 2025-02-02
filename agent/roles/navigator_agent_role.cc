#include "agent_role.h"

void NavigatorAgentRole::surveyJob(JobBoard& jobBoard, Job* job) {
    log("Evaluating job " + job->to_string());
    NavigatorJob* navigatorJob = static_cast<NavigatorJob*>(job);

    for (const auto [distance, destinationTile] : leastEnergyPathing->allDestinations) { //TODO: change this to a hashMap instead of loop

        if (destinationTile->x == navigatorJob->targetX && destinationTile->y == navigatorJob->targetY) {
            log("Found the destination tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));
            std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[destinationTile].second;
            if (pathToDestination.size() < 2) {
                log("We are already in the closest destination tile");
                continue;
            }
            Direction direction = getDirectionTo(*pathToDestination[1]);

            std::vector<int> bestPlan = {directionToInt(direction), 0, 0};
            JobApplication* jobApplication = &jobBoard.applyForJob(job, &shuttle, std::move(bestPlan));
            jobApplication->setPriority(-1 * (pathToDestination.size() - 1)); //Bigger number wins, hence the * -1
        }
    }
}