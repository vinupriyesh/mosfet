#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include "shuttle.h"
#include "parser.h"
#include <vector>
#include <string>

class ControlCenter {
private:
    std::string playerName;
    int N;
    Shuttle** shuttles; 
    void log(std::string message);   

public:    
    ControlCenter();
    ~ControlCenter();
    void update(GameState &gameState);
    std::vector<std::vector<int>> act();
};

#endif // CONTROLCENTER_H
