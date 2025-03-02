#ifndef SHUTTLE_DATA_H
#define SHUTTLE_DATA_H

#include <cstdint>
#include <vector>
#include <sstream>

enum ShuttleType : std::uint8_t {
    player,
    opponent
};

struct ShuttleData {
    int id;
    int energy;
    int lastKnownEnergy;
    int lastEnergyUpdateTime;
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

    std::string to_string() {
        std::ostringstream ss;
        ss << "ShuttleData: id=" << id << ", energy=" << energy << ", visible=" << visible << ", ghost=" << ghost << ", type=" << type
              << ", position=(" << position[0] << ", " << position[1] << ")";
        return ss.str();
    }
};

#endif // SHUTTLE_DATA_H