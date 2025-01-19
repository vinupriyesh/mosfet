#include "control_center.h"
#include <iostream>
#include "logger.h"
#include "visualizer/visualizer_client.h"
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

    log("creating GameMap");
    gameMap = new GameMap(gameEnvConfig->mapWidth, gameEnvConfig->mapHeight);

    haloConstraints = new ConstraintSet();
    planner = new Planner(shuttles);

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

    currentStep = gameState.obs.steps;
    currentMatchStep = gameState.obs.matchSteps;

    Logger::getInstance().setStepId(std::to_string(currentStep) + "/" + std::to_string(currentMatchStep));
    Metrics::getInstance().setStepId(std::to_string(currentStep));  
    log("Updating for step " + std::to_string(currentStep) + "/" + std::to_string(currentMatchStep));

    remainingOverageTime = gameState.remainingOverageTime;
    teamPointsDelta = gameState.obs.teamPoints[gameEnvConfig->teamId] - teamPoints;
    teamPoints = gameState.obs.teamPoints[gameEnvConfig->teamId];
    opponentTeamPointsDelta = gameState.obs.teamPoints[gameEnvConfig->opponentTeamId] - opponentTeamPoints;
    opponentTeamPoints = gameState.obs.teamPoints[gameEnvConfig->opponentTeamId];

    // At the start of each set, the team points delta is 0
    if (currentMatchStep == 0) {
        teamPointsDelta = 0;
        opponentTeamPointsDelta = 0;
    }

    Metrics::getInstance().add("points", teamPoints);
    Metrics::getInstance().add("opponentPoints", opponentTeamPoints);

    Metrics::getInstance().add("teamPointsDelta", teamPointsDelta);
    Metrics::getInstance().add("opponentTeamPointsDelta", opponentTeamPointsDelta);
    
    log("Exploring all units");
    // Exploring all units (cost 16)
    // REQUIREMENTS: NONE
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {

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
        }
    }

    log("Exploring all relics");
    // Exploring all relics (cost 8)
    // REQUIREMENTS:
    // 1. Visited nodes should be updated. i.e. Unit movements should've already happened
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


    log("Exploring contents of each tile");
    // Exploring contents of each tile (cost 24x24)
    // REQUIREMENTS:
    // 1. Visited nodes should be updated. i.e. Unit movements should've already happened
    allTilesExplored = true;
    allTilesVisited = true;
    tilesVisited = gameEnvConfig->mapHeight * gameEnvConfig->mapWidth;
    tilesExplored = gameEnvConfig->mapHeight * gameEnvConfig->mapWidth;

    for (int i = 0; i < gameEnvConfig->mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig->mapWidth; ++j) {
            GameTile& currentTile = gameMap->getTile(i, j);
            currentTile.setType(gameState.obs.mapFeatures.tileType[i][j], currentStep);
            currentTile.setEnergy(gameState.obs.mapFeatures.energy[i][j], currentStep);

            if (gameState.obs.sensorMask[i][j]) {
                currentTile.setExplored(true, currentStep);
            }

            currentTile.clearShuttles();
            for (int s = 0; s < gameEnvConfig->maxUnits; ++s) {
                if (shuttles[s]->getX() == i && shuttles[s]->getY() == j && !shuttles[s]->isGhost()) {
                    currentTile.addShuttle(shuttles[s]);
                }
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
    
    for (int i = 0; i < gameEnvConfig->mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig->mapWidth; ++j) { 
            GameTile& currentTile = gameMap->getTile(i, j);
            
            if (currentTile.isOccupied() && (currentTile.isVantagePoint() || currentTile.isHaloTile() || gameMap->hasPotentialInvisibleRelicNode(currentTile))) {
                // This is a halo tile, and is occupied.  This is a needed for constraint resolution
                if (!currentTile.isHaloTile() && !currentTile.isVantagePoint()) {
                    // for (auto& shuttle : currentTile.getShuttles()) {
                    //     log("Shuttles - " + std::to_string(i) + ", " + std::to_string(j) + " - Shuttle " + std::to_string(shuttle->id));
                    // }
                    log("Force setting halo tile for " + std::to_string(i) + ", " + std::to_string(j));
                    currentTile.setHaloTile(true);  //Setting if it is not already set
                }                
                constraintTiles.insert(currentTile.getId(gameEnvConfig->mapWidth));                
            }
        }
    }

    // Make sure the constraintTiles are not in the vantage points
    if (constraintTiles.size() >0) {
        haloConstraints->addConstraint(teamPointsDelta, constraintTiles);
    }
    // Collect information from the constraint set and clear it

    for (auto& regularTile : haloConstraints->identifiedRegularTiles) {
        int x = regularTile % gameEnvConfig->mapWidth;
        int y = regularTile / gameEnvConfig->mapWidth;
        log("Marking regular tile at " + std::to_string(x) + ", " + std::to_string(y));
        GameTile& currentTile = gameMap->getTile(x, y);
        currentTile.setHaloTile(false);
        currentTile.setForcedRegularTile(true);
    }

    vantagePointsFound = haloConstraints->identifiedVantagePoints.size();
    vantagePointsOccupied = 0;

    for (auto& vantagePoint : haloConstraints->identifiedVantagePoints) {
        int x = vantagePoint % gameEnvConfig->mapWidth;
        int y = vantagePoint / gameEnvConfig->mapWidth;
        log("Marking vantage point at " + std::to_string(x) + ", " + std::to_string(y));
        GameTile& currentTile = gameMap->getTile(x, y);

        currentTile.setVantagePoint(true);
        currentTile.setHaloTile(false);     

        if (currentTile.isOccupied()) {
            vantagePointsOccupied++;
        }
    }


    Metrics::getInstance().add("unexploited_vantage_points", vantagePointsFound - vantagePointsOccupied);

    if (teamPointsDelta - vantagePointsOccupied < 0 && currentMatchStep != 0) {
        log("Problem: Team points delta is less than vantage points occupied = " + std::to_string(teamPointsDelta) + " & " + std::to_string(vantagePointsOccupied));
        std::cerr<<"Problem: Team points delta < vantage points occupied"<<std::endl;
    }


    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Metrics::getInstance().add("update_duration", duration.count());

    log("Update complete");
}

void ControlCenter::plan() {
    int planIteration = 0;
    Communicator communicator;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i]->computePath();
        shuttles[i]->iteratePlan(planIteration, communicator);
    }

    

    planner->plan();
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
    shuttles = nullptr;
    opponentShuttles = nullptr;
    relics = nullptr;
}

ControlCenter::~ControlCenter() {
    delete gameEnvConfig;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
    delete haloConstraints;
    delete planner;
}


std::vector<std::vector<int>> ControlCenter::act() {
    auto start = std::chrono::high_resolution_clock::now();
    log("--- Acting step " + std::to_string(currentStep) + "/" + std::to_string(currentMatchStep) + " ---");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        std::vector<int> agentAction = shuttles[i]->act();
        std::vector<int> agentAction2 = shuttles[i]->act2();
        if (agentAction != agentAction2 && !shuttles[i]->isRandomAction()) {
            log("Problem: Rolebased actions not same as the legacy - " + std::to_string(agentAction[0]) + " != " + std::to_string(agentAction2[0]));
            std::cerr<<"Problem: Rolebased actions not same as the legacy - " + std::to_string(agentAction[0]) + " != " + std::to_string(agentAction2[0]) <<std::endl;
        }
        results.push_back(agentAction);
    }

    // Send data to live play visualizer
    if (Config::livePlayPlayer0 && gameEnvConfig->teamId == 0) {
        send_game_data(shuttles, opponentShuttles, relics, gameEnvConfig, gameMap, Config::portPlayer0);
    }
    if (Config::livePlayPlayer1 && gameEnvConfig->teamId == 1) {
        send_game_data(shuttles, opponentShuttles, relics, gameEnvConfig, gameMap, Config::portPlayer1);
    }
    // std::cerr<< "test cc" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("act_duration", duration.count());
    return results;
}
