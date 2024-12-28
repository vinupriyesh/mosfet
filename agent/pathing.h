#include "game_map.h"

class PathingBase {
    protected:
        GameMap* gameMap;

    public:
        PathingBase(GameMap* gameMap): gameMap(gameMap) {};

};

class Pathing : public PathingBase {
    public:
        Pathing(GameMap* gameMap): PathingBase(gameMap) {};
        
        std::unordered_map<GameTile*, std::pair<int, std::vector<GameTile*>>> findAllPaths(GameTile& startTile);

};