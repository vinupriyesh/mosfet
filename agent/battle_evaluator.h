#ifndef BATTLE_EVALUATOR_H
#define BATTLE_EVALUATOR_H

#include "agent/opponent_tracker.h"
#include "game_map.h"

class BattleEvaluator {
    private:
        static void log(const std::string& message);
        GameMap& gameMap;
        OpponentTracker& opponentTracker;

        std::vector<int> getOpponentsAt(int x, int y);
        bool isNearPlayerShuttle(GameTile& tile);
        

    public:
        BattleEvaluator(GameMap& gameMap, OpponentTracker& opponentTracker) : gameMap(gameMap), opponentTracker(opponentTracker) {}

        void computeTeamBattlePoints(int x, int y);
        void computeOpponentBattlePoints(int x, int y);

        void clear();
};

#endif // BATTLE_EVALUATOR_H