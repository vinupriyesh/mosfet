#include "agent/roles/agent_role.h"
#include "agent_role.h"


void AgentRole::log(std::string message) {
   
    Logger::getInstance().log("AgentRole -> " + message);
}

void AgentRole::setLeastEnergyPathing(Pathing *leastEnergyPathing) {
    this->leastEnergyPathing = leastEnergyPathing; // Set least path strategy
}

Direction AgentRole::getDirectionTo(const GameTile& destinationTile) {
    int currentX = shuttle->position[0];
    int currentY = shuttle->position[1];
    int destinationX = destinationTile.x;
    int destinationY = destinationTile.y;

    if (destinationX < currentX) {
        return LEFT;
    } else if (destinationX > currentX) {
        return RIGHT;
    } else if (destinationY < currentY) {
        return UP;
    } else if (destinationY > currentY) {
        return DOWN;
    } else {
        return CENTER; // If the destination is the same as the current position
    }
}

AgentRole::AgentRole(Shuttle* shuttle, ControlCenter* cc) : shuttle(shuttle), cc(cc), leastEnergyPathing(nullptr) {
    log("AgentRole initialized");
}
