#include <queue>
#include <vector>
#include <unordered_map>
#include <limits>
#include "pathing.h"
#include "game_map.h"

std::unordered_map<GameTile*, std::pair<int, std::vector<GameTile*>>> Pathing::findAllPaths(GameTile &startTile) {
    // Define a priority queue to store tiles with their distances
    using TileDistancePair = std::pair<int, GameTile*>;
    std::priority_queue<TileDistancePair, std::vector<TileDistancePair>, std::greater<TileDistancePair>> pq;

    // Define a map to store the shortest distance to each tile and the path to reach it
    std::unordered_map<GameTile*, std::pair<int, std::vector<GameTile*>>> distances;

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

        // Skip if the current distance is greater than the recorded distance
        if (currentDistance > distances[currentTile].first) {
            continue;
        }

        // Explore neighbors
        for (Direction direction = Direction::UP; direction < Direction::CENTER; ++direction) {
            try {
                GameTile& neighbor = gameMap->getTile(*currentTile, direction);
                int newDistance = currentDistance + 1; // Assuming uniform cost as it is a timestep

                // If a shorter path to the neighbor is found
                if (newDistance < distances[&neighbor].first) {
                    distances[&neighbor].first = newDistance;
                    distances[&neighbor].second = distances[currentTile].second;
                    distances[&neighbor].second.push_back(&neighbor);
                    pq.push({newDistance, &neighbor});
                }
            } catch (const std::out_of_range&) {
                // Ignore out-of-bounds tiles
            }
        }
    }

    return distances;
}
