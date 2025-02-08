#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include "parser.h"
#include "agent/relic.h"
#include "logger.h"
#include "metrics.h"
#include "game_env_config.h"
#include "agent/game_map.h"
#include "agent/shuttle.h"
#include "datastructures/constraint_set.h"
#include "agent/planning/planner.h"
#include "agent/roles/communicator.h"
#include "visualizer/visualizer_client.h"
#include "symmetry_util.h"

#include <vector>
#include <string>

class ControlCenter {
private:
    VisualizerClient* visualizerClientPtr;
    ConstraintSet* haloConstraints = nullptr; 
    Planner* planner = nullptr;     

    // dynamic objects
    Shuttle** shuttles; 
    Shuttle** opponentShuttles;
    std::map<int, Relic*> relics;
    std::vector<int> relicDiscoveryKey; //Matches with the relic masks array

    // private methods
    void log(std::string message);
    void init(GameState &gameState);

public: 
    GameMap* gameMap;

    ControlCenter();
    ~ControlCenter();
    void update(GameState &gameState);
    void plan();
    std::vector<std::vector<int>> act();
};

#endif // CONTROLCENTER_H
