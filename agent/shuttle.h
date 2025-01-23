#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <vector>
#include <random>
#include <map>

#include "logger.h"
#include "agent/game_map.h"
#include "agent/pathing.h"
#include "agent/roles/agent_role.h"
#include "agent/roles/communicator.h"


class GameMap; // Forward declaration
class AgentRole; //Forward declaration

enum ShuttleType {
    player,
    opponent
};

class Shuttle {

private:    
    bool visible;
    bool ghost;
    ShuttleType type;    
    
    GameMap& gameMap;

    std::mt19937 gen; // Mersenne Twister random number generator 
    std::uniform_int_distribution<> dis; // Uniform distribution

    AgentRole* activeRole = nullptr;
    std::map<std::string, AgentRole*> agentRoles;

    //Transients
    Pathing* leastEnergyPathing;  
    Pathing* leastEnergyPathingStopAtHaloTiles;
    Pathing* leastEnergyPathingStopAtVantagePoints;

public:
    int id;
    int energy;
    
    std::vector<int> previousPosition = {-1, -1};
    std::vector<int> position = {-1, -1};
    bool isTileUnvisited(Direction direction);

    bool isRandomAction();
    bool isGhost();
    std::vector<int> act();
    

    void log(std::string message);
    void updateUnitsData(std::vector<int> position, int energy);
    void updateVisibility(bool isVisible);
    int getX();
    int getY();

    GameTile* getTileAtPosition();

    void computePath();
    void iteratePlan(int planIteration, Communicator& communicator);

    Shuttle(int id, ShuttleType type, GameMap& gameMap);    
    ~Shuttle();
};

#endif // SHUTTLE_H
