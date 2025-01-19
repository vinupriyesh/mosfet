#include "agent/roles/agent_role.h"
#include "agent_role.h"


void AgentRole::log(std::string message) {
    if (shuttle != nullptr) {
        Logger::getInstance().log(roleClassName + "-" + std::to_string(shuttle->id) + " -> " + message);
    } else {
        Logger::getInstance().log(roleClassName + "-null -> " + message);
    }
}

void AgentRole::setLeastEnergyPathing(Pathing *leastEnergyPathing) {
    this->leastEnergyPathing = leastEnergyPathing; // Set least path strategy
}

void AgentRole::setLeastEnergyPathingStopAtHaloTiles(Pathing *leastEnergyPathingStopAtHaloTiles) {
    this->leastEnergyPathingStopAtHaloTiles = leastEnergyPathingStopAtHaloTiles;
}

void AgentRole::setLeastEnergyPathingStopAtVantagePoints(Pathing *leastEnergyPathingStopAtVantagePoints) {
    this->leastEnergyPathingStopAtVantagePoints = leastEnergyPathingStopAtVantagePoints;
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

AgentRole::AgentRole(Shuttle* shuttle, GameMap& gameMap) : shuttle(shuttle), gameMap(gameMap), leastEnergyPathing(nullptr) {
    roleClassName = "AgentRole";
}

void AgentRole::reset() {
    unableToAct = false;
}
