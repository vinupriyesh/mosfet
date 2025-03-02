#ifndef PLANNING_H
#define PLANNING_H

#include "agent/shuttle.h"
#include "agent/planning/jobs.h"
#include "agent/opponent_tracker.h"

class Planner {
    private:
        void log(const std::string& message);
        void populateJobs(JobBoard& jobBoard);
        
        GameMap &gameMap;
        OpponentTracker& opponentTracker;
        
    protected:
        Shuttle** shuttles;

    public:
        Planner(Shuttle** shuttles, GameMap& gameMap, OpponentTracker& opponentTracker) : shuttles(shuttles), gameMap(gameMap), opponentTracker(opponentTracker) {};

        void plan();
};

#endif //PLANNING_H
