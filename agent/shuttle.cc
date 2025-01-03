#include <random>

#include "shuttle.h"
#include "control_center.h"
#include "pathing.h"
#include "game_map.h"

#include <tuple>


void Shuttle::log(std::string message) {
    std::string visibilityMark = "";
    if (!this->visible) {
        visibilityMark = "~";
    }
    Logger::getInstance().log(visibilityMark + "Shuttle-" + std::to_string(this->id) + " -> " + message);
}

void Shuttle::updateUnitsData(std::vector<int> position, int energy) {
    this->previousPosition = this->position;
    this->position = position;
    this->energy = energy;
}

void Shuttle::updateVisbility(bool isVisible) {
    this->visible = isVisible;
}

int Shuttle::getX() {
    return position[0];
}

int Shuttle::getY() {
    return position[1];
}

Shuttle::Shuttle(int id, ShuttleType type, ControlCenter* cc) {
    this->id = id;
    this->visible = false;
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
    
    if (movable) {
        log("Tile (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ") is visited? " + std::to_string(toTile.isVisited()));
    } else {
        log("Tile is not movable - (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ") in direction " + std::to_string(direction));
    }

    return movable && !toTile.isVisited();    
}

std::vector<int> Shuttle::act() {    
    if (!visible) {
        //TODO: Check if we can still move invisible shuttle (If it is inside Nebula!)
        return {0, 0, 0};
    }

    log("Shuttle acting - (" + std::to_string(position[0]) + ", " + std::to_string(position[1]) + ")");

    GameTile& startTile = this->cc->gameMap->getTile(position[0], position[1]);

    int totalTile = this->cc->gameEnvConfig->mapHeight * this->cc->gameEnvConfig->mapWidth;
    float percentageExplored = static_cast<float>(this->cc->tilesExplored) / totalTile;

    //If we are already at a halo tile, then move to a random tile
    if (startTile.isHaloTile() && percentageExplored >= 0.33) {
        log("Moving random from a halo tile");
        int random_number = dis(gen);
        return {random_number, 0, 0};
    }

    // If all relics are found, then move towards halo nodes
    if (cc->allRelicsFound || cc->allTilesExplored || percentageExplored >= 0.7) {
        log("Going to explore the tiles");
        PathingConfig config = {};
        config.stopAtHaloTiles = true;
        config.captureHaloTileDestinations = true;

        Pathing pathing(this->cc->gameMap, config);
        
        pathing.findAllPaths(startTile);

        // Get the closest unexplored tile
        if (!pathing.haloDestinations.empty()) {
            auto [distance, destinationTile] = pathing.haloDestinations.top();
            log("Closest halo tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));

            // Move towards the destination tile
            std::vector<GameTile*> pathToDestination = pathing.distances[destinationTile].second;
            if (pathToDestination.size() < 2) {
                log("We are already in the closest halo tile");
                return {Direction::CENTER, 0, 0};
            }
            Direction direction = getDirectionTo(*pathToDestination[1]);

            return {directionToInt(direction), 0, 0};
        }
    }

    // Explore the unexplored tiles
    if (!cc->allTilesExplored) {
        PathingConfig config = {};
        config.stopAtUnexploredTiles = true;
        config.captureUnexploredTileDestinations = true;

        Pathing pathing(this->cc->gameMap, config);

        pathing.findAllPaths(startTile);

        // Get the closest unexplored tile
        if (!pathing.unexploredDestinations.empty()) {
            auto [distance, destinationTile] = pathing.unexploredDestinations.top();
            log("Closest unexplored tile - (" + std::to_string(destinationTile->x) + ", " + std::to_string(destinationTile->y) + ") with distance " + std::to_string(distance));

            // Move towards the destination tile
            std::vector<GameTile*> pathToDestination = pathing.distances[destinationTile].second;
            if (pathToDestination.size() < 2) {
                log("We are already in the closest unexplored tile");
                return {Direction::CENTER, 0, 0};
            }
            Direction direction = getDirectionTo(*pathToDestination[1]);

            return {directionToInt(direction), 0, 0};
        }
    }

    log("No actions possible for shuttle" + std::to_string(id));
    // return {0, 0, 0};

    int random_number = dis(gen); // Generate a random number in the range
    return {random_number, 0, 0};
}

Direction Shuttle::getDirectionTo(const GameTile& destinationTile) {
    int currentX = position[0];
    int currentY = position[1];
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

Shuttle::~Shuttle() {

}
