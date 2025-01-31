#include "agent_role.h"

void NavigatorAgentRole::surveyJobBoard(JobBoard& jobBoard) {

    for (Job& job : jobBoard.getJobs()) {
        if (job.type == JobType::NAVIGATOR) {
            NavigatorJob& navigatorJob = static_cast<NavigatorJob&>(job);


            for (const auto [distance, destinationTile] : leastEnergyPathing->haloDestinations) {

                if (destinationTile->x == navigatorJob.destinationX && destinationTile->y == navigatorJob.destinationY) {
                    std::vector<GameTile*> pathToDestination = leastEnergyPathing->distances[destinationTile].second;
                    if (pathToDestination.size() < 2) {
                        log("We are already in the closest destination tile");
                        continue;
                    }
                    Direction direction = getDirectionTo(*pathToDestination[1]);

                    std::vector<int> bestPlan = {directionToInt(direction), 0, 0};
                    JobApplication* jobApplication = &jobBoard.applyForJob(job, shuttle, std::move(bestPlan));
                    jobApplication->stepsNeededToExecute = pathToDestination.size() - 1;
                    jobApplication->energyNeededToExecute = distance;
                }
            }                        
        }
    }
}