#include "game_map.h"

#include <cmath>
#include <stdexcept>
#include <tuple>
#include <cmath>
#include "game_env_config.h"

void GameMap::log(const std::string& message) {
    Logger::getInstance().log("GameMap -> " + message);
}

void GameTile::log(const std::string& message) {
    Logger::getInstance().log("GameTile -> " + message);
}

GameMap::GameMap(int width, int height) : width(width), height(height) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    map.resize(height);
    for (int y = 0; y < height; ++y) {
        map[y].reserve(width);
        for (int x = 0; x < width; ++x) {
            map[y].emplace_back(x, y);
            map[y][x].manhattanFromOrigin = std::abs(x - gameEnvConfig.originX) + std::abs(y - gameEnvConfig.originY);
            map[y][x].manhattanToOpponentOrigin = std::abs(x - gameEnvConfig.opponentOriginX) + std::abs(y - gameEnvConfig.opponentOriginY);
        }
    }
}

void GameMap::addRelic(Relic *relic, int currentStep, std::vector<int>& haloTileIds) {
    int x = relic->position[0];
    int y = relic->position[1];
    map[y][x].setRelic(relic);
    for (int i = x-2;i <= x+2; ++i) {
        for (int j = y-2; j <= y+2; ++j) {            
            if (isValidTile(i, j)) {
                auto& tile = getTile(i, j);                
                if (map[j][i].isHaloTile()) {
                    // If multiple relics are seen, this can happen.  This is a overlap
                    log("Halo Overlap detected for (" + std::to_string(i) + ", " + std::to_string(j) + ")");
                }
                map[j][i].setHaloTile(true);
                haloTileIds.push_back(map[j][i].getId(width));
                //TODO: If this is already a halo node, and we have it in our constraint set then it is a problem!
                log("Forcing halo tile for (" + std::to_string(i) + ", " + std::to_string(j) + ")");
            }
        }
    }
}

void GameMap::getAllOpponentsInRadius(int radius, int x, int y, std::vector<ShuttleData *> &opponents) {
    for (int i = x-radius; i <= x+radius; ++i) {
        for (int j = y-radius; j <= y+radius; ++j) {
            if (isValidTile(i, j)) {
                auto& tile = getTile(i, j);
                if (tile.isOpponentOccupied()) {
                    for (auto& shuttle : tile.opponentShuttles) {
                        opponents.push_back(shuttle);
                    }
                }
            }
        }
    }
}

void GameMap::setRelicExplorationFrontier(GameTile &tile, int match, int cutoffTime) {
    // Check if adjacent tiles are explored recently, and mark this tile as frontier
    if (isValidTile(tile.x, tile.y - 1)) {
        auto& nextTile = getTile(tile.x, tile.y - 1);
        if (nextTile.getLastExploredTime() >= cutoffTime) {
            tile.setRelicExplorationFrontier(true, match);
        }
    }

    if (isValidTile(tile.x, tile.y + 1)) {
        auto& nextTile = getTile(tile.x, tile.y + 1);
        if (nextTile.getLastExploredTime() >= cutoffTime) {
            tile.setRelicExplorationFrontier(true, match);
        }
    }

    if (isValidTile(tile.x - 1, tile.y)) {
        auto& nextTile = getTile(tile.x - 1, tile.y);
        if (nextTile.getLastExploredTime() >= cutoffTime) {
            tile.setRelicExplorationFrontier(true, match);
        }
    }

    if (isValidTile(tile.x + 1, tile.y)) {
        auto& nextTile = getTile(tile.x +1, tile.y);
        if (nextTile.getLastExploredTime() >= cutoffTime) {
            tile.setRelicExplorationFrontier(true, match);
        }
    }
}

bool GameMap::hasPotentialInvisibleRelicNode(GameTile& gameTile) {
    if (!derivedGameState.isThereAHuntForRelic()) {
        log("No potential for a relic node nearby tile " + gameTile.toString());
        return false;
    }
    int x = gameTile.x;
    int y = gameTile.y;
    for (int i = x-2;i <= x+2; ++i) {
        for (int j = y-2; j <= y+2; ++j) {
            if (isValidTile(i, j)) {
                auto& tile = getTile(i, j);
                if (!tile.isVisible() /*&& !tile.isForcedRegularTile()*/) {
                    // There is atleaast one tile that is not explored, meaning it is a potential invisible relic node
                    return true;
                }
            }
        }
    }
    return false;
}

GameTile *GameMap::getTileAtPosition(ShuttleData& shuttleData) {
    if (isValidTile(shuttleData.position[0], shuttleData.position[1])) {
        return &getTile(shuttleData.position[0], shuttleData.position[1]);
    } else {
        return nullptr;
    }
}

void GameMap::exploreTile(GameTile &tile, int currenStep) {

    // Check if adjacent tiles are not explored and mark them frontier
    if (isValidTile(tile.x, tile.y - 1)) {
        auto& nextTile = getTile(tile.x, tile.y - 1);
        if (!nextTile.isExplored()) {
            nextTile.setUnexploredFrontier(true);
        }
    }

    if (isValidTile(tile.x, tile.y + 1)) {
        auto& nextTile = getTile(tile.x, tile.y + 1);
        if (!nextTile.isExplored()) {
            nextTile.setUnexploredFrontier(true);
        }
    }

    if (isValidTile(tile.x - 1, tile.y)) {
        auto& nextTile = getTile(tile.x - 1, tile.y);
        if (!nextTile.isExplored()) {
            nextTile.setUnexploredFrontier(true);
        }
    }

    if (isValidTile(tile.x + 1, tile.y)) {
        auto& nextTile = getTile(tile.x +1, tile.y);
        if (!nextTile.isExplored()) {
            nextTile.setUnexploredFrontier(true);
        }
    }
    
    // Set tile as explored
    tile.setExplored(true, currenStep);
}

bool GameMap::isValidTile(int x, int y){
    return x >= 0 && x < width && y >= 0 && y < height;
}

GameTile& GameMap::getTile(int x, int y){
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Tile coordinates out of range - " + std::to_string(x) + ", " + std::to_string(y));
    }
    return map[y][x];
}

GameTile& GameMap::getRolledOverTile(int x, int y) {
    if (x >= width) {
        x -= width;
    }

    if (x < 0) {
        x += width;
    }

    if (y >= height) {
        y -= height;
    }

    if (y < 0) {
        y += height;
    }

    return getTile(x, y);
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
        case Direction::CENTER:
            break;
    }

    return this->getTile(x, y);    
    
}

GameTile &GameMap::getTileFromActionId(int actionId, int x, int y) {
    int newX = x;
    int newY = y;
    switch (actionId) {
        case 1: // UP
            newY = y - 1;
            break;
        case 2: // RIGHT
            newX = x + 1;
            break;
        case 3: // DOWN
            newY = y + 1;
            break;
        case 4: // LEFT
            newX = x - 1;
            break;
        default:
            break;
    }

    if (isValidTile(newX, newY)) {
        return getTile(newX, newY);
    } 

    return getTile(x, y);
}

TileType GameMap::getEstimatedType(GameTile &tile, int step) const {
    if (step >= driftAwareTileType.size() || driftAwareTileType[step] == nullptr) {
        // Fallback to the known value.  Likely the drift is not finalized!
        // The fallback doesn't consider previous steps more than 1.  the -1 previous lookup is needed for shuttle_energy_tracker.        
        if (step == derivedGameState.currentStep -1) {
            return tile.getPreviousTypeImmediate();
        }
        // log("Returning the latest tile type instead");
        return tile.getType();
    }

    return driftAwareTileType[step]->at(tile.y).at(tile.x);
}

std::tuple<bool, GameTile&> GameMap::isMovable(GameTile &fromTile, Direction direction) {

    try {
        GameTile& toTile = this->getTile(fromTile, direction);
        TileType estimatedTileType = getEstimatedType(toTile, derivedGameState.currentStep);
        if (estimatedTileType == TileType::ASTEROID) {
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

TileType GameTile::getType() const {
    return type;
}

void GameTile::setVisited(bool visited, int time) {
    this->visited = visited;
    this->lastVisitedTime = time;
}

void GameTile::addShuttle(ShuttleData* shuttle)  {
    log("Adding shuttle at (" + std::to_string(x) + ", " + std::to_string(y) + "), id=" + std::to_string(shuttle->id));
    this->shuttles.push_back(shuttle);
}

void GameTile::addGhostShuttle(ShuttleData* shuttle)  {
    log("Adding ghost shuttle at (" + std::to_string(x) + ", " + std::to_string(y) + "), id=" + std::to_string(shuttle->id));
    this->ghostShuttles.push_back(shuttle);
}

void GameTile::addOpponentShuttle(ShuttleData* shuttle) {
    this->opponentShuttles.push_back(shuttle);
}

void GameTile::addOpponentGhostShuttle(ShuttleData* shuttle) {
    this->opponentGhostShuttles.push_back(shuttle);
}

bool GameTile::isOccupied() {
    return shuttles.size() > 0;
}

bool GameTile::isOpponentOccupied() {
    return opponentShuttles.size() > 0;
}

int GameTile::getCumulativeOpponentEnergy() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int shuttleEnergy = 0;
    for (auto& shuttle : opponentShuttles) {       
        shuttleEnergy += shuttle->energy;
    }
    return shuttleEnergy;
}

void GameTile::clearShuttle(ShuttleData* shuttle) {
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
    this->ghostShuttles.clear();
}

void GameTile::clearOpponentShuttles() {
    this->opponentShuttles.clear();
    this->opponentGhostShuttles.clear();
}

void GameTile::setExplored(bool explored, int time) {
    this->explored = explored;
    this->lastExploredTime = time;

    if (this->unexploredFrontier) {
        this->unexploredFrontier = false;
    }

    if (time >= 50 && this->relicExplorationFrontier1) {
        relicExplorationFrontier1 = false;
    }

    if (time >= 151 && this->relicExplorationFrontier2) {
        relicExplorationFrontier2 = false;
    }

    if (time >= 252 && this->relicExplorationFrontier3) {
        relicExplorationFrontier3 = false;
    }
}

TileType GameTile::translateTileType(int tileTypeCode) {
    if (tileTypeCode == 0) {
        return TileType::EMPTY;
    } else if (tileTypeCode == 1) {
        return TileType::NEBULA;
    } else if (tileTypeCode == 2) {
        return TileType::ASTEROID;
    } else {
        return TileType::UNKNOWN_TILE;
    }
}

void GameTile::setTypeImmediate(TileType tileType) {    
    previousTypeImmediate = typeImmediate;
    typeImmediate = tileType;
    // log("Immediate type previous set to " + std::to_string(previousTypeImmediate) + " for tile (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

void GameTile::setType(TileType tileType, int step, bool driftIdentified) {
    if (!driftIdentified && tileType != TileType::UNKNOWN_TILE && (previousTypes.empty() || previousTypes.top() != tileType)) {
        previousTypes.push(tileType);
        previousTypeUpdateSteps.push(step);
    }

    previousType = type;
    type = tileType;

    previousTypeUpdateStep = typeUpdateStep;
    typeUpdateStep = step;    
}

TileType GameTile::getType() {
    return type;
}

std::stack<TileType>& GameTile::getPreviousTypes() {
    return previousTypes;
}

TileType GameTile::getPreviousTypeImmediate() {
    return previousTypeImmediate;
}

std::stack<int>& GameTile::getPreviousTypeUpdateSteps() {
    return previousTypeUpdateSteps;
}

TileType GameTile::getPreviousType() {
    return previousType;
}

int GameTile::getTypeUpdateStep() {
    return typeUpdateStep;
}

int GameTile::getPreviousTypeUpdateStep() {
    return previousTypeUpdateStep;
}

void GameTile::setEnergy(int energyValue, int time) {
    previousEnergy = energy;
    energy = energyValue;

    previousEnergyUpdateTime = lastEnergyUpdateTime;
    lastEnergyUpdateTime = time;
}

void GameTile::setEstimatedEnergy(int energy) {
    estimatedEnergy = energy;
}

int GameTile::getEnergy() {
    return energy;
}

int GameTile::getPreviousEnergy() {
    return previousEnergy;
}

int GameTile::getLastEnergyUpdateTime() {
    return lastEnergyUpdateTime;
}

int GameTile::getPreviousEnergyUpdateTime() {
    return previousEnergyUpdateTime;
}

int GameTile::getLastKnownEnergy() {
    if (estimatedEnergy >= -20 and estimatedEnergy <= 20) {
        return estimatedEnergy;
    }
    return energy;
}

std::string GameTile::toString() {
    return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
}

void GameTile::setRelicExplorationFrontier(bool value, int match) {
    switch(match) {
        case 0:
            relicExplorationFrontier1 = value;
            break;
        case 1:
            relicExplorationFrontier2 = value;
            break;
        case 2:
            relicExplorationFrontier3 = value;
            break;
        default:
            throw std::invalid_argument("Invalid match value: " + std::to_string(match));
    }
}

GameTile & GameMap::getMirroredTile(int x, int y) {
    return getTile(height - y - 1, width - x - 1);
}