#ifndef RELIC_H
#define RELIC_H

#include <vector>

class Relic {

private:
    int id;
    std::vector<int> discoveryIds;
public:
    std::vector<int> position;
    Relic(int id, std::vector<int> position);
    std::vector<int> getMirroredPosition(int width, int height);
    void addDiscoveryId(int discoveryId);
};

#endif // RELIC_H
