#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "logger.h"
#include "agent/relic.h"
#include <vector>

class Shuttle; //Forward declaration

enum Direction {
    CENTER,
    UP,
    RIGHT,
    DOWN,
    LEFT    
};

/**
 Function to convert Direction to int
*/
inline int directionToInt(Direction direction) {
    return static_cast<int>(direction);
}

/**
 * Overload the ++ operator for the Direction enum to be used in loops
 * 
 * Usage:for (Direction direction = Direction::UP; direction < Direction::CENTER; ++direction) {
 * 
 */
inline Direction& operator++(Direction& c) {
    c = static_cast<Direction>(static_cast<int>(c) + 1);
    return c; 
}

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
        TileType lastKnownTileType;
        bool visited;
        bool explored;
        bool haloTile;
        bool vantagePoint;
        bool forcedRegularTile;
        Relic* relic;
        std::vector<Shuttle*> shuttles;
        int energy;

        int lastVisitedTime;
        int lastExploredTime;
        int lastEnergyUpdateTime;
        int lastTypeUpdateTime; 

    public:
        int x;
        int y;
        
        GameTile(int x, int y) : x(x), y(y), visited(false), explored(false), haloTile(false), vantagePoint(false),
                forcedRegularTile(false), relic(nullptr), shuttles({}) {};
        int getId(int width);
        bool isVisited() { return visited; };
        bool isExplored() { return explored; };
        bool isHaloTile() { return haloTile; };
        bool isVantagePoint() { return vantagePoint; };
        bool isForcedRegularTile() { return forcedRegularTile; };
        int getLastVisitedTime() { return lastVisitedTime; };

        TileType getType() const;
        TileType getLastKnownType() const;

        void setVisited(bool visited, int time);
        void setExplored(bool explored, int time);
        void setHaloTile(bool haloTile) { this->haloTile = haloTile; };
        void setVantagePoint(bool vantagePoint) { this->vantagePoint = vantagePoint; };
        void setForcedRegularTile(bool forcedRegularTile) { this->forcedRegularTile = forcedRegularTile; };
        void setRelic(Relic* relic) { this->relic = relic; };
        void addShuttle(Shuttle* shuttle);
        bool isOccupied();
        std::vector<Shuttle*>& getShuttles() { return shuttles; };
        void clearShuttle(Shuttle *shuttle);
        void clearShuttles();

        TileType setType(int tileTypeCode, int time);
        void setEnergy(int energy, int time);
        
};

class GameMap {
    private:
        void log(std::string message);
        std::vector<std::vector<GameTile>> map;

    public:
        
        int width;
        int height;
        GameMap(int width, int height);
        void addRelic(Relic* relic, int currentStep);
        bool hasPotentialInvisibleRelicNode(GameTile &gameTile);
        bool isValidTile(int x, int y);
        GameTile& getTile(int x, int y);

        GameTile& getTile(GameTile &fromTile, Direction direction);

        std::tuple<bool, GameTile&> isMovable(GameTile& fromTile, Direction direction);
        
};

#endif // GAMEMAP_H