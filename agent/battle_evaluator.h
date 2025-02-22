#ifndef BATTLE_EVALUATOR_H
#define BATTLE_EVALUATOR_H

#include "game_map.h"

class BattleEvaluator {
    private:
        static void log(std::string message);
        GameMap& gameMap;

    public:
        BattleEvaluator(GameMap& gameMap) : gameMap(gameMap) {}

        void computeTeamBattlePoints(int x, int y);
        void computeOpponentBattlePoints(int x, int y);

        void clear();
};

#endif // BATTLE_EVALUATOR_H