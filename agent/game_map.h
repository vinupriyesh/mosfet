#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "logger.h"
#include "agent/relic.h"
#include "shuttle_data.h"

#include <vector>
#include <stack>
#include <map>

enum Direction : std::uint8_t {
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

enum TileType : std::uint8_t {
    UNKNOWN,
    EMPTY,
    NEBULA,
    ASTEROID
};
    
class GameTile {    

    private:    
        static void log(const std::string& message);
        TileType type;
        TileType previousType;
        std::stack<TileType> previousTypes;
        bool visible;
        bool visited;
        bool explored;
        bool unexploredFrontier;
        bool relicExplorationFrontier1;
        bool relicExplorationFrontier2;
        bool relicExplorationFrontier3;
        bool haloTile;
        bool vantagePoint;
        bool forcedRegularTile;
        Relic* relic;
        int energy;
        int estimatedEnergy;
        int previousEnergy;

        int lastVisitedTime;
        int lastExploredTime;
        int lastEnergyUpdateTime;
        int previousEnergyUpdateTime;
        int typeUpdateStep;
        int previousTypeUpdateStep;
        std::stack<int> previousTypeUpdateSteps;

    public:
        int x;
        int y;
        int manhattanFromOrigin;
        int manhattanToOpponentOrigin;

        std::vector<ShuttleData*> shuttles;
        std::vector<ShuttleData*> opponentShuttles;
        
        GameTile(int x, int y) : x(x), y(y), visited(false), explored(false), haloTile(false), vantagePoint(false),
                 unexploredFrontier(false), relicExplorationFrontier1(false), relicExplorationFrontier2(false), relicExplorationFrontier3(false),
                forcedRegularTile(false), relic(nullptr), shuttles({}), type(TileType::UNKNOWN), previousType(TileType::UNKNOWN),
                energy(-1), estimatedEnergy(-21), previousEnergy(-1), lastVisitedTime(-1), lastExploredTime(-1), lastEnergyUpdateTime(-1), previousEnergyUpdateTime(-1), visible(false),
                previousTypes(), typeUpdateStep(-1), previousTypeUpdateStep(-1), previousTypeUpdateSteps() {};
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
        int getLastExploredTime() { return lastExploredTime; };
        TileType getType() const;

        bool isRelicExplorationFrontier1() {return relicExplorationFrontier1;};
        bool isRelicExplorationFrontier2() {return relicExplorationFrontier2;};
        bool isRelicExplorationFrontier3() {return relicExplorationFrontier3;};

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

        void setType(TileType tileType, int time, bool driftIdentified);
        TileType getType();
        std::stack<TileType>& getPreviousTypes();
        std::stack<int>& getPreviousTypeUpdateSteps();
        TileType getPreviousType();
        int getTypeUpdateStep();
        int getPreviousTypeUpdateStep();
        void setEnergy(int energy, int time);
        void setEstimatedEnergy(int energy);
        int getEnergy();
        int getPreviousEnergy();
        int getLastEnergyUpdateTime();
        int getPreviousEnergyUpdateTime();
        int getLastKnownEnergy();

        static TileType translateTileType(int tileTypeCode);
        std::string toString();

        void setRelicExplorationFrontier(bool value, int match);
        
};

enum RelicDiscoveryStatus : std::uint8_t {
    INIT,
    SEARCHING,
    FOUND,
    NOT_FOUND,
    NOT_APPLICABLE
};

struct DerivedGameState {

    private:
        inline void log(const std::string& message) {
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

        float unitSapDropOffFactor = 0.25f;
        bool unitSapDropOffFactorSet = false;

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
            for (int i = 0; i <= currentMatch ; i++) {
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
        static void log(const std::string& message);
        std::vector<std::vector<GameTile>> map;
        std::vector< std::vector<std::vector<TileType>>* > driftAwareTileType; //<stepId, y, x>
        std::map<int, std::pair<int, int>> opponentBattlePoints; //<tileId, <energyDiff, kills>> +ve energyDiff means we lose less energy than opponent
        std::map<int, std::pair<int, int>> teamBattlePoints; //<tileId, <energyDiff, kills>> +ve energyDiff means we lose less energy than opponent
    public:
        
        int width;
        int height;
        DerivedGameState derivedGameState;

        std::vector<ShuttleData*> shuttles;
        std::vector<ShuttleData*> opponentShuttles;

        GameMap(int width, int height);
        void addRelic(Relic* relic, int currentStep, std::vector<int>& haloTileIds);
        bool hasPotentialInvisibleRelicNode(GameTile &gameTile);
        GameTile *getTileAtPosition(ShuttleData &shuttleData);
        void exploreTile(GameTile &tile, int currentStep);
        bool isValidTile(int x, int y);
        GameTile& getTile(int x, int y);
        GameTile& getRolledOverTile(int x, int y);
        GameTile& getMirroredTile(int x, int y);

        GameTile& getTile(GameTile &fromTile, Direction direction);

        TileType getEstimatedType(GameTile& tile, int step) const;

        std::tuple<bool, GameTile&> isMovable(GameTile& fromTile, Direction direction);

        void getAllOpponentsInRadius(int radius, int x, int y, std::vector<ShuttleData*>& opponents);
        std::vector< std::vector<std::vector<TileType>>* >& getDriftAwareTileType() {return driftAwareTileType;};
        std::map<int, std::pair<int, int>>& getOpponentBattlePoints() {return opponentBattlePoints;};
        std::map<int, std::pair<int, int>>& getTeamBattlePoints() {return teamBattlePoints;};
        void setRelicExplorationFrontier(GameTile &tile, int match, int cutoffTime);
};

#endif // GAMEMAP_H