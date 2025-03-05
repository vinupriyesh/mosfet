#ifndef SHUTTLE_DATA_H
#define SHUTTLE_DATA_H

#include <cstdint>
#include <vector>
#include <sstream>

enum ShuttleType : std::uint8_t {
    PLAYER,
    OPPONENT
};

struct ShuttleData {
    int id;
    int energy;
    int lastKnownEnergy;
    int lastEnergyUpdateTime;
    int previousEnergy;    
    bool visible;
    bool previouslyVisible;
    bool ghost;
    ShuttleType type;

    std::vector<int> previousPosition = {-1, -1};
    std::vector<int> position = {-1, -1};

    ShuttleData(int id, ShuttleType type): id(id), type(type),
        visible(false), ghost(true), previouslyVisible(false), energy(0), previousEnergy(0), lastKnownEnergy(0), lastEnergyUpdateTime(0) {};

    int getX() const{
        return position[0];
    }

    int getY() const {
        return position[1];
    }

    int getPreviousX() const{
        return previousPosition[0];
    }

    int getPreviousY() const {
        return previousPosition[1];
    }

    bool hasMoved() const {
        return position[0] != previousPosition[0] || position[1] != previousPosition[1];
    }

    std::string to_string() const {
        std::ostringstream ss;
        ss << "ShuttleData: id=" << id << ", energy=" << energy << ", visible=" << visible << ", ghost=" << ghost << ", type=" << type
              << ", position=(" << position[0] << ", " << position[1] << ")";
        return ss.str();
    }
};

#endif // SHUTTLE_DATA_H