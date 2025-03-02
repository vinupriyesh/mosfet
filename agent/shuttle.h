#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <vector>
#include <random>
#include <map>

#include "shuttle_data.h"
#include "agent/game_map.h"
#include "agent/pathing.h"
#include "agent/roles/agent_role.h"
#include "agent/planning/jobs.h"

class Shuttle {

private:
    ShuttleData shuttleData;            
    GameMap& gameMap;

    std::mt19937 gen; // Mersenne Twister random number generator 
    std::uniform_int_distribution<> dis; // Uniform distribution

    std::map<std::string, AgentRole*> agentRoles;

    //Transients
    Pathing* leastEnergyPathing;  

public:
    std::vector<int> bestPlan;

    bool isTileUnvisited(Direction direction);

    bool isGhost();
    bool isVisible();
    std::vector<int> act();
    
    void log(const std::string& message);
    void updateUnitsData(std::vector<int> position, int energy, int currentStep);
    void updateVisibility(bool isVisible);

    void computePath();    

    void surveyJobBoard(JobBoard& jobBoard);

    ShuttleData& getShuttleData();

    Shuttle(int id, ShuttleType type, GameMap& gameMap);    
    ~Shuttle();
};

#endif // SHUTTLE_H
