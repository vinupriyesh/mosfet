#ifndef RELIC_H
#define RELIC_H

#include <vector>

class Relic {

private:
    int id;    

public:
    bool visible;
    std::vector<int> position;
    Relic(int id);    
    void updateRelicData(std::vector<int> position, bool isVisible);
};

#endif // RELIC_H
