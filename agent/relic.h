#ifndef RELIC_H
#define RELIC_H

#include <vector>

class Relic {

private:
    int id;
    bool isVisible;
    std::vector<int> position;

public:
    Relic(int id);    
    void updateRelicData(std::vector<int> position, bool isVisible);
};

#endif // RELIC_H
