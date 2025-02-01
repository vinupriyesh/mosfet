#include "agent_role.h"

void NavigatorAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job* job : jobBoard.getJobs()) {
        if (job->type == JobType::TRAILBLAZER_NAVIGATOR || job->type == JobType::HALO_NODE_NAVIGATOR || job->type == JobType::RELIC_MINING_NAVIGATOR) {
            NavigatorJob* navigatorJob = static_cast<NavigatorJob*>(job);


            for (const auto [distance, destinationTile] : leastEnergyPathing->haloDestinations) {

                if (destinationTile->x == navigatorJob->targetX && destinationTile->y == navigatorJob->targetY) {
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
    }
}