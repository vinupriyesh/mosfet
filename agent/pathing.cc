
#include <limits>
#include "agent/pathing.h"


void Pathing::log(std::string message) {
    Logger::getInstance().log("Pathing -> " + message);
}

float Pathing::getCost(GameTile &neighbor) {
    if (config.pathingHeuristics == SHORTEST_DISTANCE) {
        return 1;
    } 

    if (config.pathingHeuristics == LEAST_ENERGY) {
        // Loss calculation for each step
        int energyGain = neighbor.getLastKnownEnergy();
        // log(neighbor.toString() + "'s energy - " + std::to_string(energyGain));
        if (energyGain < -10) {
            // log("Capping the -ve energy loss to -10" + std::to_string(energyLoss));
            energyGain = -10;
        } else if (energyGain > 10) {
            energyGain = 10;
        }

        if (neighbor.getType() == TileType::NEBULA) {
            energyGain -= GameEnvConfig::getInstance().nebulaTileEnergyReduction;
        }

        return 10 + GameEnvConfig::getInstance().unitMoveCost - energyGain; // EnergyGainOffset + UnitMoveCost - EnergyGainInNewTile
    }

    throw std::runtime_error("Unknown heuristic to compute distance");
}

void Pathing::findAllPaths(GameTile &startTile) {
    // Define a priority queue to store tiles with their distances    
    std::priority_queue<TileDistancePair, std::vector<TileDistancePair>, std::greater<TileDistancePair>> pq;


    // Initialize distances to infinity and paths to empty
    for (int y = 0; y < gameMap.height; ++y) {
        for (int x = 0; x < gameMap.width; ++x) {
            GameTile& tile = gameMap.getTile(x, y);
            distances[&tile] = {std::numeric_limits<int>::max(), {}};
        }
    }

    // Set the distance to the start tile to 0 and push it to the priority queue
    distances[&startTile] = {0, {&startTile}};
    pq.push({0, &startTile});

    // Dijkstra's algorithm
    while (!pq.empty()) {
        auto [currentDistance, currentTile] = pq.top();
        pq.pop();

        // Skip if the current distance is greater than the recorded distance
        if (currentDistance > distances[currentTile].first) {
            continue;
        }

        allDestinations.push({currentDistance, currentTile});

        // Record this tile if it is explored
        if (config.captureUnexploredTileDestinations && !currentTile->isExplored()) {
            unexploredDestinations.push({currentDistance, currentTile});
        }

        // Record this tile if it is unvisited
        if (config.captureUnVisitedTileDestinations && !currentTile->isVisited()) {
            unvisitedDestinations.push({currentDistance, currentTile});
        }

        // Record this tile if it is a halo tile
        if (config.captureHaloTileDestinations && currentTile->isHaloTile()) {
            haloDestinations.push({currentDistance, currentTile});
        }

        if (config.captureVantagePointTileDestinations && currentTile->isVantagePoint()) {
            vantagePointDestinations.push({currentDistance, currentTile});
        }

        //Do not explore further if the tile is unexplored and the config is set to stop at unexplored tiles
        if (config.stopAtUnexploredTiles && !currentTile->isExplored()) {
            continue;
        }

        //Do not explore further if the tile is visited and the config is set to stop at visited tiles
        if (config.stopAtVisitedTiles && currentTile->isVisited()) {
            continue;
        }

        //Do not explore further if the tile is a halo tile and the config is set to stop at halo tiles
        if (config.stopAtHaloTiles && currentTile->isHaloTile()) {
            continue;
        }

        //Do not explore further if the tile is a vantage point and the config is set to stop at vantage point tiles
        if (config.stopAtVantagePointTiles && currentTile->isVantagePoint()) {
            continue;
        }

        // Explore neighbors
        for (Direction direction = Direction::UP; direction <= Direction::LEFT; ++direction) {
            std::tuple<bool, GameTile&> result = gameMap.isMovable(*currentTile, direction);
            bool movable = std::get<0>(result);

            // Skip if the tile is not movable
            if (!movable) {
                continue;
            }

            // Skip if the tile is occupied by an opponent shuttle
            if (config.doNotBumpIntoOpponentShuttles && std::get<1>(result).isOpponentOccupied() 
                && !std::get<1>(result).isVantagePoint() && !std::get<1>(result).isHaloTile()) {
                continue;
            }

            GameTile& neighbor = std::get<1>(result);

            int neighborCost = getCost(neighbor);
            int newDistance = currentDistance + neighborCost;

            // log("distance from " + currentTile->toString() + " to " + neighbor.toString() + " is " + std::to_string(neighborCost));

            // If a shorter path to the neighbor is found
            if (newDistance < distances[&neighbor].first) {
                distances[&neighbor].first = newDistance;
                distances[&neighbor].second = distances[currentTile].second;
                distances[&neighbor].second.push_back(&neighbor);
                pq.push({newDistance, &neighbor});
            }            
        }
    }
}
