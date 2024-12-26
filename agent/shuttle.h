#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <vector>
#include <random>
#include "agent/control_center.h"

class ControlCenter; // Forward declaration

enum ShuttleType {
    player,
    enemy
};

class Shuttle {

private:
    int id;
    bool isVisible;
    ShuttleType type;    
    int energy;
    ControlCenter* cc;

    std::mt19937 gen; // Mersenne Twister random number generator 
    std::uniform_int_distribution<> dis; // Uniform distribution

public:
    std::vector<int> position;
    std::vector<int> act();
    void updateUnitsData(std::vector<int> position, int energy);
    void updateVisbility(bool isVisible);
    Shuttle(int id, ShuttleType type, ControlCenter* cc);    
    ~Shuttle();
};

#endif // SHUTTLE_H
