#ifndef PATHING_H
#define PATHING_H

#include "agent/game_map.h"
#include "datastructures/iterable_priority_queue.h"

struct PathingConfig {
    bool stopAtHaloTiles; // Halo tiles are not explored
    bool stopAtUnexploredTiles; // Unexplored tiles can be at the leaf node, but they will not be explored further
    bool stopAtVisitedTiles; // Visited tiles can be at the leaf node, but they will not be explored further
    bool stopAtVantagePointTiles; // Vantage point tiles are not explored
    bool captureHaloTileDestinations; // Captures halo tiles destinations
    bool captureUnexploredTileDestinations; // Captures unexplored tiles destinations
    bool captureUnVisitedTileDestinations; // Captures unvisited tiles destinations
    bool captureVantagePointTileDestinations; // Captures vantage point tiles destinations
};

class PathingBase {
    protected:
        GameMap* gameMap;

    public:
        PathingBase(GameMap* gameMap): gameMap(gameMap) {};

};


// struct PathingConfig : public PathingConfigBase {
//     int maxDistance;
// };

class Pathing : public PathingBase {
    private:
        PathingConfig config;
        
    public:
        using TileDistancePair = std::pair<int, GameTile*>;
        /**
         *  Will be populated only if captureUnexploredTileDestinations is true, closest tiles first
         */
        IterablePriorityQueue<TileDistancePair> unexploredDestinations;

        /**
         *  Will be populated only if captureUnVisitedTileDestinations is true, closest tiles first
         */
        IterablePriorityQueue<TileDistancePair> unvisitedDestinations;

        /**
         * Will be populated only if captureHaloTileDestinations is true, closest tiles first
         */
        IterablePriorityQueue<TileDistancePair> haloDestinations;

        /**
         * Will be populated only if captureVantagePointTileDestinations is true, closest tiles first
         */
        IterablePriorityQueue<TileDistancePair> vantagePointDestinations;

        /**
         * The main output having all the distances and paths
         */
        std::unordered_map<GameTile*, std::pair<int, std::vector<GameTile*>>> distances;

        Pathing(GameMap* gameMap, PathingConfig config): PathingBase(gameMap), config(config) {};
        
        void findAllPaths(GameTile& startTile);

};

#endif // PATHING_H