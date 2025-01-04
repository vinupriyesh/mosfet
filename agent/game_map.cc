#include "game_map.h"
#include "shuttle.h"

#include <stdexcept>
#include <tuple>
#include <iostream>


void GameMap::log(std::string message) {
    Logger::getInstance().log("GameMap -> " + message);
}

void GameTile::log(std::string message) {
    Logger::getInstance().log("GameTile -> " + message);
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

void GameMap::addRelic(Relic *relic, int currentStep) {
    int x = relic->position[0];
    int y = relic->position[1];
    map[y][x].setRelic(relic);
    for (int i = x-2;i <= x+2; ++i) {
        for (int j = y-2; j <= y+2; ++j) {            
            if (isValidTile(i, j)) {
                auto& tile = getTile(i, j);
                // If the tile is already visited in the past and not set as halo yet, that only means this relic node was invisible
                // due to a nebula that time.  In such cases, we handle the marking of halo tile in the points-constraints
                // evaluation
                if (!tile.isVisited() || tile.getLastVisitedTime() == currentStep) {
                    if (map[j][i].isHaloTile()) {
                        // If multiple relics are seen, this can happen.  This is a overlap
                        log("Halo Overlap detected for (" + std::to_string(i) + ", " + std::to_string(j) + ")");
                    }
                    map[j][i].setHaloTile(true);
                }
            }
        }
    }
}

bool GameMap::hasPotentialInvisibleRelicNode(GameTile& gameTile) {
    int x = gameTile.x;
    int y = gameTile.y;
    for (int i = x-2;i <= x+2; ++i) {
        for (int j = y-2; j <= y+2; ++j) {
            if (isValidTile(i, j)) {
                auto& tile = getTile(i, j);
                if (!tile.isExplored() && !tile.isForcedRegularTile()) {
                    // There is atleaast one tile that is not explored, meaning it is a potential invisible relic node
                    return true;
                }
            }
        }
    }
    return false;
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

    return this->getTile(x, y);    
    
}

std::tuple<bool, GameTile&> GameMap::isMovable(GameTile &fromTile, Direction direction) {
    try {
        GameTile& toTile = this->getTile(fromTile, direction);
        if (toTile.getType() == TileType::ASTEROID) {
            // log("Tile is an asteroid - (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ")");
            return std::make_tuple(false, std::ref(fromTile));
        } else {
            // The tile exist and not an asteroid            
            return std::make_tuple(true, std::ref(toTile));

        }
    } catch (std::out_of_range& e) {
        return std::make_tuple(false, std::ref(fromTile));
    }
}

int GameTile::getId(int width){
    return y * width + x;
}

TileType GameTile::getType() const
{
    return type;
}

TileType GameTile::getLastKnownType() const {
    return lastKnownTileType;
}

void GameTile::setVisited(bool visited, int time) {
    this->visited = visited;
    this->lastVisitedTime = time;
}

void GameTile::addShuttle(Shuttle* shuttle)  {
    log("Adding shuttle at (" + std::to_string(x) + ", " + std::to_string(y) + "), id=" + std::to_string(shuttle->id));
    this->shuttles.push_back(shuttle);
}

bool GameTile::isOccupied() {
    return shuttles.size() > 0;
}

void GameTile::clearShuttle(Shuttle* shuttle) {
    for (auto it = shuttles.begin(); it != shuttles.end(); ++it) {
        if (*it == shuttle) {
            it = shuttles.erase(it);
            log("clearShuttle: Shuttle removed from (" + std::to_string(x) + ", " + std::to_string(y) + "), id=" + std::to_string(shuttle->id));
            break;
        }
    }
}

void GameTile::clearShuttles() {
    this->shuttles.clear();
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
    } else if (tileTypeCode == 2) {
        type = TileType::ASTEROID;
    } else {
        type = TileType::UNKNOWN;
    }
    
    if (type != TileType::UNKNOWN) {
        lastKnownTileType = type;
    }

    lastTypeUpdateTime = time;
    return type;
}

void GameTile::setEnergy(int energyValue, int time) {
    energy = energyValue;
    lastEnergyUpdateTime = time;
}
