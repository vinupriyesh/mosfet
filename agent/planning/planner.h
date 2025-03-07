#ifndef PLANNING_H
#define PLANNING_H

#include "agent/battle_evaluator.h"
#include "agent/shuttle.h"
#include "agent/planning/jobs.h"
#include "agent/opponent_tracker.h"

class Planner {
    private:
        void log(const std::string& message);
        void populateJobs(JobBoard& jobBoard);
        
        GameMap &gameMap;
        OpponentTracker& opponentTracker;
        BattleEvaluator& battleEvaluator;
        
    protected:
        Shuttle** shuttles;

    public:
        Planner(Shuttle** shuttles, GameMap& gameMap, OpponentTracker& opponentTracker, BattleEvaluator& battleEvaluator) 
                : shuttles(shuttles), gameMap(gameMap), opponentTracker(opponentTracker), battleEvaluator(battleEvaluator) {};

        void plan();
};

#endif //PLANNING_H
