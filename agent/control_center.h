#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include "shuttle.h"
#include "parser.h"
#include "relic.h"
#include <vector>
#include <string>

class Shuttle; //Forward declaration

class ControlCenter {
private:
    
    // one time constants
    std::string playerName;
    int maxUnits;
    int teamId;
    int relicCount;
    int enemyTeamId;    
    int unitMoveCost;
    int unitSapCost;
    int unitSapRange;
    int unitSensorRange;

    // dynamic objects
    Shuttle** shuttles; 
    Shuttle** enemyShuttles;
    Relic** relics;

    // private methods
    void log(std::string message);
    void init(GameState &gameState);

public:    
    ControlCenter();
    ~ControlCenter();
    void update(GameState &gameState);
    std::vector<std::vector<int>> act();
};

#endif // CONTROLCENTER_H
