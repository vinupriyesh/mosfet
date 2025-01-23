#include "agent_role.h"

DefenderAgentRole::DefenderAgentRole(Shuttle *shuttles, GameMap& gameMap) : AgentRole(shuttles, gameMap) {
    roleClassName = "DefenderAgentRole";
}

bool DefenderAgentRole::isRolePossible() {

    if (unableToAct) {
        return false;
    }

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    if (shuttle->energy <= gameEnvConfig.unitSapCost + gameEnvConfig.unitMoveCost) { //Have energy to attack and move
        return false;
    }
    
    opponents.clear();
    bool canSap = false;
    std::vector<Shuttle*> allOpponents;
    gameMap.getAllOpponentsInRadious(gameEnvConfig.unitSapRange, shuttle->getX(), shuttle->getY(), allOpponents);
    for (auto& opponent : allOpponents) {
        if (opponent->isGhost()) {
            continue;
        }
        if (opponent->energy <= gameEnvConfig.unitSapCost) {
            canSap = true;
            opponents.push_back(opponent);
        }
    }

    return canSap;
}

void DefenderAgentRole::iteratePlan(int planIteration, Communicator &communicator) {
    unableToAct = false;

    if (opponents.empty()) {
        unableToAct = true;
        return;
    }    
}

bool DefenderAgentRole::chooseAttackingTile(Communicator &communicator) {

    if (opponents.empty()) {
        return false;
    }

    Shuttle* opponent = opponents[0];

    for (auto& opponent : opponents) {
        GameTile* opponentTile = opponent->getTileAtPosition();
        if (opponentTile == nullptr) {
            continue;
        }

        attackingTileId = opponentTile->getId(gameMap.width);

        if (communicator.attackingTileIds.find(attackingTileId) == communicator.attackingTileIds.end()) {
            communicator.attackingTileIds.insert(attackingTileId);
            std::tuple<int, int> relativePosition = getRelativePosition(*opponentTile);
            bestPlan = {5, std::get<0>(relativePosition), std::get<1>(relativePosition)};
            log("Attacking tile - " + std::to_string(opponentTile->x) + ", " + std::to_string(opponentTile->y));
            return true;            
        }
    }

    return false;

}
