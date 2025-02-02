#include "control_center.h"
#include <iostream>
#include "logger.h"
#include "config.h"
#include "agent/planning/planner.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

/**
 * Initialize the control center object.  This is invoked during the first update.
 */
void ControlCenter::init(GameState& gameState) {
    auto start = std::chrono::high_resolution_clock::now();
    log("Initializing the control center");
    
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    gameEnvConfig.init(gameState);

    log("creating GameMap");
    gameMap = new GameMap(gameEnvConfig.mapWidth, gameEnvConfig.mapHeight);

    relics = new Relic*[gameEnvConfig.relicCount];
    for (int i = 0; i < gameEnvConfig.relicCount; ++i) {
        relics[i] = new Relic(i);     
    }

    // Shuttles (player and opponent).  This is created once and reused.
    shuttles = new Shuttle*[gameEnvConfig.maxUnits];
    opponentShuttles = new Shuttle*[gameEnvConfig.maxUnits];
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::player, *gameMap);
        opponentShuttles[i] = new Shuttle(i, ShuttleType::opponent, *gameMap);
    }

    haloConstraints = new ConstraintSet();
    planner = new Planner(shuttles, *gameMap);


    visualizerClientPtr = new VisualizerClient(*gameMap, shuttles, opponentShuttles, relics);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    log("Initialization complete " + std::to_string(duration.count()));
}

/**
 * Update the control center object.  This is invoked during each update.
 */
void ControlCenter::update(GameState& gameState) {

    auto start = std::chrono::high_resolution_clock::now();

    if (shuttles == nullptr) {
        init(gameState);
    }    

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap->derivedGameState;

    state.currentStep = gameState.obs.steps;
    state.currentMatchStep = gameState.obs.matchSteps;

    Logger::getInstance().setStepId(std::to_string(state.currentStep) + "/" + std::to_string(state.currentMatchStep));
    Metrics::getInstance().setStepId(std::to_string(state.currentStep));  
    log("Updating for step " + std::to_string(state.currentStep) + "/" + std::to_string(state.currentMatchStep));

    state.remainingOverageTime = gameState.remainingOverageTime;
    state.teamPointsDelta = gameState.obs.teamPoints[gameEnvConfig.teamId] - state.teamPoints;
    state.teamPoints = gameState.obs.teamPoints[gameEnvConfig.teamId];
    state.opponentTeamPointsDelta = gameState.obs.teamPoints[gameEnvConfig.opponentTeamId] - state.opponentTeamPoints;
    state.opponentTeamPoints = gameState.obs.teamPoints[gameEnvConfig.opponentTeamId];

    // At the start of each set, the team points delta is 0
    if (state.currentMatchStep == 0) {
        state.teamPointsDelta = 0;
        state.opponentTeamPointsDelta = 0;
    }

    Metrics::getInstance().add("points", state.teamPoints);
    Metrics::getInstance().add("opponentPoints", state.opponentTeamPoints);

    Metrics::getInstance().add("teamPointsDelta", state.teamPointsDelta);
    Metrics::getInstance().add("opponentTeamPointsDelta", state.opponentTeamPointsDelta);
    
    log("Exploring all units");
    // Exploring all units (cost 16)
    // REQUIREMENTS: NONE
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {

        // log("OldTile positions for unit " + std::to_string(i) + " - " + std::to_string(oldX) + ", " + std::to_string(oldY));

        // Update unit's current position and energy
        shuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig.teamId][i],
                                     gameState.obs.units.energy[gameEnvConfig.teamId][i]);

                                         
        opponentShuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig.opponentTeamId][i],
                                          gameState.obs.units.energy[gameEnvConfig.opponentTeamId][i]);
                                          
        shuttles[i]->updateVisibility(gameState.obs.unitsMask[gameEnvConfig.teamId][i]);
        opponentShuttles[i]->updateVisibility(gameState.obs.unitsMask[gameEnvConfig.opponentTeamId][i]);     

        if (gameMap->isValidTile(shuttles[i]->getShuttleData().getX(), shuttles[i]->getShuttleData().getY())) {
            // log("Updating visited for tile " + std::to_string(shuttles[i]->getX()) + ", " + std::to_string(shuttles[i]->getY()));
            GameTile& shuttleTile = gameMap->getTile(shuttles[i]->getShuttleData().getX(), shuttles[i]->getShuttleData().getY());
            shuttleTile.setVisited(true, state.currentStep);
        }
    }

    log("Exploring all relics");
    // Exploring all relics (cost 8)
    // REQUIREMENTS:
    // 1. Visited nodes should be updated. i.e. Unit movements should've already happened
    state.allRelicsFound = true;
    state.relicsFound = 0;

    std::vector<int> forcedHaloTileIds;
    for (int i = 0; i < gameEnvConfig.relicCount; ++i) {
        bool firstTimeRevealed = relics[i]->updateRelicData(gameState.obs.relicNodes[i], gameState.obs.relicNodesMask[i]);
        if (firstTimeRevealed) {
            log("Relic " + std::to_string(i) + " found at " + std::to_string(relics[i]->position[0]) + ", " + std::to_string(relics[i]->position[1]));
            gameMap->addRelic(relics[i], state.currentStep, forcedHaloTileIds);
        }

        if (!relics[i]->revealed) {
            state.allRelicsFound = false;
        } else {
            state.relicsFound++;
        }
    }
    haloConstraints->reconsiderNormalizedTile(forcedHaloTileIds);

    if (state.allRelicsFound) {
        log("All relics found :)");
    }
    


    log("Exploring contents of each tile");
    // Exploring contents of each tile (cost 24x24)
    // REQUIREMENTS:
    // 1. Visited nodes should be updated. i.e. Unit movements should've already happened
    state.allTilesExplored = true;
    state.allTilesVisited = true;
    state.tilesVisited = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;
    state.tilesExplored = gameEnvConfig.mapHeight * gameEnvConfig.mapWidth;

    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& currentTile = gameMap->getTile(i, j);
            currentTile.setType(gameState.obs.mapFeatures.tileType[i][j], state.currentStep);
            currentTile.setVisible(gameState.obs.sensorMask[i][j]);
            if (gameState.obs.sensorMask[i][j]) {
                // Update energy only if the node is visible.
                currentTile.setEnergy(gameState.obs.mapFeatures.energy[i][j], state.currentStep);
            }            

            if (gameState.obs.sensorMask[i][j]) {
                gameMap->exploreTile(currentTile, state.currentStep);
            }

            currentTile.clearShuttles();
            for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
                if (shuttles[s]->getShuttleData().getX() == i && shuttles[s]->getShuttleData().getY() == j && !shuttles[s]->isGhost()) {
                    currentTile.addShuttle(&shuttles[s]->getShuttleData());
                }
            }

            currentTile.clearOpponentShuttles();
            for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
                if (opponentShuttles[s]->getShuttleData().getX() == i && opponentShuttles[s]->getShuttleData().getY() == j && !opponentShuttles[s]->isGhost()) {
                    currentTile.addOpponentShuttle(&opponentShuttles[s]->getShuttleData());
                }
            }

            if (!currentTile.isVisited()) {
                state.allTilesVisited = false;
                state.tilesVisited--;
            }

            if (!currentTile.isExplored()) {
                state.allTilesExplored = false;
                state.tilesExplored--;
            }
            
        }
    }

    if (state.allTilesExplored) {
        log("All tiles explored :)");
    }

    log("Checking for constraints");
    // Monitor change in points to observe the relic capture
    // Collect all positions that are on a halo node or possibly on a halo node with an invisible relic
    //REQUIREMENTS: 
    // 1. The exploration update should've already happened
    // 2. Visited nodes should be updated. i.e. Unit movements should've already happened
    // 3. Halo node updates should've already happened
    // 4. Occupied shuttles should be populated already. i.e isOccupied() should work
        
    // Constraint tile ids
    std::set<int> constraintTiles;
    
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) { 
            GameTile& currentTile = gameMap->getTile(i, j);
            
            if (currentTile.isOccupied() && (currentTile.isVantagePoint() || currentTile.isHaloTile() || gameMap->hasPotentialInvisibleRelicNode(currentTile))) {
                // This is a halo tile, and is occupied.  This is a needed for constraint resolution
                if (!currentTile.isHaloTile() && !currentTile.isVantagePoint()) {
                    // for (auto& shuttle : currentTile.getShuttles()) {
                    //     log("Shuttles - " + std::to_string(i) + ", " + std::to_string(j) + " - Shuttle " + std::to_string(shuttle->id));
                    // }
                    log("Force setting halo tile for " + std::to_string(i) + ", " + std::to_string(j));
                    currentTile.setHaloTile(true);  //Setting if it is not already set
                    haloConstraints->reconsiderNormalizedTile(currentTile.getId(gameEnvConfig.mapWidth));
                }                
                constraintTiles.insert(currentTile.getId(gameEnvConfig.mapWidth));                
            }
        }
    }

    // Not adding constraint if the matchstep is 0
    if (constraintTiles.size() >0 && state.currentMatchStep != 0) {
        haloConstraints->addConstraint(state.teamPointsDelta, constraintTiles);
    }
    // Collect information from the constraint set and clear it

    for (auto& regularTile : haloConstraints->identifiedRegularTiles) {
        int x = regularTile % gameEnvConfig.mapWidth;
        int y = regularTile / gameEnvConfig.mapWidth;
        log("Marking regular tile at " + std::to_string(x) + ", " + std::to_string(y));
        GameTile& currentTile = gameMap->getTile(x, y);
        currentTile.setHaloTile(false);
        currentTile.setForcedRegularTile(true);
    }

    state.vantagePointsFound = haloConstraints->identifiedVantagePoints.size();
    state.vantagePointsOccupied = 0;

    for (auto& vantagePoint : haloConstraints->identifiedVantagePoints) {
        int x = vantagePoint % gameEnvConfig.mapWidth;
        int y = vantagePoint / gameEnvConfig.mapWidth;
        log("Marking vantage point at " + std::to_string(x) + ", " + std::to_string(y));
        GameTile& currentTile = gameMap->getTile(x, y);

        currentTile.setVantagePoint(true);
        currentTile.setHaloTile(false);     

        if (currentTile.isOccupied()) {
            state.vantagePointsOccupied++;
        }
    }

    Metrics::getInstance().add("unexploited_vantage_points", state.vantagePointsFound - state.vantagePointsOccupied);

    if (state.teamPointsDelta - state.vantagePointsOccupied < 0 && state.currentMatchStep != 0) {
        log("Problem: Team points delta is less than vantage points occupied = " + std::to_string(state.teamPointsDelta) + " & " + std::to_string(state.vantagePointsOccupied));
        std::cerr<<"Problem: Team points delta < vantage points occupied"<<std::endl;
    }


    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Metrics::getInstance().add("update_duration", duration.count());

    log("Update complete");
}

void ControlCenter::plan() {
    log("planning");
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();

    planner->plan();
    log("Planning complete");
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
    shuttles = nullptr;
    opponentShuttles = nullptr;
    relics = nullptr;
}

ControlCenter::~ControlCenter() {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    log("destroying cc");
    delete visualizerClientPtr;
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
    delete[] opponentShuttles;
    delete haloConstraints;
    delete planner;
}


std::vector<std::vector<int>> ControlCenter::act() {
    auto start = std::chrono::high_resolution_clock::now();

    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    DerivedGameState& state = gameMap->derivedGameState;

    log("--- Acting step " + std::to_string(state.currentStep) + "/" + std::to_string(state.currentMatchStep) + " ---");

    std::vector<std::vector<int>> results;
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        std::vector<int> agentAction = shuttles[i]->act();        
        results.push_back(agentAction);
    }

    if (visualizerClientPtr != nullptr) {
        visualizerClientPtr->send_game_data();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("act_duration", duration.count());
    return results;
}
