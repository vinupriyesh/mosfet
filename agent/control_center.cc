#include "control_center.h"
#include <iostream>
#include "logger.h"
#include "visualizer/visualizer_client.h"
#include "config.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

/**
 * Initialize the control center object.  This is invoked during the first update.
 */
void ControlCenter::init(GameState& gameState) {
    log("Initializing the control center");
    
    gameEnvConfig = new GameEnvConfig(gameState);

    // Shuttles (player and opponent).  This is created once and reused.
    shuttles = new Shuttle*[gameEnvConfig->maxUnits];
    opponentShuttles = new Shuttle*[gameEnvConfig->maxUnits];
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::player, this);
        opponentShuttles[i] = new Shuttle(i, ShuttleType::opponent, this);
    }

    relics = new Relic*[gameEnvConfig->relicCount];
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        relics[i] = new Relic(i);     
    }

    gameMap = new GameMap(gameEnvConfig->mapWidth, gameEnvConfig->mapHeight);

    haloConstraints = new ConstraintSet();
    log("Initialization complete");
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

    Logger::getInstance().setStepId(std::to_string(currentStep) + "/" + std::to_string(currentMatchStep));    
    log("Updating for step " + std::to_string(currentStep) + "/" + std::to_string(currentMatchStep));

    remainingOverageTime = gameState.remainingOverageTime;
    teamPointsDelta = gameState.obs.teamPoints[gameEnvConfig->teamId] - teamPoints;
    teamPoints = gameState.obs.teamPoints[gameEnvConfig->teamId];
    opponentTeamPointsDelta = gameState.obs.teamPoints[gameEnvConfig->opponentTeamId] - opponentTeamPoints;
    opponentTeamPoints = gameState.obs.teamPoints[gameEnvConfig->opponentTeamId];
    
    log("Exploring all units");
    // Exploring all units (cost 16)
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {

        // log("Getting old position for unit " + std::to_string(i));
        // Get the unit's older position 
        int oldX = shuttles[i]->getX();
        int oldY = shuttles[i]->getY();

        // log("OldTile positions for unit " + std::to_string(i) + " - " + std::to_string(oldX) + ", " + std::to_string(oldY));

        // Update unit's current position and energy
        shuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->teamId][i],
                                     gameState.obs.units.energy[gameEnvConfig->teamId][i]);

                                         
        opponentShuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->opponentTeamId][i],
                                          gameState.obs.units.energy[gameEnvConfig->opponentTeamId][i]);
                                          
        shuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->teamId][i]);
        opponentShuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->opponentTeamId][i]);     

        if (gameMap->isValidTile(shuttles[i]->getX(), shuttles[i]->getY())) {
            // log("Updating visited for tile " + std::to_string(shuttles[i]->getX()) + ", " + std::to_string(shuttles[i]->getY()));
            GameTile& shuttleTile = gameMap->getTile(shuttles[i]->getX(), shuttles[i]->getY());
            shuttleTile.setVisited(true, currentStep);

            if (gameMap->isValidTile(oldX, oldY)) {
                GameTile& oldShuttleTile = gameMap->getTile(oldX, oldY);
                oldShuttleTile.clearShuttle(shuttles[i]);
            }
            shuttleTile.setShuttle(shuttles[i]);
        }    
    }

    log("Exploring all relics");
    // Exploring all relics (cost 8)
    allRelicsFound = true;
    relicsFound = 0;
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        bool firstTimeRevealed = relics[i]->updateRelicData(gameState.obs.relicNodes[i], gameState.obs.relicNodesMask[i]);
        if (firstTimeRevealed) {
            log("Relic " + std::to_string(i) + " found at " + std::to_string(relics[i]->position[0]) + ", " + std::to_string(relics[i]->position[1]));
            gameMap->addRelic(relics[i], currentStep);
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

    log("Clearing the halo nodes");
    // If team did not score points, clear halo nodes where the shuttle is currently in (Cost 16)
    if (teamPointsDelta == 0) {
        for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
            int x = shuttles[i]->getX();
            int y = shuttles[i]->getY();
            if (gameMap->isValidTile(x, y)) {
                GameTile& currentTile = gameMap->getTile(x, y);
                if (currentTile.isHaloTile()) {
                    currentTile.setHaloTile(false);
                }
            }
        }
    }

    log("Exploring contents of each tile");
    // Exploring contents of each tile (cost 24x24)
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


    // Monitor change in points to observe the relic capture
    // Collect all positions that are on a halo node or possibly on a halo node with an invisible relic
    if (teamPointsDelta > 0) {
        
        // Constraint tile ids
        std::set<int> constraintTiles;
        
        for (int i = 0; i < gameEnvConfig->mapHeight; ++i) {
            for (int j = 0; j < gameEnvConfig->mapWidth; ++j) { 
                GameTile& currentTile = gameMap->getTile(i, j);
                
                if (currentTile.isOccupied() && (currentTile.isHaloTile() || gameMap->hasPotentialInvisibleRelicNode(currentTile))) {
                    // This is a halo tile, and is occupied.  This is a needed for constraint resolution
                    currentTile.setHaloTile(true);  //Setting if it is not already set                   
                    constraintTiles.insert(currentTile.getId(gameEnvConfig->mapWidth));
                }
            }
        }

        haloConstraints->addConstraint(teamPointsDelta, constraintTiles);
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
    delete haloConstraints;
}


std::vector<std::vector<int>> ControlCenter::act() {
    log("--- Acting step " + std::to_string(currentStep) + "/" + std::to_string(currentMatchStep) + " ---");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        results.push_back(shuttles[i]->act());
    }

    // Send data to live play visualizer
    if (Config::livePlayPlayer0 && gameEnvConfig->teamId == 0) {
        send_game_data(shuttles, opponentShuttles, relics, gameEnvConfig, gameMap, Config::portPlayer0);
    }
    if (Config::livePlayPlayer1 && gameEnvConfig->teamId == 1) {
        send_game_data(shuttles, opponentShuttles, relics, gameEnvConfig, gameMap, Config::portPlayer1);
    }
    // std::cerr<< "test cc" << std::endl;
    return results;
}
