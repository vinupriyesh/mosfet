#include "energy_estimator.h"
#include "logger.h"
#include "symmetry_util.h"

#include <cmath>

const int MIN_ENERGY = -20;
const int MAX_ENERGY = 20;
const int POSSIBLE_ENERGY_DRIFT_SPEEDS[] = {1, 2, 3, 4, 5};  //Representing as x100 to avoid floating point precision inside map

void EnergyEstimator::log(std::string message) {
    Logger::getInstance().log("EnergyEstimator -> " + message);
}

EnergyEstimator::EnergyEstimator(GameMap &gameMap)  : gameMap(gameMap) {
    currentEnergyNode = -1;
    energyValuesBuffer = nullptr;
    for (int speed: POSSIBLE_ENERGY_DRIFT_SPEEDS) {
        driftSpeedToStatusMap[speed] = EnergyDriftStatus::UNKNOWN_ENERGY_DRIFT;
    }
}

EnergyEstimator::~EnergyEstimator() {
    if (energyValuesBuffer != nullptr) {
        delete energyValuesBuffer;
    }
}

// Function to compute the Euclidean distance between two points (a_x, a_y) and (b_x, b_y)
double euclideanDistance(int aX, int aY, int bX, int bY) {
    int deltaX = aX - bX;
    int deltaY = aY - bY;
    return std::sqrt(deltaX * deltaX + deltaY * deltaY);
}

void EnergyEstimator::getPossibleDrifts(int step, std::unordered_set<int>& possibleDrifts) {
    step -= 2; // Offset as energy nodes are reflected only after a step
    for (int speed: POSSIBLE_ENERGY_DRIFT_SPEEDS) {
        if (std::fmod((step - 1) * std::fabs(speed/100.0f), 1.0f) > std::fmod(step * std::fabs(speed/100.0f), 1.0f)) {
            possibleDrifts.insert(speed);
            // log("Drift at " + std::to_string(speed) + " possible for step " + std::to_string(step));
        }        
    }
}

bool EnergyEstimator::estimate(int energyNodeTileId) {
    // log("Estimating for energy node " + std::to_string(energyNodeTileId));
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    int sizeOfEnergyNodeFns = sizeof(energyNodeFns) / sizeof(energyNodeFns[0]);

    std::vector<std::vector<std::vector<double>>>  estimatedEnergy(
                                                    gameEnvConfig.mapHeight, 
                                                    std::vector<std::vector<double>>(
                                                        gameEnvConfig.mapWidth, 
                                                        std::vector<double>(sizeOfEnergyNodeFns, 0.0)
                                                ));

    double meanValue = 0;    

    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            for (int k = 0; k < sizeOfEnergyNodeFns; ++k) {

                if (k == 0) {                    
                    int x, y;
                    symmetry_utils::toXY(energyNodeTileId, x, y);
                    double distance = euclideanDistance(i, j, x, y);
                    estimatedEnergy[i][j][k] =  sin(distance * energyNodeFns[0][1] + energyNodeFns[0][2]) * energyNodeFns[0][3];
                } else if (k == 3) {
                    int x, y, orgX, orgY;
                    symmetry_utils::toXY(energyNodeTileId, orgX, orgY);
                    symmetry_utils::toMirroredXY(orgX, orgY, x, y);
                    double distance = euclideanDistance(i, j, x, y);
                    estimatedEnergy[i][j][k] += sin(distance * energyNodeFns[0][1] + energyNodeFns[0][2]) * energyNodeFns[0][3];
                } else {
                    estimatedEnergy[i][j][k] = 0;
                }
            
                meanValue += estimatedEnergy[i][j][k];
            }
        }
    }    

    meanValue /= gameEnvConfig.mapHeight * gameEnvConfig.mapWidth * sizeOfEnergyNodeFns;
    // log("Mean value of estimated energy: " + std::to_string(meanValue));    

    std::vector<std::vector<double>>* energyValuesLocalBuffer = new std::vector<std::vector<double>>(
        gameMap.height, std::vector<double>(gameMap.width, 0.0)
    );

    bool allGood = true;
    double newMeanValue = 0;
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& tile = gameMap.getTile(i, j);
            double estimatedEnergyValue = 0;
            for (int k = 0; k < sizeOfEnergyNodeFns; ++k) {                
                if (meanValue < 0.25) {
                    estimatedEnergy[i][j][k] += 0.25 - meanValue;
                }
                estimatedEnergyValue += estimatedEnergy[i][j][k];
                newMeanValue += estimatedEnergy[i][j][k];
            }

            int estimatedEnergyValueRounded = std::round(estimatedEnergyValue);
            estimatedEnergyValueRounded =  std::clamp(estimatedEnergyValueRounded, MIN_ENERGY, MAX_ENERGY);
            (*energyValuesLocalBuffer)[i][j] = estimatedEnergyValueRounded;
            // log( "(" + std::to_string(i) + "," + std::to_string(j) + ") Estimated = " + std::to_string(estimatedEnergyValueRounded) + " vs " + std::to_string(tile.getEnergy()));
            if (tile.isVisible() && estimatedEnergyValueRounded != tile.getEnergy()) {
                allGood = false;
            }            
        }
    }
    
    newMeanValue /= gameEnvConfig.mapHeight * gameEnvConfig.mapWidth * sizeOfEnergyNodeFns;
    // log("New Mean value of estimated energy: " + std::to_string(newMeanValue));

    // log("All Good? " + std::to_string(energyNodeTileId) + " = " + std::to_string(allGood));
    if (allGood) {
        log("Energy node matched at " + std::to_string(energyNodeTileId));
        energyValuesBuffer = energyValuesLocalBuffer;
    } else {
        delete energyValuesLocalBuffer;        
    }
    return allGood;
}

void EnergyEstimator::updateEstimatedEnergies() {
    if (energyValuesBuffer == nullptr) {
        log("Problem: Energy values buffer is null.  Not updating the estimated energies");
        std::cerr<<"Problem: Energy values buffer is null.  Not updating the estimated energies"<<std::endl;
        return;
    }

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& tile = gameMap.getTile(i, j);
            tile.setEstimatedEnergy((*energyValuesBuffer)[i][j]);
        }
    }
}

void EnergyEstimator::clearEstimatedEnergies() {
    log("WARN: Clearing estimated energies.  Ideally this should not happen as it is easy to identify the energy drift");
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& tile = gameMap.getTile(i, j);
            tile.setEstimatedEnergy(-21);
        }
    }
}

void EnergyEstimator::reportEnergyDrift(GameTile &tile) {    
    log("reportEnergyDrift for tile " + tile.toString());

    if (gameMap.derivedGameState.currentStep <= 20) {
        return;
    }

    int fromStep = tile.getPreviousEnergyUpdateTime() + 1;
    int toStep = tile.getLastEnergyUpdateTime();
    
    std::unordered_set<int> possibleDrifts;

    for (int i = fromStep; i <= toStep; ++i) {
        if (i <= 2){
            // Not considering reports less than 2 steps as the 2nd step always has a drift
            return;
        }
        getPossibleDrifts(i, possibleDrifts);
    }

    if (possibleDrifts.find(1) != possibleDrifts.end() && driftSpeedToStatusMap[1] != EnergyDriftStatus::NO_ENERGY_DRIFT) {
        log("Drift at 1 possible, but it is not already set to NO_ENERGY_DRIFT");
        driftSpeedToStatusMap[2] = EnergyDriftStatus::NO_ENERGY_DRIFT;
        driftSpeedToStatusMap[3] = EnergyDriftStatus::NO_ENERGY_DRIFT;
        driftSpeedToStatusMap[4] = EnergyDriftStatus::NO_ENERGY_DRIFT;
        driftSpeedToStatusMap[5] = EnergyDriftStatus::NO_ENERGY_DRIFT;
    }

    if (possibleDrifts.find(2) != possibleDrifts.end() && driftSpeedToStatusMap[2] != EnergyDriftStatus::NO_ENERGY_DRIFT) {
        log("Drift at 2 possible, but it is not already set to NO_ENERGY_DRIFT");
        driftSpeedToStatusMap[4] = EnergyDriftStatus::NO_ENERGY_DRIFT;
    }    

    int totalUnconclusiveDrifts = driftSpeedToStatusMap.size();
    for (int speed: POSSIBLE_ENERGY_DRIFT_SPEEDS) {
        if (possibleDrifts.find(speed) == possibleDrifts.end()) {
            if (driftSpeedToStatusMap[speed] == EnergyDriftStatus::FOUND_ENERGY_DRIFT) {
                // This is an edge case when the first smaller drifts like 0.04 or 0.02 did not drift as the energy nodes went into the same tile
                log("Need to recover from wrongly identified drift tile for speed " + std::to_string(speed));

                finalEnergyDriftSpeed = -1;                
                int currentMax = -1;
                for (int possibleSpeed : possibleDrifts) {
                    if (possibleSpeed > currentMax) {
                        currentMax = possibleSpeed;
                    }                    
                }
                if (currentMax == -1) {
                    log("Problem: No possible drifts found for tile " + tile.toString());
                    std::cerr<<"Problem: No possible drifts found for tile "<<tile.toString()<<std::endl;
                } else {
                    driftSpeedToStatusMap[currentMax] = EnergyDriftStatus::UNKNOWN_ENERGY_DRIFT;
                    log("Recovering the speed " + std::to_string(currentMax));
                }
            }
            driftSpeedToStatusMap[speed] = EnergyDriftStatus::NO_ENERGY_DRIFT;
        }
    }    

    for (int speed: POSSIBLE_ENERGY_DRIFT_SPEEDS) {
        if (driftSpeedToStatusMap[speed] == EnergyDriftStatus::NO_ENERGY_DRIFT) {
            totalUnconclusiveDrifts--;
        }
    }

    if (totalUnconclusiveDrifts == 1) {
        for (int speed: POSSIBLE_ENERGY_DRIFT_SPEEDS) {
            if (driftSpeedToStatusMap[speed] == EnergyDriftStatus::UNKNOWN_ENERGY_DRIFT) {
                driftSpeedToStatusMap[speed] = EnergyDriftStatus::FOUND_ENERGY_DRIFT;
                log("Energy drift found at speed " + std::to_string(speed));

                finalEnergyDriftSpeed = speed;                
                break;
            }
        }
    }
}

void EnergyEstimator::updateEnergyNodes() {
    log("Updating energy nodes");

    int x1, x2, y1, y2;
    if (currentEnergyNode == -1) {
        log("Computing energy node for the first time");
        x1 = 0;
        x2 = 23;
        y1 = 0;
        y2 = 23;        
    } else {
        int x, y;
        symmetry_utils::toXY(currentEnergyNode, x, y);
        x1 = x - energyNodeDriftMagnitude;
        x2 = x + energyNodeDriftMagnitude;
        y1 = y - energyNodeDriftMagnitude;
        y2 = y + energyNodeDriftMagnitude;
    }

    std::unordered_set<int> possibleEnergyNodeTileIds;

    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            GameTile& tile = gameMap.getRolledOverTile(i, j);
            int tileId = tile.getId(gameMap.width);
            if (!symmetry_utils::isFirstHalfID(tileId)) {
                continue;
            }

            if (estimate(tileId)) {
                possibleEnergyNodeTileIds.insert(tileId);                
            }
        }
    }

    if (possibleEnergyNodeTileIds.size() == 1) {
        currentEnergyNode = *possibleEnergyNodeTileIds.begin();
        log("Energy node found at " + std::to_string(currentEnergyNode));
        updateEstimatedEnergies();
        delete energyValuesBuffer;
        energyValuesBuffer = nullptr;
    } else {
        log("Energy node not found, this is rare");
        clearEstimatedEnergies();
    }
}
