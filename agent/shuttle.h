#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <vector>
#include <random>

#include "logger.h"
#include "agent/control_center.h"
#include "agent/game_map.h"

class ControlCenter; // Forward declaration

enum ShuttleType {
    player,
    enemy
};

class Shuttle {

private:
    int id;
    bool visible;
    ShuttleType type;    
    int energy;
    ControlCenter* cc;

    std::mt19937 gen; // Mersenne Twister random number generator 
    std::uniform_int_distribution<> dis; // Uniform distribution
    Direction getDirectionTo(const GameTile &destinationTile);
public:
    std::vector<int> position;
    bool isTileUnvisited(Direction direction);
    std::vector<int> act();

    

    void log(std::string message);
    void updateUnitsData(std::vector<int> position, int energy);
    void updateVisbility(bool isVisible);
    int getX();
    int getY();
    Shuttle(int id, ShuttleType type, ControlCenter* cc);    
    ~Shuttle();
};

#endif // SHUTTLE_H
