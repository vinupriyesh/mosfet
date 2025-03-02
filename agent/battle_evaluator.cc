#include "battle_evaluator.h"
#include "logger.h"

#include <algorithm>
#include "game_env_config.h"
#include "constants.h"

void BattleEvaluator::log(const std::string& message) {
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

std::vector<int> BattleEvaluator::getOpponentsAt(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    auto& probabilities = opponentTracker.getOpponentPositionProbabilities();

    std::vector<int> shuttles;
    for (int s = 0; s < gameEnvConfig.maxUnits; s ++) {
        if (probabilities[s][x][y] > 1.0 - LOWEST_DOUBLE && probabilities[s][x][y] < 1.0 + LOWEST_DOUBLE) {
            shuttles.push_back(s);
        }
    }

    return shuttles;
}

bool BattleEvaluator::isNearPlayerShuttle(GameTile& tile) {
    
    for (int pmi = 0; pmi < POSSIBLE_NEIGHBORS_SIZE; pmi++) {
        int xNext = POSSIBLE_NEIGHBORS[pmi][0] + tile.x;
        int yNext = POSSIBLE_NEIGHBORS[pmi][1] + tile.y;

        if (gameMap.isValidTile(xNext, yNext)) {
            auto& tile = gameMap.getTile(xNext, yNext);

            if (tile.isOccupied()) {
                return true;
            }
        }
    }
    
    return false;
}

void BattleEvaluator::computeOpponentBattlePoints(int x, int y) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    auto& energies = opponentTracker.getOpponentMaxPossibleEnergies();

    int tileId = gameMap.getTile(x, y).getId(gameMap.width);
    int energyDiff = -gameEnvConfig.unitSapCost; //TODO: Remove this, we need to attack from multiple shuttles
    int kills = 0;
    bool rangedSapPossible = false;
    for (int i = x-1; i <= x+1; ++i) {
        for (int j = y-1; j <= y+1; ++j) {
            if (gameMap.isValidTile(i, j)) {
                auto& tile = gameMap.getTile(i, j);
                if (isNearPlayerShuttle(tile)) {
                    continue;
                }
                int sapCost = gameEnvConfig.unitSapCost * gameMap.derivedGameState.unitSapDropOffFactor;
                if (i == x and j == y) {
                    sapCost = gameEnvConfig.unitSapCost;
                }
                                
                for (auto& shuttleId : getOpponentsAt(i, j)) {

                    int maxPossibleEnergy = energies[shuttleId][i][j];

                    energyDiff += std::min(maxPossibleEnergy, sapCost);
                    if (maxPossibleEnergy < sapCost) {
                        // This shuttle is going
                        kills++;
                    }
                    if (!rangedSapPossible) {
                        rangedSapPossible = true;
                    }
                }
            }
        }
    }
    
    if (rangedSapPossible) {
        log("If we sap at " + std::to_string(x) + ", " + std::to_string(y) + " then energy diff will be " + std::to_string(energyDiff) + " and " + std::to_string(kills) + " lost units.");
        gameMap.getOpponentBattlePoints()[tileId] = std::make_pair(energyDiff, kills); 
    }
}

void BattleEvaluator::clear() {
    gameMap.getTeamBattlePoints().clear();
    gameMap.getOpponentBattlePoints().clear();
}
