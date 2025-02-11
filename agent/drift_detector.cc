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

    GameTile& targetTile = gameMap.getTile(x, y);

    if (!targetTile.isVisible()) {
        return -1; // Not possible to verify
    }

    if (sourceTile.getPreviousType() == targetTile.getType()) {
        return 1; //Found the same type
    } else {
        return 0; //Did not find the same type
    }
}

void DriftDetector::reportNebulaDrift(GameTile& gameTile) {    
    int lastSeenStart = gameTile.getPreviousTypeUpdateStep();
    int lastSeenEnd = gameTile.getTypeUpdateStep();

    log("Drift reported by " + gameTile.toString() + " last seen " + std::to_string(gameTile.getPreviousType()) + " at " + std::to_string(lastSeenStart) + " but now " + std::to_string(gameTile.getType()));

    std::map<int, int> speedCounter;
    for (int i = lastSeenStart + 1; i <= lastSeenEnd; i++) {
        // log("Checking for step " + std::to_string(i));
        auto it = stepToDriftSpeedMap.find(i);
        if (it != stepToDriftSpeedMap.end()) {
            for (int speed : it->second) {
                if (driftSpeedToStatusMap[speed] != NebulaDriftStatus::NO_DRIFT) {
                    speedCounter[speed] += 1;
                }
            }
        }
    }

    for ( const auto &speed : POSSIBLE_NEBULA_DRIFT_SPEEDS) {
        if (speedCounter.find(speed) == speedCounter.end()) {
            //This speed isn't possible in the provided drift times.
            driftSpeedToStatusMap[speed] = NebulaDriftStatus::NO_DRIFT;
        }
    }

    for (const auto &pair : speedCounter) {
        int moveCount = pair.second % gameMap.width;

        int multiplier = 1;
    
        if (pair.first < 0) {
            multiplier = -1;
        }
    
        int x = gameTile.x + moveCount * multiplier;
        int y = gameTile.y - moveCount * multiplier;

        int driftStatus =  compareDrift(gameTile, x, y);

        if (driftStatus == 0) {
            driftSpeedToStatusMap[pair.first] = NebulaDriftStatus::NO_DRIFT;
        }

        if (driftSpeedToStatusMap[pair.first] != NebulaDriftStatus::NO_DRIFT) {
            //NO_DRIFT is a conclusive evidence, do not override that!
            if (driftStatus == 1) {
                log("Possible drift found - " + std::to_string(pair.first) + ", that has moved steps - " + std::to_string(pair.second));
                driftSpeedToStatusMap[pair.first] = NebulaDriftStatus::FOUND_DRIFT;
            } else if (driftStatus == -1) {
                driftSpeedToStatusMap[pair.first] = NebulaDriftStatus::UNCONCLUSIVE_DRIFT;
            }
        }
    }

    int outstandingPossibilities = driftSpeedToStatusMap.size();        
    for (const auto &speedStatus : driftSpeedToStatusMap) {
        if (speedStatus.second == NebulaDriftStatus::NO_DRIFT) {
            outstandingPossibilities--;
        } else if(speedStatus.second == NebulaDriftStatus::FOUND_DRIFT) {
            finalSpeed = speedStatus.first;
        }
    }

    log("Drift outstanding possibilities " + std::to_string(outstandingPossibilities) + ",size of map - " + std::to_string(driftSpeedToStatusMap.size()));

    if (outstandingPossibilities == 1) {
        log("Drift finalized - " + std::to_string(finalSpeed));
        driftFinalized = true;
    }

    if (outstandingPossibilities == 0) {
        log("Problem: Drift Detector bugged!");
        std::cerr<<"Drift Detector bugged!"<<std::endl;
        driftFinalized = false;
    }
}

DriftDetector::DriftDetector(GameMap &gameMap) : gameMap(gameMap) {
    driftFinalized = false;
    GameEnvConfig& config = GameEnvConfig::getInstance();

    for (int speed: POSSIBLE_NEBULA_DRIFT_SPEEDS) {
        driftSpeedToStatusMap[speed] = NebulaDriftStatus::UNKNOWN_DRIFT;
    }

    for (int i = 0; i <= config.matchCountPerEpisode * (config.maxStepsInMatch + 1); i++ ) {        
        for (int speed: POSSIBLE_NEBULA_DRIFT_SPEEDS) {            
            if (std::fmod((i - 1) * std::fabs(speed/1000.0f), 1.0f) > std::fmod(i * std::fabs(speed/1000.0f), 1.0f)) {
                stepToDriftSpeedMap[i+1].push_back(speed); //We track everything from the start of the match, so +1.  It will be 8, 11, 15, 21, 28, 3, etc
            }
        }
    }
}
