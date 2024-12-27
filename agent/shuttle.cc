#include <random>

#include "shuttle.h"
#include "control_center.h"

#include <tuple>

void Shuttle::updateUnitsData(std::vector<int> position, int energy) {
    this->position = position;
    this->energy = energy;
}

void Shuttle::updateVisbility(bool isVisible) {
    this->isVisible = isVisible;
}

int Shuttle::getX() {
    return position[0];
}

int Shuttle::getY() {
    return position[1];
}

Shuttle::Shuttle(int id, ShuttleType type, ControlCenter* cc) {
    this->id = id;
    this->isVisible = false;
    this->type = type;
    this->cc = cc;

    std::random_device rd;
    gen = std::mt19937(rd()); // Initialize the random number generator 
    dis = std::uniform_int_distribution<>(0, 4); // Initialize the distribution with the range
}


bool Shuttle::isTileUnvisited(Direction direction) {
    GameTile& shuttleTile = this->cc->gameMap->getTile(position[0], position[1]);
    std::tuple<bool, GameTile&> result = this->cc->gameMap->isMovable(shuttleTile, direction);
    bool movable = std::get<0>(result);    
    GameTile& toTile = std::get<1>(result);
    
    return movable && !toTile.isVisited();
}

std::vector<int> Shuttle::act() {
    if (!isVisible) {
        // Check if we can still move invisible shuttle (If it is inside Nebula!)
        return {0, 0, 0};
    }

    if (isTileUnvisited(Direction::UP)) {
        return {1, 0, 0};
    }

    if (isTileUnvisited(Direction::RIGHT)) {
        return {2, 0, 0};
    }

    if (isTileUnvisited(Direction::DOWN)) {
        return {3, 0, 0};
    }

    if (isTileUnvisited(Direction::LEFT)) {
        return {4, 0, 0};
    }


    // int random_number = dis(gen); // Generate a random number in the range


    // return {random_number, 0, 0};
    return {0, 0, 0};
}

Shuttle::~Shuttle() {

}
