#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "logger.h"
#include <vector>

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

enum TileType {
    UNKNOWN,
    EMPTY,
    NEBULA,
    ASTEROID
};
    
class GameTile {    

    private:    
        void log(std::string message);
        TileType type;
        bool visited;
        bool explored;
        int energy;

        int lastVisitedTime;
        int lastExploredTime;
        int lastEnergyUpdateTime;
        int lastTypeUpdateTime; 

    public:
        int x;
        int y;
        
        GameTile(int x, int y) : x(x), y(y), visited(false), explored(false) {};
        bool isVisited() { return visited; };
        bool isExplored() { return explored; };

        TileType getType() const;

        void setVisited(bool visited, int time);
        void setExplored(bool explored, int time);

        TileType setType(int tileTypeCode, int time);
        void setEnergy(int energy, int time);
        
};

class GameMap {
    private:
        void log(std::string message);
        int width;
        int height;
        std::vector<std::vector<GameTile>> map;

    public:
        
        GameMap(int width, int height);
        bool isValidTile(int x, int y);
        GameTile& getTile(int x, int y);

        GameTile& getTile(GameTile &fromTile, Direction direction);

        std::tuple<bool, GameTile&> isMovable(GameTile& fromTile, Direction direction);
};

#endif // GAMEMAP_H