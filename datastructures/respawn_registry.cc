#include "respawn_registry.h"
#include "logger.h"

#include <iostream>
#include <sstream>

std::string mapToString(const std::unordered_map<int, int>& map) {
    std::ostringstream oss;
    for (const auto& pair : map) {
        oss << "{" << pair.first << ": " << pair.second << "} ";
    }
    return oss.str();
}

void RespawnRegistry::log(const std::string& message) {
    Logger::getInstance().log("RespawnRegistry -> " + message);
}

void RespawnRegistry::logCurrentState() {
    log("=== RESPAWN REGISTRY STATE ===");
    log("Player respawn record: " + mapToString(playerRespawnRecord));
    log("Player respawn step index: " + mapToString(playerRespawnStepIndex));
    log("Player death record: " + mapToString(playerDeathRecord));
    log("Player queue size: " + std::to_string(playerRespawnQueueSize));
    
    log("Opponent respawn record: " + mapToString(opponentRespawnRecord));
    log("Opponent respawn step index: " + mapToString(opponentRespawnStepIndex));
    log("Opponent death record: " + mapToString(opponentDeathRecord));
    log("Opponent queue size: " + std::to_string(opponentRespawnQueueSize));
    
    if (playerUnitRespawned != -1) {
        log("Last player unit respawned: " + std::to_string(playerUnitRespawned));
    }
    if (opponentUnitRespawned != -1) {
        log("Last opponent unit respawned: " + std::to_string(opponentUnitRespawned));
    }
    log("=============================");
}

int RespawnRegistry::getNextSpawnStep(int currentStep, int queueSize) {
    int offset = ((currentStep - 1) % 3 + 3) % 3;
    return currentStep + (3 - offset) + (queueSize * 3);
}

int RespawnRegistry::slotTheCurrentUnit(int currentStep, int& queueSize, std::unordered_map<int, int>& respawnRecord, std::unordered_map<int, int>& respawnStepIndex,  std::unordered_map<int, int>& deathRecord, int unitId) {
    int currentPosition = -1;    
    for (int i = 0; i < queueSize; i++) {
        int currentSpawnStep = getNextSpawnStep(currentStep, i);
        if (respawnStepIndex.find(currentSpawnStep) != respawnStepIndex.end()) {
            int iterUnitId = respawnStepIndex[currentSpawnStep];

            if (iterUnitId > unitId) {
                break;
            }

            currentPosition = i;
        } else {
            log("Problem: spawn step " + std::to_string(currentSpawnStep) + " is not found in the respawn queue for index " + std::to_string(i));
            std::cerr<<"Problem: spawn step is not found in the respawn queue"<<std::endl;
        }
    }

    for (int i = queueSize - 1; i > currentPosition; i--) {
        int currentSpawnStep = getNextSpawnStep(currentStep, i);
        int olderUnitId = respawnStepIndex[currentSpawnStep];
        int nextSpawnStep = getNextSpawnStep(currentStep, i + 1);
        respawnRecord[olderUnitId] = nextSpawnStep;
        respawnStepIndex[nextSpawnStep] = olderUnitId;
    }

    int nextSpawnStep =  getNextSpawnStep(currentStep, currentPosition + 1);
    respawnRecord[unitId] = nextSpawnStep;
    respawnStepIndex[nextSpawnStep] = unitId;
    deathRecord[unitId] = currentStep;
    queueSize++;
    return nextSpawnStep;
}

int RespawnRegistry::pushPlayerUnit(int unitId, int step){

    if (playerRespawnRecord.find(unitId) != playerRespawnRecord.end()) {
        log("Player unit " + std::to_string(unitId) + " is already in the respawn queue");
        return playerRespawnRecord[unitId];
    }

    // int spawnStep = getNextSpawnStep(step, playerRespawnQueueSize);
    int spawnStep = slotTheCurrentUnit(step, playerRespawnQueueSize, playerRespawnRecord, playerRespawnStepIndex, playerDeathRecord, unitId);

    // playerDeathRecord[unitId] = step;
    // playerRespawnRecord[unitId] = spawnStep;
    // playerRespawnStepIndex[spawnStep] = unitId;
    // playerRespawnQueueSize++;

    log("Player unit " + std::to_string(unitId) + " will respawn at " + std::to_string(spawnStep) + " as the queue size is " + std::to_string(playerRespawnQueueSize));
    return spawnStep;
}

int RespawnRegistry::pushOpponentUnit(int unitId, int step){

    if (opponentRespawnRecord.find(unitId) != opponentRespawnRecord.end()) {
        log("Opponent unit " + std::to_string(unitId) + " is already in the respawn queue");
        return opponentRespawnRecord[unitId];
    }
    
    // int spawnStep = getNextSpawnStep(step, opponentRespawnQueueSize);
    int spawnStep = slotTheCurrentUnit(step, opponentRespawnQueueSize, opponentRespawnRecord, opponentRespawnStepIndex, opponentDeathRecord, unitId);

    // opponentDeathRecord[unitId] = step;
    // opponentRespawnRecord[unitId] = spawnStep;
    // opponentRespawnStepIndex[spawnStep] = unitId;
    // opponentRespawnQueueSize++;

    log("Opponent unit " + std::to_string(unitId) + " will respawn at " + std::to_string(spawnStep) + " as the queue size is " + std::to_string(opponentRespawnQueueSize));
    return spawnStep;
}

int RespawnRegistry::getPlayerUnitSpawnStep(int unitId) {
    return playerRespawnRecord[unitId];
}

int RespawnRegistry::getOpponentUnitSpawnStep(int unitId) {
    return opponentRespawnRecord[unitId];
}

int RespawnRegistry::getPlayerUnitThatCanSpawnAtStep(int step){
    if (playerRespawnStepIndex.find(step) != playerRespawnStepIndex.end()) {
        return playerRespawnStepIndex[step];
    }
    return -1;
}

int RespawnRegistry::getOpponentUnitThatCanSpawnAtStep(int step){
    if (opponentRespawnStepIndex.find(step) != opponentRespawnStepIndex.end()) {
        return opponentRespawnStepIndex[step];
    }
    return -1;
}

void RespawnRegistry::reset() {
    playerRespawnRecord.clear();
    opponentRespawnRecord.clear();
    playerRespawnStepIndex.clear();
    opponentRespawnStepIndex.clear();

    playerRespawnQueueSize = 0;
    opponentRespawnQueueSize = 0;
}

void RespawnRegistry::step(int currentStep) {
    logCurrentState();

    playerUnitRespawned = -1;
    if (playerRespawnQueueSize > 0 && playerRespawnStepIndex.find(currentStep) != playerRespawnStepIndex.end()) {
        playerRespawnQueueSize--;
        int unitId = playerRespawnStepIndex[currentStep];
        playerRespawnRecord.erase(unitId);
        playerDeathRecord.erase(unitId);
        playerRespawnStepIndex.erase(currentStep);
        log("Player unit " + std::to_string(unitId) + " has respawned");
        playerUnitRespawned = unitId;
    }

    opponentUnitRespawned = -1;
    if (opponentRespawnQueueSize > 0 && opponentRespawnStepIndex.find(currentStep) != opponentRespawnStepIndex.end()) {
        opponentRespawnQueueSize--;
        int unitId = opponentRespawnStepIndex[currentStep];
        opponentRespawnRecord.erase(unitId);
        opponentDeathRecord.erase(unitId);
        opponentRespawnStepIndex.erase(currentStep);
        log("Opponent unit " + std::to_string(unitId) + " has respawned");
        opponentUnitRespawned = unitId;
    }
}

void RespawnRegistry::printUpcomingRespawns(int currentStep) {
    log("player spawns -> " + mapToString(playerRespawnRecord));
    log("opponent spawns -> " + mapToString(opponentRespawnRecord));
}

bool RespawnRegistry::isOpponentShuttleAlive(int shuttleId, int stepId) {
    if (opponentRespawnRecord.find(shuttleId) != opponentRespawnRecord.end() && opponentDeathRecord.find(shuttleId) != opponentDeathRecord.end()) {
        return opponentRespawnRecord[shuttleId] <= stepId && opponentDeathRecord[shuttleId] < opponentRespawnRecord[shuttleId];
    }
    return true;
}
