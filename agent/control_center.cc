#include "control_center.h"
#include <iostream>
#include "agent/opponent_tracker.h"
#include "agent/shuttle_energy_tracker.h"
#include "constants.h"
#include "logger.h"
#include "config.h"
#include "agent/planning/planner.h"
#include "game_env_config.h"
#include "symmetry_util.h"

void ControlCenter::log(const std::string& message) {
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

    if (Config::enableLogging) {
        Logger::getInstance().enableLogging("application_" + std::to_string(gameEnvConfig.teamId)+ ".log");
    }

    log("creating GameMap");
    gameMap = new GameMap(gameEnvConfig.mapWidth, gameEnvConfig.mapHeight);

    // Shuttles (player and opponent).  This is created once and reused.
    shuttles = new Shuttle*[gameEnvConfig.maxUnits];
    opponentShuttles = new Shuttle*[gameEnvConfig.maxUnits];
    relicDiscoveryKey.insert(relicDiscoveryKey.end(),  gameState.obs.relicNodes.size(), -1);
    gameMap->derivedGameState.relicDiscoveryStatus.resize(gameEnvConfig.matchCountPerEpisode, RelicDiscoveryStatus::INIT);

    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::PLAYER, *gameMap);
        opponentShuttles[i] = new Shuttle(i, ShuttleType::OPPONENT, *gameMap);
        gameMap->shuttles.push_back(&shuttles[i]->getShuttleData());
        gameMap->opponentShuttles.push_back(&opponentShuttles[i]->getShuttleData());
    }

    haloConstraints = new ConstraintSet();    
    driftDetector = new DriftDetector(*gameMap);
    energyEstimator = new EnergyEstimator(*gameMap);
    opponentTracker = new OpponentTracker(*gameMap, respawnRegistry);
    battleEvaluator = new BattleEvaluator(*gameMap, *opponentTracker);
    planner = new Planner(shuttles, *gameMap, *opponentTracker);
    shuttleEnergyTracker = new ShuttleEnergyTracker(*gameMap, *opponentTracker, respawnRegistry);

    visualizerClientPtr = new VisualizerClient(*gameMap, shuttles, opponentShuttles, relics, *opponentTracker);

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

    state.teamWins = gameState.obs.teamWins[gameEnvConfig.teamId];
    state.opponentWins = gameState.obs.teamWins[gameEnvConfig.opponentTeamId];

    // At the start of each set, the team points delta is 0
    if (state.currentMatchStep == 0) {
        state.teamPointsDelta = 0;
        state.opponentTeamPointsDelta = 0;        
        state.currentMatch += 1;
        log(" --> Match " + std::to_string(state.currentMatch) + " start <----");  
        if (state.currentMatch >= 3) {
            state.relicDiscoveryStatus[state.currentMatch] = RelicDiscoveryStatus::NOT_APPLICABLE;
        } else {
            state.relicDiscoveryStatus[state.currentMatch] = RelicDiscoveryStatus::SEARCHING;
        }
        respawnRegistry.reset();

        for (int i = 0; i < gameEnvConfig.maxUnits; i ++) {
            respawnRegistry.pushPlayerUnit(i, state.currentMatchStep);
            respawnRegistry.pushOpponentUnit(i, state.currentMatchStep);
        }
    }

    respawnRegistry.step(state.currentMatchStep);

    Metrics::getInstance().add("points", state.teamPoints);
    Metrics::getInstance().add("opponentPoints", state.opponentTeamPoints);

    Metrics::getInstance().add("teamPointsDelta", state.teamPointsDelta);
    Metrics::getInstance().add("opponentTeamPointsDelta", state.opponentTeamPointsDelta);
    
    log("Exploring all units");
    // Exploring all units (cost 16)
    // REQUIREMENTS: NONE
    for (int i = 0; i < gameEnvConfig.maxUnits; ++i) {

        // log("OldTile positions for unit " + std::to_string(i) + " - " + std::to_string(oldX) + ", " + std::to_string(oldY));
        
        shuttles[i]->updateVisibility(gameState.obs.unitsMask[gameEnvConfig.teamId][i]);
        opponentShuttles[i]->updateVisibility(gameState.obs.unitsMask[gameEnvConfig.opponentTeamId][i]);
        
        // Update unit's current position and energy
        shuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig.teamId][i],
                                     gameState.obs.units.energy[gameEnvConfig.teamId][i],
                                     state.currentStep);

                                         
        opponentShuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig.opponentTeamId][i],
                                          gameState.obs.units.energy[gameEnvConfig.opponentTeamId][i],
                                          state.currentStep);
                                                  

        if (gameMap->isValidTile(shuttles[i]->getShuttleData().getX(), shuttles[i]->getShuttleData().getY())) {
            // log("Updating visited for tile " + std::to_string(shuttles[i]->getX()) + ", " + std::to_string(shuttles[i]->getY()));
            GameTile& shuttleTile = gameMap->getTile(shuttles[i]->getShuttleData().getX(), shuttles[i]->getShuttleData().getY());
            shuttleTile.setVisited(true, state.currentStep);
        }

        if (opponentShuttles[i]->isVisible() && opponentShuttles[i]->isGhost()) {
            respawnRegistry.pushOpponentUnit(i, state.currentMatchStep);
        }
    }

    log("Exploring all relics");
    // Exploring all relics (cost 8)
    // REQUIREMENTS:
    // 1. Visited nodes should be updated. i.e. Unit movements should've already happened

    std::vector<int> forcedHaloTileIds;
    for (int i = 0; i < gameState.obs.relicNodesMask.size(); ++i) {
        if (!gameState.obs.relicNodesMask[i]) {
            // Return if not visible
            continue;
        }
        
        int positionId = symmetry_utils::toID(gameState.obs.relicNodes[i][0], gameState.obs.relicNodes[i][1]);                

        if (relics.find(positionId) == relics.end()) {
            // This is a new relic
            Relic* relic = new Relic(positionId, gameState.obs.relicNodes[i]);
            relics[positionId] = relic;
            relic->addDiscoveryId(i);
            relicDiscoveryKey[i] = positionId;
            state.setRelicDiscoveryStatus(RelicDiscoveryStatus::FOUND);

            log("Relic " + std::to_string(positionId) + " found at " + std::to_string(relic->position[0]) + ", " + std::to_string(relic->position[1]));
            gameMap->addRelic(relic, state.currentStep, forcedHaloTileIds);

            std::vector<int> mirroredPosition = relic->getMirroredPosition(gameEnvConfig.mapWidth, gameEnvConfig.mapHeight);
            int mirroredPositionId = mirroredPosition[1] * gameEnvConfig.mapWidth + mirroredPosition[0];

            if (mirroredPositionId != positionId) {
                //Mirrored relic does not sit on the diagonal, so it is a new relic
                Relic* mirroredRelic = new Relic(mirroredPositionId, mirroredPosition);
                relics[mirroredPositionId] = mirroredRelic;

                log("Mirrored Relic " + std::to_string(mirroredPositionId) + " found at " + std::to_string(mirroredRelic->position[0]) + ", " + std::to_string(mirroredRelic->position[1]));
                gameMap->addRelic(mirroredRelic, state.currentStep, forcedHaloTileIds);
            }
        }

        if (relicDiscoveryKey[i] == -1) {
            //This could mean one of the following
            // 1. This relic is a mirror that we have already onboarded
            // 2. This relic is on the diagonal that is already created
            // 3. This relic is a re-spawn in the same tile
            log("Relic discovery key is null for relic " + std::to_string(i));

            bool isDiagonal = symmetry_utils::isOnDiagonal(positionId);
            bool respawned = false;
            int diagonalCount = 0;
            for (int j = 0; j < gameState.obs.relicNodesMask.size() ; ++j) {
                if (j == i) {
                    continue;
                }
                if (relicDiscoveryKey[j] == positionId) {
                    if (isDiagonal) {
                        if (diagonalCount > 0) {
                            //This is a respawned diagonal, atleast 3rd time encounting the same positionId
                            respawned = true;
                        } else {
                            diagonalCount++;
                        }
                    } else {
                        //This is not a diagonal, so it should've been respawned
                        respawned = true;
                    }
                }
            }

            Relic* relic = relics[positionId];
            relic->addDiscoveryId(i);
            relicDiscoveryKey[i] = positionId;

            if (respawned) {
                gameMap->addRelic(relic, state.currentStep, forcedHaloTileIds);
                state.setRelicDiscoveryStatus(RelicDiscoveryStatus::FOUND);
            }
        }
    }

    log("Count of relics " + std::to_string(relics.size()));

    haloConstraints->reconsiderNormalizedTile(forcedHaloTileIds);    

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
            GameTile& currentMirrorTile = gameMap->getMirroredTile(i, j);

            currentTile.setVisible(gameState.obs.sensorMask[i][j] != 0);

            auto tileType = GameTile::translateTileType(gameState.obs.mapFeatures.tileType[i][j]);            
            
            if (gameState.obs.sensorMask[i][j] != 0) {
                //Visible tile updates

                currentTile.setType(tileType, state.currentStep, driftDetector->driftFinalized);                
                currentTile.setEnergy(gameState.obs.mapFeatures.energy[i][j], state.currentStep);
                gameMap->exploreTile(currentTile, state.currentStep);

                currentMirrorTile.setType(tileType, state.currentStep, driftDetector->driftFinalized);                
                currentMirrorTile.setEnergy(gameState.obs.mapFeatures.energy[i][j], state.currentStep);
                gameMap->exploreTile(currentMirrorTile, state.currentStep);
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

    log("Identify nebula tiles using the invisibility data");

    if (state.currentStep > 1 && driftDetector->isDriftPossible(state.currentStep - 1) == TruthValue::FALSE) { // There is a bug in the environment, it doesn't show the nebula visibility mask properly if there was a drift the previous step! See seed 245923829
        for (int s = 0; s < gameEnvConfig.maxUnits; ++s) {
            if (shuttles[s]->isVisible()) {
                auto& shuttle = shuttles[s]->getShuttleData();
                for (int x = shuttle.getX() - gameEnvConfig.unitSensorRange; x <= shuttle.getX() + gameEnvConfig.unitSensorRange; x++) {
                    for (int y = shuttle.getY() - gameEnvConfig.unitSensorRange; y <= shuttle.getY() + gameEnvConfig.unitSensorRange; y++) {
                        if (gameMap->isValidTile(x, y)) {
                            GameTile& currentTile = gameMap->getTile(x, y);
                            if (currentTile.getType() == TileType::UNKNOWN_TILE) {
                                currentTile.setType(TileType::NEBULA, state.currentStep, driftDetector->driftFinalized);
                            }
                        }
                    }
                }
            }
        }
    }

    log("Check if all tiles are explored for relic discovery");    

    if (state.isThereAHuntForRelic()) {
        bool relicNotFound[3] = {true, true, true};

        //We have crossed 50 steps, relic should be there if all nodes are explored
        for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
            for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
                GameTile& currentTile = gameMap->getTile(i, j);
                for (int k = 0; k < 3; ++k) {
                    if (state.relicDiscoveryStatus[k] == RelicDiscoveryStatus::SEARCHING) {
                        int cutoffTime = 101 * k + 50;
                        if (currentTile.getLastExploredTime() < cutoffTime) {
                            //Assess for Frontier set here!
                            gameMap->setRelicExplorationFrontier(currentTile, k, cutoffTime);
                            relicNotFound[k] = false;
                        }
                    } else if (relicNotFound[k]) {
                        //We are not searching for this relic
                        relicNotFound[k] = false;
                    }
                }
            }
        }

        for (int i = 0;i < 3; i++) {
            if (relicNotFound[i]) {
                log("Relic not found for match " + std::to_string(i+1));
                state.relicDiscoveryStatus[i] = RelicDiscoveryStatus::NOT_FOUND;
                // state.setRelicDiscoveryStatus(RelicDiscoveryStatus::NOT_FOUND);
                for (int j = i+1;j < 3; j++) {
                    log("Relic not applicable for match " + std::to_string(j+1));
                    state.relicDiscoveryStatus[j] = RelicDiscoveryStatus::NOT_APPLICABLE;
                }
                break;
            }
        }
    }

    log("Detecting drift");

    bool needToUpdateEnergyNodes = false;
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& currentTile = gameMap->getTile(i, j);
            if (currentTile.isVisible()) {
                if (currentTile.getTypeUpdateStep() == state.currentStep
                        && currentTile.getType() != currentTile.getPreviousType() && currentTile.getPreviousType() != TileType::UNKNOWN_TILE) {
                    // This tile is visible and drifted since last seen
                    driftDetector->reportNebulaDrift(currentTile);
                }

                if (currentTile.getEnergy() != currentTile.getPreviousEnergy() && currentTile.getPreviousEnergy() != -1 && currentTile.getEnergy() != -1) {
                    // This tile is visible and drifted since last seen
                    energyEstimator->reportEnergyDrift(currentTile);
                    needToUpdateEnergyNodes = true;                    
                }
            }
        }
    }

    if (needToUpdateEnergyNodes) {
        energyEstimator->updateEnergyNodes();
    }

    driftDetector->step();    

    log("Updating the newly identified tile types to drift tile type vector");

    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            GameTile& currentTile = gameMap->getTile(i, j);
            
            //Verifying if the energy estimate is correct
            if (currentTile.isVisible() && currentTile.getEnergy() != currentTile.getLastKnownEnergy()) {
                log("Problem: energy estimate has messed up for " + std::to_string(i) + ", " + std::to_string(j) + " - " + std::to_string(currentTile.getEnergy()) + " & " + std::to_string(currentTile.getLastKnownEnergy()));
                std::cerr<<"Problem: Energy estimate messed up"<<std::endl;
            }

            // Updating the drift chain to the future
            if (gameState.obs.sensorMask[i][j]) {                
                GameTile& currentMirrorTile = gameMap->getMirroredTile(i, j);

                driftDetector->exploreTile(currentTile);
                driftDetector->exploreTile(currentMirrorTile);
            }
        }
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
            
            if (state.currentMatchStep != 0 && currentTile.isOccupied() && 
                    (currentTile.isVantagePoint() || currentTile.isHaloTile() || gameMap->hasPotentialInvisibleRelicNode(currentTile))) {
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
        // log("Marking regular tile " + std::to_string(regularTile) + " at " + std::to_string(x) + ", " + std::to_string(y));
        GameTile& currentTile = gameMap->getTile(x, y);
        currentTile.setHaloTile(false);
        currentTile.setForcedRegularTile(true);
    }

    state.vantagePointsFound = haloConstraints->identifiedVantagePoints.size();
    state.vantagePointsOccupied = 0;

    for (auto& vantagePoint : haloConstraints->identifiedVantagePoints) {
        int x = vantagePoint % gameEnvConfig.mapWidth;
        int y = vantagePoint / gameEnvConfig.mapWidth;
        // log("Marking vantage point at " + std::to_string(x) + ", " + std::to_string(y));
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

    log("Calculating previous step losses");
    shuttleEnergyTracker->step();

    log("Tracking opponent units");
    opponentTracker->step();

    log ("Computing battle points");
    battleEvaluator->clear();
    for (int i = 0; i < gameEnvConfig.mapHeight; ++i) {
        for (int j = 0; j < gameEnvConfig.mapWidth; ++j) {
            battleEvaluator->computeTeamBattlePoints(i, j);
            battleEvaluator->computeOpponentBattlePoints(i, j);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
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
    delete driftDetector;
    delete energyEstimator;
    delete battleEvaluator;
    delete shuttleEnergyTracker;
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
        visualizerClientPtr->sendGameData(results);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("act_duration", duration.count());
    return results;
}
