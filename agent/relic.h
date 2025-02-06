#ifndef RELIC_H
#define RELIC_H

#include <vector>

class Relic {

private:
    int id;

public:
    std::vector<int> position;
    Relic(int id, std::vector<int> position);
    std::vector<int> getMirroredPosition(int width, int height);
};

#endif // RELIC_H
