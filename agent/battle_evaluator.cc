#include "battle_evaluator.h"
#include "logger.h"

#include <algorithm>
#include "game_env_config.h"

void BattleEvaluator::log(std::string message) {
    Logger::getInstance().log("BattleEvaluator -> " + message);
}

void BattleEvaluator::computeTeamBattlePoints(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int tileId = gameMap.getTile(x, y).getId(gameMap.width);
    int energyDiff = gameEnvConfig.unitSapCost;
    int kills = 0;
    bool attackPossible = false;
    for (int i = x-1; i <= x+1; ++i) {
        for (int j = y-1; j <= y+1; ++j) {
            if (gameMap.isValidTile(i, j)) {
                auto& tile = gameMap.getTile(i, j);
                int sapCost = gameEnvConfig.unitSapCost * gameMap.derivedGameState.unitSapDropOffFactor;
                if (i == x and j == y) {
                    sapCost = gameEnvConfig.unitSapCost;
                }
                for (auto& shuttle : tile.shuttles) {
                    if (!shuttle->visible || shuttle->ghost) {
                        continue;
                    }
                    energyDiff -= std::min(shuttle->energy, sapCost);
                    if (shuttle->energy < sapCost) {
                        // This shuttle is going
                        kills++;
                    }
                    if (!attackPossible) {
                        attackPossible = true;
                    }
                }
            }
        }
    }
    
    if (attackPossible) {
        log("If we get sapped at " + std::to_string(x) + ", " + std::to_string(y) + " then energy diff will be " + std::to_string(energyDiff) + " and " + std::to_string(kills) + " kills");
        gameMap.getTeamBattlePoints()[tileId] = std::make_pair(energyDiff, kills);
    }
}

void BattleEvaluator::computeOpponentBattlePoints(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    int tileId = gameMap.getTile(x, y).getId(gameMap.width);
    int energyDiff = -gameEnvConfig.unitSapCost;
    int kills = 0;
    bool attackPossible = false;
    for (int i = x-1; i <= x+1; ++i) {
        for (int j = y-1; j <= y+1; ++j) {
            if (gameMap.isValidTile(i, j)) {
                auto& tile = gameMap.getTile(i, j);
                int sapCost = gameEnvConfig.unitSapCost * gameMap.derivedGameState.unitSapDropOffFactor;
                if (i == x and j == y) {
                    sapCost = gameEnvConfig.unitSapCost;
                }
                for (auto& shuttle : tile.opponentShuttles) {
                    if (!shuttle->visible || shuttle->ghost) {
                        continue;
                    }
                    energyDiff += std::min(shuttle->energy, sapCost);
                    if (shuttle->energy < sapCost) {
                        // This shuttle is going
                        kills++;
                    }
                    if (!attackPossible) {
                        attackPossible = true;
                    }
                }
            }
        }
    }
    if (attackPossible) {
        log("If we sap at " + std::to_string(x) + ", " + std::to_string(y) + " then energy diff will be " + std::to_string(energyDiff) + " and " + std::to_string(kills) + " lost units.");
        gameMap.getOpponentBattlePoints()[tileId] = std::make_pair(energyDiff, kills); 
    }
}

void BattleEvaluator::clear() {
    gameMap.getTeamBattlePoints().clear();
    gameMap.getOpponentBattlePoints().clear();
}
