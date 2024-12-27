#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>

enum TileType {
    EMPTY,
    NEBULA,
    ASTEROID
};
    
class GameTile {    
    int x;
    int y;
    bool isVisited;
    bool isExplored;

    public:
        GameTile(int x, int y) : x(x), y(y), isVisited(false), isExplored(false) {};
        void setVisited(bool visited) { isVisited = visited; };
        void setExplored(bool explored) { isExplored = explored;};     
};

class GameMap {
    private:
        int width;
        int height;
        std::vector<std::vector<GameTile>> map;
    public:    
        GameMap(int width, int height);
        GameTile& getTile(int x, int y) { return map[y][x]; };
};

#endif // GAMEMAP_H