#include "drift_detector.h"
#include <cmath>

const int POSSIBLE_NEBULA_DRIFT_SPEEDS[] = {-150, -100, -50, -25, 150, 100, 50, 25};  //Representing as x1000 to avoid floating point precision inside map

void DriftDetector::log(std::string message) {
    Logger::getInstance().log("DriftDetector -> " + message);
}

int DriftDetector::compareDrift(GameTile& sourceTile, int x, int y) {
    if (x >= gameMap.width) {
        x -= gameMap.width;
    }

    if (x < 0) {
        x += gameMap.width;
    }

    if (y >= gameMap.height) {
        y -= gameMap.height;
    }

    if (y < 0) {
        y += gameMap.height;
    }

    GameTile& targetTile = gameMap.getTile(x, x);

    if (!targetTile.isVisible()) {
        return -1; // Not possible to verify
    }

    if (sourceTile.getPreviousType() == targetTile.getType()) {
        return 1; //Found the same type
    } else {
        return 0; //Did not find the same type
    }
}

NebulaDriftStatus DriftDetector::identifyDriftType(GameTile& gameTile, int moveCount) {
    moveCount = moveCount % gameMap.width;
    int xPlus = gameTile.x + moveCount;
    int yPlus = gameTile.y - moveCount;
    
    int positiveSpeedStatus = compareDrift(gameTile, xPlus, yPlus);

    int xMinus = gameTile.x - moveCount;
    int yMinus = gameTile.y + moveCount;
    
    int negativeSpeedStatus = compareDrift(gameTile, xMinus, yMinus);
    
    if (positiveSpeedStatus == 1 && negativeSpeedStatus == 0) {        
        return NebulaDriftStatus::POSITIVE_DRIFT;
    }

    if (negativeSpeedStatus == 1 && positiveSpeedStatus == 0) {
        return NebulaDriftStatus::NEGATIVE_DRIFT;
    }

    log("Unconclusive from " + gameTile.toString() + " negative - " + std::to_string(negativeSpeedStatus) + ", positive = " + std::to_string(positiveSpeedStatus));
    return NebulaDriftStatus::UNCONCLUSIVE_DRIFT;
}

void DriftDetector::reportNebulaDrift(GameTile& gameTile) {
    log("Drift reported by " + gameTile.toString());

    int lastSeenStart = gameTile.getPreviousTypeUpdateStep();
    int lastSeenEnd = gameTile.getTypeUpdateStep();

    std::map<int, int> speedCounter;
    for (int i = lastSeenStart; i <= lastSeenEnd; i++) {
        log("Checking for step " + std::to_string(i));
        auto it = stepToDriftSpeedMap.find(i);
        if (it != stepToDriftSpeedMap.end()) {
            for (int speed : it->second) {
                speedCounter[speed] += 1;
            }
        }
    }

    for (const auto &pair : speedCounter) {
        log("Checking possibility for speed - " + std::to_string(pair.first) + ", steps - " + std::to_string(pair.second));
        auto status = identifyDriftType(gameTile, pair.second);
        if (status == NebulaDriftStatus::POSITIVE_DRIFT || NebulaDriftStatus::NEGATIVE_DRIFT) {
            log("Drift found at speed " + std::to_string(pair.first) + " direction=" + std::to_string(status));
            driftSpeedToStatusMap[pair.first] = status;
        }        
    }    
}

NebulaDriftStatus DriftDetector::getCurrentNebulaDriftStatus() {
    return NebulaDriftStatus::UNKNOWN_DRIFT;
}

DriftDetector::DriftDetector(GameMap &gameMap) : gameMap(gameMap) {
    GameEnvConfig& config = GameEnvConfig::getInstance();

    for (int speed: POSSIBLE_NEBULA_DRIFT_SPEEDS) {
        driftSpeedToStatusMap[speed] = NebulaDriftStatus::UNKNOWN_DRIFT;
    }

    for (int i = 1; i <= config.matchCountPerEpisode * (config.maxStepsInMatch + 1); i++ ) {        
        for (int speed: POSSIBLE_NEBULA_DRIFT_SPEEDS) {            
            if (std::fmod((i - 1) * std::fabs(speed/1000.0f), 1.0f) > std::fmod(i * std::fabs(speed/1000.0f), 1.0f)) {
                stepToDriftSpeedMap[i+1].push_back(speed); //We track everything from the start of the match, so +1.  It will be 8, 11, 15, 21, 28, 3, etc
            }
        }
    }

    // for (const auto &pair : stepToDriftSpeedMap) {
    //     log("Speeds for step: " + std::to_string(pair.first));
    //     for (const auto& vpair: pair.second) {
    //         log("Speed -- " + std::to_string(vpair));
    //     }
    // }    
}
