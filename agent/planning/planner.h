#ifndef PLANNING_H
#define PLANNING_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle.h"
#include "agent/planning/jobs.h"

class Planner {
    private:
        void log(std::string message);
        void populateJobs(JobBoard& jobBoard);
        
        GameMap &gameMap;
        
    protected:
        Shuttle** shuttles;

    public:
        Planner(Shuttle** shuttles, GameMap& gameMap) : shuttles(shuttles), gameMap(gameMap) {};

        void plan();
};

#endif //PLANNING_H
