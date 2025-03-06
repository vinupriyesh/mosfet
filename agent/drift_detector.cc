#include "drift_detector.h"
#include <cmath>
#include "game_env_config.h"

const int POSSIBLE_NEBULA_DRIFT_SPEEDS[] = {-150, -100, -50, -25, 150, 100, 50, 25};  //Representing as x1000 to avoid floating point precision inside map

void DriftDetector::log(std::string message) {
    Logger::getInstance().log("DriftDetector -> " + message);
}

void DriftDetector::exploreTile(GameTile &gameTile) {
    exploreTile(gameTile.x, gameTile.y, gameTile.getType(), currentDriftTileTypeVectorIndex);
}

void DriftDetector::exploreTile(int x, int y, TileType tileType, int driftTileTypeIndex) {
    // log("Entering exploreTile " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(tileType) + ", " + std::to_string(driftTileTypeIndex));
    if (tileType == TileType::UNKNOWN_TILE) {
        //No point in exploring unknown tiles, However this is not possible!
        return;
    }
    if (driftTileTypeIndex >= allDriftTileTypeVectors.size()) {
        //Nothing to explore, return
        return;
    }
    if ((*allDriftTileTypeVectors[driftTileTypeIndex])[y][x] == tileType) {
        //This tile type is already same, no action
        return;
    }

    (*allDriftTileTypeVectors[driftTileTypeIndex])[y][x] = tileType;

    int nextX = x;
    int nextY = y;

    if (finalSpeed > 0) {
        nextX += 1;
        nextY -= 1;
    } else {
        nextX -= 1;
        nextY += 1;
    }

    auto& targetTile = gameMap.getRolledOverTile(nextX, nextY);
    exploreTile(targetTile.x, targetTile.y, tileType, driftTileTypeIndex+1);
}

TileType DriftDetector::getEstimatedDriftTile(int x, int y, std::vector<std::vector<TileType>> *previousStepValues) {    
    if (finalSpeed < 0) { //This x, y is the source tile, so this is inverted
        x += 1;
        y -= 1;
    } else {
        x -= 1;
        y += 1;
    }

    auto& sourceTile = gameMap.getRolledOverTile(x, y);
    return (*previousStepValues)[sourceTile.y][sourceTile.x];
}

std::vector<std::vector<TileType>>* DriftDetector::estimateDrift(std::vector<std::vector<TileType>> *previousStepValues) {
    GameEnvConfig& config = GameEnvConfig::getInstance();
    std::vector<std::vector<TileType>>* tileTypesArray = new std::vector<std::vector<TileType>>;
    tileTypesArray->resize(config.mapHeight);
    for (int y = 0; y < config.mapHeight; ++y) {
        tileTypesArray->operator[](y).reserve(config.mapWidth);
        for (int x = 0; x < config.mapWidth; ++x) {
            tileTypesArray->operator[](y).emplace_back(getEstimatedDriftTile(x, y, previousStepValues));
        }
    }
    return tileTypesArray;
}

int DriftDetector::computeMoveCountBetween(int from, int to) {
    if (from == to) {
        return 0;
    }

    int start = from;
    int end = to;
    int multiplier = 1;

    if (to < from) {
        start = to;
        end = from;
        multiplier = -1;
    }

    int moveCount = 0;
    for (int i = start + 1; i <= end; i++) {
        auto& speedVector = stepToDriftSpeedMap[i];
        if (std::find(speedVector.begin(), speedVector.end(), finalSpeed) != speedVector.end()) {
            moveCount++;
        }
    }

    moveCount %= gameMap.width;
    log("Retuning moveCount " + std::to_string(moveCount * multiplier) + " between " + std::to_string(from) + " and " + std::to_string(to));
    return moveCount * multiplier;
}

void DriftDetector::forecastTileTypeAt(int step, GameTile& tile, std::vector<std::vector<TileType>>& tileTypesArray) {

    while(!tile.getPreviousTypes().empty()) {
        // log("The item count for the tile " + tile.toString() + " is " + std::to_string(tile.getPreviousTypes().size()));
        TileType tileType = tile.getPreviousTypes().top();
        int typeUpdateStep = tile.getPreviousTypeUpdateSteps().top();    
        
        tile.getPreviousTypes().pop();
        tile.getPreviousTypeUpdateSteps().pop();

        if (tileType == TileType::UNKNOWN_TILE) {
            //No point in updating to UNKNOWN, continue to the next iteration. But this can never happen!
            continue;
        }

        int moveCount = computeMoveCountBetween(typeUpdateStep, step);

        int multiplier = 1;
        
        if (finalSpeed < 0) {
            multiplier = -1;
        }

        int x = tile.x + moveCount * multiplier;
        int y = tile.y - moveCount * multiplier;

        auto& targetTile = gameMap.getRolledOverTile(x, y);
        TileType targetTileType = tileTypesArray[targetTile.y][targetTile.x];    

        if (targetTileType == TileType::UNKNOWN_TILE) {
            log("setting tile " + targetTile.toString() + " to type " + std::to_string(tileType) + " from " + tile.toString() + " moveCount=" + std::to_string(moveCount) + ", last seen =" + std::to_string(typeUpdateStep));
            tileTypesArray[targetTile.y][targetTile.x] = tileType;
        } else if (targetTileType != tileType) {
            log("Problem: Wrongly identified tile type " + std::to_string(targetTileType) + " != " + std::to_string(tileType) + " for tile=" + tile.toString());
            log("Speed = " + std::to_string(finalSpeed) + ", moveCount = " + std::to_string(moveCount) + ", targetTile=" + targetTile.toString());
            std::cerr<<"Problem: Wrongly identified tile type"<<std::endl;
        }
    }
}

std::vector<std::vector<TileType>>* DriftDetector::prepareCurrentTileTypes() {
    GameEnvConfig& config = GameEnvConfig::getInstance();
    std::vector<std::vector<TileType>>* tileTypesArray = new std::vector<std::vector<TileType>>;
    tileTypesArray->resize(config.mapHeight);
    for (int y = 0; y < config.mapHeight; ++y) {
        tileTypesArray->operator[](y).reserve(config.mapWidth);
        for (int x = 0; x < config.mapWidth; ++x) {
            tileTypesArray->operator[](y).emplace_back(TileType::UNKNOWN_TILE);            
        }
    }

    for (int y = 0; y < config.mapHeight; ++y) {
        for (int x = 0; x < config.mapWidth; ++x) {
            // Assign whatever this tile has seen in the past to the current step.  Current step will be extrapolated to future later
            forecastTileTypeAt(gameMap.derivedGameState.currentStep - 1, gameMap.getTile(x, y), *tileTypesArray);
        }
    }
    
    return tileTypesArray;
}


void DriftDetector::estimateTileTypesforFinalizedDrift() {

    if (finalSpeed == 0) {
        log("Problem: Speed not finalized to estimate drift");
        std::cerr<<"Problem: Speed not finalized to estimate drift"<<std::endl;
        return;
    }    

    GameEnvConfig& config = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap.derivedGameState;

    auto& driftAwareTileType = gameMap.getDriftAwareTileType();

    if (driftAwareTileType.size() == config.matchCountPerEpisode * (config.maxStepsInMatch + 1)) {
        log("Already prepared the drift aware tile arrays");
        return;
    }

    int lastKnownKey = driftAwareTileType.size() - 1;

    if (lastKnownKey != state.currentStep - 1) {
        // We are checking if the last known key is of the last step!
        log("Problem: Previous step's tileTypes are unknown, lastKnownKey=" + std::to_string(lastKnownKey));
        std::cerr<<"Problem: Previous step's tileTypes are unknown"<<std::endl;
        return;
    }

    std::vector<std::vector<TileType>>* currentTileTypes = prepareCurrentTileTypes();

    //Backfill the current step with the known values
    for (int i = 0; i < state.currentStep;i++) {
        //Note: We are not estimating the previous drifts.  Expecting the drift detector to detect the drift in first attempt, else this is flawed!
        driftAwareTileType[i] = currentTileTypes;
    }
    
    //Forecast the future
    for (int i = state.currentStep; i <  config.matchCountPerEpisode * (config.maxStepsInMatch + 1); i++) {
        auto& possibleSpeedsThisStep = stepToDriftSpeedMap[i];

        if (std::find(possibleSpeedsThisStep.begin(), possibleSpeedsThisStep.end(), finalSpeed) != possibleSpeedsThisStep.end()) {
            currentTileTypes = estimateDrift(currentTileTypes);
            allDriftTileTypeVectors.emplace_back(currentTileTypes);
        }

        if (allDriftTileTypeVectors.size() == 0) {
            //If the above condition is not satisfied, allDriftTileTypeVectors should not be empty
            allDriftTileTypeVectors.emplace_back(currentTileTypes);
        }

        driftAwareTileType.emplace_back(currentTileTypes);

        std::ostringstream oss;
        oss << currentTileTypes;
        log("Pointer added " + oss.str() + " allDriftTileTypeVectors=" + std::to_string(allDriftTileTypeVectors.size() - 1) + ", driftAwareTileType=" + std::to_string(driftAwareTileType.size() -1));
    }
}

int DriftDetector::compareDrift(GameTile& sourceTile, int x, int y) {

    GameTile& targetTile = gameMap.getRolledOverTile(x, y);

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

    std::map<int, int> speedCounter; // (speed, moveCount)
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
        } else {
            finalSpeed = speedStatus.first;
        }
    }

    log("Drift outstanding possibilities " + std::to_string(outstandingPossibilities) + ",size of map - " + std::to_string(driftSpeedToStatusMap.size()));

    if (outstandingPossibilities == 1) {
        log("Drift finalized = " + std::to_string(finalSpeed));
        driftFinalized = true;
        estimateTileTypesforFinalizedDrift();
    }

    if (outstandingPossibilities == 0) {
        log("Problem: Drift Detector bugged!");
        std::cerr<<"Drift Detector bugged!"<<std::endl;
        driftFinalized = false;
    }
}

void DriftDetector::step() {
    auto& driftAwareTileType = gameMap.getDriftAwareTileType();
    GameEnvConfig& config = GameEnvConfig::getInstance();
    auto& state = gameMap.derivedGameState;

    if (!driftFinalized) {
        // Just set null value value to this step as the drift is not finalized!        
        // int lastKnownKey = driftAwareTileType.size() - 1;
        // std::vector<std::vector<TileType>>* currentTileTypes = driftAwareTileType[lastKnownKey];
        driftAwareTileType.push_back(nullptr);
    }

    if (driftAwareTileType.size() <= state.currentStep) {
        log("Problem: There is no tileType available for current step, size of vector = " + std::to_string(driftAwareTileType.size()));
        std::cerr<<"Problem: There is no tileType available for current step"<<std::endl;
    }

    if (driftAwareTileType[state.currentStep] != nullptr && allDriftTileTypeVectors.size() > 0 &&
             driftAwareTileType[state.currentStep] != allDriftTileTypeVectors[currentDriftTileTypeVectorIndex]) {
        if (allDriftTileTypeVectors.size() <= currentDriftTileTypeVectorIndex + 1) {
            log("Problem: Unable to switch next drift tile vector = " + std::to_string(allDriftTileTypeVectors.size()) + ", " + std::to_string(currentDriftTileTypeVectorIndex));
            std::cerr<<"Problem: Unable to switch next drift tile vector"<<std::endl;
        }        

        currentDriftTileTypeVectorIndex++;

        if (driftAwareTileType[state.currentStep] != allDriftTileTypeVectors[currentDriftTileTypeVectorIndex]) {
            log("Problem: Next drift tile doesn't match = " + std::to_string(allDriftTileTypeVectors.size()) + ", " + std::to_string(currentDriftTileTypeVectorIndex));

            std::ostringstream oss;
            oss << driftAwareTileType[state.currentStep];

            std::ostringstream oss1;
            oss1 << allDriftTileTypeVectors[currentDriftTileTypeVectorIndex];

            log(oss.str() + " != " + oss1.str());

            std::cerr<<"Problem: Next drift tile doesn't match"<<std::endl;
        }
    }
}

TruthValue DriftDetector::isDriftPossible(int stepId) {
    if (driftFinalized) {
        if (stepToDriftSpeedMap.find(stepId) == stepToDriftSpeedMap.end()) {
            return TruthValue::FALSE;
        }

        auto& possibleSpeedsThisStep = stepToDriftSpeedMap[stepId];
        if (std::find(possibleSpeedsThisStep.begin(), possibleSpeedsThisStep.end(), finalSpeed) != possibleSpeedsThisStep.end()) {
            return TruthValue::TRUE;
        } else {
            return TruthValue::FALSE;
        }
    }
    return TruthValue::UNKNOWN_TRUTH;
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

    currentDriftTileTypeVectorIndex = 0;
}

DriftDetector::~DriftDetector() {
    auto& driftAwareTileType = gameMap.getDriftAwareTileType();
    
    for (auto value : allDriftTileTypeVectors) {
        delete value;
    }

    allDriftTileTypeVectors.clear();
    driftAwareTileType.clear();    
}
