#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include "shuttle.h"
#include <vector>

class ControlCenter {
private:
    int N;
    Shuttle** shuttles;

public:
    ControlCenter(int n);
    ~ControlCenter();
    std::vector<std::vector<int>> act();
};

#endif // CONTROLCENTER_H
