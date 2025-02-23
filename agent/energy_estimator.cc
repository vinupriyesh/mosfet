#include "energy_estimator.h"
#include "logger.h"
#include "symmetry_util.h"

#include <cmath>

void EnergyEstimator::log(std::string message) {
    Logger::getInstance().log("EnergyEstimator -> " + message);
}

// Function to compute the Euclidean distance between two points (a_x, a_y) and (b_x, b_y)
double euclideanDistance(int aX, int aY, int bX, int bY) {
    int deltaX = aX - bX;
    int deltaY = aY - bY;
    return std::sqrt(deltaX * deltaX + deltaY * deltaY);
}

void EnergyEstimator::estimate(std::vector<int> energyNodeTileIds) {
    log("Inside estimate");
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
                    symmetry_utils::toXY(energyNodeTileIds[0], x, y);
                    double distance = euclideanDistance(i, j, x, y);
                    estimatedEnergy[i][j][k] =  sin(distance * energyNodeFns[0][1] + energyNodeFns[0][2]) * energyNodeFns[0][3];
                    if (i == 0 && j == 0) {
                        log("D(0, 0) @1 = " + std::to_string(distance));                        
                    }
                    if (i == 1 && j == 0) {
                        log("D(1, 0) @1 = " + std::to_string(distance));                        
                    }
                } else if (k == 3) {
                    int x, y;
                    symmetry_utils::toXY(energyNodeTileIds[1], x, y);
                    double distance = euclideanDistance(i, j, x, y);
                    estimatedEnergy[i][j][k] += sin(distance * energyNodeFns[0][1] + energyNodeFns[0][2]) * energyNodeFns[0][3];
                    if (i == 0 && j == 0) {
                        log("D(0, 0) @2 = " + std::to_string(distance));
                    }
                    if (i == 1 && j == 0) {
                        log("D(1, 0) @2 = " + std::to_string(distance));
                    }
                } else {
                    estimatedEnergy[i][j][k] = 0;
                }
            
                meanValue += estimatedEnergy[i][j][k];
            }

            if (i == 0 && j == 0) {
                log("E(0, 0) " + std::to_string(estimatedEnergy[i][j][0]) + ", " + std::to_string(estimatedEnergy[i][j][1]) + ", " + std::to_string(estimatedEnergy[i][j][2]) + ", " + std::to_string(estimatedEnergy[i][j][3]) + ", " + std::to_string(estimatedEnergy[i][j][4]) + ", " + std::to_string(estimatedEnergy[i][j][5]) );
            }
            if (i == 1 && j == 0) {
                log("E(1, 0) " + std::to_string(estimatedEnergy[i][j][0]) + ", " + std::to_string(estimatedEnergy[i][j][1]) + ", " + std::to_string(estimatedEnergy[i][j][2]) + ", " + std::to_string(estimatedEnergy[i][j][3]) + ", " + std::to_string(estimatedEnergy[i][j][4]) + ", " + std::to_string(estimatedEnergy[i][j][5]) );
            }
            // GameTile& tile = gameMap.getTile(i, j);
            // log("Estimated vs actual at (" + std::to_string(i) + "," + std::to_string(j) + ") with distance = " + std::to_string(distance) + " / " + std::to_string(mirrorDistance) + " is " + std::to_string(estimatedEnergy[i][j]) + " vs " + std::to_string(tile.getEnergy()));
        }
    }    

    meanValue /= gameEnvConfig.mapHeight * gameEnvConfig.mapWidth * sizeOfEnergyNodeFns;
    log("Mean value of estimated energy: " + std::to_string(meanValue));

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

            if (i == 0 && j == 0) {
                log("E'(0, 0) " + std::to_string(estimatedEnergy[i][j][0]) + ", " + std::to_string(estimatedEnergy[i][j][1]) + ", " + std::to_string(estimatedEnergy[i][j][2]) + ", " + std::to_string(estimatedEnergy[i][j][3]) + ", " + std::to_string(estimatedEnergy[i][j][4]) + ", " + std::to_string(estimatedEnergy[i][j][5]) );
            }

            if (i == 1 && j == 0) {
                log("E'(1, 0) " + std::to_string(estimatedEnergy[i][j][0]) + ", " + std::to_string(estimatedEnergy[i][j][1]) + ", " + std::to_string(estimatedEnergy[i][j][2]) + ", " + std::to_string(estimatedEnergy[i][j][3]) + ", " + std::to_string(estimatedEnergy[i][j][4]) + ", " + std::to_string(estimatedEnergy[i][j][5]) );
            }

            int estimatedEnergyValueRounded = std::round(estimatedEnergyValue);
            log( "(" + std::to_string(i) + "," + std::to_string(j) + ") Estimated = " + std::to_string(estimatedEnergyValueRounded) + " vs " + std::to_string(tile.getEnergy()));
            if (estimatedEnergyValueRounded != tile.getEnergy()) {
                allGood = false;
            }
        }
    }
    
    newMeanValue /= gameEnvConfig.mapHeight * gameEnvConfig.mapWidth * sizeOfEnergyNodeFns;
    log("New Mean value of estimated energy: " + std::to_string(newMeanValue));

    log("All Good? " + std::to_string(allGood));
}


