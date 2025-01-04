#include <queue>
#include <vector>
#include <unordered_map>
#include <limits>
#include "agent/pathing.h"

void Pathing::findAllPaths(GameTile &startTile) {
    // Define a priority queue to store tiles with their distances    
    std::priority_queue<TileDistancePair, std::vector<TileDistancePair>, std::greater<TileDistancePair>> pq;


    // Initialize distances to infinity and paths to empty
    for (int y = 0; y < gameMap->height; ++y) {
        for (int x = 0; x < gameMap->width; ++x) {
            GameTile& tile = gameMap->getTile(x, y);
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

        // Skip if the current distance is greater than the recorded distance
        if (currentDistance > distances[currentTile].first) {
            continue;
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
            std::tuple<bool, GameTile&> result = gameMap->isMovable(*currentTile, direction);
            bool movable = std::get<0>(result);

            // Skip if the tile is not movable
            if (!movable) {
                continue;
            }

            GameTile& neighbor = std::get<1>(result);

            // Loss calculation for each step
            int energyLoss = neighbor.getEnergy();
            if (energyLoss < -10) {
                // log("Capping the -ve energy loss to -10" + std::to_string(energyLoss));
                energyLoss = -10;
            } else if (energyLoss > 10) {
                energyLoss = 10;
            }

            if (neighbor.getLastKnownTileType() == TileType::NEBULA) {
                energyLoss -= 10;
            }

            int newDistance = currentDistance + 11 - energyLoss; // 10 (bias of negative energy) + 4 move_cost - max(tile energy_cost)

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
