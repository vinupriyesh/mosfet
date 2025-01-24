#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "logger.h"
#include "agent/relic.h"
#include "shuttle_data.h"

#include <vector>

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
        bool visible;
        bool visited;
        bool explored;
        bool haloTile;
        bool vantagePoint;
        bool forcedRegularTile;
        Relic* relic;
        int energy;

        int lastVisitedTime;
        int lastExploredTime;
        int lastEnergyUpdateTime;
        int lastTypeUpdateTime; 

    public:
        int x;
        int y;

        std::vector<ShuttleData*> shuttles;
        std::vector<ShuttleData*> opponentShuttles;
        
        GameTile(int x, int y) : x(x), y(y), visited(false), explored(false), haloTile(false), vantagePoint(false),
                forcedRegularTile(false), relic(nullptr), shuttles({}), lastKnownTileType(UNKNOWN) {};
        int getId(int width);
        bool isVisible() {return visible;};
        bool isVisited() { return visited; };
        bool isExplored() { return explored; };
        bool isHaloTile() { return haloTile; };
        bool isVantagePoint() { return vantagePoint; };
        bool isForcedRegularTile() { return forcedRegularTile; };
        int getLastVisitedTime() { return lastVisitedTime; };

        TileType getType() const;
        TileType getLastKnownType() const;

        void setVisible(bool visible) { this->visible = visible; };
        void setVisited(bool visited, int time);
        void setExplored(bool explored, int time);
        void setHaloTile(bool haloTile) { this->haloTile = haloTile; };
        void setVantagePoint(bool vantagePoint) { this->vantagePoint = vantagePoint; };
        void setForcedRegularTile(bool forcedRegularTile) { this->forcedRegularTile = forcedRegularTile; };
        void setRelic(Relic* relic) { this->relic = relic; };
        void addShuttle(ShuttleData* shuttle);
        void addOpponentShuttle(ShuttleData* shuttle);
        bool isOccupied();
        bool isOpponentOccupied();        
        std::vector<ShuttleData*>& getShuttles() { return shuttles; };
        void clearShuttle(ShuttleData *shuttle);
        void clearShuttles();
        void clearOpponentShuttles();

        TileType setType(int tileTypeCode, int time);
        TileType getLastKnownTileType();
        void setEnergy(int energy, int time);
        int getEnergy();
        int getLastKnownEnergy();

        std::string toString();
        
};

struct DerivedGameState {
    int teamPoints;
    int opponentTeamPoints;
    int teamPointsDelta;
    int opponentTeamPointsDelta;
    int currentStep;
    int currentMatchStep;
    int remainingOverageTime;

    bool allRelicsFound = false;
    bool allTilesExplored = false;
    bool allTilesVisited = false;

    int relicsFound = 0;
    int tilesExplored = 0;
    int tilesVisited = 0;
    int vantagePointsFound = 0;
    int vantagePointsOccupied = 0;
};

class GameMap {
    private:
        void log(std::string message);
        std::vector<std::vector<GameTile>> map;

    public:
        
        int width;
        int height;
        DerivedGameState derivedGameState;
        GameMap(int width, int height);
        void addRelic(Relic* relic, int currentStep, std::vector<int>& haloTileIds);
        bool hasPotentialInvisibleRelicNode(GameTile &gameTile);
        GameTile *getTileAtPosition(ShuttleData &shuttleData);
        bool isValidTile(int x, int y);
        GameTile& getTile(int x, int y);

        GameTile& getTile(GameTile &fromTile, Direction direction);

        std::tuple<bool, GameTile&> isMovable(GameTile& fromTile, Direction direction);

        void getAllOpponentsInRadious(int radius, int x, int y, std::vector<ShuttleData*>& opponents);
        
        
};

#endif // GAMEMAP_H