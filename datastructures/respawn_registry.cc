#include "respawn_registry.h"
#include "logger.h"

#include <iostream>
#include <sstream>

void RespawnRegistry::log(std::string message) {
    Logger::getInstance().log("RespawnRegistry -> " + message);
}

int RespawnRegistry::pushPlayerUnit(int unitId, int step){
    int delayTimer = 3;

    if (currentPlayerCooldownStep == 0) {
        delayTimer = 0;        
    }

    currentPlayerCooldownStep = std::max(currentPlayerCooldownStep, step) + delayTimer;
    playerUnitRespawnRecord[unitId] = currentPlayerCooldownStep;
    playerUnitRespawnIndex[currentPlayerCooldownStep] = unitId;

    log("Player unit " + std::to_string(unitId) + " will respawn at " + std::to_string(currentPlayerCooldownStep));
    return currentPlayerCooldownStep;
}

int RespawnRegistry::pushOpponentUnit(int unitId, int step){
    int delayTimer = 3;

    if (currentOpponentCooldownStep == 0) {
        delayTimer = 0;
    }

    currentOpponentCooldownStep = std::max(currentOpponentCooldownStep, step) + delayTimer;    
    opponentUnitRespawnRecord[unitId] = currentOpponentCooldownStep;
    opponentUnitRespawnIndex[currentOpponentCooldownStep] = unitId;

    log("Opponent unit " + std::to_string(unitId) + " will respawn at " + std::to_string(currentOpponentCooldownStep));
    return currentOpponentCooldownStep;
}

int RespawnRegistry::getPlayerUnitSpawnStep(int unitId) {
    return playerUnitRespawnRecord[unitId];
}

int RespawnRegistry::getOpponentUnitSpawnStep(int unitId) {
    return opponentUnitRespawnRecord[unitId];
}

int RespawnRegistry::getPlayerUnitThatCanSpawnAtStep(int step){
    if (playerUnitRespawnIndex.find(step) != playerUnitRespawnIndex.end()) {
        return playerUnitRespawnIndex[step];
    }
    return -1;
}

int RespawnRegistry::getOpponentUnitThatCanSpawnAtStep(int step){
    if (opponentUnitRespawnIndex.find(step) != opponentUnitRespawnIndex.end()) {
        return opponentUnitRespawnIndex[step];
    }
    return -1;
}

void RespawnRegistry::reset() {
    playerUnitRespawnRecord.clear();
    opponentUnitRespawnRecord.clear();
    playerUnitRespawnIndex.clear();
    opponentUnitRespawnIndex.clear();

    currentPlayerCooldownStep = 0;
    currentOpponentCooldownStep = 0;
}

std::string mapToString(const std::unordered_map<int, int>& map) {
    std::ostringstream oss;
    for (const auto& pair : map) {
        oss << "{" << pair.first << ": " << pair.second << "} ";
    }
    return oss.str();
}

void RespawnRegistry::printUpcomingRespawns(int currentStep) {
    log("player spawns -> " + mapToString(playerUnitRespawnRecord));
    log("opponent spawns -> " + mapToString(opponentUnitRespawnRecord));
}
