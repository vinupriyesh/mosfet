
#include "game_map.h"
#include <stdexcept>
#include <tuple>
#include <iostream>


void GameMap::log(std::string message) {
    Logger::getInstance().log("GameMap -> " + message);
}

void GameTile::log(std::string message) {
    Logger::getInstance().log("GameMap -> " + message);
}

GameMap::GameMap(int width, int height) : width(width), height(height) {
    map.resize(height);
    for (int y = 0; y < height; ++y) {
        map[y].reserve(width);
        for (int x = 0; x < width; ++x) {
            map[y].emplace_back(x, y);
        }
    }
}

bool GameMap::isValidTile(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

GameTile& GameMap::getTile(int x, int y){
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Tile coordinates out of range - " + std::to_string(x) + ", " + std::to_string(y));
    }
    return map[y][x];
}

GameTile &GameMap::getTile(GameTile &fromTile, Direction direction) {
    int x = fromTile.x;
    int y = fromTile.y;
    switch (direction) {
        case Direction::UP:
            y -= 1;
            break;
        case Direction::DOWN:
            y += 1;
            break;
        case Direction::LEFT:
            x -= 1;
            break;
        case Direction::RIGHT:
            x += 1;
            break;
    }

    try {
        return this->getTile(x, y);
    } catch (std::out_of_range& e) {
        log("Tile coordinates out of range - " + std::to_string(x) + ", " + std::to_string(y));
        throw e;
    }
    
}

std::tuple<bool, GameTile&> GameMap::isMovable(GameTile &fromTile, Direction direction) {
    try {
        GameTile& toTile = this->getTile(fromTile, direction);
        if (toTile.getType() == TileType::ASTEROID) {
            log("Tile is an asteroid - (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ")");
            return std::make_tuple(false, std::ref(fromTile));
        } else {
            // The tile exist and not an asteroid            
            return std::make_tuple(true, std::ref(toTile));

        }
    } catch (std::out_of_range& e) {
        return std::make_tuple(false, std::ref(fromTile));
    }
}

TileType GameTile::getType() const {
    return type;
}

void GameTile::setVisited(bool visited, int time) {
    this->visited = visited;
    this->lastVisitedTime = time;
}

void GameTile::setExplored(bool explored, int time) {
    this->explored = explored;
    this->lastExploredTime = time;
}

TileType GameTile::setType(int tileTypeCode, int time) {
    if (tileTypeCode == 0) {
        type = TileType::EMPTY;
    } else if (tileTypeCode == 1) {
        type = TileType::NEBULA;
    } else {
        type = TileType::ASTEROID;
    }
    lastTypeUpdateTime = time;
    return type;
}

void GameTile::setEnergy(int energyValue, int time) {
    energy = energyValue;
    lastEnergyUpdateTime = time;
}
