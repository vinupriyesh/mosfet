#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "logger.h"
#include "agent/relic.h"
#include "shuttle_data.h"
#include "game_env_config.h"

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
        TileType previousType;
        bool visible;
        bool visited;
        bool explored;
        bool unexploredFrontier;
        bool haloTile;
        bool vantagePoint;
        bool forcedRegularTile;
        Relic* relic;
        int energy;

        int lastVisitedTime;
        int lastExploredTime;
        int lastEnergyUpdateTime;
        int typeUpdateStep;
        int previousTypeUpdateStep;

    public:
        int x;
        int y;
        int manhattanFromOrigin;
        int manhattanToOpponentOrigin;

        std::vector<ShuttleData*> shuttles;
        std::vector<ShuttleData*> opponentShuttles;
        
        GameTile(int x, int y) : x(x), y(y), visited(false), explored(false), haloTile(false), vantagePoint(false),
                forcedRegularTile(false), relic(nullptr), shuttles({}), type(TileType::UNKNOWN), previousType(TileType::UNKNOWN),
                typeUpdateStep(-1), previousTypeUpdateStep(-1) {};
        int getId(int width);        
        bool isVisible() {return visible;};
        bool isVisited() { return visited; };
        bool isExplored() { return explored; };
        bool isUnExploredFrontier() { return unexploredFrontier; };
        bool isHaloTile() { return haloTile; };
        bool isVantagePoint() { return vantagePoint; };
        bool isForcedRegularTile() { return forcedRegularTile; };
        bool isOccupied();
        bool isOpponentOccupied();
        int getLastVisitedTime() { return lastVisitedTime; };
        TileType getType() const;

        void setVisible(bool visible) { this->visible = visible; };
        void setVisited(bool visited, int time);
        void setExplored(bool explored, int time);
        void setUnexploredFrontier(bool unexploredFrontier) { this->unexploredFrontier = unexploredFrontier; };
        void setHaloTile(bool haloTile) { this->haloTile = haloTile; };
        void setVantagePoint(bool vantagePoint) { this->vantagePoint = vantagePoint; };
        void setForcedRegularTile(bool forcedRegularTile) { this->forcedRegularTile = forcedRegularTile; };
        void setRelic(Relic* relic) { this->relic = relic; };
        void addShuttle(ShuttleData* shuttle);
        void addOpponentShuttle(ShuttleData* shuttle);

        std::vector<ShuttleData*>& getShuttles() { return shuttles; };
        void clearShuttle(ShuttleData *shuttle);
        void clearShuttles();
        void clearOpponentShuttles();

        void setType(TileType tileType, int time);
        TileType getType();
        TileType getPreviousType();
        int getTypeUpdateStep();
        int getPreviousTypeUpdateStep();
        void setEnergy(int energy, int time);
        int getEnergy();
        int getLastKnownEnergy();

        static TileType translateTileType(int tileTypeCode);
        std::string toString();
        
};

enum RelicDiscoveryStatus {
    INIT,
    SEARCHING,
    FOUND,
    NOT_FOUND,
    NOT_APPLICABLE
};

struct DerivedGameState {

    private:
        inline void log(std::string message) {
            Logger::getInstance().log("DerivedGameState -> " + message);
        }

    public:
        int teamPoints;
        int opponentTeamPoints;
        int teamWins;
        int opponentWins;
        int teamPointsDelta;
        int opponentTeamPointsDelta;
        int currentMatch = -1;
        int currentStep;
        int currentMatchStep;
        int remainingOverageTime;

        bool allTilesExplored = false;
        bool allTilesVisited = false;
        
        int tilesExplored = 0;
        int tilesVisited = 0;
        int vantagePointsFound = 0;
        int vantagePointsOccupied = 0;

        std::vector<RelicDiscoveryStatus> relicDiscoveryStatus;

        inline bool isThereAHuntForRelic() {
            for (int i = currentMatch; i >= 0 ; i--) {
                if (relicDiscoveryStatus[i] == RelicDiscoveryStatus::SEARCHING) {
                    // log("There is hunt for relic from match " + std::to_string(i) + ", currentMatch = " + std::to_string(currentMatch));
                    return true;
                }
            }
            return false;
        }

        inline void setRelicDiscoveryStatus(RelicDiscoveryStatus status) {
            bool success = false;
            for (int i = currentMatch; i >= 0 ; i--) {
                if (relicDiscoveryStatus[i] == RelicDiscoveryStatus::SEARCHING) {
                    relicDiscoveryStatus[i] = status;
                    log("Relic search for match #" + std::to_string(i+1) + " is found in match #" + std::to_string(currentMatch + 1));
                    success = true;
                    break;
                }
            }
            if (!success) {
                log("Problem: The relic discovery status doesn't match with any searching relics");
                std::cerr<<"Problem: The relic discovery status cannot be set to any match"<<std::endl;
            }
        }
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
        void exploreTile(GameTile &tile, int currentStep);
        bool isValidTile(int x, int y);
        GameTile& getTile(int x, int y);
        GameTile& getMirroredTile(int x, int y);

        GameTile& getTile(GameTile &fromTile, Direction direction);

        std::tuple<bool, GameTile&> isMovable(GameTile& fromTile, Direction direction);

        void getAllOpponentsInRadius(int radius, int x, int y, std::vector<ShuttleData*>& opponents);
        
        
};

#endif // GAMEMAP_H