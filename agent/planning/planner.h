#ifndef PLANNING_H
#define PLANNING_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle.h"

class Planner {
    private:
        void log(std::string message);
    protected:
        Shuttle** shuttles;

    public:
        Planner(Shuttle** shuttles) : shuttles(shuttles){};

        void plan();
};

#endif //PLANNING_H
