#include <random>

#include "shuttle.h"
#include "control_center.h"

void Shuttle::updateUnitsData(std::vector<int> position, int energy) {
    this->position = position;
    this->energy = energy;
}

void Shuttle::updateVisbility(bool isVisible) {
    this->isVisible = isVisible;
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

std::vector<int> Shuttle::act() {
    if (!isVisible) {
        // Check if we can still move invisible shuttle (If it is inside Nebula!)
        return {0, 0, 0};
    }
    int random_number = dis(gen); // Generate a random number in the range


    return {random_number, 0, 0};
}

Shuttle::~Shuttle() {

}
