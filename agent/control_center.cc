#include "control_center.h"
#include <iostream>
#include "logger.h"
#include "visualizer/visualizer_client.h"
#include "game_map.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

/**
 * Initialize the control center object.  This is invoked during the first update.
 */
void ControlCenter::init(GameState& gameState) {
    log("Initializing the control center");
    
    gameEnvConfig = new GameEnvConfig(gameState);

    // Shuttles (player and enemy).  This is created once and reused.
    shuttles = new Shuttle*[gameEnvConfig->maxUnits];
    enemyShuttles = new Shuttle*[gameEnvConfig->maxUnits];
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::player, this);
        enemyShuttles[i] = new Shuttle(i, ShuttleType::enemy, this);
    }

    relics = new Relic*[gameEnvConfig->relicCount];
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        relics[i] = new Relic(i);     
    }

    gameMap = new GameMap(gameEnvConfig->mapWidth, gameEnvConfig->mapHeight);
}

/**
 * Update the control center object.  This is invoked during each update.
 */
void ControlCenter::update(GameState& gameState) {

    if (shuttles == nullptr) {
        init(gameState);
    }

    currentStep = gameState.obs.steps;
    currentMatchStep = gameState.obs.matchSteps;
    remainingOverageTime = gameState.remainingOverageTime;

    Logger::getInstance().setStepId(std::to_string(currentStep) + "/" + std::to_string(currentMatchStep));
    
    // Exploring all units
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->teamId][i],
                                     gameState.obs.units.energy[gameEnvConfig->teamId][i]);
        enemyShuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->enemyTeamId][i],
                                          gameState.obs.units.energy[gameEnvConfig->enemyTeamId][i]);
                                          
        shuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->teamId][i]);
        enemyShuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->enemyTeamId][i]);     

        if (gameMap->isValidTile(shuttles[i]->getX(), shuttles[i]->getY())) {
            // log("Updating visited for tile " + std::to_string(shuttles[i]->getX()) + ", " + std::to_string(shuttles[i]->getY()));
            gameMap->getTile(shuttles[i]->getX(), shuttles[i]->getY()).setVisited(true, currentStep); 
        }            
    }

    // Exploring contents of each tile
    allTilesExplored = true;
    allTilesVisited = true;
    tilesVisited = gameEnvConfig->mapHeight * gameEnvConfig->mapWidth;
    tilesExplored = gameEnvConfig->mapHeight * gameEnvConfig->mapWidth;
    for (int i = 0; i < gameEnvConfig->mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig->mapWidth; ++j) {
            GameTile& currentTile = gameMap->getTile(i, j);
            currentTile.setType(gameState.obs.mapFeatures.tileType[i][j], currentStep);
            currentTile.setEnergy(gameState.obs.mapFeatures.tileType[i][j], currentStep);

            if (gameState.obs.sensorMask[i][j]) {
                currentTile.setExplored(true, currentStep);
            }

            if (!currentTile.isVisited()) {
                allTilesVisited = false;
                tilesVisited--;
            }

            if (!currentTile.isExplored()) {
                allTilesExplored = false;
                tilesExplored--;
            }
        }
    }

    if (allTilesExplored) {
        log("All tiles explored :)");
    }

    // Exploring all relics
    allRelicsFound = true;
    relicsFound = 0;
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        bool firstTimeRevealed = relics[i]->updateRelicData(gameState.obs.relicNodes[i], gameState.obs.relicNodesMask[i]);
        if (firstTimeRevealed) {
            log("Relic " + std::to_string(i) + " found at " + std::to_string(relics[i]->position[0]) + ", " + std::to_string(relics[i]->position[1]));
            gameMap->addRelic(relics[i]);
        }

        if (!relics[i]->revealed) {
            allRelicsFound = false;
        } else {
            relicsFound++;
        }
    }

    if (allRelicsFound) {
        log("All relics found :)");
    }
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
}

ControlCenter::~ControlCenter() {
    delete gameEnvConfig;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
}


std::vector<std::vector<int>> ControlCenter::act() {
    log("--- Acting step " + std::to_string(currentStep) + "/" + std::to_string(currentMatchStep) + " ---");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        results.push_back(shuttles[i]->act());
    }

    // Send data to live play visualizer
    if (Logger::getInstance().isDebugEnabled() && gameEnvConfig->playerName == "player_0") {
        send_game_data(shuttles, enemyShuttles, relics, gameEnvConfig, gameMap);
    }
    // std::cerr<< "test cc" << std::endl;
    return results;
}
