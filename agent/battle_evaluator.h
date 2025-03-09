#ifndef BATTLE_EVALUATOR_H
#define BATTLE_EVALUATOR_H

#include "agent/opponent_tracker.h"
#include "game_map.h"
#include <string>

#include "symmetry_util.h"

struct TileEvaluation {
    int tileId;
    int possibleCumulativeOpponentEnergy;
    bool isRelicMiningOpponent;
    int possibleKills;

    std::string toString() {
        int x, y;
        symmetry_utils::toXY(tileId, x, y);
        return "If we sap at " + std::to_string(x) + ", " + std::to_string(y) + " then energy diff will be " + std::to_string(possibleCumulativeOpponentEnergy) 
        + " and " + std::to_string(possibleKills) + " lost units. Opponent is mining? " + std::to_string(isRelicMiningOpponent);
    }
};

class BattleEvaluator {
    private:
        static void log(const std::string& message);
        GameMap& gameMap;
        OpponentTracker& opponentTracker;

        std::vector<int> getOpponentsAt(int x, int y);
        bool isNearPlayerShuttle(GameTile& tile);
                
    public:

        std::unordered_map<int, TileEvaluation> opponentBattlePoints;

        std::unordered_map<int, std::tuple<int, int, int>> crashCollisionPossibilities; // OpponentEnergy, Kills, ShuttleId

        BattleEvaluator(GameMap& gameMap, OpponentTracker& opponentTracker) : gameMap(gameMap), opponentTracker(opponentTracker) {}        

        void computeTeamBattlePoints(int x, int y);
        void computeOpponentBattlePoints(int x, int y);

        void announceSOSSingals();
        void announceCollision(int shuttleId);
        void computeCrashCollisionPossibilities();

        void clear();

        
};

#endif // BATTLE_EVALUATOR_H