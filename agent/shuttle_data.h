#ifndef SHUTTLE_DATA_H
#define SHUTTLE_DATA_H

#include <vector>

enum ShuttleType {
    player,
    opponent
};

struct ShuttleData {
    int id;
    int energy;
    bool visible;
    bool ghost;
    ShuttleType type;

    std::vector<int> previousPosition = {-1, -1};
    std::vector<int> position = {-1, -1};

    ShuttleData(int id, ShuttleType type): id(id), type(type),
        visible(false), ghost(false) {};

    int getX() {
    return position[0];
    }

    int getY() {
        return position[1];
    }
};

#endif // SHUTTLE_DATA_H